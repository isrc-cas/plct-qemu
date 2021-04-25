/*
 *  GD32VF103 USART interface
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
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/sysbus.h"
#include "chardev/char.h"
#include "chardev/char-fe.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/char/gd32vf103_usart.h"

/*
 * Not yet implemented:
 *
 * Transmit FIFO using "qemu/fifo8.h"
 */

static void update_irq(GD32VF103USARTState *s)
{
    static int cond = 0;
    int new_cond = 0;
    s->usart_stat |= USART_TBE;
    if (s->rx_fifo_len)
        s->usart_stat |= USART_RBNE;
    else
        s->usart_stat &= ~USART_RBNE;
    if (((s->usart_ctl0 & USART_TCIE) && (s->usart_ctl0 & USART_TEN)) ||
        ((s->usart_ctl0 & USART_RBNEIE) && (s->usart_ctl0 & USART_REN) && s->rx_fifo_len))
    {
        new_cond = 1;
    }
    if (!cond && new_cond)
    {
        cond = new_cond;
        qemu_irq_raise(s->irq);
    }
    else if (cond && !new_cond)
    {
        cond = new_cond;
        qemu_irq_lower(s->irq);
    }
}

static uint64_t
uart_read(void *opaque, hwaddr offset, unsigned int size)
{
    GD32VF103USARTState *s = opaque;
    uint64_t value = 0;
    uint8_t fifo_val;
    switch (offset)
    {
    case USART_STAT:
        value = s->usart_stat;
        break;
    case USART_DATA:
        if (s->rx_fifo_len)
        {
            fifo_val = s->rx_fifo[0];
            memmove(s->rx_fifo, s->rx_fifo + 1, s->rx_fifo_len - 1);
            s->rx_fifo_len--;
            qemu_chr_fe_accept_input(&s->chr);
            update_irq(s);
            return fifo_val;
        }
        value = 0x0;
        break;

    case USART_BAUD:
        value = s->usart_baud;
        break;
    case USART_CTL0:
        value = s->usart_ctl0;
        break;
    case USART_CTL1:
        value = s->usart_ctl1;
        break;
    case USART_CTL2:
        value = s->usart_ctl2;
        break;
    case USART_GP:
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
    GD32VF103USARTState *s = opaque;
    unsigned char ch = value;
    switch (offset)
    {
    case USART_STAT:
        s->usart_stat = value;
        break;
    case USART_DATA:
        qemu_chr_fe_write(&s->chr, &ch, 1);
        update_irq(s);
        break;
    case USART_BAUD:
        s->usart_baud = value;
        break;
    case USART_CTL0:
        s->usart_ctl0 = value;
        break;
    case USART_CTL1:
        s->usart_ctl1 = value;
        update_irq(s);
        break;
    case USART_CTL2:
        s->usart_ctl2 = value;
        break;
    case USART_GP:
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
        .max_access_size = 4}};

static void uart_rx(void *opaque, const uint8_t *buf, int size)
{
    GD32VF103USARTState *s = opaque;

    /* Got a byte.  */
    if (s->rx_fifo_len >= sizeof(s->rx_fifo))
    {
        printf("WARNING: UART dropped char.\n");
        return;
    }
    s->rx_fifo[s->rx_fifo_len++] = *buf;

    update_irq(s);
}

static int uart_can_rx(void *opaque)
{
    GD32VF103USARTState *s = opaque;
    return s->rx_fifo_len < sizeof(s->rx_fifo);
}

static void uart_event(void *opaque, QEMUChrEvent event)
{
}

static int uart_be_change(void *opaque)
{
    GD32VF103USARTState *s = opaque;

    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx, uart_event,
                             uart_be_change, s, NULL, true);

    return 0;
}

/*
 * Create UART device.
 */
GD32VF103USARTState *gd32vf103_usart_create(MemoryRegion *address_space, hwaddr base, uint64_t size,
                                            Chardev *chr, qemu_irq irq)
{

    GD32VF103USARTState *s = g_malloc0(sizeof(GD32VF103USARTState));
    s->irq = irq;
    qemu_chr_fe_init(&s->chr, chr, &error_abort);
    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx, uart_event,
                             uart_be_change, s, NULL, true);
    memory_region_init_io(&s->mmio, NULL, &uart_ops, s,
                          TYPE_GD32VF103_USART, size);
    memory_region_add_subregion(address_space, base, &s->mmio);

    s->usart_stat = 0xC0;
    s->usart_baud = 0x0;
    s->usart_ctl0 = 0x0;
    s->usart_ctl1 = 0x0;
    s->usart_ctl2 = 0x0;
    s->usart_gp = 0x0;

    return s;
}