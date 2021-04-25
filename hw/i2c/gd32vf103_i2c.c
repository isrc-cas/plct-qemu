/*
 *  GD32VF103 I2C interface
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
#include "qemu/timer.h"
#include "hw/i2c/gd32vf103_i2c.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void gd32vf103_i2c_reset(DeviceState *dev)
{
    GD32VF103I2CState *s = GD32VF103_I2C(dev);
    s->i2c_ctl0 = 0x0;
    s->i2c_ctl1 = 0x0;
    s->i2c_saddr0 = 0x0;
    s->i2c_saddr1 = 0x0;
    s->i2c_data = 0x0;
    s->i2c_stat0 = 0x0;
    s->i2c_stat1 = 0x0;
    s->i2c_ckcfg = 0x0;
    s->i2c_rt = 0x2;
}

static uint64_t gd32vf103_i2c_read(void *opaque, hwaddr offset,
                                   unsigned size)
{
    GD32VF103I2CState *s = GD32VF103_I2C(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case I2C_REG_I2C_CTL0:
        value = s->i2c_ctl0;
        break;
    case I2C_REG_I2C_CTL1:
        value = s->i2c_ctl1;
        break;
    case I2C_REG_I2C_SADDR0:
        value = s->i2c_saddr0;
        break;
    case I2C_REG_I2C_SADDR1:
        value = s->i2c_saddr1;
        break;
    case I2C_REG_I2C_DATA:
        value = s->i2c_data;
        break;
    case I2C_REG_I2C_STAT0:
        value = s->i2c_stat0;
        break;
    case I2C_REG_I2C_STAT1:
        value = s->i2c_stat1;
        break;
    case I2C_REG_I2C_CKCFG:
        value = s->i2c_ckcfg;
        break;
    case I2C_REG_I2C_RT:
        value = s->i2c_rt;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void gd32vf103_i2c_write(void *opaque, hwaddr offset,
                                uint64_t value, unsigned size)
{
    GD32VF103I2CState *s = GD32VF103_I2C(opaque);

    switch (offset)
    {
    case I2C_REG_I2C_CTL0:
        s->i2c_ctl0 = value;
        break;
    case I2C_REG_I2C_CTL1:
        s->i2c_ctl1 = value;
        break;
    case I2C_REG_I2C_SADDR0:
        s->i2c_saddr0 = value;
        break;
    case I2C_REG_I2C_SADDR1:
        s->i2c_saddr1 = value;
        break;
    case I2C_REG_I2C_DATA:
        s->i2c_data = value;
        break;
    case I2C_REG_I2C_STAT0:
        s->i2c_stat0 = value;
        break;
    case I2C_REG_I2C_STAT1:
        s->i2c_stat1 = value;
        break;
    case I2C_REG_I2C_CKCFG:
        s->i2c_ckcfg = value;
        break;
    case I2C_REG_I2C_RT:
        s->i2c_rt = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps gd32vf103_i2c_ops = {
    .read = gd32vf103_i2c_read,
    .write = gd32vf103_i2c_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void gd32vf103_i2c_realize(DeviceState *dev, Error **errp)
{
    GD32VF103I2CState *s = GD32VF103_I2C(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &gd32vf103_i2c_ops,
                          s, TYPE_GD32VF103_I2C, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void gd32vf103_i2c_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = gd32vf103_i2c_realize;
    dc->reset = gd32vf103_i2c_reset;
    dc->desc = "GD32VF1-3 IIC";
}

static const TypeInfo gd32vf103_i2c_info = {
    .name = TYPE_GD32VF103_I2C,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103I2CState),
    .class_init = gd32vf103_i2c_class_init,
};

static void gd32vf103_i2c_register_types(void)
{
    type_register_static(&gd32vf103_i2c_info);
}
type_init(gd32vf103_i2c_register_types);