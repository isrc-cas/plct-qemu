/*
 *  NUCLEI ADC interface
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
#include "hw/riscv/nuclei_adc.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void nuclei_adc_reset(DeviceState *dev)
{
    NucLeiADCState *s = NUCLEI_ADC(dev);
    uint8_t i;

    s->adc_stat = 0x0;
    s->adc_ctl0 = 0x0;
    s->adc_ctl1 = 0x0;
    s->adc_sampt0 = 0x0;
    s->adc_sampt1 = 0x0;
    for(i = 0; i < NUCLEI_ADC_MAX; i++)
    {
        s->adc_ioff[i] = 0x0;
        s->adc_idata[i] = 0x0;
    }
    
    s->adc_wdht = 0xFFF;
    s->adc_wdlt = 0x0;
    s->adc_rsq0 = 0x0;
    s->adc_rsq1 = 0x0;
    s->adc_rsq2 = 0x0;
    s->adc_isq = 0x0;
    s->adc_rdata = 0x0;
    s->adc_ovsampctl = 0x0;
}

static uint64_t nuclei_adc_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiADCState *s = NUCLEI_ADC(opaque);
    uint64_t value = 0;
    uint8_t id = 0;

    if(offset >= NUCLEI_ADC_REG_ADC_IOFFx 
        && offset < NUCLEI_ADC_REG_ADC_WDHT)
    {
        id = (offset - NUCLEI_ADC_REG_ADC_IOFFx)/0x4;
        offset = NUCLEI_ADC_REG_ADC_IOFFx;
         
    }

    if(offset >= NUCLEI_ADC_REG_ADC_IDATAx 
        && offset < NUCLEI_ADC_REG_ADC_RDATA)
    {
        id = (offset - NUCLEI_ADC_REG_ADC_IDATAx)/0x4;
        offset = NUCLEI_ADC_REG_ADC_IDATAx;
    }


    switch (offset)
    {
    case NUCLEI_ADC_REG_ADC_STAT:
        value = s->adc_stat;
        break;
    case NUCLEI_ADC_REG_ADC_CTL0:
        value = s->adc_ctl0;
        break;
    case NUCLEI_ADC_REG_ADC_CTL1:
        value = s->adc_ctl1;
        break;
    case NUCLEI_ADC_REG_ADC_SAMPT0:
        value = s->adc_sampt0;
        break;
    case NUCLEI_ADC_REG_ADC_SAMPT1:
        value = s->adc_sampt1;
        break;
    case NUCLEI_ADC_REG_ADC_IOFFx:
        value = s->adc_ioff[id];
        break;
    case NUCLEI_ADC_REG_ADC_WDHT:
        value = s->adc_wdht;
        break;
    case NUCLEI_ADC_REG_ADC_WDLT:
        value = s->adc_wdlt;
        break;
    case NUCLEI_ADC_REG_ADC_RSQ0:
        value = s->adc_rsq0;
        break;
    case NUCLEI_ADC_REG_ADC_RSQ1:
        value = s->adc_rsq1;
        break;
    case NUCLEI_ADC_REG_ADC_RSQ2:
        value = s->adc_rsq2;
        break;
    case NUCLEI_ADC_REG_ADC_ISQ:
        value = s->adc_isq;
        break;
    case NUCLEI_ADC_REG_ADC_IDATAx:
        value = s->adc_idata[id];
        break;
    case NUCLEI_ADC_REG_ADC_RDATA:
        value = s->adc_rdata;
        break;
    case NUCLEI_ADC_REG_ADC_OVSAMPCTL:
        value = s->adc_ovsampctl;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_adc_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiADCState *s = NUCLEI_ADC(opaque);
    uint8_t id = 0;

    if(offset >= NUCLEI_ADC_REG_ADC_IOFFx 
        && offset < NUCLEI_ADC_REG_ADC_WDHT)
    {
        id = (offset - NUCLEI_ADC_REG_ADC_IOFFx)/0x4;
        offset = NUCLEI_ADC_REG_ADC_IOFFx;
         
    }

    if(offset >= NUCLEI_ADC_REG_ADC_IDATAx 
        && offset < NUCLEI_ADC_REG_ADC_RDATA)
    {
        id = (offset - NUCLEI_ADC_REG_ADC_IDATAx)/0x4;
        offset = NUCLEI_ADC_REG_ADC_IDATAx;
    }

    switch (offset)
    {
    case NUCLEI_ADC_REG_ADC_STAT:
        s->adc_stat = value;
        break;
    case NUCLEI_ADC_REG_ADC_CTL0:
        s->adc_ctl0 = value;
        break;
    case NUCLEI_ADC_REG_ADC_CTL1:
        s->adc_ctl1 = value;
        break;
    case NUCLEI_ADC_REG_ADC_SAMPT0:
        s->adc_sampt0 = value;
        break;
    case NUCLEI_ADC_REG_ADC_SAMPT1:
        s->adc_sampt1 = value;
        break;
    case NUCLEI_ADC_REG_ADC_IOFFx:
        s->adc_ioff[id] = value;
        break;
    case NUCLEI_ADC_REG_ADC_WDHT:
        s->adc_wdht = value;
        break;
    case NUCLEI_ADC_REG_ADC_WDLT:
        s->adc_wdlt = value;
        break;
    case NUCLEI_ADC_REG_ADC_RSQ0:
        s->adc_rsq0 = value;
        break;
    case NUCLEI_ADC_REG_ADC_RSQ1:
        s->adc_rsq1 = value;
        break;
    case NUCLEI_ADC_REG_ADC_RSQ2:
        s->adc_rsq2 = value;
        break;
    case NUCLEI_ADC_REG_ADC_ISQ:
        s->adc_isq = value;
        break;
    case NUCLEI_ADC_REG_ADC_IDATAx:
        s->adc_idata[id] = value;
        break;
    case NUCLEI_ADC_REG_ADC_RDATA:
        s->adc_rdata = value;
        break;
    case NUCLEI_ADC_REG_ADC_OVSAMPCTL:
        s->adc_ovsampctl = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_adc_ops = {
    .read = nuclei_adc_read,
    .write = nuclei_adc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void nuclei_adc_realize(DeviceState *dev, Error **errp)
{
    NucLeiADCState *s = NUCLEI_ADC(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_adc_ops,
                          s,TYPE_NUCLEI_ADC, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void nuclei_adc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_adc_realize;
    dc->reset = nuclei_adc_reset;
    dc->desc = "NucLei Timer";
}

static const TypeInfo nuclei_adc_info = {
    .name = TYPE_NUCLEI_ADC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiADCState),
    .class_init = nuclei_adc_class_init,
};

static void nuclei_adc_register_types(void)
{
    type_register_static(&nuclei_adc_info);
}
type_init(nuclei_adc_register_types);
