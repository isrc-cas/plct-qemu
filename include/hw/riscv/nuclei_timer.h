/*
 *  NUCLEI TIMER (Timer Unit) interface
 *
 * Copyright (c) 2020 Nuclei, Inc.
 *
 * This provides a NUCLEI RISC-V TIMER device
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HW_NUCLEI_TIMER_H
#define HW_NUCLEI_TIMER_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_TIMER "riscv.nuclei.timer"

#define NUCLEI_TIMER(obj) \
    OBJECT_CHECK(NucLeiTIMERState, (obj), TYPE_NUCLEI_TIMER)


#define NUCLEI_TIMER_REG_MTIMELO   0x0000
#define NUCLEI_TIMER_REG_MTIMEHI    0x0004
#define NUCLEI_TIMER_REG_MTIMECMPLO   0x0008
#define NUCLEI_TIMER_REG_MTIMECMPHI   0x000C
#define NUCLEI_TIMER_REG_MSFTRST   0xFF0
#define NUCLEI_TIMER_REG_MSTOP   0xFF8
#define NUCLEI_TIMER_REG_MSIP   0xFFC

typedef struct NucLeiTIMERState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    qemu_irq timer_irq;
    qemu_irq soft_irq;

    uint32_t mtime_lo;
    uint32_t mtime_hi;
    uint32_t mtimecmp_lo;
    uint32_t mtimecmp_hi;
    uint32_t mstop;
    uint32_t msip;

} NucLeiTIMERState;

#endif
