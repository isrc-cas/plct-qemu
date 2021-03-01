/*
 * QEMU RISC-V PLCT Stone Board  base VirtIO and SIFIVE_E
 *
 * Copyright (c) 2021 PLCT, lab.
 * Copyright (c) 2017 SiFive, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/qdev-properties.h"
#include "hw/char/serial.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/plct_stone.h"
#include "hw/riscv/boot.h"
#include "hw/riscv/numa.h"
#include "hw/intc/sifive_clint.h"
#include "hw/intc/sifive_plic.h"
#include "hw/misc/sifive_test.h"
#include "chardev/char.h"
#include "sysemu/arch_init.h"
#include "sysemu/device_tree.h"
#include "sysemu/sysemu.h"
#include "hw/pci/pci.h"
#include "hw/pci-host/gpex.h"
#include "hw/misc/unimp.h"
#include "hw/char/sifive_uart.h"
#include "hw/misc/sifive_e_prci.h"
#include "exec/address-spaces.h"

typedef struct MemmapEntry {
    hwaddr base;
    hwaddr size;
}MemmapEntry;

MemmapEntry plct_stone_memmap[] = {
    [PLCT_STONE_DEBUG] =       {        0x0,         0x100 },
    [PLCT_STONE_MROM] =        {     0x1000,        0xf000 },
    [PLCT_STONE_TEST] =        {   0x100000,        0x1000 },
    [PLCT_STONE_RTC] =         {   0x101000,        0x1000 },
    [PLCT_STONE_CLINT] =       {  0x2000000,       0x10000 },
    [PLCT_STONE_PCIE_PIO] =    {  0x3000000,       0x10000 },
    [PLCT_STONE_PLIC] =        {  0xc000000, PLCT_STONE_PLIC_SIZE(STONE_CPUS_MAX * 2) },
    [PLCT_STONE_UART0] =       { 0x10000000,         0x100 },
    [PLCT_STONE_VIRTIO] =      { 0x10001000,        0x1000 },
    [PLCT_STONE_PCIE_ECAM] =   { 0x30000000,    0x10000000 },
    [PLCT_STONE_PCIE_MMIO] =   { 0x40000000,    0x40000000 },
    [PLCT_STONE_DRAM] =        { 0x80000000,           0x0 },
};

MemmapEntry plct_machine_memmap[] = {
    [PLCT_MACHINE_DEV_DEBUG] =    {        0x0,     0x1000 },
    [PLCT_MACHINE_DEV_MROM] =     {     0x1000,     0x2000 },
    [PLCT_MACHINE_DEV_OTP] =      {    0x20000,     0x2000 },
    [PLCT_MACHINE_DEV_CLINT] =    {  0x2000000,    0x10000 },
    [PLCT_MACHINE_DEV_PLIC] =     {  0xc000000,  0x4000000 },
    [PLCT_MACHINE_DEV_AON] =      { 0x10000000,     0x8000 },
    [PLCT_MACHINE_DEV_PRCI] =     { 0x10008000,     0x8000 },
    [PLCT_MACHINE_DEV_OTP_CTRL] = { 0x10010000,     0x1000 },
    [PLCT_MACHINE_DEV_GPIO0] =    { 0x10012000,     0x1000 },
    [PLCT_MACHINE_DEV_UART0] =    { 0x10013000,     0x1000 },
    [PLCT_MACHINE_DEV_QSPI0] =    { 0x10014000,     0x1000 },
    [PLCT_MACHINE_DEV_PWM0] =     { 0x10015000,     0x1000 },
    [PLCT_MACHINE_DEV_UART1] =    { 0x10023000,     0x1000 },
    [PLCT_MACHINE_DEV_QSPI1] =    { 0x10024000,     0x1000 },
    [PLCT_MACHINE_DEV_PWM1] =     { 0x10025000,     0x1000 },
    [PLCT_MACHINE_DEV_QSPI2] =    { 0x10034000,     0x1000 },
    [PLCT_MACHINE_DEV_PWM2] =     { 0x10035000,     0x1000 },
    [PLCT_MACHINE_DEV_XIP] =      { 0x20000000, 0x20000000 },
    [PLCT_MACHINE_DEV_DTIM] =     { 0x80000000,     0x4000 }
};

static void create_pcie_irq_map(void *fdt, char *nodename,
                                uint32_t plic_phandle)
{
    int pin, dev;
    uint32_t
        full_irq_map[GPEX_NUM_IRQS * GPEX_NUM_IRQS * FDT_INT_MAP_WIDTH] = {};
    uint32_t *irq_map = full_irq_map;

    /* This code creates a standard swizzle of interrupts such that
     * each device's first interrupt is based on it's PCI_SLOT number.
     * (See pci_swizzle_map_irq_fn())
     *
     * We only need one entry per interrupt in the table (not one per
     * possible slot) seeing the interrupt-map-mask will allow the table
     * to wrap to any number of devices.
     */
    for (dev = 0; dev < GPEX_NUM_IRQS; dev++) {
        int devfn = dev * 0x8;

        for (pin = 0; pin < GPEX_NUM_IRQS; pin++) {
            int irq_nr = PCIE_IRQ + ((pin + PCI_SLOT(devfn)) % GPEX_NUM_IRQS);
            int i = 0;

            irq_map[i] = cpu_to_be32(devfn << 8);

            i += FDT_PCI_ADDR_CELLS;
            irq_map[i] = cpu_to_be32(pin + 1);

            i += FDT_PCI_INT_CELLS;
            irq_map[i++] = cpu_to_be32(plic_phandle);

            i += FDT_PLIC_ADDR_CELLS;
            irq_map[i] = cpu_to_be32(irq_nr);

            irq_map += FDT_INT_MAP_WIDTH;
        }
    }

    qemu_fdt_setprop(fdt, nodename, "interrupt-map",
                     full_irq_map, sizeof(full_irq_map));

    qemu_fdt_setprop_cells(fdt, nodename, "interrupt-map-mask",
                           0x1800, 0, 0, 0x7);
}

