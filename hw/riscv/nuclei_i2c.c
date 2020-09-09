/*
 *  NUCLEI IIC interface
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
#include "hw/riscv/nuclei_i2c.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void nuclei_i2c_reset(DeviceState *dev)
{
    NucLeiIICState *s = NUCLEI_IIC(dev);
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

static uint64_t nuclei_i2c_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiIICState *s = NUCLEI_IIC(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case NUCLEI_I2C_REG_I2C_CTL0:
        value = s->i2c_ctl0;
        break;
    case NUCLEI_I2C_REG_I2C_CTL1:
        value = s->i2c_ctl1;
        break;
    case NUCLEI_I2C_REG_I2C_SADDR0:
        value = s->i2c_saddr0;
        break;
    case NUCLEI_I2C_REG_I2C_SADDR1:
        value = s->i2c_saddr1;
        break;
    case NUCLEI_I2C_REG_I2C_DATA:
        value = s->i2c_data;
        break;
    case NUCLEI_I2C_REG_I2C_STAT0:
        value = s->i2c_stat0;
        break;
    case NUCLEI_I2C_REG_I2C_STAT1:
        value = s->i2c_stat1;
        break;
    case NUCLEI_I2C_REG_I2C_CKCFG:
        value = s->i2c_ckcfg;
        break;
    case NUCLEI_I2C_REG_I2C_RT:
        value = s->i2c_rt;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_i2c_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiIICState *s = NUCLEI_IIC(opaque);

    switch (offset)
    {
    case NUCLEI_I2C_REG_I2C_CTL0:
        s->i2c_ctl0 = value;
        break;
    case NUCLEI_I2C_REG_I2C_CTL1:
        s->i2c_ctl1 = value;
        break;
    case NUCLEI_I2C_REG_I2C_SADDR0:
        s->i2c_saddr0 = value;
        break;
    case NUCLEI_I2C_REG_I2C_SADDR1:
        s->i2c_saddr1 = value;
        break;
    case NUCLEI_I2C_REG_I2C_DATA:
        s->i2c_data = value;
        break;
    case NUCLEI_I2C_REG_I2C_STAT0:
        s->i2c_stat0 = value;
        break;
    case NUCLEI_I2C_REG_I2C_STAT1:
        s->i2c_stat1 = value;
        break;
    case NUCLEI_I2C_REG_I2C_CKCFG:
        s->i2c_ckcfg = value;
        break;
    case NUCLEI_I2C_REG_I2C_RT:
        s->i2c_rt = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_i2c_ops = {
    .read = nuclei_i2c_read,
    .write = nuclei_i2c_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void nuclei_i2c_realize(DeviceState *dev, Error **errp)
{
    NucLeiIICState *s = NUCLEI_IIC(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_i2c_ops,
                          s,TYPE_NUCLEI_IIC, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void nuclei_i2c_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_i2c_realize;
    dc->reset = nuclei_i2c_reset;
    dc->desc = "NucLei IIC";
}

static const TypeInfo nuclei_i2c_info = {
    .name = TYPE_NUCLEI_IIC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiIICState),
    .class_init = nuclei_i2c_class_init,
};

static void nuclei_i2c_register_types(void)
{
    type_register_static(&nuclei_i2c_info);
}
type_init(nuclei_i2c_register_types);
