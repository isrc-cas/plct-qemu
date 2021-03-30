/*
 * QEMU RISC-V Board Compatible with SiFive Freedom U SDK
 *
 * Copyright (c) 2016-2017 Sagar Karandikar, sagark@eecs.berkeley.edu
 * Copyright (c) 2017 SiFive, Inc.
 * Copyright (c) 2019 Bin Meng <bmeng.cn@gmail.com>
 *
 * Provides a board compatible with the SiFive Freedom U SDK:
 *
 * 0) UART
 * 1) CLINT (Core Level Interruptor)
 * 2) PLIC (Platform Level Interrupt Controller)
 * 3) PRCI (Power, Reset, Clock, Interrupt)
 * 4) GPIO (General Purpose Input/Output Controller)
 * 5) OTP (One-Time Programmable) memory with stored serial number
 * 6) GEM (Gigabit Ethernet Controller) and management block
 * 7) DMA (Direct Memory Access Controller)
 *
 * This board currently generates devicetree dynamically that indicates at least
 * two harts and up to five harts.
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
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "qapi/visitor.h"
#include "hw/hw.h"
#include "hw/ssi/ssi.h"
#include "hw/boards.h"
#include "hw/irq.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/char/serial.h"
#include "hw/cpu/cluster.h"
#include "hw/misc/unimp.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/nuclei_u.h"
#include "hw/riscv/boot.h"
#include "hw/char/sifive_uart.h"
#include "hw/intc/sifive_clint.h"
#include "hw/intc/sifive_plic.h"
#include "chardev/char.h"
#include "net/eth.h"
#include "sysemu/arch_init.h"
#include "sysemu/device_tree.h"
#include "sysemu/runstate.h"
#include "sysemu/sysemu.h"

#include <libfdt.h>

#if defined(TARGET_RISCV32)
# define BIOS_FILENAME "opensbi-riscv32-generic-fw_dynamic.bin"
#else
# define BIOS_FILENAME "opensbi-riscv64-generic-fw_dynamic.bin"
#endif

static const struct MemmapEntry {
    hwaddr base;
    hwaddr size;
} nuclei_u_memmap[] = {
    [NUCLEI_U_DEV_MROM] =     {     0x1000,     0xf000 },
    [NUCLEI_U_DEV_TIMER] =    {  0x2000000,    0x10000 },
    [NUCLEI_U_DEV_CLINT] =    {  0x2001000,    0x10000 },
    [NUCLEI_U_DEV_PLIC] =     {  0x8000000,  0x4000000 },
    [NUCLEI_U_DEV_UART0] =    { 0x10013000,     0x1000 },
    [NUCLEI_U_DEV_UART1] =    { 0x10023000,     0x1000 },
    [NUCLEI_U_DEV_GPIO] =     { 0x10012000,     0x1000 },
    [NUCLEI_U_SPI0] =     { 0x10014000,     0x1000 },
    [NUCLEI_U_SPI2] =     { 0x10034000,     0x1000 },
    [NUCLEI_U_DEV_FLASH0] =   { 0x20000000, 0x10000000 },
    [NUCLEI_U_DEV_DRAM] =     { 0xa0000000,        0x0 },
};

#define OTP_SERIAL          1

static void create_fdt(NucLeiUState *s, const struct MemmapEntry *memmap,
    uint64_t mem_size, const char *cmdline)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    void *fdt;
    int cpu;
    uint32_t *cells;
    char *nodename;
    uint32_t plic_phandle, uart_phandle, gpio_phandle, phandle = 1;
    uint32_t hfclk_phandle;

    if (ms->dtb) {
        fdt = s->fdt = load_device_tree(ms->dtb, &s->fdt_size);
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

    qemu_fdt_setprop_string(fdt, "/", "model", "nuclei,ux600");
    qemu_fdt_setprop_string(fdt, "/", "compatible",
                            "nuclei,ux600");
    qemu_fdt_setprop_cell(fdt, "/", "#size-cells", 0x2);
    qemu_fdt_setprop_cell(fdt, "/", "#address-cells", 0x2);

    qemu_fdt_add_subnode(fdt, "/soc");
    qemu_fdt_setprop(fdt, "/soc", "ranges", NULL, 0);
    qemu_fdt_setprop_string(fdt, "/soc", "compatible", "simple-bus");
    qemu_fdt_setprop_cell(fdt, "/soc", "#size-cells", 0x2);
    qemu_fdt_setprop_cell(fdt, "/soc", "#address-cells", 0x2);

    qemu_fdt_add_subnode(fdt, "/chosen");

    hfclk_phandle = phandle++;
    nodename = g_strdup_printf("/hfclk");
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "clock-output-names", "hfclk");
    qemu_fdt_setprop_cell(fdt, nodename, "clock-frequency",
        NUCLEI_U_HFCLK_FREQ);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "fixed-clock");
    qemu_fdt_setprop_cell(fdt, nodename, "#clock-cells", 0x0);
    qemu_fdt_setprop_cell(fdt, "/hfclk", "phandle", hfclk_phandle);
    g_free(nodename);

    nodename = g_strdup_printf("/memory@%lx",
        (long)memmap[NUCLEI_U_DEV_DRAM].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        memmap[NUCLEI_U_DEV_DRAM].base >> 32, memmap[NUCLEI_U_DEV_DRAM].base,
        mem_size >> 32, mem_size);
    qemu_fdt_setprop_string(fdt, nodename, "device_type", "memory");
    g_free(nodename);

    qemu_fdt_add_subnode(fdt, "/cpus");
    qemu_fdt_setprop_cell(fdt, "/cpus", "timebase-frequency",
        SIFIVE_CLINT_TIMEBASE_FREQ);
    qemu_fdt_setprop_cell(fdt, "/cpus", "#size-cells", 0x0);
    qemu_fdt_setprop_cell(fdt, "/cpus", "#address-cells", 0x1);

    for (cpu = ms->smp.cpus - 1; cpu >= 0; cpu--) {
        int cpu_phandle = phandle++;
        nodename = g_strdup_printf("/cpus/cpu@%d", cpu);
        char *intc = g_strdup_printf("/cpus/cpu@%d/interrupt-controller", cpu);
        char *isa;
        qemu_fdt_add_subnode(fdt, nodename);
        /* cpu 0 is the management hart that does not have mmu */
        if (cpu != 0) {
            qemu_fdt_setprop_string(fdt, nodename, "mmu-type", "riscv,sv39");
            isa = riscv_isa_string(&s->soc.u_cpus.harts[cpu - 1]);
        } else {
            isa = riscv_isa_string(&s->soc.e_cpus.harts[0]);
        }
        qemu_fdt_setprop_string(fdt, nodename, "riscv,isa", isa);
        qemu_fdt_setprop_string(fdt, nodename, "compatible", "riscv");
        qemu_fdt_setprop_string(fdt, nodename, "status", "okay");
        qemu_fdt_setprop_cell(fdt, nodename, "reg", cpu);
        qemu_fdt_setprop_string(fdt, nodename, "device_type", "cpu");
        qemu_fdt_add_subnode(fdt, intc);
        qemu_fdt_setprop_cell(fdt, intc, "phandle", cpu_phandle);
        qemu_fdt_setprop_string(fdt, intc, "compatible", "riscv,cpu-intc");
        qemu_fdt_setprop(fdt, intc, "interrupt-controller", NULL, 0);
        qemu_fdt_setprop_cell(fdt, intc, "#interrupt-cells", 1);
        g_free(isa);
        g_free(intc);
        g_free(nodename);
    }

    cells =  g_new0(uint32_t, ms->smp.cpus * 4);
    for (cpu = 0; cpu < ms->smp.cpus; cpu++) {
        nodename =
            g_strdup_printf("/cpus/cpu@%d/interrupt-controller", cpu);
        uint32_t intc_phandle = qemu_fdt_get_phandle(fdt, nodename);
        cells[cpu * 4 + 0] = cpu_to_be32(intc_phandle);
        cells[cpu * 4 + 1] = cpu_to_be32(IRQ_M_SOFT);
        cells[cpu * 4 + 2] = cpu_to_be32(intc_phandle);
        cells[cpu * 4 + 3] = cpu_to_be32(IRQ_M_TIMER);
        g_free(nodename);
    }

    nodename = g_strdup_printf("/clint@%lx",
        (long)memmap[NUCLEI_U_DEV_CLINT].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "riscv,clint0");
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_DEV_CLINT].base,
        0x0, memmap[NUCLEI_U_DEV_CLINT].size);
    qemu_fdt_setprop(fdt, nodename, "interrupts-extended",
        cells, ms->smp.cpus * sizeof(uint32_t) * 4);
    g_free(cells);
    g_free(nodename);

    nodename = g_strdup_printf("/timer@%lx",
        (long)memmap[NUCLEI_U_DEV_TIMER].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "riscv,timer0");
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_DEV_TIMER].base,
        0x0, memmap[NUCLEI_U_DEV_TIMER].size);
    g_free(nodename);

    plic_phandle = phandle++;
    cells =  g_new0(uint32_t, ms->smp.cpus * 4 - 2);
    for (cpu = 0; cpu < ms->smp.cpus; cpu++) {
        nodename =
            g_strdup_printf("/cpus/cpu@%d/interrupt-controller", cpu);
        uint32_t intc_phandle = qemu_fdt_get_phandle(fdt, nodename);
        /* cpu 0 is the management hart that does not have S-mode */
        if (cpu == 0) {
            cells[0] = cpu_to_be32(intc_phandle);
            cells[1] = cpu_to_be32(IRQ_M_EXT);
        } else {
            cells[cpu * 4 - 2] = cpu_to_be32(intc_phandle);
            cells[cpu * 4 - 1] = cpu_to_be32(IRQ_M_EXT);
            cells[cpu * 4 + 0] = cpu_to_be32(intc_phandle);
            cells[cpu * 4 + 1] = cpu_to_be32(IRQ_S_EXT);
        }
        g_free(nodename);
    }

    plic_phandle = phandle++;
    nodename = g_strdup_printf("/interrupt-controller@%lx",
        (long)memmap[NUCLEI_U_DEV_PLIC].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_cell(fdt, nodename, "#interrupt-cells", 1);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "riscv,plic0");
    qemu_fdt_setprop(fdt, nodename, "interrupt-controller", NULL, 0);
    qemu_fdt_setprop(fdt, nodename, "interrupts-extended",
        cells, (ms->smp.cpus * 4 - 2) * sizeof(uint32_t));
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_DEV_PLIC].base,
        0x0, memmap[NUCLEI_U_DEV_PLIC].size);
    qemu_fdt_setprop_cell(fdt, nodename, "riscv,ndev", 0x35);
    qemu_fdt_setprop_cell(fdt, nodename, "phandle", plic_phandle);
    g_free(cells);
    g_free(nodename);

    uart_phandle = phandle++;
    nodename = g_strdup_printf("/serial@%lx",
        (long)memmap[NUCLEI_U_DEV_UART0].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "sifive,uart0");
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_DEV_UART0].base,
        0x0, memmap[NUCLEI_U_DEV_UART0].size);
    qemu_fdt_setprop_cell(fdt, nodename, "clocks", hfclk_phandle);
    qemu_fdt_setprop_cell(fdt, nodename, "baud", 57600);
    qemu_fdt_setprop_cell(fdt, nodename, "interrupt-parent", plic_phandle);
    qemu_fdt_setprop_cell(fdt, nodename, "interrupts", NUCLEI_U_UART0_IRQ);
    qemu_fdt_setprop_cell(fdt, nodename, "phandle", uart_phandle);
    qemu_fdt_setprop_string(fdt, nodename, "status", "disabled");
    g_free(nodename);

    nodename = g_strdup_printf("/serial@%lx",
        (long)memmap[NUCLEI_U_DEV_UART1].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "sifive,uart0");
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_DEV_UART1].base,
        0x0, memmap[NUCLEI_U_DEV_UART1].size);
    qemu_fdt_setprop_cell(fdt, nodename, "clocks", hfclk_phandle);
    qemu_fdt_setprop_cell(fdt, nodename, "interrupt-parent", plic_phandle);
    qemu_fdt_setprop_cell(fdt, nodename, "interrupts", NUCLEI_U_UART1_IRQ);
    qemu_fdt_setprop_cell(fdt, nodename, "phandle", uart_phandle);
     qemu_fdt_setprop_string(fdt, nodename, "status", "disabled");
    g_free(nodename);


    nodename = g_strdup_printf("/spi@%lx",
        memmap[NUCLEI_U_SPI0].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "sifive,spi0");
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_SPI0].base,
        0x0, memmap[NUCLEI_U_SPI0].size,
        0x0, 0x20000000,
        0x0, 0x10000000);
    qemu_fdt_setprop_string(fdt, nodename, "reg-names", "control");
    qemu_fdt_setprop_cells(fdt, nodename, "clocks", hfclk_phandle);
    qemu_fdt_setprop_cells(fdt, nodename, "interrupt-parent", plic_phandle);
    qemu_fdt_setprop_cells(fdt, nodename, "interrupts", NUCLEI_U_SPI0_IRQ);
    qemu_fdt_setprop_cell(fdt, nodename, "#address-cells", 1);
    qemu_fdt_setprop_cell(fdt, nodename, "#size-cells", 0);
    qemu_fdt_setprop_string(fdt, nodename, "status", "okay");
    g_free(nodename);


    nodename = g_strdup_printf("/spi@%lx/flash@0",
        (long)memmap[NUCLEI_U_SPI0].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "jedec,spi-nor");
    qemu_fdt_setprop_cells(fdt, nodename, "reg", 0x0);
    qemu_fdt_setprop_cells(fdt, nodename, "spi-max-frequency", 1000000);
    qemu_fdt_setprop_cells(fdt, nodename, "m25p,fast-read");
    qemu_fdt_setprop_cells(fdt, nodename, "#spi-tx-bus-width", 0x1);
    qemu_fdt_setprop_cells(fdt, nodename, "#spi-rx-bus-width", 0x1);
    g_free(nodename);

    nodename = g_strdup_printf("/spi@%lx",
        (long)memmap[NUCLEI_U_SPI2].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "sifive,spi0");
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_SPI2].base,
        0x0, memmap[NUCLEI_U_SPI2].size);
    qemu_fdt_setprop_string(fdt, nodename, "reg-names", "control");
    qemu_fdt_setprop_cells(fdt, nodename, "clocks", hfclk_phandle);
    qemu_fdt_setprop_cells(fdt, nodename, "interrupt-parent", plic_phandle);
    qemu_fdt_setprop_cells(fdt, nodename, "interrupts", NUCLEI_U_SPI2_IRQ);
    qemu_fdt_setprop_cell(fdt, nodename, "#address-cells", 1);
    qemu_fdt_setprop_cell(fdt, nodename, "#size-cells", 0);
    qemu_fdt_setprop_string(fdt, nodename, "status", "okay");
    g_free(nodename);

    gpio_phandle = phandle++;
    nodename = g_strdup_printf("/gpio@%lx",
        (long)memmap[NUCLEI_U_DEV_GPIO].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_cell(fdt, nodename, "clocks", hfclk_phandle);
    qemu_fdt_setprop_cell(fdt, nodename, "#interrupt-cells", 2);
    qemu_fdt_setprop(fdt, nodename, "interrupt-controller", NULL, 0);
    qemu_fdt_setprop_cell(fdt, nodename, "#gpio-cells", 2);
    qemu_fdt_setprop(fdt, nodename, "gpio-controller", NULL, 0);
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[NUCLEI_U_DEV_GPIO].base,
        0x0, memmap[NUCLEI_U_DEV_GPIO].size);
    qemu_fdt_setprop_cells(fdt, nodename, "interrupts", NUCLEI_U_GPIO_IRQ0,
        NUCLEI_U_GPIO_IRQ1, NUCLEI_U_GPIO_IRQ2, NUCLEI_U_GPIO_IRQ3,
        NUCLEI_U_GPIO_IRQ4, NUCLEI_U_GPIO_IRQ5, NUCLEI_U_GPIO_IRQ6,
        NUCLEI_U_GPIO_IRQ7, NUCLEI_U_GPIO_IRQ8, NUCLEI_U_GPIO_IRQ9,
        NUCLEI_U_GPIO_IRQ10, NUCLEI_U_GPIO_IRQ11, NUCLEI_U_GPIO_IRQ12,
        NUCLEI_U_GPIO_IRQ13, NUCLEI_U_GPIO_IRQ14, NUCLEI_U_GPIO_IRQ15);
    qemu_fdt_setprop_cell(fdt, nodename, "interrupt-parent", plic_phandle);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "sifive,gpio0");
    qemu_fdt_setprop_cell(fdt, nodename, "phandle", gpio_phandle);
    qemu_fdt_setprop_string(fdt, nodename, "status", "okay");
    g_free(nodename);

    nodename = g_strdup_printf("/spi@%lx/mmc@0",
        (long)memmap[NUCLEI_U_SPI2].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "mmc-spi-slot");
    qemu_fdt_setprop_cells(fdt, nodename, "reg", 0x0);
    qemu_fdt_setprop_cells(fdt, nodename, "spi-max-frequency", 20000000);
    qemu_fdt_setprop_cells(fdt, nodename, "voltage-ranges", 3300, 3300);
    qemu_fdt_setprop_cells(fdt, nodename, "disable-wp");
    qemu_fdt_setprop_cells(fdt, nodename, "gpios", gpio_phandle,  30, 31);
    g_free(nodename);

