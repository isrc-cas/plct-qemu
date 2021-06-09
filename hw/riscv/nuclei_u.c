/*
 * Nuclei U series  SOC machine interface
 *
 * Copyright (c) 2020-2021 PLCT Lab.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "qapi/visitor.h"
#include "hw/hw.h"
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
#include "net/eth.h"
#include "sysemu/arch_init.h"
#include "sysemu/device_tree.h"
#include "sysemu/runstate.h"
#include "sysemu/sysemu.h"

#if defined(TARGET_RISCV32)
#define BIOS_FILENAME "opensbi-riscv32-generic-fw_dynamic.bin"
#else
#define BIOS_FILENAME "opensbi-riscv64-generic-fw_dynamic.bin"
#endif

static MemMapEntry nuclei_u_memmap[] = {
    [NUCLEI_U_MROM] = {0x1000, 0xf000},
    [NUCLEI_U_TIMER] = {0x2000000, 0x10000},
    [NUCLEI_U_CLINT] = {0x2001000, 0x10000},
    [NUCLEI_U_PLIC] = {0x8000000, 0x4000000},
    [NUCLEI_U_UART0] = {0x10013000, 0x1000},
    [NUCLEI_U_UART1] = {0x10023000, 0x1000},
    [NUCLEI_U_GPIO] = {0x10012000, 0x1000},
    [NUCLEI_U_SPI0] = {0x10014000, 0x1000},
    [NUCLEI_U_SPI2] = {0x10034000, 0x1000},
    [NUCLEI_U_FLASH0] = {0x20000000, 0x10000000},
    [NUCLEI_U_DRAM] = {0xa0000000, 0x0},
};

static void create_fdt(NucLeiUState *s, const struct MemMapEntry *memmap,
                       uint64_t mem_size, const char *cmdline)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    void *fdt;
    char *nodename;
    uint32_t phandle = 1;

    if (ms->dtb)
    {
        fdt = s->fdt = load_device_tree(ms->dtb, &s->fdt_size);
        if (!fdt)
        {
            error_report("load_device_tree() failed");
            exit(1);
        }
        goto update_bootargs;
    }
    else
    {
        fdt = s->fdt = create_device_tree(&s->fdt_size);
        if (!fdt)
        {
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

    qemu_fdt_add_subnode(fdt, "/console");
    qemu_fdt_setprop_string(fdt, "/console", "compatible", "sbi,console");

    nodename = g_strdup_printf("/memory@%lx",
                               (long)memmap[NUCLEI_U_DRAM].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
                           memmap[NUCLEI_U_DRAM].base >> 32, memmap[NUCLEI_U_DRAM].base,
                           mem_size >> 32, mem_size);
    qemu_fdt_setprop_string(fdt, nodename, "device_type", "memory");
    g_free(nodename);

    qemu_fdt_add_subnode(fdt, "/cpus");
    qemu_fdt_setprop_cell(fdt, "/cpus", "#size-cells", 0x0);
    qemu_fdt_setprop_cell(fdt, "/cpus", "#address-cells", 0x1);


    int cpu_phandle = phandle++;
    int cpu = 0;
    nodename = g_strdup_printf("/cpus/cpu@%d", cpu);
    char *intc = g_strdup_printf("/cpus/cpu@%d/interrupt-controller", cpu);
    char *isa;
    qemu_fdt_add_subnode(fdt, nodename);
    /* cpu 0 is the management hart that does not have mmu */
    qemu_fdt_setprop_string(fdt, nodename, "mmu-type", "riscv,sv39");
    isa = riscv_isa_string(&s->soc.cpus.harts[0]);
    qemu_fdt_setprop_string(fdt, nodename, "riscv,isa", isa);
    qemu_fdt_setprop_string(fdt, nodename, "compatible", "riscv");
    qemu_fdt_setprop_string(fdt, nodename, "status", "okay");
    qemu_fdt_setprop_cell(fdt, nodename, "reg", 0);
    qemu_fdt_setprop_string(fdt, nodename, "device_type", "cpu");
    qemu_fdt_add_subnode(fdt, intc);
    qemu_fdt_setprop_cell(fdt, intc, "phandle", cpu_phandle);
    qemu_fdt_setprop_string(fdt, intc, "compatible", "riscv,cpu-intc");
    qemu_fdt_setprop(fdt, intc, "interrupt-controller", NULL, 0);
    qemu_fdt_setprop_cell(fdt, intc, "#interrupt-cells", 1);
    g_free(isa);
    g_free(intc);
    g_free(nodename);

