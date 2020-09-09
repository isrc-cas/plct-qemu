/*
 * QEMU RISC-V Nuclei SoC
 *
 * Copyright (c) 2020 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "target/riscv/cpu.h"
#include "hw/misc/unimp.h"
#include "hw/riscv/riscv_htif.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/nuclei_eclic.h"
#include "hw/riscv/nuclei_uart.h"
#include "hw/riscv/nuclei_hbird.h"
#include "hw/riscv/boot.h"
#include "chardev/char.h"
#include "sysemu/arch_init.h"
#include "sysemu/device_tree.h"
#include "sysemu/qtest.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"

#include <libfdt.h>

static const struct MemmapEntry {
    hwaddr base;
    hwaddr size;
} nuclei_memmap[] = {
    [NUCLEI_DEBUG] = {        0x0,     0x1000 },
    [NUCLEI_ROM]   = {     0x1000,     0x1000 },
    [NUCLEI_TIMER] = {  0x2000000,     0x1000 },
    [NUCLEI_ECLIC] = {  0xc000000,    0x10000 },
    [NUCLEI_GPIO]  = { 0x10012000,     0x1000 },
    [NUCLEI_UART0] = { 0x10013000,     0x1000 },
    [NUCLEI_QSPI0] = { 0x10014000,     0x1000 },
    [NUCLEI_PWM0]  = { 0x10015000,     0x1000 },
    [NUCLEI_UART1] = { 0x10023000,     0x1000 },
    [NUCLEI_QSPI1] = { 0x10024000,     0x1000 },
    [NUCLEI_PWM1]  = { 0x10025000,     0x1000 },
    [NUCLEI_QSPI2] = { 0x10034000,     0x1000 },
    [NUCLEI_PWM2]  = { 0x10035000,     0x1000 },
    [NUCLEI_XIP]   = { 0x20000000, 0x10000000 },   //XIP FLASH
    [NUCLEI_ILM]   = { 0x80000000,    0x20000 },  
    [NUCLEI_DLM]   = { 0x90000000,    0x20000 },
};

static void nuclei_board_init(MachineState *machine)
{
    const struct MemmapEntry *memmap = nuclei_memmap;
    NucleiHBState *s = g_new0(NucleiHBState, 1);
    MemoryRegion *system_memory = get_system_memory();
    int i;

    /* TODO: Add qtest support */
    /* Initialize SOC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc, sizeof(s->soc),
                            TYPE_NUCLEI_HBIRD_SOC, &error_abort, NULL);
    object_property_set_bool(OBJECT(&s->soc), true, "realized",
                            &error_abort);

    memory_region_init_ram(&s->soc.ilm, NULL, "riscv.nuclei.ram.ilm",
                           memmap[NUCLEI_ILM].size, &error_fatal);
    memory_region_add_subregion(system_memory, 
    memmap[NUCLEI_ILM].base, &s->soc.ilm);

    memory_region_init_ram(&s->soc.dlm, NULL, "riscv.nuclei.ram.dlm",
                           memmap[NUCLEI_DLM].size, &error_fatal);
    memory_region_add_subregion(system_memory, 
    memmap[NUCLEI_DLM].base, &s->soc.dlm);

     /* reset vector */
    uint32_t reset_vec[8] = {
        0x00000297,                  /* 1:  auipc  t0, %pcrel_hi(dtb) */
        0x02028593,                  /*     addi   a1, t0, %pcrel_lo(1b) */
        0xf1402573,                  /*     csrr   a0, mhartid  */
#if defined(TARGET_RISCV32)
        0x0182a283,                  /*     lw     t0, 24(t0) */
#elif defined(TARGET_RISCV64)
        0x0182b283,                  /*     ld     t0, 24(t0) */
#endif
        0x00028067,                  /*     jr     t0 */
        0x00000000,
        memmap[NUCLEI_ILM].base,     /* start: .dword DRAM_BASE */
        0x00000000,
                                     /* dtb: */
    };

    /* copy in the reset vector in little_endian byte order */
    for (i = 0; i < sizeof(reset_vec) >> 2; i++) {
        reset_vec[i] = cpu_to_le32(reset_vec[i]);
    }
    rom_add_blob_fixed_as("mrom.reset", reset_vec, sizeof(reset_vec),
                          memmap[NUCLEI_ROM].base, &address_space_memory);

    /* boot rom */
    if (machine->kernel_filename) {
        riscv_load_kernel(machine->kernel_filename, NULL);
    }
}

