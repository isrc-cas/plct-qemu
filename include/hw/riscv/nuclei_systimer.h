/*
 *  NUCLEI TIMER (Timer Unit) interface
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


#ifndef HW_NUCLEI_SYSTIMER_H
#define HW_NUCLEI_SYSTIMER_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_SYSTIMER "riscv.nuclei.systimer"

#define NUCLEI_SYSTIMER(obj) \
    OBJECT_CHECK(NucLeiSYSTIMERState, (obj), TYPE_NUCLEI_SYSTIMER)

#define NUCLEI_SYSTIMER_REG_MTIMELO   0x0000
#define NUCLEI_SYSTIMER_REG_MTIMEHI    0x0004
#define NUCLEI_SYSTIMER_REG_MTIMECMPLO   0x0008
#define NUCLEI_SYSTIMER_REG_MTIMECMPHI   0x000C
#define NUCLEI_SYSTIMER_REG_MSFTRST   0xFF0
#define NUCLEI_SYSTIMER_REG_MSTOP   0xFF8
#define NUCLEI_SYSTIMER_REG_MSIP   0xFFC

typedef struct NucLeiSYSTIMERState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    qemu_irq *timer_irq;
    qemu_irq *soft_irq;

    uint32_t mtime_lo;
    uint32_t mtime_hi;
    uint32_t mtimecmp_lo;
    uint32_t mtimecmp_hi;
    uint32_t mstop;
    uint32_t msip;

    uint64_t timebase_freq;

} NucLeiSYSTIMERState;

#define  NUCLEI_GD32_TIMEBASE_FREQ  (108000000*2)
#define  NUCLEI_HBIRD_TIMEBASE_FREQ  (10000000)

#endif