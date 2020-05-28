/*
 *  NUCLEI RCU (RCU Unit) interface
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
#include "qemu/timer.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/nuclei_rcu.h"
#include "hw/riscv/nuclei_eclic.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static uint64_t nuclei_rcu_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiRCUState *s = NUCLEI_RCU(opaque);
    // CPUState *cpu = qemu_get_cpu(0);
    // CPURISCVState *env = cpu ? cpu->env_ptr : NULL;
    uint64_t value = 0;

    switch (offset) {
    case NUCLEI_RCU_REG_RCU_CTL:
        
        value = s->rcu_ctl;
        break;
    case NUCLEI_RCU_REG_RCU_CFG0:
        value = s->rcu_cfg0;
        break;
    case NUCLEI_RCU_REG_RCU_INT:
         value = s->rcu_int;
        break;
    case NUCLEI_RCU_REG_RCU_APB2RST:
        value = s->rcu_apb2rst;
        break;
    case NUCLEI_RCU_REG_RCU_APB1RST:
        value = s->rcu_apb1rst;
        break;
    case NUCLEI_RCU_REG_RCU_AHBEN:
        value = s->rcu_ahben;
        break;
    case NUCLEI_RCU_REG_RCU_APB2EN:
        value = s->rcu_apb2en;
        break;
    case NUCLEI_RCU_REG_RCU_APB1EN:
         value = s->rcu_apb1en;
        break;
    case NUCLEI_RCU_REG_RCU_BDCTL:
        value = s->rcu_bdctl;
        break;
    case NUCLEI_RCU_REG_RCU_RSTSCK:
        value = s->rcu_rstsck;
        break;
    case NUCLEI_RCU_REG_RCU_AHBRST:
        value = s->rcu_ahbrst;
        break;
    case NUCLEI_RCU_REG_RCU_CFG1:
        value = s->rcu_cfg1;
        break;
    case NUCLEI_RCU_REG_RCU_DSV:
        value = s->rcu_dsv;
        break;
    default:
        break;
    }

    return value;
}

static void nuclei_rcu_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiRCUState *s = NUCLEI_RCU(opaque);

    switch (offset) {
    case NUCLEI_RCU_REG_RCU_CTL:
        if((value & RCU_CTL_HXTALEN) != 0)
            value |= RCU_CTL_HXTALSTB;
        if((value & RCU_CTL_PLL1EN) != 0)
            value |= RCU_CTL_PLL1STB;
        if((value & RCU_CTL_PLL2EN) != 0)
            value |= RCU_CTL_PLL2STB;
        if((value & RCU_CTL_PLLEN) != 0)
            value |= RCU_CTL_PLLSTB;
        s->rcu_ctl = value;
        break;
    case NUCLEI_RCU_REG_RCU_CFG0:
        s->rcu_cfg0 = value | 0x8;
        break;
    case NUCLEI_RCU_REG_RCU_INT:
        s->rcu_int = value;
        break;
    case NUCLEI_RCU_REG_RCU_APB2RST:
        s->rcu_apb2rst = value;
        break;
    case NUCLEI_RCU_REG_RCU_APB1RST:
        s->rcu_apb1rst = value;
        break;
    case NUCLEI_RCU_REG_RCU_AHBEN:
        s->rcu_ahben = value;
        break;
    case NUCLEI_RCU_REG_RCU_APB2EN:
        s->rcu_apb2en = value;
        break;
    case NUCLEI_RCU_REG_RCU_APB1EN:
        s->rcu_apb1en = value;
        break;
    case NUCLEI_RCU_REG_RCU_BDCTL:
        s->rcu_bdctl = value;
        break;
    case NUCLEI_RCU_REG_RCU_RSTSCK:
        s->rcu_rstsck = value;
        break;
    case NUCLEI_RCU_REG_RCU_AHBRST:
        s->rcu_ahbrst = value;
        break;
    case NUCLEI_RCU_REG_RCU_CFG1:
        s->rcu_cfg1 = value;
        break;
    case NUCLEI_RCU_REG_RCU_DSV:
        s->rcu_dsv = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_rcu_ops = {
    .read = nuclei_rcu_read,
    .write = nuclei_rcu_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void nuclei_rcu_reset(DeviceState *dev)
{
    NucLeiRCUState *s = NUCLEI_RCU(dev);
    s->rcu_ctl = 0x0000083;
    s->rcu_cfg0 = 0x0;
    s->rcu_int = 0x0;
    s->rcu_apb2rst = 0x0;
    s->rcu_ahben = 0x14;
    s->rcu_apb2en = 0x0;
    s->rcu_apb1en = 0x0;
    s->rcu_rstsck = 0x0C000000;
    s->rcu_cfg1 = 0x0;
    s->rcu_dsv = 0x0;
}

static void nuclei_rcu_realize(DeviceState *dev, Error **errp)
{
    NucLeiRCUState *s = NUCLEI_RCU(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_rcu_ops,
                          s,TYPE_NUCLEI_RCU, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);

    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->rcu_irq);
}

static void nuclei_rcu_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_rcu_realize;
    dc->reset = nuclei_rcu_reset;
    dc->desc = "NucLei RCU";
}

static const TypeInfo nuclei_rcu_info = {
    .name = TYPE_NUCLEI_RCU,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiRCUState),
    .class_init = nuclei_rcu_class_init,
};

static void nuclei_rcu_register_types(void)
{
    type_register_static(&nuclei_rcu_info);
}
type_init(nuclei_rcu_register_types);
