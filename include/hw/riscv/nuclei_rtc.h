/*
 *  NUCLEI RTC interface
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


#ifndef HW_NUCLEI_RTC_H
#define HW_NUCLEI_RTC_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_RTC "riscv.nuclei.rtc"

#define NUCLEI_RTC(obj) \
    OBJECT_CHECK(NucLeiRTCState, (obj), TYPE_NUCLEI_RTC)

#define NUCLEI_RTC_REG_RTC_INTEN   0x00
#define NUCLEI_RTC_REG_RTC_CTL    0x04
#define NUCLEI_RTC_REG_RTC_PSCH   0x08
#define NUCLEI_RTC_REG_RTC_PSCL   0x0C
#define NUCLEI_RTC_REG_RTC_DIVH   0x10
#define NUCLEI_RTC_REG_RTC_DIVL   0x14
#define NUCLEI_RTC_REG_RTC_CNTH   0x18
#define NUCLEI_RTC_REG_RTC_CNTL   0x1C
#define NUCLEI_RTC_REG_RTC_ALRMH   0x20
#define NUCLEI_RTC_REG_RTC_ALRML   0x24

typedef struct NucLeiRTCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t rtc_inten;
    uint32_t rtc_ctl;
    uint32_t rtc_psch;
    uint32_t rtc_pscl;
    uint32_t rtc_divh;
    uint32_t rtc_divl;
    uint32_t rtc_cnth;
    uint32_t rtc_cntl;
    uint32_t rtc_alrmh;
    uint32_t rtc_alrml;

} NucLeiRTCState;

#endif