update_bootargs:
    if (cmdline)
    {
        qemu_fdt_setprop_string(fdt, "/chosen", "bootargs", cmdline);
    }
}

static void nuclei_u_machine_get_uint32_prop(Object *obj, Visitor *v,
                                           const char *name, void *opaque,
                                           Error **errp)
{
    visit_type_uint32(v, name, (uint32_t *)opaque, errp);
}

static void nuclei_u_machine_set_uint32_prop(Object *obj, Visitor *v,
                                           const char *name, void *opaque,
                                           Error **errp)
{
    visit_type_uint32(v, name, (uint32_t *)opaque, errp);
}

static void nuclei_ddr_machine_init(MachineState *machine)
{
    NucLeiUState *s = DDR_FPGA_MACHINE(machine);
    const MemMapEntry *memmap = nuclei_u_memmap;
    struct MemoryRegion *system_memory = get_system_memory();
    struct MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    struct MemoryRegion *flash0 = g_new(MemoryRegion, 1);
    target_ulong start_addr;
    target_ulong firmware_end_addr;
    target_ulong kernel_start_addr;
    uint64_t kernel_entry;
    uint32_t fdt_load_addr;

    /* Initialize SOC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_NUCLEI_U_SOC);
    qdev_realize(DEVICE(&s->soc), NULL, &error_abort);

    /* register RAM */
    memory_region_init_ram(main_mem, NULL, "riscv.nuclei.u.ram",
                           machine->ram_size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[NUCLEI_U_DRAM].base,
                                main_mem);

    /* register QSPI0 Flash */
    memory_region_init_ram(flash0, NULL, "riscv.nuclei.u.flash0",
                           memmap[NUCLEI_U_FLASH0].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[NUCLEI_U_FLASH0].base,
                                flash0);

    /* create device tree */
    create_fdt(s, memmap, machine->ram_size, machine->kernel_cmdline);

    switch (s->msel)
    {
    case MSEL_FLASH:
        start_addr = memmap[NUCLEI_U_FLASH0].base;
        break;
    default:
        start_addr = memmap[NUCLEI_U_DRAM].base;
        break;
    }

    firmware_end_addr = riscv_find_and_load_firmware(machine, BIOS_FILENAME,
                                                     start_addr, NULL);

    if (machine->kernel_filename)
    {
        kernel_start_addr = riscv_calc_kernel_start_addr(&s->soc.cpus,
                                                         firmware_end_addr);

        kernel_entry = riscv_load_kernel(machine->kernel_filename,
                                         kernel_start_addr, NULL);

        if (machine->initrd_filename)
        {
            hwaddr start;
            hwaddr end = riscv_load_initrd(machine->initrd_filename,
                                           machine->ram_size, kernel_entry,
                                           &start);
            qemu_fdt_setprop_cell(s->fdt, "/chosen",
                                  "linux,initrd-start", start);
            qemu_fdt_setprop_cell(s->fdt, "/chosen", "linux,initrd-end",
                                  end);
        }
    }
    else
    {
        kernel_entry = 0;
    }

    /* Compute the fdt load address in dram */
    fdt_load_addr = riscv_load_fdt(memmap[NUCLEI_U_DRAM].base,
                                   machine->ram_size, s->fdt);

        /* load the reset vector */
    riscv_setup_rom_reset_vec(machine, &s->soc.cpus, start_addr,
                              memmap[NUCLEI_U_MROM].base,
                              memmap[NUCLEI_U_MROM].size, kernel_entry,
                              fdt_load_addr, s->fdt);
}

