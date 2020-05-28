/*
 *  NUCLEI ADC interface
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


#ifndef HW_NUCLEI_ADC_H
#define HW_NUCLEI_ADC_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_ADC "riscv.nuclei.adc"

#define NUCLEI_ADC(obj) \
    OBJECT_CHECK(NucLeiADCState, (obj), TYPE_NUCLEI_ADC)

#define NUCLEI_ADC_REG_ADC_STAT   0x00
#define NUCLEI_ADC_REG_ADC_CTL0    0x04
#define NUCLEI_ADC_REG_ADC_CTL1   0x08
#define NUCLEI_ADC_REG_ADC_SAMPT0   0x0C
#define NUCLEI_ADC_REG_ADC_SAMPT1   0x10
#define NUCLEI_ADC_REG_ADC_IOFFx   0x14
#define NUCLEI_ADC_REG_ADC_WDHT   0x24
#define NUCLEI_ADC_REG_ADC_WDLT   0x28
#define NUCLEI_ADC_REG_ADC_RSQ0   0x2C
#define NUCLEI_ADC_REG_ADC_RSQ1   0x30
#define NUCLEI_ADC_REG_ADC_RSQ2   0x34
#define NUCLEI_ADC_REG_ADC_ISQ    0x38
#define NUCLEI_ADC_REG_ADC_IDATAx   0x3C
#define NUCLEI_ADC_REG_ADC_RDATA   0x4C
#define NUCLEI_ADC_REG_ADC_OVSAMPCTL   0x80

#define NUCLEI_ADC_MAX  4

typedef struct NucLeiADCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t adc_stat;
    uint32_t adc_ctl0;
    uint32_t adc_ctl1;
    uint32_t adc_sampt0;
    uint32_t adc_sampt1;
    uint32_t adc_ioff[NUCLEI_ADC_MAX];
    uint32_t adc_wdht;
    uint32_t adc_wdlt;
    uint32_t adc_rsq0;
    uint32_t adc_rsq1;
    uint32_t adc_rsq2;
    uint32_t adc_isq;
    uint32_t adc_idata[NUCLEI_ADC_MAX];
    uint32_t adc_rdata;
    uint32_t adc_ovsampctl;

} NucLeiADCState;

#endif
