/*
 *  GD32VF103 RTC interface
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
#ifndef HW_GD32VF103_RTC_H
#define HW_GD32VF103_RTC_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_GD32VF103_RTC "gd32vf103-rtc"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103RTCState, GD32VF103_RTC)

#define RTC_REG_RTC_INTEN 0x00
#define RTC_REG_RTC_CTL 0x04
#define RTC_REG_RTC_PSCH 0x08
#define RTC_REG_RTC_PSCL 0x0C
#define RTC_REG_RTC_DIVH 0x10
#define RTC_REG_RTC_DIVL 0x14
#define RTC_REG_RTC_CNTH 0x18
#define RTC_REG_RTC_CNTL 0x1C
#define RTC_REG_RTC_ALRMH 0x20
#define RTC_REG_RTC_ALRML 0x24

typedef struct GD32VF103RTCState
{
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

} GD32VF103RTCState;

#endif