update_bootargs:
    if (cmdline) {
        qemu_fdt_setprop_string(fdt, "/chosen", "bootargs", cmdline);
    }
}

// static void sifive_u_machine_reset(void *opaque, int n, int level)
// {
//     /* gpio pin active low triggers reset */
//     if (!level) {
//         qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
//     }
// }

static void sifive_u_machine_init(MachineState *machine)
{
    const struct MemmapEntry *memmap = nuclei_u_memmap;
    NucLeiUState *s = RISCV_NUCLEI_U_MACHINE(machine);
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    MemoryRegion *flash0 = g_new(MemoryRegion, 1);
    target_ulong start_addr = memmap[NUCLEI_U_DEV_DRAM].base;
    target_ulong firmware_end_addr, kernel_start_addr;
    uint32_t start_addr_hi32 = 0x00000000;
    int i;
    uint32_t fdt_load_addr;
    uint64_t kernel_entry;
     DriveInfo *dinfo;
    DeviceState *flash_dev, *sd_dev;
    qemu_irq flash_cs, sd_cs;

    /* Initialize SoC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_RISCV_NUCLEI_U_SOC);
    object_property_set_uint(OBJECT(&s->soc), "serial", s->serial,
                             &error_abort);
    object_property_set_str(OBJECT(&s->soc), "cpu-type", machine->cpu_type,
                             &error_abort);
    qdev_realize(DEVICE(&s->soc), NULL, &error_abort);

    /* register RAM */
    memory_region_init_ram(main_mem, NULL, "riscv.nuclei.u.ram",
                           machine->ram_size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[NUCLEI_U_DEV_DRAM].base,
                                main_mem);

    /* register QSPI0 Flash */
    memory_region_init_ram(flash0, NULL, "riscv.nuclei.u.flash0",
                           memmap[NUCLEI_U_DEV_FLASH0].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[NUCLEI_U_DEV_FLASH0].base,
                                flash0);


    /* create device tree */
    create_fdt(s, memmap, machine->ram_size, machine->kernel_cmdline);

    if (s->start_in_flash) {
        /*
         * If start_in_flash property is given, assign s->msel to a value
         * that representing booting from QSPI0 memory-mapped flash.
         *
         * This also means that when both start_in_flash and msel properties
         * are given, start_in_flash takes the precedence over msel.
         *
         * Note this is to keep backward compatibility not to break existing
         * users that use start_in_flash property.
         */
        s->msel = MSEL_MEMMAP_QSPI0_FLASH;
    }

    switch (s->msel) {
    case MSEL_MEMMAP_QSPI0_FLASH:
        start_addr = memmap[NUCLEI_U_DEV_FLASH0].base;
        break;
    default:
        start_addr = memmap[NUCLEI_U_DEV_DRAM].base;
        break;
    }

    firmware_end_addr = riscv_find_and_load_firmware(machine, BIOS_FILENAME,
                                                     start_addr, NULL);

    if (machine->kernel_filename) {
        kernel_start_addr = riscv_calc_kernel_start_addr(&s->soc.u_cpus,
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
    fdt_load_addr = riscv_load_fdt(memmap[NUCLEI_U_DEV_DRAM].base,
                                   machine->ram_size, s->fdt);
    #if defined(TARGET_RISCV64)
    start_addr_hi32 = start_addr >> 32;
    #endif

    /* reset vector */
    uint32_t reset_vec[11] = {
        s->msel,                       /* MSEL pin state */
        0x00000297,                    /* 1:  auipc  t0, %pcrel_hi(fw_dyn) */
        0x02828613,                    /*     addi   a2, t0, %pcrel_lo(1b) */
        0xf1402573,                    /*     csrr   a0, mhartid  */
#if defined(TARGET_RISCV32)
        0x0202a583,                    /*     lw     a1, 32(t0) */
        0x0182a283,                    /*     lw     t0, 24(t0) */
#elif defined(TARGET_RISCV64)
        0x0202b583,                    /*     ld     a1, 32(t0) */
        0x0182b283,                    /*     ld     t0, 24(t0) */
#endif
        0x00028067,                    /*     jr     t0 */
        start_addr,                    /* start: .dword */
        start_addr_hi32,
        fdt_load_addr,                 /* fdt_laddr: .dword */
        0x00000000,
                                       /* fw_dyn: */
    };

    /* copy in the reset vector in little_endian byte order */
    for (i = 0; i < ARRAY_SIZE(reset_vec); i++) {
        reset_vec[i] = cpu_to_le32(reset_vec[i]);
    }
    rom_add_blob_fixed_as("mrom.reset", reset_vec, sizeof(reset_vec),
                          memmap[NUCLEI_U_DEV_MROM].base, &address_space_memory);

    riscv_rom_copy_firmware_info(machine, memmap[NUCLEI_U_DEV_MROM].base,
                                 memmap[NUCLEI_U_DEV_MROM].size,
                                 sizeof(reset_vec), kernel_entry);

    /* Connect an SPI flash to SPI0 */
    flash_dev = qdev_new("is25wp256");
    dinfo = drive_get_next(IF_MTD);
    if (dinfo) {
        qdev_prop_set_drive_err(flash_dev, "drive",
                                blk_by_legacy_dinfo(dinfo),
                                &error_fatal);
    }
    qdev_realize_and_unref(flash_dev, BUS(s->soc.spi0.spi), &error_fatal);

    flash_cs = qdev_get_gpio_in_named(flash_dev, SSI_GPIO_CS, 0);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->soc.spi0), 1, flash_cs);

    /* Connect an SD card to SPI2 */
    sd_dev = ssi_create_peripheral(s->soc.spi2.spi, "ssi-sd");

    sd_cs = qdev_get_gpio_in_named(sd_dev, SSI_GPIO_CS, 0);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->soc.spi2), 1, sd_cs);
}

