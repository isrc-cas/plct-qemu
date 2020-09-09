/*
 *  NUCLEI Timer interface
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


#ifndef HW_NUCLEI_TIMER_H
#define HW_NUCLEI_TIMER_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_TIMER "riscv.nuclei.timer"

#define NUCLEI_TIMER(obj) \
    OBJECT_CHECK(NucLeiTIMERState, (obj), TYPE_NUCLEI_TIMER)

#define NUCLEI_TIMER_REG_TIMERx_CTL0   0x00
#define NUCLEI_TIMER_REG_TIMERx_CTL1    0x04
#define NUCLEI_TIMER_REG_TIMERx_SMCFG   0x08
#define NUCLEI_TIMER_REG_TIMERx_DMAINTEN   0x0C
#define NUCLEI_TIMER_REG_TIMERx_INTF  0x10
#define NUCLEI_TIMER_REG_TIMERx_SWEVG   0x14
#define NUCLEI_TIMER_REG_TIMERx_CHCTL0   0x18
#define NUCLEI_TIMER_REG_TIMERx_CHCTL1   0x1C
#define NUCLEI_TIMER_REG_TIMERx_CHCTL2   0x20
#define NUCLEI_TIMER_REG_TIMERx_CNT   0x24
#define NUCLEI_TIMER_REG_TIMERx_PSC   0x28
#define NUCLEI_TIMER_REG_TIMERx_CAR   0x2C
#define NUCLEI_TIMER_REG_TIMERx_CREP   0x30
#define NUCLEI_TIMER_REG_TIMERx_CH0CV   0x34
#define NUCLEI_TIMER_REG_TIMERx_CH1CV   0x38
#define NUCLEI_TIMER_REG_TIMERx_CH2CV   0x3C
#define NUCLEI_TIMER_REG_TIMERx_CH3CV   0x40
#define NUCLEI_TIMER_REG_TIMERx_DMACFG   0x48
#define NUCLEI_TIMER_REG_TIMERx_DMATB   0x4C

typedef struct NucLeiTIMERState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint16_t timerx_ctl0;
    uint16_t timerx_ctl1;
    uint16_t timerx_smcfg;
    uint16_t timerx_dmainten;
    uint16_t timerx_int;
    uint16_t timerx_swevg;
    uint16_t timerx_chctl0;
    uint16_t timerx_chctl1;
    uint16_t timerx_chctl2;
    uint16_t timerx_cnt;
    uint16_t timerx_psc;
    uint16_t timerx_car;
    uint16_t timerx_crep;
    uint16_t timerx_ch0cv;
    uint16_t timerx_ch1cv;
    uint16_t timerx_ch2cv;
    uint16_t timerx_ch3cv;
    uint16_t timerx_dmacfg;
    uint16_t timerx_dmatb;

} NucLeiTIMERState;

#endif