static void create_fdt(PLCTStoneState *s, const struct MemmapEntry *memmap,
                       uint64_t mem_size, const char *cmdline, bool is_32_bit)
{
    void *fdt;
    int i, cpu, socket;
    MachineState *mc = MACHINE(s);
    uint64_t addr, size;
    uint32_t *clint_cells, *plic_cells;
    unsigned long clint_addr, plic_addr;
    uint32_t plic_phandle[MAX_NODES];
    uint32_t cpu_phandle, intc_phandle, test_phandle;
    uint32_t phandle = 1, plic_mmio_phandle = 1;
    uint32_t plic_pcie_phandle = 1, plic_virtio_phandle = 1;
    char *mem_name, *cpu_name, *core_name, *intc_name;
    char *name, *clint_name, *plic_name, *clust_name;

    if (mc->dtb) {
        fdt = s->fdt = load_device_tree(mc->dtb, &s->fdt_size);
        if (!fdt) {
            error_report("load_device_tree() failed");
            exit(1);
        }
        goto update_bootargs;
    } else {
        fdt = s->fdt = create_device_tree(&s->fdt_size);
        if (!fdt) {
            error_report("create_device_tree() failed");
            exit(1);
        }
    }

    qemu_fdt_setprop_string(fdt, "/", "model", "riscv-virtio,qemu");
    qemu_fdt_setprop_string(fdt, "/", "compatible", "riscv-virtio");
    qemu_fdt_setprop_cell(fdt, "/", "#size-cells", 0x2);
    qemu_fdt_setprop_cell(fdt, "/", "#address-cells", 0x2);

    qemu_fdt_add_subnode(fdt, "/soc");
    qemu_fdt_setprop(fdt, "/soc", "ranges", NULL, 0);
    qemu_fdt_setprop_string(fdt, "/soc", "compatible", "simple-bus");
    qemu_fdt_setprop_cell(fdt, "/soc", "#size-cells", 0x2);
    qemu_fdt_setprop_cell(fdt, "/soc", "#address-cells", 0x2);

    qemu_fdt_add_subnode(fdt, "/cpus");
    qemu_fdt_setprop_cell(fdt, "/cpus", "timebase-frequency",
                          SIFIVE_CLINT_TIMEBASE_FREQ);
    qemu_fdt_setprop_cell(fdt, "/cpus", "#size-cells", 0x0);
    qemu_fdt_setprop_cell(fdt, "/cpus", "#address-cells", 0x1);
    qemu_fdt_add_subnode(fdt, "/cpus/cpu-map");

    for (socket = (riscv_socket_count(mc) - 1); socket >= 0; socket--) {
        clust_name = g_strdup_printf("/cpus/cpu-map/cluster%d", socket);
        qemu_fdt_add_subnode(fdt, clust_name);

        plic_cells = g_new0(uint32_t, s->soc.cpus[socket].num_harts * 4);
        clint_cells = g_new0(uint32_t, s->soc.cpus[socket].num_harts * 4);

        for (cpu = s->soc.cpus[socket].num_harts - 1; cpu >= 0; cpu--) {
            cpu_phandle = phandle++;

            cpu_name = g_strdup_printf("/cpus/cpu@%d",
                s->soc.cpus[socket].hartid_base + cpu);
            qemu_fdt_add_subnode(fdt, cpu_name);
            if (is_32_bit) {
                qemu_fdt_setprop_string(fdt, cpu_name, "mmu-type", "riscv,sv32");
            } else {
                qemu_fdt_setprop_string(fdt, cpu_name, "mmu-type", "riscv,sv48");
            }
            name = riscv_isa_string(&s->soc.cpus[socket].harts[cpu]);
            qemu_fdt_setprop_string(fdt, cpu_name, "riscv,isa", name);
            g_free(name);
            qemu_fdt_setprop_string(fdt, cpu_name, "compatible", "riscv");
            qemu_fdt_setprop_string(fdt, cpu_name, "status", "okay");
            qemu_fdt_setprop_cell(fdt, cpu_name, "reg",
                s->soc.cpus[socket].hartid_base + cpu);
            qemu_fdt_setprop_string(fdt, cpu_name, "device_type", "cpu");
            riscv_socket_fdt_write_id(mc, fdt, cpu_name, socket);
            qemu_fdt_setprop_cell(fdt, cpu_name, "phandle", cpu_phandle);

            intc_name = g_strdup_printf("%s/interrupt-controller", cpu_name);
            qemu_fdt_add_subnode(fdt, intc_name);
            intc_phandle = phandle++;
            qemu_fdt_setprop_cell(fdt, intc_name, "phandle", intc_phandle);
            qemu_fdt_setprop_string(fdt, intc_name, "compatible",
                "riscv,cpu-intc");
            qemu_fdt_setprop(fdt, intc_name, "interrupt-controller", NULL, 0);
            qemu_fdt_setprop_cell(fdt, intc_name, "#interrupt-cells", 1);

            clint_cells[cpu * 4 + 0] = cpu_to_be32(intc_phandle);
            clint_cells[cpu * 4 + 1] = cpu_to_be32(IRQ_M_SOFT);
            clint_cells[cpu * 4 + 2] = cpu_to_be32(intc_phandle);
            clint_cells[cpu * 4 + 3] = cpu_to_be32(IRQ_M_TIMER);

            plic_cells[cpu * 4 + 0] = cpu_to_be32(intc_phandle);
            plic_cells[cpu * 4 + 1] = cpu_to_be32(IRQ_M_EXT);
            plic_cells[cpu * 4 + 2] = cpu_to_be32(intc_phandle);
            plic_cells[cpu * 4 + 3] = cpu_to_be32(IRQ_S_EXT);

            core_name = g_strdup_printf("%s/core%d", clust_name, cpu);
            qemu_fdt_add_subnode(fdt, core_name);
            qemu_fdt_setprop_cell(fdt, core_name, "cpu", cpu_phandle);

            g_free(core_name);
            g_free(intc_name);
            g_free(cpu_name);
        }

        addr = memmap[PLCT_STONE_DRAM].base + riscv_socket_mem_offset(mc, socket);
        size = riscv_socket_mem_size(mc, socket);
        mem_name = g_strdup_printf("/memory@%lx", (long)addr);
        qemu_fdt_add_subnode(fdt, mem_name);
        qemu_fdt_setprop_cells(fdt, mem_name, "reg",
            addr >> 32, addr, size >> 32, size);
        qemu_fdt_setprop_string(fdt, mem_name, "device_type", "memory");
        riscv_socket_fdt_write_id(mc, fdt, mem_name, socket);
        g_free(mem_name);

        clint_addr = memmap[PLCT_STONE_CLINT].base +
            (memmap[PLCT_STONE_CLINT].size * socket);
        clint_name = g_strdup_printf("/soc/clint@%lx", clint_addr);
        qemu_fdt_add_subnode(fdt, clint_name);
        qemu_fdt_setprop_string(fdt, clint_name, "compatible", "riscv,clint0");
        qemu_fdt_setprop_cells(fdt, clint_name, "reg",
            0x0, clint_addr, 0x0, memmap[PLCT_STONE_CLINT].size);
        qemu_fdt_setprop(fdt, clint_name, "interrupts-extended",
            clint_cells, s->soc.cpus[socket].num_harts * sizeof(uint32_t) * 4);
        riscv_socket_fdt_write_id(mc, fdt, clint_name, socket);
        g_free(clint_name);

        plic_phandle[socket] = phandle++;
        plic_addr = memmap[PLCT_STONE_PLIC].base + (memmap[PLCT_STONE_PLIC].size * socket);
        plic_name = g_strdup_printf("/soc/plic@%lx", plic_addr);
        qemu_fdt_add_subnode(fdt, plic_name);
        qemu_fdt_setprop_cell(fdt, plic_name,
            "#address-cells", FDT_PLIC_ADDR_CELLS);
        qemu_fdt_setprop_cell(fdt, plic_name,
            "#interrupt-cells", FDT_PLIC_INT_CELLS);
        qemu_fdt_setprop_string(fdt, plic_name, "compatible", "riscv,plic0");
        qemu_fdt_setprop(fdt, plic_name, "interrupt-controller", NULL, 0);
        qemu_fdt_setprop(fdt, plic_name, "interrupts-extended",
            plic_cells, s->soc.cpus[socket].num_harts * sizeof(uint32_t) * 4);
        qemu_fdt_setprop_cells(fdt, plic_name, "reg",
            0x0, plic_addr, 0x0, memmap[PLCT_STONE_PLIC].size);
        qemu_fdt_setprop_cell(fdt, plic_name, "riscv,ndev", VIRTIO_NDEV);
        riscv_socket_fdt_write_id(mc, fdt, plic_name, socket);
        qemu_fdt_setprop_cell(fdt, plic_name, "phandle", plic_phandle[socket]);
        g_free(plic_name);

        g_free(clint_cells);
        g_free(plic_cells);
        g_free(clust_name);
    }

    for (socket = 0; socket < riscv_socket_count(mc); socket++) {
        if (socket == 0) {
            plic_mmio_phandle = plic_phandle[socket];
            plic_virtio_phandle = plic_phandle[socket];
            plic_pcie_phandle = plic_phandle[socket];
        }
        if (socket == 1) {
            plic_virtio_phandle = plic_phandle[socket];
            plic_pcie_phandle = plic_phandle[socket];
        }
        if (socket == 2) {
            plic_pcie_phandle = plic_phandle[socket];
        }
    }

    riscv_socket_fdt_write_distance_matrix(mc, fdt);

    for (i = 0; i < VIRTIO_COUNT; i++) {
        name = g_strdup_printf("/soc/virtio_mmio@%lx",
            (long)(memmap[PLCT_STONE_VIRTIO].base + i * memmap[PLCT_STONE_VIRTIO].size));
        qemu_fdt_add_subnode(fdt, name);
        qemu_fdt_setprop_string(fdt, name, "compatible", "virtio,mmio");
        qemu_fdt_setprop_cells(fdt, name, "reg",
            0x0, memmap[PLCT_STONE_VIRTIO].base + i * memmap[PLCT_STONE_VIRTIO].size,
            0x0, memmap[PLCT_STONE_VIRTIO].size);
        qemu_fdt_setprop_cell(fdt, name, "interrupt-parent",
            plic_virtio_phandle);
        qemu_fdt_setprop_cell(fdt, name, "interrupts", VIRTIO_IRQ + i);
        g_free(name);
    }

    name = g_strdup_printf("/soc/pci@%lx",
        (long) memmap[PLCT_STONE_PCIE_ECAM].base);
    qemu_fdt_add_subnode(fdt, name);
    qemu_fdt_setprop_cell(fdt, name, "#address-cells", FDT_PCI_ADDR_CELLS);
    qemu_fdt_setprop_cell(fdt, name, "#interrupt-cells", FDT_PCI_INT_CELLS);
    qemu_fdt_setprop_cell(fdt, name, "#size-cells", 0x2);
    qemu_fdt_setprop_string(fdt, name, "compatible", "pci-host-ecam-generic");
    qemu_fdt_setprop_string(fdt, name, "device_type", "pci");
    qemu_fdt_setprop_cell(fdt, name, "linux,pci-domain", 0);
    qemu_fdt_setprop_cells(fdt, name, "bus-range", 0,
        memmap[PLCT_STONE_PCIE_ECAM].size / PCIE_MMCFG_SIZE_MIN - 1);
    qemu_fdt_setprop(fdt, name, "dma-coherent", NULL, 0);
    qemu_fdt_setprop_cells(fdt, name, "reg", 0,
        memmap[PLCT_STONE_PCIE_ECAM].base, 0, memmap[PLCT_STONE_PCIE_ECAM].size);
    qemu_fdt_setprop_sized_cells(fdt, name, "ranges",
        1, FDT_PCI_RANGE_IOPORT, 2, 0,
        2, memmap[PLCT_STONE_PCIE_PIO].base, 2, memmap[PLCT_STONE_PCIE_PIO].size,
        1, FDT_PCI_RANGE_MMIO,
        2, memmap[PLCT_STONE_PCIE_MMIO].base,
        2, memmap[PLCT_STONE_PCIE_MMIO].base, 2, memmap[PLCT_STONE_PCIE_MMIO].size);
    create_pcie_irq_map(fdt, name, plic_pcie_phandle);
    g_free(name);

    test_phandle = phandle++;
    name = g_strdup_printf("/soc/test@%lx",
        (long)memmap[PLCT_STONE_TEST].base);
    qemu_fdt_add_subnode(fdt, name);
    {
        const char compat[] = "sifive,test1\0sifive,test0\0syscon";
        qemu_fdt_setprop(fdt, name, "compatible", compat, sizeof(compat));
    }
    qemu_fdt_setprop_cells(fdt, name, "reg",
        0x0, memmap[PLCT_STONE_TEST].base,
        0x0, memmap[PLCT_STONE_TEST].size);
    qemu_fdt_setprop_cell(fdt, name, "phandle", test_phandle);
    test_phandle = qemu_fdt_get_phandle(fdt, name);
    g_free(name);

    name = g_strdup_printf("/soc/reboot");
    qemu_fdt_add_subnode(fdt, name);
    qemu_fdt_setprop_string(fdt, name, "compatible", "syscon-reboot");
    qemu_fdt_setprop_cell(fdt, name, "regmap", test_phandle);
    qemu_fdt_setprop_cell(fdt, name, "offset", 0x0);
    qemu_fdt_setprop_cell(fdt, name, "value", FINISHER_RESET);
    g_free(name);

    name = g_strdup_printf("/soc/poweroff");
    qemu_fdt_add_subnode(fdt, name);
    qemu_fdt_setprop_string(fdt, name, "compatible", "syscon-poweroff");
    qemu_fdt_setprop_cell(fdt, name, "regmap", test_phandle);
    qemu_fdt_setprop_cell(fdt, name, "offset", 0x0);
    qemu_fdt_setprop_cell(fdt, name, "value", FINISHER_PASS);
    g_free(name);

    name = g_strdup_printf("/soc/uart@%lx", (long)memmap[PLCT_STONE_UART0].base);
    qemu_fdt_add_subnode(fdt, name);
    qemu_fdt_setprop_string(fdt, name, "compatible", "ns16550a");
    qemu_fdt_setprop_cells(fdt, name, "reg",
        0x0, memmap[PLCT_STONE_UART0].base,
        0x0, memmap[PLCT_STONE_UART0].size);
    qemu_fdt_setprop_cell(fdt, name, "clock-frequency", 3686400);
    qemu_fdt_setprop_cell(fdt, name, "interrupt-parent", plic_mmio_phandle);
    qemu_fdt_setprop_cell(fdt, name, "interrupts", UART0_IRQ);

    qemu_fdt_add_subnode(fdt, "/chosen");
    qemu_fdt_setprop_string(fdt, "/chosen", "stdout-path", name);
    g_free(name);

    name = g_strdup_printf("/soc/rtc@%lx", (long)memmap[PLCT_STONE_RTC].base);
    qemu_fdt_add_subnode(fdt, name);
    qemu_fdt_setprop_string(fdt, name, "compatible", "google,goldfish-rtc");
    qemu_fdt_setprop_cells(fdt, name, "reg",
        0x0, memmap[PLCT_STONE_RTC].base,
        0x0, memmap[PLCT_STONE_RTC].size);
    qemu_fdt_setprop_cell(fdt, name, "interrupt-parent", plic_mmio_phandle);
    qemu_fdt_setprop_cell(fdt, name, "interrupts", RTC_IRQ);
    g_free(name);

update_bootargs:
    if (cmdline) {
        qemu_fdt_setprop_string(fdt, "/chosen", "bootargs", cmdline);
    }
}