static void nuclei_ddr_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Nuclei DDR 200T FPGA Evaluation Kit";
    mc->init = nuclei_ddr_machine_init;
    mc->max_cpus = 1;
    mc->default_cpu_type = NUCLEI_U_CPU;
}

static void nuclei_ddr_machine_instance_init(Object *obj)
{
    NucLeiUState *s = DDR_FPGA_MACHINE(obj);
    s->msel = 0;
    object_property_add(obj, "msel", "uint32",
                        nuclei_u_machine_get_uint32_prop,
                        nuclei_u_machine_set_uint32_prop, NULL, &s->msel);

}

static const TypeInfo nuclei_ddr_machine_typeinfo = {
    .name = TYPE_NUCLEI_DDR_FPGA_MACHINE,
    .parent = TYPE_MACHINE,
    .class_init = nuclei_ddr_machine_class_init,
    .instance_init = nuclei_ddr_machine_instance_init,
    .instance_size = sizeof(NucLeiUState),
};

static void nuclei_u_machine_init_register_types(void)
{
    type_register_static(&nuclei_ddr_machine_typeinfo);
}

type_init(nuclei_u_machine_init_register_types)

static void nuclei_u_soc_realize(DeviceState *dev, Error **errp)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    NucLeiUSoCState *s = NUCLEI_U_SOC(dev);
    const MemMapEntry *memmap = nuclei_u_memmap;
    struct MemoryRegion *system_memory = get_system_memory();
    struct MemoryRegion *internal_rom = g_new(MemoryRegion, 1);

    object_property_set_str(OBJECT(&s->cpus), "cpu-type", ms->cpu_type,
                            &error_abort);
    object_property_set_int(OBJECT(&s->cpus), "num-harts", ms->smp.cpus,
                            &error_abort);
    object_property_set_int(OBJECT(&s->cpus), "resetvec", 0x1004, &error_abort);

    sysbus_realize(SYS_BUS_DEVICE(&s->cpus), &error_abort);

    /* boot rom */
    memory_region_init_rom(internal_rom, OBJECT(dev), "riscv.nuclei.u.mrom",
                           memmap[NUCLEI_U_MROM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[NUCLEI_U_MROM].base,
                                internal_rom);

    /* Timer*/
    create_unimplemented_device("riscv.nuclei.u.timer",
        memmap[NUCLEI_U_TIMER].base, memmap[NUCLEI_U_TIMER].size);

    /* CLINT*/
    create_unimplemented_device("riscv.nuclei.u.clint",
        memmap[NUCLEI_U_CLINT].base, memmap[NUCLEI_U_CLINT].size);

    /* PLIC*/
    create_unimplemented_device("riscv.nuclei.u.plic",
        memmap[NUCLEI_U_PLIC].base, memmap[NUCLEI_U_PLIC].size);

    /* UART 0~1*/
    create_unimplemented_device("riscv.nuclei.u.uart0",
        memmap[NUCLEI_U_UART0].base, memmap[NUCLEI_U_UART0].size);
    create_unimplemented_device("riscv.nuclei.u.uart1",
        memmap[NUCLEI_U_UART1].base, memmap[NUCLEI_U_UART1].size);

    /* GPIO*/
    create_unimplemented_device("riscv.nuclei.u.gpio",
        memmap[NUCLEI_U_GPIO].base, memmap[NUCLEI_U_GPIO].size);

    /* SPI 0~1*/
    create_unimplemented_device("riscv.nuclei.u.spi0",
        memmap[NUCLEI_U_SPI0].base, memmap[NUCLEI_U_SPI0].size);
    create_unimplemented_device("riscv.nuclei.u.spi1",
        memmap[NUCLEI_U_SPI2].base, memmap[NUCLEI_U_SPI2].size);

}

static void nuclei_u_soc_instance_init(Object *obj)
{
    NucLeiUSoCState *s = NUCLEI_U_SOC(obj);
    object_initialize_child(obj, "cpus", &s->cpus, TYPE_RISCV_HART_ARRAY);
}

static void nuclei_u_soc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = nuclei_u_soc_realize;
}

static const TypeInfo nuclei_u_soc_type_info = {
    .name = TYPE_NUCLEI_U_SOC,
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