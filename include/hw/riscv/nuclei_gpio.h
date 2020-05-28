/*
 * NUCLEI GPIO  interface
 *
 * Copyright (c) 2020 Nuclei, Inc.
 *
 * This provides a NUCLEI RISC-V GPIO device
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

#ifndef NUCLEI_GPIO_H
#define NUCLEI_GPIO_H

#include "hw/sysbus.h"
#define TYPE_NUCLEI_GPIO "nuclei.soc.gpio"
#define NUCLEI_GPIO(obj) OBJECT_CHECK(NucLeiGPIOState, (obj), TYPE_NUCLEI_GPIO)

#define NUCLEI_GPIO_PINS 16

#define NUCLEI_GPIO_SIZE 0x400

#define NUCLEI_GPIOx_CTL0      0x00
#define NUCLEI_GPIOx_CTL1      0x04
#define NUCLEI_GPIOx_ISTAT     0x08
#define NUCLEI_GPIOx_OCTL     0x0C
#define NUCLEI_GPIOx_BOP       0x10
#define NUCLEI_GPIOx_BC          0x14
#define NUCLEI_GPIOx_LOCK     0x18

typedef struct NucLeiGPIOState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;

    // qemu_irq irq[NUCLEI_GPIO_PINS];
    // qemu_irq output[NUCLEI_GPIO_PINS];

    uint32_t gpiox_ctl0;
    uint32_t gpiox_ctl1;
    uint32_t gpiox_istat;
    uint32_t gpiox_octl;
    uint32_t gpiox_bop;
    uint32_t gpiox_bc;
    uint32_t gpiox_lock;

} NucLeiGPIOState;

#endif
