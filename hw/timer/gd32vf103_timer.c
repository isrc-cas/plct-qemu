/*
 *  GD32VF103 TIMER interface
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
#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/timer.h"
#include "hw/timer/gd32vf103_timer.h"
#include "hw/registerfields.h"
#include "hw/qdev-core.h"
#include "migration/vmstate.h"
#include "trace.h"

static void gd32vf103_timer_reset(DeviceState *dev)
{
    GD32VF103TimerState *s = GD32VF103_TIMER(dev);

    s->timerx_ctl0 = 0x0;
    s->timerx_ctl1 = 0x0;
    s->timerx_smcfg = 0x0;
    s->timerx_dmainten = 0x0;
    s->timerx_int = 0x0;
    s->timerx_swevg = 0x0;
    s->timerx_chctl0 = 0x0;
    s->timerx_chctl1 = 0x0;
    s->timerx_chctl2 = 0x0;
    s->timerx_cnt = 0x0;
    s->timerx_psc = 0x0;
    s->timerx_car = 0x0;
    s->timerx_crep = 0x0;
    s->timerx_ch0cv = 0x0;
    s->timerx_ch1cv = 0x0;
    s->timerx_ch2cv = 0x0;
    s->timerx_ch3cv = 0x0;
    s->timerx_dmacfg = 0x0;
    s->timerx_dmatb = 0x0;

}

static uint64_t gd32vf103_timer_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    GD32VF103TimerState *s = GD32VF103_TIMER(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case TIMER_REG_TIMERx_CTL0:
        value = s->timerx_ctl0;
        break;
    case TIMER_REG_TIMERx_CTL1:
        value = s->timerx_ctl1;
        break;
    case TIMER_REG_TIMERx_SMCFG:
        value = s->timerx_smcfg;
        break;
    case TIMER_REG_TIMERx_DMAINTEN:
        value = s->timerx_dmainten;
        break;
    case TIMER_REG_TIMERx_INTF:
        value = s->timerx_int;
        break;
    case TIMER_REG_TIMERx_SWEVG:
        value = s->timerx_swevg;
        break;
    case TIMER_REG_TIMERx_CHCTL0:
        value = s->timerx_chctl0;
        break;
    case TIMER_REG_TIMERx_CHCTL1:
        value = s->timerx_chctl1;
        break;
    case TIMER_REG_TIMERx_CHCTL2:
        value = s->timerx_chctl2;
        break;
    case TIMER_REG_TIMERx_CNT:
        value = s->timerx_cnt;
        break;
    case TIMER_REG_TIMERx_PSC:
        value = s->timerx_psc;
        break;
    case TIMER_REG_TIMERx_CAR:
        value = s->timerx_car;
        break;
    case TIMER_REG_TIMERx_CREP:
        value = s->timerx_crep;
        break;
    case TIMER_REG_TIMERx_CH0CV:
        value = s->timerx_ch0cv;
        break;
    case TIMER_REG_TIMERx_CH1CV:
        value = s->timerx_ch1cv;
        break;
    case TIMER_REG_TIMERx_CH2CV:
        value = s->timerx_ch2cv;
        break;
    case TIMER_REG_TIMERx_CH3CV:
        value = s->timerx_ch3cv;
        break;
    case TIMER_REG_TIMERx_DMACFG:
        value = s->timerx_dmacfg;
        break;
    case TIMER_REG_TIMERx_DMATB:
        value = s->timerx_dmatb;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void gd32vf103_timer_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    GD32VF103TimerState *s = GD32VF103_TIMER(opaque);

    switch (offset)
    {
    case TIMER_REG_TIMERx_CTL0:
        s->timerx_ctl0 = value;
        break;
    case TIMER_REG_TIMERx_CTL1:
        s->timerx_ctl1 = value;
        break;
    case TIMER_REG_TIMERx_SMCFG:
        s->timerx_smcfg = value;
        break;
    case TIMER_REG_TIMERx_DMAINTEN:
        s->timerx_dmainten = value;
        break;
    case TIMER_REG_TIMERx_INTF:
        s->timerx_int = value;
        break;
    case TIMER_REG_TIMERx_SWEVG:
        s->timerx_swevg = value;
        break;
    case TIMER_REG_TIMERx_CHCTL0:
        s->timerx_chctl0 = value;
        break;
    case TIMER_REG_TIMERx_CHCTL1:
        s->timerx_chctl1 = value;
        break;
    case TIMER_REG_TIMERx_CHCTL2:
        s->timerx_chctl2 = value;
        break;
    case TIMER_REG_TIMERx_CNT:
        s->timerx_cnt = value;
        break;
    case TIMER_REG_TIMERx_PSC:
        s->timerx_psc = value;
        break;
    case TIMER_REG_TIMERx_CAR:
        s->timerx_car = value;
        break;
    case TIMER_REG_TIMERx_CREP:
        s->timerx_crep = value;
        break;
    case TIMER_REG_TIMERx_CH0CV:
        s->timerx_ch0cv = value;
        break;
    case TIMER_REG_TIMERx_CH1CV:
        s->timerx_ch1cv = value;
        break;
    case TIMER_REG_TIMERx_CH2CV:
        s->timerx_ch2cv = value;
        break;
    case TIMER_REG_TIMERx_CH3CV:
        s->timerx_ch3cv = value;
        break;
    case TIMER_REG_TIMERx_DMACFG:
        s->timerx_dmacfg = value;
        break;
    case TIMER_REG_TIMERx_DMATB:
        s->timerx_dmatb = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps gd32vf103_timer_ops = {
    .read = gd32vf103_timer_read,
    .write = gd32vf103_timer_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void gd32vf103_timer_realize(DeviceState *dev, Error **errp)
{
    GD32VF103TimerState *s = GD32VF103_TIMER(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &gd32vf103_timer_ops,
                          s,TYPE_GD32VF103_TIMER, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void gd32vf103_timer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = gd32vf103_timer_realize;
    dc->reset = gd32vf103_timer_reset;
    dc->desc = "GD32VF103 HW TIMER";
}

static const TypeInfo gd32vf103_timer_info = {
    .name = TYPE_GD32VF103_TIMER,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103TimerState),
    .class_init = gd32vf103_timer_class_init,
};

static void gd32vf103_timer_register_types(void)
{
    type_register_static(&gd32vf103_timer_info);
}
type_init(gd32vf103_timer_register_types);