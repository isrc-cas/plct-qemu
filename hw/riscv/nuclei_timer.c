/*
 *  NUCLEI TIMER interface
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
#include "qemu/log.h"
#include "qemu/timer.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/nuclei_timer.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void nuclei_timer_reset(DeviceState *dev)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(dev);

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

static uint64_t nuclei_timer_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case NUCLEI_TIMER_REG_TIMERx_CTL0:
        value = s->timerx_ctl0;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CTL1:
        value = s->timerx_ctl1;
        break;
    case NUCLEI_TIMER_REG_TIMERx_SMCFG:
        value = s->timerx_smcfg;
        break;
    case NUCLEI_TIMER_REG_TIMERx_DMAINTEN:
        value = s->timerx_dmainten;
        break;
    case NUCLEI_TIMER_REG_TIMERx_INTF:
        value = s->timerx_int;
        break;
    case NUCLEI_TIMER_REG_TIMERx_SWEVG:
        value = s->timerx_swevg;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CHCTL0:
        value = s->timerx_chctl0;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CHCTL1:
        value = s->timerx_chctl1;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CHCTL2:
        value = s->timerx_chctl2;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CNT:
        value = s->timerx_cnt;
        break;
    case NUCLEI_TIMER_REG_TIMERx_PSC:
        value = s->timerx_psc;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CAR:
        value = s->timerx_car;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CREP:
        value = s->timerx_crep;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH0CV:
        value = s->timerx_ch0cv;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH1CV:
        value = s->timerx_ch1cv;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH2CV:
        value = s->timerx_ch2cv;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH3CV:
        value = s->timerx_ch3cv;
        break;
    case NUCLEI_TIMER_REG_TIMERx_DMACFG:
        value = s->timerx_dmacfg;
        break;
    case NUCLEI_TIMER_REG_TIMERx_DMATB:
        value = s->timerx_dmatb;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_timer_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(opaque);

    switch (offset)
    {
    case NUCLEI_TIMER_REG_TIMERx_CTL0:
        s->timerx_ctl0 = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CTL1:
        s->timerx_ctl1 = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_SMCFG:
        s->timerx_smcfg = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_DMAINTEN:
        s->timerx_dmainten = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_INTF:
        s->timerx_int = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_SWEVG:
        s->timerx_swevg = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CHCTL0:
        s->timerx_chctl0 = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CHCTL1:
        s->timerx_chctl1 = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CHCTL2:
        s->timerx_chctl2 = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CNT:
        s->timerx_cnt = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_PSC:
        s->timerx_psc = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CAR:
        s->timerx_car = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CREP:
        s->timerx_crep = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH0CV:
        s->timerx_ch0cv = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH1CV:
        s->timerx_ch1cv = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH2CV:
        s->timerx_ch2cv = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_CH3CV:
        s->timerx_ch3cv = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_DMACFG:
        s->timerx_dmacfg = value;
        break;
    case NUCLEI_TIMER_REG_TIMERx_DMATB:
        s->timerx_dmatb = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_timer_ops = {
    .read = nuclei_timer_read,
    .write = nuclei_timer_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};



static void nuclei_timer_realize(DeviceState *dev, Error **errp)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_timer_ops,
                          s,TYPE_NUCLEI_TIMER, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void nuclei_timer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_timer_realize;
    dc->reset = nuclei_timer_reset;
    dc->desc = "NucLei HW TIMER";
}

static const TypeInfo nuclei_timer_info = {
    .name = TYPE_NUCLEI_TIMER,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiTIMERState),
    .class_init = nuclei_timer_class_init,
};

static void nuclei_timer_register_types(void)
{
    type_register_static(&nuclei_timer_info);
}
type_init(nuclei_timer_register_types);