static bool sifive_u_machine_get_start_in_flash(Object *obj, Error **errp)
{
    NucLeiUState *s = RISCV_NUCLEI_U_MACHINE(obj);

    return s->start_in_flash;
}

static void sifive_u_machine_set_start_in_flash(Object *obj, bool value, Error **errp)
{
    NucLeiUState *s = RISCV_NUCLEI_U_MACHINE(obj);

    s->start_in_flash = value;
}

static void sifive_u_machine_get_uint32_prop(Object *obj, Visitor *v,
                                             const char *name, void *opaque,
                                             Error **errp)
{
    visit_type_uint32(v, name, (uint32_t *)opaque, errp);
}

static void sifive_u_machine_set_uint32_prop(Object *obj, Visitor *v,
                                             const char *name, void *opaque,
                                             Error **errp)
{
    visit_type_uint32(v, name, (uint32_t *)opaque, errp);
}

static void nuclei_u_machine_instance_init(Object *obj)
{
    NucLeiUState *s = RISCV_NUCLEI_U_MACHINE(obj);

    s->start_in_flash = false;
    s->msel = 0;
    object_property_add(obj, "msel", "uint32",
                        sifive_u_machine_get_uint32_prop,
                        sifive_u_machine_set_uint32_prop, NULL, &s->msel);
    object_property_set_description(obj, "msel",
                                    "Mode Select (MSEL[3:0]) pin state");

    s->serial = OTP_SERIAL;
    object_property_add(obj, "serial", "uint32",
                        sifive_u_machine_get_uint32_prop,
                        sifive_u_machine_set_uint32_prop, NULL, &s->serial);
    object_property_set_description(obj, "serial", "Board serial number");
}

