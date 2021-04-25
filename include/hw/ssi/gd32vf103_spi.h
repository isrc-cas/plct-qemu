/*
 *  GD32VF103 SPI interface
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
#ifndef HW_GD32VF103_SPI_H
#define HW_GD32VF103_SPI_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_GD32VF103_SPI "gd32vf103-spi"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103SPIState, GD32VF103_SPI)

#define SPI_REG_SPI_CTL0 0x00
#define SPI_REG_SPI_CTL1 0x04
#define SPI_REG_SPI_STAT 0x08
#define SPI_REG_SPI_DATA 0x0C
#define SPI_REG_SPI_CRCPOLY 0x10
#define SPI_REG_SPI_RCRC 0x14
#define SPI_REG_SPI_TCRC 0x18
#define SPI_REG_SPI_I2SCTL 0x1C
#define SPI_REG_SPI_I2SPSC 0x20

typedef struct GD32VF103SPIState
{
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t spi_ctl0;
    uint32_t spi_ctl1;
    uint32_t spi_stat;
    uint32_t spi_data;
    uint32_t spi_crcpoly;
    uint32_t spi_rcrc;
    uint32_t spi_tcrc;
    uint32_t spi_i2sctl;
    uint32_t spi_i2spsc;

} GD32VF103SPIState;

#endif