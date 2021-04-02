/*
 *  GD32VF103 GPIO interface
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
#ifndef HW_GD32VF103_GPIO_H
#define HW_GD32VF103_GPIO_H

#include "hw/sysbus.h"

#define TYPE_GD32VF103_GPIO "gd32vf103-gpio"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103GPIOState, GD32VF103_GPIO)


#define GPIO_PINS 16
#define GPIO_SIZE 0x400

#define GPIOx_CTL0      0x00
#define GPIOx_CTL1      0x04
#define GPIOx_ISTAT     0x08
#define GPIOx_OCTL     0x0C
#define GPIOx_BOP       0x10
#define GPIOx_BC          0x14
#define GPIOx_LOCK     0x18

typedef struct GD32VF103GPIOState {
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

} GD32VF103GPIOState;

#endif