static void nuclei_u_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "RISC-V NUCLEI U Board";
    mc->init = sifive_u_machine_init;
    mc->max_cpus = NUCLEI_U_MANAGEMENT_CPU_COUNT + NUCLEI_U_COMPUTE_CPU_COUNT;
    mc->min_cpus = NUCLEI_U_MANAGEMENT_CPU_COUNT + 1;
    mc->default_cpu_type = TYPE_RISCV_CPU_SIFIVE_U54;
    mc->default_cpus = mc->min_cpus;

    object_class_property_add_bool(oc, "start-in-flash",
                                   sifive_u_machine_get_start_in_flash,
                                   sifive_u_machine_set_start_in_flash);
    object_class_property_set_description(oc, "start-in-flash",
                                          "Set on to tell QEMU's ROM to jump to "
                                          "flash. Otherwise QEMU will jump to DRAM "
                                          "or L2LIM depending on the msel value");
}

static const TypeInfo nuclei_u_machine_typeinfo = {
    .name       = MACHINE_TYPE_NAME("nuclei_u"),
    .parent     = TYPE_MACHINE,
    .class_init = nuclei_u_machine_class_init,
    .instance_init = nuclei_u_machine_instance_init,
    .instance_size = sizeof(NucLeiUState),
};

static void nuclei_u_machine_init_register_types(void)
{
    type_register_static(&nuclei_u_machine_typeinfo);
}

