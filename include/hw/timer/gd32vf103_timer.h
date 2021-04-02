/*
 *  GD32VF103 TIMER interface
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
#ifndef HW_GD32VF103_TIMER_H
#define HW_GD32VF103_TIMER_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_GD32VF103_TIMER "gd32vf103-timer"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103TimerState, GD32VF103_TIMER)

#define TIMER_REG_TIMERx_CTL0   0x00
#define TIMER_REG_TIMERx_CTL1    0x04
#define TIMER_REG_TIMERx_SMCFG   0x08
#define TIMER_REG_TIMERx_DMAINTEN   0x0C
#define TIMER_REG_TIMERx_INTF  0x10
#define TIMER_REG_TIMERx_SWEVG   0x14
#define TIMER_REG_TIMERx_CHCTL0   0x18
#define TIMER_REG_TIMERx_CHCTL1   0x1C
#define TIMER_REG_TIMERx_CHCTL2   0x20
#define TIMER_REG_TIMERx_CNT   0x24
#define TIMER_REG_TIMERx_PSC   0x28
#define TIMER_REG_TIMERx_CAR   0x2C
#define TIMER_REG_TIMERx_CREP   0x30
#define TIMER_REG_TIMERx_CH0CV   0x34
#define TIMER_REG_TIMERx_CH1CV   0x38
#define TIMER_REG_TIMERx_CH2CV   0x3C
#define TIMER_REG_TIMERx_CH3CV   0x40
#define TIMER_REG_TIMERx_DMACFG   0x48
#define TIMER_REG_TIMERx_DMATB   0x4C

typedef struct GD32VF103TimerState {
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

} GD32VF103TimerState;

#endif