static inline DeviceState *gpex_pcie_init(MemoryRegion *sys_mem,
                                          hwaddr ecam_base, hwaddr ecam_size,
                                          hwaddr mmio_base, hwaddr mmio_size,
                                          hwaddr pio_base,
                                          DeviceState *plic, bool link_up)
{
    DeviceState *dev;
    MemoryRegion *ecam_alias, *ecam_reg;
    MemoryRegion *mmio_alias, *mmio_reg;
    qemu_irq irq;
    int i;

    dev = qdev_new(TYPE_GPEX_HOST);

    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

    ecam_alias = g_new0(MemoryRegion, 1);
    ecam_reg = sysbus_mmio_get_region(SYS_BUS_DEVICE(dev), 0);
    memory_region_init_alias(ecam_alias, OBJECT(dev), "pcie-ecam",
                             ecam_reg, 0, ecam_size);
    memory_region_add_subregion(get_system_memory(), ecam_base, ecam_alias);

    mmio_alias = g_new0(MemoryRegion, 1);
    mmio_reg = sysbus_mmio_get_region(SYS_BUS_DEVICE(dev), 1);
    memory_region_init_alias(mmio_alias, OBJECT(dev), "pcie-mmio",
                             mmio_reg, mmio_base, mmio_size);
    memory_region_add_subregion(get_system_memory(), mmio_base, mmio_alias);

    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 2, pio_base);

    for (i = 0; i < GPEX_NUM_IRQS; i++) {
        irq = qdev_get_gpio_in(plic, PCIE_IRQ + i);

        sysbus_connect_irq(SYS_BUS_DEVICE(dev), i, irq);
        gpex_set_irq_num(GPEX_HOST(dev), i, PCIE_IRQ + i);
    }

    return dev;
}