type_init(nuclei_u_machine_init_register_types)

static void nuclei_u_soc_instance_init(Object *obj)
{
    NucLeiUSoCState *s = RISCV_NUCLEI_U_SOC(obj);

    object_initialize_child(obj, "e-cluster", &s->e_cluster, TYPE_CPU_CLUSTER);
    qdev_prop_set_uint32(DEVICE(&s->e_cluster), "cluster-id", 0);

    object_initialize_child(OBJECT(&s->e_cluster), "e-cpus", &s->e_cpus,
                            TYPE_RISCV_HART_ARRAY);
    qdev_prop_set_uint32(DEVICE(&s->e_cpus), "num-harts", 1);
    qdev_prop_set_uint32(DEVICE(&s->e_cpus), "hartid-base", 0);
    qdev_prop_set_string(DEVICE(&s->e_cpus), "cpu-type", SIFIVE_U_CPU);
    qdev_prop_set_uint64(DEVICE(&s->e_cpus), "resetvec", 0x1004);

    object_initialize_child(obj, "u-cluster", &s->u_cluster, TYPE_CPU_CLUSTER);
    qdev_prop_set_uint32(DEVICE(&s->u_cluster), "cluster-id", 1);

    object_initialize_child(OBJECT(&s->u_cluster), "u-cpus", &s->u_cpus,
                            TYPE_RISCV_HART_ARRAY);

    object_initialize_child(obj, "gpio", &s->gpio, TYPE_SIFIVE_GPIO);
    object_initialize_child(obj, "spi0", &s->spi0, TYPE_SIFIVE_SPI);
    object_initialize_child(obj, "spi2", &s->spi2, TYPE_SIFIVE_SPI);
    object_initialize_child(obj, "timer",&s->timer,TYPE_NUCLEI_SYSTIMER);

}

