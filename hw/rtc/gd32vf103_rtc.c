/*
 *  GD32VF103 RTC interface
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
#include "qemu/timer.h"
#include "hw/rtc/gd32vf103_rtc.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void gd32vf103_rtc_reset(DeviceState *dev)
{
    GD32VF103RTCState *s = GD32VF103_RTC(dev);
    s->rtc_inten = 0x0;
    s->rtc_ctl = 0x0;
    s->rtc_psch = 0x0;
    s->rtc_pscl = 0x0;
    s->rtc_divh = 0x0;
    s->rtc_divl = 0x0;
    s->rtc_cnth = 0x0;
    s->rtc_cntl = 0x0;
    s->rtc_alrmh = 0x0;
    s->rtc_alrml = 0x0;
}

static uint64_t gd32vf103_rtc_read(void *opaque, hwaddr offset,
                                   unsigned size)
{
    GD32VF103RTCState *s = GD32VF103_RTC(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case RTC_REG_RTC_INTEN:
        value = s->rtc_inten;
        break;
    case RTC_REG_RTC_CTL:
        value = s->rtc_ctl;
        break;
    case RTC_REG_RTC_PSCH:
        value = s->rtc_psch;
        break;
    case RTC_REG_RTC_PSCL:
        value = s->rtc_pscl;
        break;
    case RTC_REG_RTC_DIVH:
        value = s->rtc_divh;
        break;
    case RTC_REG_RTC_DIVL:
        value = s->rtc_divl;
        break;
    case RTC_REG_RTC_CNTH:
        value = s->rtc_cnth;
        break;
    case RTC_REG_RTC_CNTL:
        value = s->rtc_cntl;
        break;
    case RTC_REG_RTC_ALRMH:
        value = s->rtc_alrmh;
        break;
    case RTC_REG_RTC_ALRML:
        value = s->rtc_inten;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void gd32vf103_rtc_write(void *opaque, hwaddr offset,
                                uint64_t value, unsigned size)
{
    GD32VF103RTCState *s = GD32VF103_RTC(opaque);
    switch (offset)
    {
    case RTC_REG_RTC_INTEN:
        s->rtc_inten = value;
        break;
    case RTC_REG_RTC_CTL:
        s->rtc_ctl = value;
        break;
    case RTC_REG_RTC_PSCH:
        s->rtc_psch = value;
        break;
    case RTC_REG_RTC_PSCL:
        s->rtc_pscl = value;
        break;
    case RTC_REG_RTC_DIVH:
        s->rtc_divh = value;
        break;
    case RTC_REG_RTC_DIVL:
        s->rtc_divl = value;
        break;
    case RTC_REG_RTC_CNTH:
        s->rtc_cnth = value;
        break;
    case RTC_REG_RTC_CNTL:
        s->rtc_cntl = value;
        break;
    case RTC_REG_RTC_ALRMH:
        s->rtc_alrmh = value;
        break;
    case RTC_REG_RTC_ALRML:
        s->rtc_inten = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps gd32vf103_rtc_ops = {
    .read = gd32vf103_rtc_read,
    .write = gd32vf103_rtc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void gd32vf103_rtc_realize(DeviceState *dev, Error **errp)
{
    GD32VF103RTCState *s = GD32VF103_RTC(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &gd32vf103_rtc_ops,
                          s, TYPE_GD32VF103_RTC, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void gd32vf103_rtc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = gd32vf103_rtc_realize;
    dc->reset = gd32vf103_rtc_reset;
    dc->desc = "NucLei RTC";
}

static const TypeInfo gd32vf103_rtc_info = {
    .name = TYPE_GD32VF103_RTC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103RTCState),
    .class_init = gd32vf103_rtc_class_init,
};

static void gd32vf103_rtc_register_types(void)
{
    type_register_static(&gd32vf103_rtc_info);
}
type_init(gd32vf103_rtc_register_types);