static void plct_stone_machine_init(MachineState *machine)
{
    const struct MemmapEntry *memmap = plct_stone_memmap;
    PLCTStoneState *s = PLCT_STONE_MACHINE(machine);
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    MemoryRegion *mask_rom = g_new(MemoryRegion, 1);
    target_ulong start_addr = memmap[PLCT_STONE_DRAM].base;
    target_ulong firmware_end_addr, kernel_start_addr;
    uint32_t fdt_load_addr;
    uint64_t kernel_entry;
    int i;

    /* Initialize SoC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_RISCV_PLCT_STONE_SOC);
    object_property_set_str(OBJECT(&s->soc.cpus[0]), "cpu-type", machine->cpu_type,
                             &error_abort);
    qdev_realize(DEVICE(&s->soc), NULL, &error_abort);

    /* register system main memory (actual RAM) */
    memory_region_init_ram(main_mem, NULL, "plct_stone_board.ram",
                           machine->ram_size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[PLCT_STONE_DRAM].base,
        main_mem);

    /* create device tree */
    create_fdt(s, memmap, machine->ram_size, machine->kernel_cmdline,
               riscv_is_32bit(&s->soc.cpus[0]));

    /* boot rom */
    memory_region_init_rom(mask_rom, NULL, "plct_stone_board.mrom",
                           memmap[PLCT_STONE_MROM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[PLCT_STONE_MROM].base,
                                mask_rom);

    if (riscv_is_32bit(&s->soc.cpus[0])) {
        firmware_end_addr = riscv_find_and_load_firmware(machine,
                                    "opensbi-riscv32-generic-fw_dynamic.bin",
                                    start_addr, NULL);
    } else {
        firmware_end_addr = riscv_find_and_load_firmware(machine,
                                    "opensbi-riscv64-generic-fw_dynamic.bin",
                                    start_addr, NULL);
    }

    if (machine->kernel_filename) {
        kernel_start_addr = riscv_calc_kernel_start_addr(&s->soc.cpus[0],
                                                         firmware_end_addr);

        kernel_entry = riscv_load_kernel(machine->kernel_filename,
                                         kernel_start_addr, NULL);

        if (machine->initrd_filename) {
            hwaddr start;
            hwaddr end = riscv_load_initrd(machine->initrd_filename,
                                           machine->ram_size, kernel_entry,
                                           &start);
            qemu_fdt_setprop_cell(s->fdt, "/chosen",
                                  "linux,initrd-start", start);
            qemu_fdt_setprop_cell(s->fdt, "/chosen", "linux,initrd-end",
                                  end);
        }
    } else {
       /*
        * If dynamic firmware is used, it doesn't know where is the next mode
        * if kernel argument is not set.
        */
        kernel_entry = 0;
    }

    /* Compute the fdt load address in dram */
    fdt_load_addr = riscv_load_fdt(memmap[PLCT_STONE_DRAM].base,
                                   machine->ram_size, s->fdt);
    /* load the reset vector */
    riscv_setup_rom_reset_vec(machine, &s->soc.cpus[0], start_addr,
                              plct_stone_memmap[PLCT_STONE_MROM].base,
                              plct_stone_memmap[PLCT_STONE_MROM].size, kernel_entry,
                              fdt_load_addr, s->fdt);


    /* VirtIO MMIO devices */
    for (i = 0; i < VIRTIO_COUNT; i++) {
        sysbus_create_simple("virtio-mmio",
            memmap[PLCT_STONE_VIRTIO].base + i * memmap[PLCT_STONE_VIRTIO].size,
            qdev_get_gpio_in(DEVICE(s->soc.virtio_plic), VIRTIO_IRQ + i));
    }

    gpex_pcie_init(system_memory,
                         memmap[PLCT_STONE_PCIE_ECAM].base,
                         memmap[PLCT_STONE_PCIE_ECAM].size,
                         memmap[PLCT_STONE_PCIE_MMIO].base,
                         memmap[PLCT_STONE_PCIE_MMIO].size,
                         memmap[PLCT_STONE_PCIE_PIO].base,
                         DEVICE(s->soc.pcie_plic), true);

    serial_mm_init(system_memory, memmap[PLCT_STONE_UART0].base,
        0, qdev_get_gpio_in(DEVICE(s->soc.mmio_plic), UART0_IRQ), 399193,
        serial_hd(0), DEVICE_LITTLE_ENDIAN);

    sysbus_create_simple("goldfish_rtc", memmap[PLCT_STONE_RTC].base,
        qdev_get_gpio_in(DEVICE(s->soc.mmio_plic), RTC_IRQ));

}

static void plct_stone_machine_instance_init(Object *obj)
{
}

static void plct_stone_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "PLCT RISC-V Stone board";
    mc->init = plct_stone_machine_init;
    mc->max_cpus = STONE_CPUS_MAX;
    mc->default_cpu_type = PLCT_U_CPU;
}