static void riscv_nuclei_soc_init(Object *obj)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    NucleiHBSoCState *s = RISCV_NUCLEI_HBIRD_SOC(obj);

    object_initialize_child(obj, "cpus", &s->cpus,
                            sizeof(s->cpus), TYPE_RISCV_HART_ARRAY,
                            &error_abort, NULL);

    object_property_set_int(OBJECT(&s->cpus), ms->smp.cpus, "num-harts",
                            &error_abort);

    sysbus_init_child_obj(obj, "timer",
                          &s->timer, sizeof(s->timer),
                          TYPE_NUCLEI_SYSTIMER);

    sysbus_init_child_obj(obj, "riscv.nuclei.gpio",
                          &s->gpio, sizeof(s->gpio),
                          TYPE_SIFIVE_GPIO);
}

static void riscv_nuclei_soc_realize(DeviceState *dev, Error **errp)
{
    const struct MemmapEntry *memmap = nuclei_memmap;
    MachineState *ms = MACHINE(qdev_get_machine());
    NucleiHBSoCState *s = RISCV_NUCLEI_HBIRD_SOC(dev);
    MemoryRegion *sys_mem = get_system_memory();
    Error *err = NULL;

    object_property_set_str(OBJECT(&s->cpus), ms->cpu_type, "cpu-type",
                            &error_abort);
    object_property_set_bool(OBJECT(&s->cpus), true, "realized",
                            &error_abort);

    /* Mask ROM */
    memory_region_init_rom(&s->internal_rom, OBJECT(dev), "riscv.nuclei.irom",
                           memmap[NUCLEI_ROM].size, &error_fatal);
    memory_region_add_subregion(sys_mem,
        memmap[NUCLEI_ROM].base, &s->internal_rom);

    /* MMIO */
    s->eclic = nuclei_eclic_create(memmap[NUCLEI_ECLIC].base,
        memmap[NUCLEI_ECLIC].size, HBIRD_SOC_INT_MAX);

    /* Timer*/
    object_property_set_bool(OBJECT(&s->timer), true, "realized", &err);
    if (err) {
        error_propagate(errp, err);
        return;
    }
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->timer), 0, memmap[NUCLEI_TIMER].base);
    s->timer.timebase_freq = NUCLEI_HBIRD_TIMEBASE_FREQ;

    /* GPIO */
    object_property_set_bool(OBJECT(&s->gpio), true, "realized", &err);
    if (err) {
        error_propagate(errp, err);
        return;
    }

    /* Map GPIO registers */
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->gpio), 0, memmap[NUCLEI_GPIO].base);
    /* Pass all GPIOs to the SOC layer so they are available to the board */

    /* Create and connect UART interrupts to the ECLIC */
    nuclei_uart_create(sys_mem,
                    memmap[NUCLEI_UART0].base,
                    memmap[NUCLEI_UART0].size,
                    serial_hd(0),
                    nuclei_eclic_get_irq(DEVICE(s->eclic),
                    HBIRD_SOC_INT22_IRQn));

    s->timer.soft_irq = &(s->eclic->irqs[Internal_SysTimerSW_IRQn]);
    s->timer.timer_irq = &(s->eclic->irqs[Internal_SysTimer_IRQn]);

    /* Flash memory */
    memory_region_init_rom(&s->xip_mem, OBJECT(dev), "riscv.nuclei.xip",
                        memmap[NUCLEI_XIP].size, &error_fatal);
    memory_region_add_subregion(sys_mem, 
                        memmap[NUCLEI_XIP].base, &s->xip_mem);

}

static void nuclei_machine_init(MachineClass *mc)
{
    mc->desc = "RISC-V Nuclei HBird Eval Board";
    mc->init = nuclei_board_init;
    mc->max_cpus = 1;
    mc->is_default = false;
    mc->default_cpu_type = NUCLEI_CPU;
}

DEFINE_MACHINE("hbird_eval", nuclei_machine_init)

static void riscv_nuclei_soc_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    dc->realize = riscv_nuclei_soc_realize;
    dc->user_creatable = false;
}

static const TypeInfo riscv_nuclei_soc_type_info = {
    .name = TYPE_NUCLEI_HBIRD_SOC,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(NucleiHBSoCState),
    .instance_init = riscv_nuclei_soc_init,
    .class_init = riscv_nuclei_soc_class_init,
};

static void riscv_nuclei_soc_register_types(void)
{
    type_register_static(&riscv_nuclei_soc_type_info);
}

type_init(riscv_nuclei_soc_register_types)
