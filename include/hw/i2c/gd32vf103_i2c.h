/*
 *  GD32VF103 I2C interface
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
#ifndef HW_GD32VF103_I2C_H
#define HW_GD32VF103_I2C_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_GD32VF103_I2C "gd32vf103-i2c"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103I2CState, GD32VF103_I2C)

#define I2C_REG_I2C_CTL0   0x00
#define I2C_REG_I2C_CTL1    0x04
#define I2C_REG_I2C_SADDR0   0x08
#define I2C_REG_I2C_SADDR1   0x0C
#define I2C_REG_I2C_DATA   0x10
#define I2C_REG_I2C_STAT0   0x14
#define I2C_REG_I2C_STAT1   0x18
#define I2C_REG_I2C_CKCFG   0x1C
#define I2C_REG_I2C_RT   0x20


typedef struct GD32VF103I2CState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint16_t i2c_ctl0;
    uint16_t i2c_ctl1;
    uint16_t i2c_saddr0;
    uint16_t i2c_saddr1;
    uint16_t i2c_data;
    uint16_t i2c_stat0;
    uint16_t i2c_stat1;
    uint16_t i2c_ckcfg;
    uint16_t i2c_rt;

} GD32VF103I2CState;

#endif