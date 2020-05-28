/*
 *  NUCLEI I2C interface
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


#ifndef HW_NUCLEI_IIC_H
#define HW_NUCLEI_IIC_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_IIC "riscv.nuclei.i2c"

#define NUCLEI_IIC(obj) \
    OBJECT_CHECK(NucLeiIICState, (obj), TYPE_NUCLEI_IIC)

#define NUCLEI_I2C_REG_I2C_CTL0   0x00
#define NUCLEI_I2C_REG_I2C_CTL1    0x04
#define NUCLEI_I2C_REG_I2C_SADDR0   0x08
#define NUCLEI_I2C_REG_I2C_SADDR1   0x0C
#define NUCLEI_I2C_REG_I2C_DATA   0x10
#define NUCLEI_I2C_REG_I2C_STAT0   0x14
#define NUCLEI_I2C_REG_I2C_STAT1   0x18
#define NUCLEI_I2C_REG_I2C_CKCFG   0x1C
#define NUCLEI_I2C_REG_I2C_RT   0x20


typedef struct NucLeiIICState {
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


} NucLeiIICState;

#endif
