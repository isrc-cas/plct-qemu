/*
 *  NUCLEI WDT interface
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
#include "hw/riscv/nuclei_wdt.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void nuclei_fwdgt_reset(DeviceState *dev)
{
    NucLeiFWDGTState *s = NUCLEI_FWDGT(dev);
    s->fwdgt_ctl = 0x0;
    s->fwdgt_psc = 0x0;
    s->fwdgt_rld = 0x0;
    s->fwdgt_stat = 0x0;
}

static uint64_t nuclei_fwdgt_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiFWDGTState *s = NUCLEI_FWDGT(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case NUCLEI_FWDGT_REG_FWDGT_CTL:
        value = s->fwdgt_ctl;
        break;
    case NUCLEI_FWDGT_REG_FWDGT_PSC:
        value = s->fwdgt_psc;
        break;
    case NUCLEI_FWDGT_REG_FWDGT_RLD:
        value = s->fwdgt_rld;
        break;
    case NUCLEI_FWDGT_REG_FWDGT_STAT:
        value = s->fwdgt_stat;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_fwdgt_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiFWDGTState *s = NUCLEI_FWDGT(opaque);
    switch (offset)
    {
    case NUCLEI_FWDGT_REG_FWDGT_CTL:
        s->fwdgt_ctl = value;
        break;
    case NUCLEI_FWDGT_REG_FWDGT_PSC:
        s->fwdgt_psc = value;
        break;
    case NUCLEI_FWDGT_REG_FWDGT_RLD:
        s->fwdgt_rld = value;
        break;
    case NUCLEI_FWDGT_REG_FWDGT_STAT:
        s->fwdgt_stat = value;
        break;
    default:
        break;
    }
}

static void nuclei_wwdgt_reset(DeviceState *dev)
{
    NucLeiWWDGTState *s = NUCLEI_WWDGT(dev);
    s->wwdgt_ctl = 0x0;
    s->wwdgt_cfg = 0x0;
    s->wwdgt_stat = 0x0;
}

static uint64_t nuclei_wwdgt_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiWWDGTState *s = NUCLEI_WWDGT(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case NUCLEI_WWDGT_REG_WWDGT_CTL:
        value = s->wwdgt_ctl;
        break;
    case NUCLEI_WWDGT_REG_WWDGT_CFG:
        value = s->wwdgt_cfg;
        break;
    case NUCLEI_WWDGT_REG_WWDGT_STAT:
        value = s->wwdgt_stat;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_wwdgt_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiWWDGTState *s = NUCLEI_WWDGT(opaque);

    switch (offset)
    {
    case NUCLEI_WWDGT_REG_WWDGT_CTL:
        s->wwdgt_ctl = value;
        break;
    case NUCLEI_WWDGT_REG_WWDGT_CFG:
        s->wwdgt_cfg = value;
        break;
    case NUCLEI_WWDGT_REG_WWDGT_STAT:
        s->wwdgt_stat = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_fwdgt_ops = {
    .read = nuclei_fwdgt_read,
    .write = nuclei_fwdgt_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static const MemoryRegionOps nuclei_wwdgt_ops = {
    .read = nuclei_wwdgt_read,
    .write = nuclei_wwdgt_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};


static void nuclei_fwdgt_realize(DeviceState *dev, Error **errp)
{
    NucLeiFWDGTState *s = NUCLEI_FWDGT(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_fwdgt_ops,
                          s,TYPE_NUCLEI_FWDGT, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}


static void nuclei_wwdgt_realize(DeviceState *dev, Error **errp)
{
    NucLeiWWDGTState *s = NUCLEI_WWDGT(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_wwdgt_ops,
                          s,TYPE_NUCLEI_WWDGT, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}


static void nuclei_fwdgt_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_fwdgt_realize;
    dc->reset = nuclei_fwdgt_reset;
    dc->desc = "NucLei FWDGT";
}

static const TypeInfo nuclei_fwdgt_info = {
    .name = TYPE_NUCLEI_FWDGT,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiFWDGTState),
    .class_init = nuclei_fwdgt_class_init,
};

static void nuclei_fwdgt_register_types(void)
{
    type_register_static(&nuclei_fwdgt_info);
}
type_init(nuclei_fwdgt_register_types);

static void nuclei_wwdgt_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_wwdgt_realize;
    dc->reset = nuclei_wwdgt_reset;
    dc->desc = "NucLei WWDGT";
}

static const TypeInfo nuclei_wwdgt_info = {
    .name = TYPE_NUCLEI_WWDGT,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiWWDGTState),
    .class_init = nuclei_wwdgt_class_init,
};

static void nuclei_wwdgt_register_types(void)
{
    type_register_static(&nuclei_wwdgt_info);
}
type_init(nuclei_wwdgt_register_types);