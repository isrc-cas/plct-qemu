/*
 * Nuclei N series  SOC machine interface
 *
 * Copyright (c) 2020 Gao ZhiYuan <alapha23@gmail.com>
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
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "target/riscv/cpu.h"
#include "hw/misc/unimp.h"
#include "hw/char/riscv_htif.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/nuclei_n.h"
#include "sysemu/arch_init.h"
#include "sysemu/device_tree.h"
#include "sysemu/qtest.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"

static void nuclei_mcu_machine_init(MachineState *machine)
{
    NucLeiNState *s = MCU_FPGA_MACHINE(machine);
    qemu_log(">>nuclei_mcu_machine_init \n");

    /* Initialize SOC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_NUCLEI_N_SOC);
    qdev_realize(DEVICE(&s->soc), NULL, &error_abort);
}

static void nuclei_n_soc_realize(DeviceState *dev, Error **errp)
{
    qemu_log(">>nuclei_n_soc_realize \n");
}

static void nuclei_mcu_machine_instance_init(Object *obj)
{
    qemu_log(">>nuclei_machine_instance_init \n");
}

static void nuclei_mcu_machine_class_init(ObjectClass *oc, void *data)
{
    qemu_log(">>nuclei_mcu_machine_class_init \n");
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->desc = "Nuclei MCU 200T FPGA Evaluation Kit";
    mc->init = nuclei_mcu_machine_init;
}

static const TypeInfo nuclei_mcu_machine_typeinfo = {
    .name = TYPE_NUCLEI_MCU_FPGA_MACHINE,
    .parent = TYPE_MACHINE,
    .class_init = nuclei_mcu_machine_class_init,
    .instance_init = nuclei_mcu_machine_instance_init,
    .instance_size = sizeof(NucLeiNState),
};

static void nuclei_mcu_machine_register_types(void)
{
    type_register_static(&nuclei_mcu_machine_typeinfo);
}

type_init(nuclei_mcu_machine_register_types)

static void nuclei_n_soc_instance_init(Object *obj)
{
    qemu_log(">>nuclei_n_soc_instance_init \n");
}

static void nuclei_n_soc_class_init(ObjectClass *oc, void *data)
{
    qemu_log(">>nuclei_n_soc_class_init \n");
    DeviceClass *dc = DEVICE_CLASS(oc);
    dc->realize = nuclei_n_soc_realize;
    dc->user_creatable = false;
}

static const TypeInfo nuclei_n_soc_type_info = {
    .name = TYPE_NUCLEI_N_SOC,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(NucLeiNSoCState),
    .instance_init = nuclei_n_soc_instance_init,
    .class_init = nuclei_n_soc_class_init,
};

static void nuclei_n_soc_register_types(void)
{
    type_register_static(&nuclei_n_soc_type_info);
}

type_init(nuclei_n_soc_register_types)