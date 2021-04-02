/*
 *  GD32VF103 USART interface
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
#ifndef HW_GD32VF103_USART_H
#define HW_GD32VF103_USART_H

#include "chardev/char-fe.h"
#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_GD32VF103_USART "gd32vf103-usart"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103USARTState, GD32VF103_USART)

#define USART_STAT      0x00
#define USART_DATA     0x04
#define USART_BAUD    0x08
#define USART_CTL0      0x0C
#define USART_CTL1      0x10
#define USART_CTL2      0x14
#define USART_GP          0x18

#define  USART_PERR     (1 << 8)
#define  USART_TBE     (1 << 7)
#define  USART_TC     (1 << 6)
#define  USART_RBNE     (1 << 5)

#define  USART_PERRIE     (1 << 8)
#define  USART_TBEIE     (1 << 7)
#define  USART_TCIE     (1 << 6)
#define  USART_RBNEIE     (1 << 5)
#define  USART_IDLEIE     (1 << 4)
#define  USART_TEN     (1 << 3)
#define  USART_REN     (1 << 2)

typedef struct GD32VF103USARTState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    qemu_irq irq;
    MemoryRegion mmio;
    CharBackend chr;

    uint8_t rx_fifo[8];
    unsigned int rx_fifo_len;
    unsigned int rx_fifo_wpos;

    uint32_t usart_stat;
    uint32_t usart_data;
    uint32_t usart_baud;
    uint32_t usart_ctl0;
    uint32_t usart_ctl1;
    uint32_t usart_ctl2;
    uint32_t usart_gp;
} GD32VF103USARTState;

GD32VF103USARTState *gd32vf103_usart_create(MemoryRegion *address_space, hwaddr base,uint64_t size,
    Chardev *chr, qemu_irq irq);

#endif