static const TypeInfo plct_stone_machine_typeinfo = {
    .name       = MACHINE_TYPE_NAME("plct_stone"),
    .parent     = TYPE_MACHINE,
    .class_init = plct_stone_machine_class_init,
    .instance_init = plct_stone_machine_instance_init,
    .instance_size = sizeof(PLCTStoneState),
};

static void plct_stone_machine_init_register_types(void)
{
    type_register_static(&plct_stone_machine_typeinfo);
}

type_init(plct_stone_machine_init_register_types)

static void plct_machine_machine_init(MachineState *machine)
{
    const struct MemmapEntry *memmap = plct_machine_memmap;

    PlctMachineState *s = RISCV_PLCT_MACHINE_MACHINE(machine);
    MemoryRegion *sys_mem = get_system_memory();
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    int i;

    /* Initialize SoC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_RISCV_PLCT_MACHINE_SOC);
    qdev_realize(DEVICE(&s->soc), NULL, &error_abort);

    /* Data Tightly Integrated Memory */
    memory_region_init_ram(main_mem, NULL, "riscv.plct.machine.ram",
        memmap[PLCT_MACHINE_DEV_DTIM].size, &error_fatal);
    memory_region_add_subregion(sys_mem,
        memmap[PLCT_MACHINE_DEV_DTIM].base, main_mem);

    /* Mask ROM reset vector */
    uint32_t reset_vec[4];
    reset_vec[1] = 0x204002b7;  /* 0x1004: lui     t0,0x20400 */
    reset_vec[2] = 0x00028067;      /* 0x1008: jr      t0 */

    reset_vec[0] = reset_vec[3] = 0;

    /* copy in the reset vector in little_endian byte order */
    for (i = 0; i < sizeof(reset_vec) >> 2; i++) {
        reset_vec[i] = cpu_to_le32(reset_vec[i]);
    }
    rom_add_blob_fixed_as("mrom.reset", reset_vec, sizeof(reset_vec),
                          memmap[PLCT_MACHINE_DEV_MROM].base, &address_space_memory);

    if (machine->kernel_filename) {
        riscv_load_kernel(machine->kernel_filename,
                          memmap[PLCT_MACHINE_DEV_DTIM].base, NULL);
    }
}

