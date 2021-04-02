/*
 *  GD32VF103 RTC interface
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
#include "hw/ssi/gd32vf103_spi.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void gd32vf103_spi_reset(DeviceState *dev)
{
    GD32VF103SPIState *s = GD32VF103_SPI(dev);

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

static uint64_t gd32vf103_spi_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    GD32VF103SPIState *s = GD32VF103_SPI(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case SPI_REG_SPI_CTL0:
        value = s->spi_ctl0;
        break;
    case SPI_REG_SPI_CTL1:
        value = s->spi_ctl1;
        break;
    case SPI_REG_SPI_STAT:
        value = s->spi_stat;
        break;
    case SPI_REG_SPI_DATA:
        value = s->spi_data;
        break;
    case SPI_REG_SPI_CRCPOLY:
        value = s->spi_crcpoly;
        break;
    case SPI_REG_SPI_RCRC:
        value = s->spi_rcrc;
        break;
    case SPI_REG_SPI_TCRC:
        value = s->spi_tcrc;
        break;
    case SPI_REG_SPI_I2SCTL:
        value = s->spi_i2sctl;
        break;
    case SPI_REG_SPI_I2SPSC:
        value = s->spi_i2spsc;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void gd32vf103_spi_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    GD32VF103SPIState *s = GD32VF103_SPI(opaque);
    switch (offset)
    {
    case SPI_REG_SPI_CTL0:
        s->spi_ctl0 = value;
        break;
    case SPI_REG_SPI_CTL1:
        s->spi_ctl1 = value;
        break;
    case SPI_REG_SPI_STAT:
        s->spi_stat = value;
        break;
    case SPI_REG_SPI_DATA:
        s->spi_data = value;
        break;
    case SPI_REG_SPI_CRCPOLY:
        s->spi_crcpoly = value;
        break;
    case SPI_REG_SPI_RCRC:
        s->spi_rcrc = value;
        break;
    case SPI_REG_SPI_TCRC:
        s->spi_tcrc = value;
        break;
    case SPI_REG_SPI_I2SCTL:
        s->spi_i2sctl = value;
        break;
    case SPI_REG_SPI_I2SPSC:
        s->spi_i2spsc = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps gd32vf103_spi_ops = {
    .read = gd32vf103_spi_read,
    .write = gd32vf103_spi_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void gd32vf103_spi_realize(DeviceState *dev, Error **errp)
{
    GD32VF103SPIState *s = GD32VF103_SPI(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &gd32vf103_spi_ops,
                          s,TYPE_GD32VF103_SPI, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void gd32vf103_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = gd32vf103_spi_realize;
    dc->reset = gd32vf103_spi_reset;
    dc->desc = "GD32VF103 SPI";
}

static const TypeInfo gd32vf103_spi_info = {
    .name = TYPE_GD32VF103_SPI,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103SPIState),
    .class_init = gd32vf103_spi_class_init,
};

static void gd32vf103_spi_register_types(void)
{
    type_register_static(&gd32vf103_spi_info);
}
type_init(gd32vf103_spi_register_types);