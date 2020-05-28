/*
 *  NUCLEI RTC interface
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
#include "hw/riscv/nuclei_rtc.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void nuclei_rtc_reset(DeviceState *dev)
{
    NucLeiRTCState *s = NUCLEI_RTC(dev);
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

static uint64_t nuclei_rtc_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiRTCState *s = NUCLEI_RTC(opaque);
    uint64_t value = 0;

    switch (offset)
    {
    case NUCLEI_RTC_REG_RTC_INTEN:
        value = s->rtc_inten;
        break;
    case NUCLEI_RTC_REG_RTC_CTL:
        value = s->rtc_ctl;
        break;
    case NUCLEI_RTC_REG_RTC_PSCH:
        value = s->rtc_psch;
        break;
    case NUCLEI_RTC_REG_RTC_PSCL:
        value = s->rtc_pscl;
        break;
    case NUCLEI_RTC_REG_RTC_DIVH:
        value = s->rtc_divh;
        break;
    case NUCLEI_RTC_REG_RTC_DIVL:
        value = s->rtc_divl;
        break;
    case NUCLEI_RTC_REG_RTC_CNTH:
        value = s->rtc_cnth;
        break;
    case NUCLEI_RTC_REG_RTC_CNTL:
        value = s->rtc_cntl;
        break;
    case NUCLEI_RTC_REG_RTC_ALRMH:
        value = s->rtc_alrmh;
        break;
    case NUCLEI_RTC_REG_RTC_ALRML:
        value = s->rtc_inten;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_rtc_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiRTCState *s = NUCLEI_RTC(opaque);
    switch (offset)
    {
    case NUCLEI_RTC_REG_RTC_INTEN:
        s->rtc_inten = value;
        break;
    case NUCLEI_RTC_REG_RTC_CTL:
        s->rtc_ctl = value;
        break;
    case NUCLEI_RTC_REG_RTC_PSCH:
        s->rtc_psch = value;
        break;
    case NUCLEI_RTC_REG_RTC_PSCL:
        s->rtc_pscl = value;
        break;
    case NUCLEI_RTC_REG_RTC_DIVH:
        s->rtc_divh = value;
        break;
    case NUCLEI_RTC_REG_RTC_DIVL:
        s->rtc_divl = value;
        break;
    case NUCLEI_RTC_REG_RTC_CNTH:
        s->rtc_cnth = value;
        break;
    case NUCLEI_RTC_REG_RTC_CNTL:
        s->rtc_cntl = value;
        break;
    case NUCLEI_RTC_REG_RTC_ALRMH:
        s->rtc_alrmh = value;
        break;
    case NUCLEI_RTC_REG_RTC_ALRML:
        s->rtc_inten = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_rtc_ops = {
    .read = nuclei_rtc_read,
    .write = nuclei_rtc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void nuclei_rtc_realize(DeviceState *dev, Error **errp)
{
    NucLeiRTCState *s = NUCLEI_RTC(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_rtc_ops,
                          s,TYPE_NUCLEI_RTC, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void nuclei_rtc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_rtc_realize;
    dc->reset = nuclei_rtc_reset;
    dc->desc = "NucLei RTC";
}

static const TypeInfo nuclei_rtc_info = {
    .name = TYPE_NUCLEI_RTC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiRTCState),
    .class_init = nuclei_rtc_class_init,
};

static void nuclei_rtc_register_types(void)
{
    type_register_static(&nuclei_rtc_info);
}
type_init(nuclei_rtc_register_types);