static void plct_machine_machine_instance_init(Object *obj)
{

}

static void plct_machine_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "PLCT RISC-V board";
    mc->init = plct_machine_machine_init;
    mc->max_cpus = 1;
    mc->default_cpu_type = PLCT_N_CPU;
}

static const TypeInfo plct_machine_machine_typeinfo = {
    .name       = MACHINE_TYPE_NAME("plct_machine"),
    .parent     = TYPE_MACHINE,
    .class_init = plct_machine_machine_class_init,
    .instance_init = plct_machine_machine_instance_init,
    .instance_size = sizeof(PlctMachineState),
};

static void plct_machine_machine_init_register_types(void)
{
    type_register_static(&plct_machine_machine_typeinfo);
}

type_init(plct_machine_machine_init_register_types)


static void plct_machine_soc_init(Object *obj)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    PlctMachineSoCState *s = RISCV_PLCT_MACHINE_SOC(obj);

    object_initialize_child(obj, "cpus", &s->cpus, TYPE_RISCV_HART_ARRAY);
    object_property_set_int(OBJECT(&s->cpus), "num-harts", ms->smp.cpus,
                            &error_abort);
    object_property_set_int(OBJECT(&s->cpus), "resetvec", 0x1004, &error_abort);
    object_initialize_child(obj, "riscv.plct.machine.gpio0", &s->gpio,
                            TYPE_SIFIVE_GPIO);
}

