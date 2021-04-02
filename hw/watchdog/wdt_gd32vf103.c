/*
 *  GD32VF103 WDT interface
 *
 * Copyright (c) 2020 PLCT Lab
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
#include "qemu/timer.h"
#include "hw/watchdog/wdt_gd32vf103.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void gd32vf103_fwdgt_reset(DeviceState *dev)
{
    GD32VF103FWDGTState *s = GD32VF103_FWDGT(dev);
    s->fwdgt_ctl = 0x0;
    s->fwdgt_psc = 0x0;
    s->fwdgt_rld = 0x0;
    s->fwdgt_stat = 0x0;
}

static uint64_t gd32vf103_fwdgt_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    GD32VF103FWDGTState *s = GD32VF103_FWDGT(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case FWDGT_REG_FWDGT_CTL:
        value = s->fwdgt_ctl;
        break;
    case FWDGT_REG_FWDGT_PSC:
        value = s->fwdgt_psc;
        break;
    case FWDGT_REG_FWDGT_RLD:
        value = s->fwdgt_rld;
        break;
    case FWDGT_REG_FWDGT_STAT:
        value = s->fwdgt_stat;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void gd32vf103_fwdgt_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    GD32VF103FWDGTState *s = GD32VF103_FWDGT(opaque);
    switch (offset)
    {
    case FWDGT_REG_FWDGT_CTL:
        s->fwdgt_ctl = value;
        break;
    case FWDGT_REG_FWDGT_PSC:
        s->fwdgt_psc = value;
        break;
    case FWDGT_REG_FWDGT_RLD:
        s->fwdgt_rld = value;
        break;
    case FWDGT_REG_FWDGT_STAT:
        s->fwdgt_stat = value;
        break;
    default:
        break;
    }
}

static void gd32vf103_wwdgt_reset(DeviceState *dev)
{
    GD32VF103WWDGTState *s = GD32VF103_WWDGT(dev);
    s->wwdgt_ctl = 0x0;
    s->wwdgt_cfg = 0x0;
    s->wwdgt_stat = 0x0;
}

static uint64_t gd32vf103_wwdgt_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    GD32VF103WWDGTState *s = GD32VF103_WWDGT(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case WWDGT_REG_WWDGT_CTL:
        value = s->wwdgt_ctl;
        break;
    case WWDGT_REG_WWDGT_CFG:
        value = s->wwdgt_cfg;
        break;
    case WWDGT_REG_WWDGT_STAT:
        value = s->wwdgt_stat;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void gd32vf103_wwdgt_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    GD32VF103WWDGTState *s = GD32VF103_WWDGT(opaque);

    switch (offset)
    {
    case WWDGT_REG_WWDGT_CTL:
        s->wwdgt_ctl = value;
        break;
    case WWDGT_REG_WWDGT_CFG:
        s->wwdgt_cfg = value;
        break;
    case WWDGT_REG_WWDGT_STAT:
        s->wwdgt_stat = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps gd32vf103_fwdgt_ops = {
    .read = gd32vf103_fwdgt_read,
    .write = gd32vf103_fwdgt_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static const MemoryRegionOps gd32vf103_wwdgt_ops = {
    .read = gd32vf103_wwdgt_read,
    .write = gd32vf103_wwdgt_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};


static void gd32vf103_fwdgt_realize(DeviceState *dev, Error **errp)
{
    GD32VF103FWDGTState *s = GD32VF103_FWDGT(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &gd32vf103_fwdgt_ops,
                          s,TYPE_GD32VF103_FWDGT, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}


static void gd32vf103_wwdgt_realize(DeviceState *dev, Error **errp)
{
    GD32VF103WWDGTState *s = GD32VF103_WWDGT(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &gd32vf103_wwdgt_ops,
                          s,TYPE_GD32VF103_WWDGT, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}


static void gd32vf103_fwdgt_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = gd32vf103_fwdgt_realize;
    dc->reset = gd32vf103_fwdgt_reset;
    dc->desc = "GD32VF103 FWDGT";
}

static const TypeInfo gd32vf103_fwdgt_info = {
    .name = TYPE_GD32VF103_FWDGT,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103FWDGTState),
    .class_init = gd32vf103_fwdgt_class_init,
};

static void gd32vf103_fwdgt_register_types(void)
{
    type_register_static(&gd32vf103_fwdgt_info);
}
type_init(gd32vf103_fwdgt_register_types);

static void gd32vf103_wwdgt_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = gd32vf103_wwdgt_realize;
    dc->reset = gd32vf103_wwdgt_reset;
    dc->desc = "GD32VF103 WWDGT";
}

static const TypeInfo gd32vf103_wwdgt_info = {
    .name = TYPE_GD32VF103_WWDGT,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103WWDGTState),
    .class_init = gd32vf103_wwdgt_class_init,
};

static void gd32vf103_wwdgt_register_types(void)
{
    type_register_static(&gd32vf103_wwdgt_info);
}
type_init(gd32vf103_wwdgt_register_types);