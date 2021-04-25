/*
 *  GD32VF103 GPIO interface
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
#include "qemu/log.h"
#include "hw/irq.h"
#include "hw/gpio/gd32vf103_gpio.h"
#include "migration/vmstate.h"
#include "trace.h"

static uint64_t gd32vf103_gpio_read(void *opaque, hwaddr offset, unsigned int size)
{
    GD32VF103GPIOState *s = GD32VF103_GPIO(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case GPIOx_CTL0:
        value = s->gpiox_ctl0;
        break;
    case GPIOx_CTL1:
        value = s->gpiox_ctl1;
        break;
    case GPIOx_ISTAT:
        value = s->gpiox_istat;
        break;
    case GPIOx_OCTL:
        value = s->gpiox_octl;
        break;
    case GPIOx_BOP:
        value = s->gpiox_bop;
        break;
    case GPIOx_BC:
        value = s->gpiox_bc;
        break;
    case GPIOx_LOCK:
        value = s->gpiox_lock;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: bad read offset 0x%" HWADDR_PRIx "\n",
                      __func__, offset);
    }
    return value;
}

static void gd32vf103_gpio_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned int size)
{
    GD32VF103GPIOState *s = GD32VF103_GPIO(opaque);

    switch (offset)
    {
    case GPIOx_CTL0:
        s->gpiox_ctl0 = value;
        break;
    case GPIOx_CTL1:
        s->gpiox_ctl1 = value;
        break;
    case GPIOx_ISTAT:
        s->gpiox_istat = value;
        break;
    case GPIOx_OCTL:
        s->gpiox_octl = value;
        break;
    case GPIOx_BOP:
        s->gpiox_bop = value;
        break;
    case GPIOx_BC:
        s->gpiox_bc = value;
        break;
    case GPIOx_LOCK:
        s->gpiox_lock = value;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: bad write offset 0x%" HWADDR_PRIx "\n",
                      __func__, offset);
    }
}

static const MemoryRegionOps gpio_ops = {
    .read = gd32vf103_gpio_read,
    .write = gd32vf103_gpio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 4,
    .impl.max_access_size = 4,
};

static void gd32vf103_gpio_reset(DeviceState *dev)
{
    GD32VF103GPIOState *s = GD32VF103_GPIO(dev);

    s->gpiox_ctl0 = 0x44444444;
    s->gpiox_ctl1 = 0x44444444;
    s->gpiox_istat = 0x0;
    s->gpiox_octl = 0x0;
    s->gpiox_bop = 0x0;
    s->gpiox_bc = 0x0;
    s->gpiox_lock = 0x0;
}

static void gd32vf103_gpio_init(Object *obj)
{
    GD32VF103GPIOState *s = GD32VF103_GPIO(obj);

    memory_region_init_io(&s->mmio, obj, &gpio_ops, s,
                          TYPE_GD32VF103_GPIO, GPIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
}

static void gd32vf103_gpio_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = gd32vf103_gpio_reset;
    dc->desc = "GD32VF103 GPIO";
}

static const TypeInfo gd32vf103_gpio_info = {
    .name = TYPE_GD32VF103_GPIO,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103GPIOState),
    .instance_init = gd32vf103_gpio_init,
    .class_init = gd32vf103_gpio_class_init};

static void gd32vf103_gpio_register_types(void)
{
    type_register_static(&gd32vf103_gpio_info);
}

type_init(gd32vf103_gpio_register_types)