static void plct_machine_soc_realize(DeviceState *dev, Error **errp)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    const struct MemmapEntry *memmap = plct_machine_memmap;
    PlctMachineSoCState *s = RISCV_PLCT_MACHINE_SOC(dev);
    MemoryRegion *sys_mem = get_system_memory();

    object_property_set_str(OBJECT(&s->cpus), "cpu-type", ms->cpu_type,
                            &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->cpus), &error_abort);

    /* Mask ROM */
    memory_region_init_rom(&s->mask_rom, OBJECT(dev), "riscv.plct.machine.mrom",
                           memmap[PLCT_MACHINE_DEV_MROM].size, &error_fatal);
    memory_region_add_subregion(sys_mem,
        memmap[PLCT_MACHINE_DEV_MROM].base, &s->mask_rom);

    /* MMIO */
    s->plic = sifive_plic_create(memmap[PLCT_MACHINE_DEV_PLIC].base,
        (char *)PLCT_MACHINE_PLIC_HART_CONFIG, 0,
        PLCT_STONE_PLIC_NUM_SOURCES,
        PLCT_STONE_PLIC_NUM_PRIORITIES,
        PLCT_STONE_PLIC_PRIORITY_BASE,
        PLCT_STONE_PLIC_PENDING_BASE,
        PLCT_STONE_PLIC_ENABLE_BASE,
        PLCT_STONE_PLIC_ENABLE_STRIDE,
        PLCT_STONE_PLIC_CONTEXT_BASE,
        PLCT_STONE_PLIC_CONTEXT_STRIDE,
        memmap[PLCT_MACHINE_DEV_PLIC].size);

    sifive_clint_create(memmap[PLCT_MACHINE_DEV_CLINT].base,
        memmap[PLCT_MACHINE_DEV_CLINT].size, 0, ms->smp.cpus,
        SIFIVE_SIP_BASE, SIFIVE_TIMECMP_BASE, SIFIVE_TIME_BASE,
        SIFIVE_CLINT_TIMEBASE_FREQ, false);
    create_unimplemented_device("riscv.sifive.e.aon",
        memmap[PLCT_MACHINE_DEV_AON].base, memmap[PLCT_MACHINE_DEV_AON].size);
    sifive_e_prci_create(memmap[PLCT_MACHINE_DEV_PRCI].base);

    /* GPIO */
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->gpio), errp)) {
        return;
    }

    /* Map GPIO registers */
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->gpio), 0, memmap[PLCT_MACHINE_DEV_GPIO0].base);

    /* Pass all GPIOs to the SOC layer so they are available to the board */
    qdev_pass_gpios(DEVICE(&s->gpio), dev, NULL);

    /* Connect GPIO interrupts to the PLIC */
    for (int i = 0; i < 32; i++) {
        sysbus_connect_irq(SYS_BUS_DEVICE(&s->gpio), i,
                           qdev_get_gpio_in(DEVICE(s->plic),
                                            PLCT_MACHINE_GPIO0_IRQ0 + i));
    }

    sifive_uart_create(sys_mem, memmap[PLCT_MACHINE_DEV_UART0].base,
        serial_hd(0), qdev_get_gpio_in(DEVICE(s->plic), PLCT_MACHINE_UART0_IRQ));
    create_unimplemented_device("riscv.plct.machine.qspi0",
        memmap[PLCT_MACHINE_DEV_QSPI0].base, memmap[PLCT_MACHINE_DEV_QSPI0].size);
    create_unimplemented_device("riscv.plct.machine.pwm0",
        memmap[PLCT_MACHINE_DEV_PWM0].base, memmap[PLCT_MACHINE_DEV_PWM0].size);
    sifive_uart_create(sys_mem, memmap[PLCT_MACHINE_DEV_UART1].base,
        serial_hd(1), qdev_get_gpio_in(DEVICE(s->plic), PLCT_MACHINE_UART1_IRQ));
    create_unimplemented_device("riscv.plct.machine.qspi1",
        memmap[PLCT_MACHINE_DEV_QSPI1].base, memmap[PLCT_MACHINE_DEV_QSPI1].size);
    create_unimplemented_device("riscv.plct.machine.pwm1",
        memmap[PLCT_MACHINE_DEV_PWM1].base, memmap[PLCT_MACHINE_DEV_PWM1].size);
    create_unimplemented_device("riscv.plct.machine.qspi2",
        memmap[PLCT_MACHINE_DEV_QSPI2].base, memmap[PLCT_MACHINE_DEV_QSPI2].size);
    create_unimplemented_device("riscv.plct.machine.pwm2",
        memmap[PLCT_MACHINE_DEV_PWM2].base, memmap[PLCT_MACHINE_DEV_PWM2].size);

    /* Flash memory */
    memory_region_init_rom(&s->xip_mem, OBJECT(dev), "riscv.plct.machine.xip",
                           memmap[PLCT_MACHINE_DEV_XIP].size, &error_fatal);
    memory_region_add_subregion(sys_mem, memmap[PLCT_MACHINE_DEV_XIP].base,
        &s->xip_mem);
}

static void plct_machine_soc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = plct_machine_soc_realize;
    /* Reason: Uses serial_hds in realize function, thus can't be used twice */
    dc->user_creatable = false;
}

static const TypeInfo plct_machine_soc_type_info = {
    .name = TYPE_RISCV_PLCT_MACHINE_SOC,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(PlctMachineSoCState),
    .instance_init = plct_machine_soc_init,
    .class_init = plct_machine_soc_class_init,
};

static void plct_machine_soc_register_types(void)
{
    type_register_static(&plct_machine_soc_type_info);
}

type_init(plct_machine_soc_register_types)

