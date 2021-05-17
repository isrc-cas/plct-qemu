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
#include "net/eth.h"
#include "sysemu/arch_init.h"
#include "sysemu/device_tree.h"
#include "sysemu/runstate.h"
#include "sysemu/sysemu.h"

static void nuclei_ddr_machine_init(MachineState *machine)
{
    NucLeiUState *s = DDR_FPGA_MACHINE(machine);

    /* Initialize SOC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_NUCLEI_U_SOC);
    qdev_realize(DEVICE(&s->soc), NULL, &error_abort);
}

static void nuclei_ddr_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "Nuclei DDR 200T FPGA Evaluation Kit";
    mc->init = nuclei_ddr_machine_init;
}

static void nuclei_ddr_machine_instance_init(Object *obj)
{
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
}

static void nuclei_u_soc_instance_init(Object *obj)
{
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