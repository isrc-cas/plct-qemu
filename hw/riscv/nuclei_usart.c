/*
 * QEMU model of the USART on the NUCLEI SOCs.
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


#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/sysbus.h"
#include "chardev/char.h"
#include "chardev/char-fe.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/riscv/nuclei_usart.h"

/*
 * Not yet implemented:
 *
 * Transmit FIFO using "qemu/fifo8.h"
 */

static void update_irq(NucLeiUSARTState *s)
{
     static int cond = 0;
     int new_cond = 0;
     s->usart_stat |=  NUCLEI_USART_TBE;
     if (s->rx_fifo_len)
    	 s->usart_stat |= NUCLEI_USART_RBNE;
     else
    	 s->usart_stat &= ~NUCLEI_USART_RBNE;
     if (((s->usart_ctl0 & NUCLEI_USART_TCIE) && (s->usart_ctl0 & NUCLEI_USART_TEN)) ||
         ((s->usart_ctl0 & NUCLEI_USART_RBNEIE) && (s->usart_ctl0 & NUCLEI_USART_REN) && s->rx_fifo_len)) {
         new_cond = 1;
     }
     if (!cond && new_cond) {
    	 cond = new_cond;
         qemu_irq_raise(s->irq);
     } else if(cond && !new_cond) {
    	 cond = new_cond;
         qemu_irq_lower(s->irq);
     }
}

static uint64_t
uart_read(void *opaque, hwaddr offset, unsigned int size)
{
    NucLeiUSARTState *s = opaque;
    uint64_t value = 0;
    uint8_t fifo_val;
    switch (offset)
    {
    case NUCLEI_USART_STAT:
        value = s->usart_stat;
        break;
    case NUCLEI_USART_DATA:
        if (s->rx_fifo_len) {
            fifo_val = s->rx_fifo[0];
            memmove(s->rx_fifo, s->rx_fifo + 1, s->rx_fifo_len - 1);
            s->rx_fifo_len--;
            qemu_chr_fe_accept_input(&s->chr);
            update_irq(s);
            return fifo_val;
        }
        value = 0x0;
        break;

    case NUCLEI_USART_BAUD:
        value = s->usart_baud;
        break;
    case NUCLEI_USART_CTL0:
        value = s->usart_ctl0;
        break;
    case NUCLEI_USART_CTL1:
        value = s->usart_ctl1;
        break;
    case NUCLEI_USART_CTL2:
        value = s->usart_ctl2;
        break;
    case NUCLEI_USART_GP:
        value = s->usart_gp;
        break;
    default:
        break;
    }
    return value;
}

static void
uart_write(void *opaque, hwaddr offset,
           uint64_t value, unsigned int size)
{
    NucLeiUSARTState *s = opaque;
    unsigned char ch = value;
    switch (offset)
    {
    case  NUCLEI_USART_STAT:
        s->usart_stat = value;
        break;
    case NUCLEI_USART_DATA:
        qemu_chr_fe_write(&s->chr, &ch, 1);
        update_irq(s);
        break;
    case NUCLEI_USART_BAUD:
        s->usart_baud = value;
        break;
    case NUCLEI_USART_CTL0:
        s->usart_ctl0 = value;
        break;
    case NUCLEI_USART_CTL1:
        s->usart_ctl1  = value;
        update_irq(s);
        break;
    case NUCLEI_USART_CTL2:
        s->usart_ctl2 = value;
        break;
    case NUCLEI_USART_GP:
        s->usart_gp = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps uart_ops = {
    .read = uart_read,
    .write = uart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4
    }
};

static void uart_rx(void *opaque, const uint8_t *buf, int size)
{
    NucLeiUSARTState *s = opaque;

    /* Got a byte.  */
    if (s->rx_fifo_len >= sizeof(s->rx_fifo)) {
        printf("WARNING: UART dropped char.\n");
        return;
    }
    s->rx_fifo[s->rx_fifo_len++] = *buf;

    update_irq(s);
}

static int uart_can_rx(void *opaque)
{
    NucLeiUSARTState *s = opaque;
    return s->rx_fifo_len < sizeof(s->rx_fifo);
}

static void uart_event(void *opaque, QEMUChrEvent event)
{
}

static int uart_be_change(void *opaque)
{
    NucLeiUSARTState *s = opaque;

    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx, uart_event,
        uart_be_change, s, NULL, true);

    return 0;
}

/*
 * Create UART device.
 */
NucLeiUSARTState *nuclei_usart_create(MemoryRegion *address_space, hwaddr base,  uint64_t size,
    Chardev *chr, qemu_irq irq)
{

    NucLeiUSARTState *s = g_malloc0(sizeof(NucLeiUSARTState));
    s->irq = irq;
    qemu_chr_fe_init(&s->chr, chr, &error_abort);
    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx, uart_event,
        uart_be_change, s, NULL, true);
    memory_region_init_io(&s->mmio, NULL, &uart_ops, s,
                          TYPE_NUCLEI_USART, size);
    memory_region_add_subregion(address_space, base, &s->mmio);

    s->usart_stat = 0xC0;
    s->usart_baud = 0x0;
    s->usart_ctl0 = 0x0;
    s->usart_ctl1 = 0x0;
    s->usart_ctl2 = 0x0;
    s->usart_gp = 0x0;

    return s;
}