static void nuclei_u_soc_realize(DeviceState *dev, Error **errp)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    NucLeiUSoCState *s = RISCV_NUCLEI_U_SOC(dev);
    const struct MemmapEntry *memmap = nuclei_u_memmap;
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *mask_rom = g_new(MemoryRegion, 1);
    char *plic_hart_config;
    size_t plic_hart_config_len;
    int i;

    qdev_prop_set_uint32(DEVICE(&s->u_cpus), "num-harts", ms->smp.cpus - 1);
    qdev_prop_set_uint32(DEVICE(&s->u_cpus), "hartid-base", 1);
    qdev_prop_set_string(DEVICE(&s->u_cpus), "cpu-type", s->cpu_type);
    qdev_prop_set_uint64(DEVICE(&s->u_cpus), "resetvec", 0x1004);

    sysbus_realize(SYS_BUS_DEVICE(&s->e_cpus), &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->u_cpus), &error_abort);
    /*
     * The cluster must be realized after the RISC-V hart array container,
     * as the container's CPU object is only created on realize, and the
     * CPU must exist and have been parented into the cluster before the
     * cluster is realized.
     */
    qdev_realize(DEVICE(&s->e_cluster), NULL, &error_abort);
    qdev_realize(DEVICE(&s->u_cluster), NULL, &error_abort);

    /* boot rom */
    memory_region_init_rom(mask_rom, OBJECT(dev), "riscv.nuclei.u.mrom",
                           memmap[NUCLEI_U_DEV_MROM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[NUCLEI_U_DEV_MROM].base,
                                mask_rom);

    /* create PLIC hart topology configuration string */
    plic_hart_config_len = (strlen(NUCLEI_U_PLIC_HART_CONFIG) + 1) *
                           ms->smp.cpus;
    plic_hart_config = g_malloc0(plic_hart_config_len);
    for (i = 0; i < ms->smp.cpus; i++) {
        if (i != 0) {
            strncat(plic_hart_config, "," NUCLEI_U_PLIC_HART_CONFIG,
                    plic_hart_config_len);
        } else {
            strncat(plic_hart_config, "M", plic_hart_config_len);
        }
        plic_hart_config_len -= (strlen(NUCLEI_U_PLIC_HART_CONFIG) + 1);
    }


    /* MMIO */
    s->plic = sifive_plic_create(memmap[NUCLEI_U_DEV_PLIC].base,
        plic_hart_config, 0,
        NUCLEI_U_PLIC_NUM_SOURCES,
        NUCLEI_U_PLIC_NUM_PRIORITIES,
        NUCLEI_U_PLIC_PRIORITY_BASE,
        NUCLEI_U_PLIC_PENDING_BASE,
        NUCLEI_U_PLIC_ENABLE_BASE,
        NUCLEI_U_PLIC_ENABLE_STRIDE,
        NUCLEI_U_PLIC_CONTEXT_BASE,
        NUCLEI_U_PLIC_CONTEXT_STRIDE,
        memmap[NUCLEI_U_DEV_PLIC].size);
    g_free(plic_hart_config);

    sifive_uart_create(system_memory, memmap[NUCLEI_U_DEV_UART0].base,
        serial_hd(0), qdev_get_gpio_in(DEVICE(s->plic), NUCLEI_U_UART0_IRQ));

    sifive_uart_create(system_memory, memmap[NUCLEI_U_DEV_UART1].base,
        serial_hd(1), qdev_get_gpio_in(DEVICE(s->plic), NUCLEI_U_UART1_IRQ));

    sifive_clint_create(memmap[NUCLEI_U_DEV_CLINT].base,
        memmap[NUCLEI_U_DEV_CLINT].size, 0, ms->smp.cpus,
        SIFIVE_SIP_BASE, SIFIVE_TIMECMP_BASE, SIFIVE_TIME_BASE,
        SIFIVE_CLINT_TIMEBASE_FREQ, false);

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->timer), errp)) {
        return;
    }
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->timer), 0, memmap[NUCLEI_U_DEV_TIMER].base);
    s->timer.timebase_freq = NUCLEI_HBIRD_TIMEBASE_FREQ;

    qdev_prop_set_uint32(DEVICE(&s->gpio), "ngpio", 16);
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->gpio), errp)) {
        return;
    }
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->gpio), 0, memmap[NUCLEI_U_DEV_GPIO].base);

    /* Pass all GPIOs to the SOC layer so they are available to the board */
    qdev_pass_gpios(DEVICE(&s->gpio), dev, NULL);


    /* Connect GPIO interrupts to the PLIC */
    for (i = 0; i < 16; i++) {
        sysbus_connect_irq(SYS_BUS_DEVICE(&s->gpio), i,
                           qdev_get_gpio_in(DEVICE(s->plic),
                                            NUCLEI_U_GPIO_IRQ0 + i));
    }

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->spi[0]), errp)) {
        return;
    }

    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi[0]), 0, memmap[NUCLEI_U_SPI0].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->spi[0]), 0,
                            qdev_get_gpio_in(DEVICE(s->plic), NUCLEI_U_SPI0_IRQ));

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->spi[2]), errp)) {
        return;
    }

    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi[2]), 0, memmap[NUCLEI_U_SPI2].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->spi[2]), 0,
                            qdev_get_gpio_in(DEVICE(s->plic), NUCLEI_U_SPI2_IRQ));

    sysbus_realize(SYS_BUS_DEVICE(&s->spi0), errp);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi0), 0,
                    memmap[NUCLEI_U_SPI0_IRQ].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->spi0), 0,
                       qdev_get_gpio_in(DEVICE(s->plic), NUCLEI_U_SPI0_IRQ));
    sysbus_realize(SYS_BUS_DEVICE(&s->spi2), errp);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi2), 0,
                    memmap[NUCLEI_U_SPI2_IRQ].base);
    sysbus_connect_irq(SYS_BUS_DEVICE(&s->spi2), 0,
                       qdev_get_gpio_in(DEVICE(s->plic), NUCLEI_U_SPI2_IRQ));
    qemu_log(">> nuclei_u_soc_realize\n");

}

static Property nuclei_u_soc_props[] = {
    DEFINE_PROP_UINT32("serial", NucLeiUSoCState, serial, OTP_SERIAL),
    DEFINE_PROP_STRING("cpu-type", NucLeiUSoCState, cpu_type),
    DEFINE_PROP_END_OF_LIST()
};

static void nuclei_u_soc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    device_class_set_props(dc, nuclei_u_soc_props);
    dc->realize = nuclei_u_soc_realize;
    /* Reason: Uses serial_hds in realize function, thus can't be used twice */
    dc->user_creatable = false;
}

static const TypeInfo nuclei_u_soc_type_info = {
    .name = TYPE_RISCV_NUCLEI_U_SOC,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(NucLeiUSoCState),
    .instance_init = nuclei_u_soc_instance_init,
    .class_init = nuclei_u_soc_class_init,
};

static void nuclei_u_soc_register_types(void)
{
    type_register_static(&nuclei_u_soc_type_info);
}

type_init(nuclei_u_soc_register_types)