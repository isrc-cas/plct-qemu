/*
 * NUCLEI UART interface
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


#ifndef HW_NNUCLEI_USART_H
#define HW_NNUCLEI_USART_H

#include "chardev/char-fe.h"
#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_USART "riscv.nuclei.usart"

#define NUCLEI_USART(obj) \
    OBJECT_CHECK(NucLeiUSARTState, (obj), TYPE_NUCLEI_USART)

#define NUCLEI_USART_STAT      0x00
#define NUCLEI_USART_DATA     0x04
#define NUCLEI_USART_BAUD    0x08
#define NUCLEI_USART_CTL0      0x0C
#define NUCLEI_USART_CTL1      0x10
#define NUCLEI_USART_CTL2      0x14
#define NUCLEI_USART_GP          0x18

#define   NUCLEI_USART_PERR     (1 << 8)
#define   NUCLEI_USART_TBE     (1 << 7)
#define   NUCLEI_USART_TC     (1 << 6)
#define   NUCLEI_USART_RBNE     (1 << 5)

#define   NUCLEI_USART_PERRIE     (1 << 8)
#define   NUCLEI_USART_TBEIE     (1 << 7)
#define   NUCLEI_USART_TCIE     (1 << 6)
#define   NUCLEI_USART_RBNEIE     (1 << 5)
#define   NUCLEI_USART_IDLEIE     (1 << 4)
#define   NUCLEI_USART_TEN     (1 << 3)
#define   NUCLEI_USART_REN     (1 << 2)

typedef struct NucLeiUSARTState {
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
} NucLeiUSARTState;

NucLeiUSARTState *nuclei_usart_create(MemoryRegion *address_space, hwaddr base,uint64_t size,
    Chardev *chr, qemu_irq irq);

#endif