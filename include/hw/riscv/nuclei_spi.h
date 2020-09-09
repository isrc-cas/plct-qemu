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


#ifndef HW_NUCLEI_SPI_H
#define HW_NUCLEI_SPI_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_SPI "riscv.nuclei.spi"

#define NUCLEI_SPI(obj) \
    OBJECT_CHECK(NucLeiSPIState, (obj), TYPE_NUCLEI_SPI)

#define NUCLEI_SPI_REG_SPI_CTL0   0x00
#define NUCLEI_SPI_REG_SPI_CTL1   0x04
#define NUCLEI_SPI_REG_SPI_STAT   0x08
#define NUCLEI_SPI_REG_SPI_DATA   0x0C
#define NUCLEI_SPI_REG_SPI_CRCPOLY   0x10
#define NUCLEI_SPI_REG_SPI_RCRC   0x14
#define NUCLEI_SPI_REG_SPI_TCRC   0x18
#define NUCLEI_SPI_REG_SPI_I2SCTL   0x1C
#define NUCLEI_SPI_REG_SPI_I2SPSC   0x20

typedef struct NucLeiSPIState {
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

} NucLeiSPIState;

#endif
