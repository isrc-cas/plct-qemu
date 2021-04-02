/*
 *  GD32VF103 ADC interface
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

#ifndef HW_GD32VF103_ADC_H
#define HW_GD32VF103_ADC_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_GD32VF103_ADC "gd32vf103-adc"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103ADCState, GD32VF103_ADC)

#define ADC_REG_ADC_STAT   0x00
#define ADC_REG_ADC_CTL0    0x04
#define ADC_REG_ADC_CTL1   0x08
#define ADC_REG_ADC_SAMPT0   0x0C
#define ADC_REG_ADC_SAMPT1   0x10
#define ADC_REG_ADC_IOFFx   0x14
#define ADC_REG_ADC_WDHT   0x24
#define ADC_REG_ADC_WDLT   0x28
#define ADC_REG_ADC_RSQ0   0x2C
#define ADC_REG_ADC_RSQ1   0x30
#define ADC_REG_ADC_RSQ2   0x34
#define ADC_REG_ADC_ISQ    0x38
#define ADC_REG_ADC_IDATAx   0x3C
#define ADC_REG_ADC_RDATA   0x4C
#define ADC_REG_ADC_OVSAMPCTL   0x80

#define ADC_MAX  4

typedef struct GD32VF103ADCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t adc_stat;
    uint32_t adc_ctl0;
    uint32_t adc_ctl1;
    uint32_t adc_sampt0;
    uint32_t adc_sampt1;
    uint32_t adc_ioff[ADC_MAX];
    uint32_t adc_wdht;
    uint32_t adc_wdlt;
    uint32_t adc_rsq0;
    uint32_t adc_rsq1;
    uint32_t adc_rsq2;
    uint32_t adc_isq;
    uint32_t adc_idata[ADC_MAX];
    uint32_t adc_rdata;
    uint32_t adc_ovsampctl;

} GD32VF103ADCState;

#endif