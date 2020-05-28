/*
 * QEMU model of the UART on the NUCLEI SOCs.
 *
 * Copyright (c) 2020 NUCLEI Inc
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

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/sysbus.h"
#include "chardev/char.h"
#include "chardev/char-fe.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/riscv/nuclei_uart.h"

/*
 * Not yet implemented:
 *
 * Transmit FIFO using "qemu/fifo8.h"
 */

static void update_irq(NucLeiUARTState *s)
{
    int cond = 0;
    if ((s->ie & NUCLEI_UART_IE_TXWM) ||
        ((s->ie & NUCLEI_UART_IE_RXWM) && s->rx_fifo_len)) {
        cond = 1;
    }
    if (cond) {
        qemu_irq_raise(s->irq);
    } else {
        qemu_irq_lower(s->irq);
    }
}

static uint64_t
uart_read(void *opaque, hwaddr offset, unsigned int size)
{
    NucLeiUARTState *s = opaque;
    uint64_t value = 0;
    uint8_t fifo_val;

    switch (offset)
    {
    case NUCLEI_UART_REG_TXDATA:
        value = s->rxdata;
        break;
    case NUCLEI_UART_REG_RXDATA:
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
    case NUCLEI_UART_REG_TXCTRL:
        value = s->txctrl;
        break;
    case NUCLEI_UART_REG_RXCTRL:
        value = s->rxctrl;
        break;
    case NUCLEI_UART_REG_IE:
        value = s->ie;
        break;
    case NUCLEI_UART_REG_IP:
        value = s->ip;
        break;
    case NUCLEI_UART_REG_DIV:
        value = s->div;
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
    NucLeiUARTState *s = opaque;
    unsigned char ch = value;

    switch (offset)
    {
    case NUCLEI_UART_REG_TXDATA:
        qemu_chr_fe_write(&s->chr, &ch, 1);
        update_irq(s);
        break;
    case NUCLEI_UART_REG_TXCTRL:
        s->txctrl = value;
        break;
    case NUCLEI_UART_REG_RXCTRL:
        s->rxctrl = value;
        break;
    case NUCLEI_UART_REG_IE:
        s->ie  = value;
        update_irq(s);
        break;
    case NUCLEI_UART_REG_IP:
        s->ip = value;
        break;
    case NUCLEI_UART_REG_DIV:
        s->div = value;
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
    NucLeiUARTState *s = opaque;

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
    NucLeiUARTState *s = opaque;
    return s->rx_fifo_len < sizeof(s->rx_fifo);
}

static void uart_event(void *opaque, QEMUChrEvent event)
{
}

static int uart_be_change(void *opaque)
{
    NucLeiUARTState *s = opaque;

    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx, uart_event,
        uart_be_change, s, NULL, true);

    return 0;
}

/*
 * Create UART device.
 */
NucLeiUARTState *nuclei_uart_create(MemoryRegion *address_space, hwaddr base,  uint64_t size,
    Chardev *chr, qemu_irq irq)
{
    NucLeiUARTState *s = g_malloc0(sizeof(NucLeiUARTState));
    s->irq = irq;
    qemu_chr_fe_init(&s->chr, chr, &error_abort);
    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx, uart_event,
        uart_be_change, s, NULL, true);
    memory_region_init_io(&s->mmio, NULL, &uart_ops, s,
                          TYPE_NUCLEI_UART, size);
    memory_region_add_subregion(address_space, base, &s->mmio);
    return s;
}
