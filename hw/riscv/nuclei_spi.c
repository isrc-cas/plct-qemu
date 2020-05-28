/*
 *  NUCLEI SPI interface
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
#include "hw/riscv/nuclei_spi.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void nuclei_spi_reset(DeviceState *dev)
{
    NucLeiSPIState *s = NUCLEI_SPI(dev);

    s->spi_ctl0 = 0x0;
    s->spi_ctl1 = 0x0;
    s->spi_stat = 0x0;
    s->spi_data = 0x0;
    s->spi_crcpoly = 0x0;
    s->spi_rcrc = 0x0;
    s->spi_tcrc = 0x0;
    s->spi_i2sctl = 0x0;
    s->spi_i2spsc = 0x0;
}

static uint64_t nuclei_spi_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiSPIState *s = NUCLEI_SPI(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case NUCLEI_SPI_REG_SPI_CTL0:
        value = s->spi_ctl0;
        break;
    case NUCLEI_SPI_REG_SPI_CTL1:
        value = s->spi_ctl1;
        break;
    case NUCLEI_SPI_REG_SPI_STAT:
        value = s->spi_stat;
        break;
    case NUCLEI_SPI_REG_SPI_DATA:
        value = s->spi_data;
        break;
    case NUCLEI_SPI_REG_SPI_CRCPOLY:
        value = s->spi_crcpoly;
        break;
    case NUCLEI_SPI_REG_SPI_RCRC:
        value = s->spi_rcrc;
        break;
    case NUCLEI_SPI_REG_SPI_TCRC:
        value = s->spi_tcrc;
        break;
    case NUCLEI_SPI_REG_SPI_I2SCTL:
        value = s->spi_i2sctl;
        break;
    case NUCLEI_SPI_REG_SPI_I2SPSC:
        value = s->spi_i2spsc;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_spi_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiSPIState *s = NUCLEI_SPI(opaque);
    switch (offset)
    {
    case NUCLEI_SPI_REG_SPI_CTL0:
        s->spi_ctl0 = value;
        break;
    case NUCLEI_SPI_REG_SPI_CTL1:
        s->spi_ctl1 = value;
        break;
    case NUCLEI_SPI_REG_SPI_STAT:
        s->spi_stat = value;
        break;
    case NUCLEI_SPI_REG_SPI_DATA:
        s->spi_data = value;
        break;
    case NUCLEI_SPI_REG_SPI_CRCPOLY:
        s->spi_crcpoly = value;
        break;
    case NUCLEI_SPI_REG_SPI_RCRC:
        s->spi_rcrc = value;
        break;
    case NUCLEI_SPI_REG_SPI_TCRC:
        s->spi_tcrc = value;
        break;
    case NUCLEI_SPI_REG_SPI_I2SCTL:
        s->spi_i2sctl = value;
        break;
    case NUCLEI_SPI_REG_SPI_I2SPSC:
        s->spi_i2spsc = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_spi_ops = {
    .read = nuclei_spi_read,
    .write = nuclei_spi_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void nuclei_spi_realize(DeviceState *dev, Error **errp)
{
    NucLeiSPIState *s = NUCLEI_SPI(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_spi_ops,
                          s,TYPE_NUCLEI_SPI, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void nuclei_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_spi_realize;
    dc->reset = nuclei_spi_reset;
    dc->desc = "NucLei SPI";
}

static const TypeInfo nuclei_spi_info = {
    .name = TYPE_NUCLEI_SPI,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiSPIState),
    .class_init = nuclei_spi_class_init,
};

static void nuclei_spi_register_types(void)
{
    type_register_static(&nuclei_spi_info);
}
type_init(nuclei_spi_register_types);