//======================================================
static void plct_stone_soc_realize(DeviceState *dev, Error **errp)
{
    MachineState *machine = MACHINE(qdev_get_machine());
    PLCTStoneSoCState *s = RISCV_PLCT_STONE_SOC(dev);
    const struct MemmapEntry *memmap = plct_stone_memmap;
    // MemoryRegion *system_memory = get_system_memory();
    // MemoryRegion *mask_rom = g_new(MemoryRegion, 1);
    char *plic_hart_config, *soc_name;
    size_t plic_hart_config_len;
    // DeviceState *mmio_plic, *virtio_plic, *pcie_plic;
    int i, j, base_hartid, hart_count;

        /* Check socket count limit */
    if (STONE_SOCKETS_MAX < riscv_socket_count(machine)) {
        error_report("number of sockets/nodes should be less than %d",
            STONE_SOCKETS_MAX);
        exit(1);
    }

    /* Initialize sockets */
    // mmio_plic = virtio_plic = pcie_plic = NULL;
    for (i = 0; i < riscv_socket_count(machine); i++) {
        if (!riscv_socket_check_hartids(machine, i)) {
            error_report("discontinuous hartids in socket%d", i);
            exit(1);
        }

        base_hartid = riscv_socket_first_hartid(machine, i);
        if (base_hartid < 0) {
            error_report("can't find hartid base for socket%d", i);
            exit(1);
        }

        hart_count = riscv_socket_hart_count(machine, i);
        if (hart_count < 0) {
            error_report("can't find hart count for socket%d", i);
            exit(1);
        }

        soc_name = g_strdup_printf("soc%d", i);
        object_initialize_child(OBJECT(machine), soc_name, &s->cpus[i],
                                TYPE_RISCV_HART_ARRAY);
        g_free(soc_name);
        object_property_set_str(OBJECT(&s->cpus[i]), "cpu-type",
                                machine->cpu_type, &error_abort);
        object_property_set_int(OBJECT(&s->cpus[i]), "hartid-base",
                                base_hartid, &error_abort);
        object_property_set_int(OBJECT(&s->cpus[i]), "num-harts",
                                hart_count, &error_abort);
        sysbus_realize(SYS_BUS_DEVICE(&s->cpus[i]), &error_abort);

        /* Per-socket CLINT */
        sifive_clint_create(
            memmap[PLCT_STONE_CLINT].base + i * memmap[PLCT_STONE_CLINT].size,
            memmap[PLCT_STONE_CLINT].size, base_hartid, hart_count,
            SIFIVE_SIP_BASE, SIFIVE_TIMECMP_BASE, SIFIVE_TIME_BASE,
            SIFIVE_CLINT_TIMEBASE_FREQ, true);

        /* Per-socket PLIC hart topology configuration string */
        plic_hart_config_len =
            (strlen(PLCT_STONE_PLIC_HART_CONFIG) + 1) * hart_count;
        plic_hart_config = g_malloc0(plic_hart_config_len);
        for (j = 0; j < hart_count; j++) {
            if (j != 0) {
                strncat(plic_hart_config, ",", plic_hart_config_len);
            }
            strncat(plic_hart_config, PLCT_STONE_PLIC_HART_CONFIG,
                plic_hart_config_len);
            plic_hart_config_len -= (strlen(PLCT_STONE_PLIC_HART_CONFIG) + 1);
        }

        /* Per-socket PLIC */
        s->plic[i] = sifive_plic_create(
            memmap[PLCT_STONE_PLIC].base + i * memmap[PLCT_STONE_PLIC].size,
            plic_hart_config, base_hartid,
            PLCT_STONE_PLIC_NUM_SOURCES,
            PLCT_STONE_PLIC_NUM_PRIORITIES,
            PLCT_STONE_PLIC_PRIORITY_BASE,
            PLCT_STONE_PLIC_PENDING_BASE,
            PLCT_STONE_PLIC_ENABLE_BASE,
            PLCT_STONE_PLIC_ENABLE_STRIDE,
            PLCT_STONE_PLIC_CONTEXT_BASE,
            PLCT_STONE_PLIC_CONTEXT_STRIDE,
            memmap[PLCT_STONE_PLIC].size);
        g_free(plic_hart_config);

        /* Try to use different PLIC instance based device type */
        if (i == 0) {
            s->mmio_plic = s->plic[i];
            s->virtio_plic = s->plic[i];
            s->pcie_plic = s->plic[i];
        }
        if (i == 1) {
            s->virtio_plic = s->plic[i];
            s->pcie_plic = s->plic[i];
        }
        if (i == 2) {
            s->pcie_plic = s->plic[i];
        }
    }
}

static void plct_stone_soc_init(Object *obj)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    PLCTStoneSoCState *s = RISCV_PLCT_STONE_SOC(obj);

    object_initialize_child(obj, "cpus", &s->cpus, TYPE_RISCV_HART_ARRAY);
    object_property_set_int(OBJECT(&s->cpus), "num-harts", ms->smp.cpus,
                            &error_abort);
}

static void plct_stone_soc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = plct_stone_soc_realize;
    /* Reason: Uses serial_hds in realize function, thus can't be used twice */
    dc->user_creatable = false;
}

static const TypeInfo plct_stone_soc_type_info = {
    .name = TYPE_RISCV_PLCT_STONE_SOC,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(PLCTStoneSoCState),
    .instance_init = plct_stone_soc_init,
    .class_init = plct_stone_soc_class_init,
};

static void plct_stone_soc_register_types(void)
{
    type_register_static(&plct_stone_soc_type_info);
}

type_init(plct_stone_soc_register_types)
