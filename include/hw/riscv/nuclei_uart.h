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


#ifndef HW_NUCLEI_UART_H
#define HW_NUCLEI_UART_H

#include "chardev/char-fe.h"
#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_UART "riscv.nuclei.uart"

#define NUCLEI_UART(obj) \
    OBJECT_CHECK(NucLeiUARTState, (obj), TYPE_NUCLEI_UART)

#define NUCLEI_UART_REG_TXDATA     0x000
#define NUCLEI_UART_REG_RXDATA     0x004
#define NUCLEI_UART_REG_TXCTRL     0x008
#define NUCLEI_UART_REG_RXCTRL     0x00C
#define NUCLEI_UART_REG_IE         0x010
#define NUCLEI_UART_REG_IP         0x014
#define NUCLEI_UART_REG_DIV        0x018

#define NUCLEI_UART_GET_TXCNT(txctrl)   (txctrl & 0x1)
#define NUCLEI_UART_GET_RXCNT(rxctrl)   (rxctrl & 0x1)

enum {
    NUCLEI_UART_IE_TXWM       = 1, /* Transmit watermark interrupt enable */
    NUCLEI_UART_IE_RXWM       = 2  /* Receive watermark interrupt enable */
};

enum {
    NUCLEI_UART_IP_TXWM       = 1, /* Transmit watermark interrupt pending */
    NUCLEI_UART_IP_RXWM       = 2  /* Receive watermark interrupt pending */
};

typedef struct NucLeiUARTState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    qemu_irq irq;
    MemoryRegion mmio;
    CharBackend chr;
    uint8_t rx_fifo[8];
    unsigned int rx_fifo_len;

    uint32_t txdata;
    uint32_t rxdata;
    uint32_t txctrl;
    uint32_t rxctrl;
    uint32_t ie;
    uint32_t ip;
    uint32_t div;
} NucLeiUARTState;

NucLeiUARTState *nuclei_uart_create(MemoryRegion *address_space, hwaddr base,uint64_t size,
    Chardev *chr, qemu_irq irq);

#endif
