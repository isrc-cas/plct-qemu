/*
 *  GD32VF103 ADC interface
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
#include "hw/adc/gd32vf103_adc.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void gd32vf103_adc_reset(DeviceState *dev)
{
    GD32VF103ADCState *s = GD32VF103_ADC(dev);
    uint8_t i;

    s->adc_stat = 0x0;
    s->adc_ctl0 = 0x0;
    s->adc_ctl1 = 0x0;
    s->adc_sampt0 = 0x0;
    s->adc_sampt1 = 0x0;
    for(i = 0; i < ADC_MAX; i++)
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

static uint64_t gd32vf103_adc_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    GD32VF103ADCState *s = GD32VF103_ADC(opaque);
    uint64_t value = 0;
    uint8_t id = 0;

    if(offset >= ADC_REG_ADC_IOFFx 
        && offset < ADC_REG_ADC_WDHT)
    {
        id = (offset - ADC_REG_ADC_IOFFx)/0x4;
        offset = ADC_REG_ADC_IOFFx;
    }

    if(offset >= ADC_REG_ADC_IDATAx 
        && offset < ADC_REG_ADC_RDATA)
    {
        id = (offset - ADC_REG_ADC_IDATAx)/0x4;
        offset = ADC_REG_ADC_IDATAx;
    }


    switch (offset)
    {
    case ADC_REG_ADC_STAT:
        value = s->adc_stat;
        break;
    case ADC_REG_ADC_CTL0:
        value = s->adc_ctl0;
        break;
    case ADC_REG_ADC_CTL1:
        value = s->adc_ctl1;
        break;
    case ADC_REG_ADC_SAMPT0:
        value = s->adc_sampt0;
        break;
    case ADC_REG_ADC_SAMPT1:
        value = s->adc_sampt1;
        break;
    case ADC_REG_ADC_IOFFx:
        value = s->adc_ioff[id];
        break;
    case ADC_REG_ADC_WDHT:
        value = s->adc_wdht;
        break;
    case ADC_REG_ADC_WDLT:
        value = s->adc_wdlt;
        break;
    case ADC_REG_ADC_RSQ0:
        value = s->adc_rsq0;
        break;
    case ADC_REG_ADC_RSQ1:
        value = s->adc_rsq1;
        break;
    case ADC_REG_ADC_RSQ2:
        value = s->adc_rsq2;
        break;
    case ADC_REG_ADC_ISQ:
        value = s->adc_isq;
        break;
    case ADC_REG_ADC_IDATAx:
        value = s->adc_idata[id];
        break;
    case ADC_REG_ADC_RDATA:
        value = s->adc_rdata;
        break;
    case ADC_REG_ADC_OVSAMPCTL:
        value = s->adc_ovsampctl;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void gd32vf103_adc_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    GD32VF103ADCState *s = GD32VF103_ADC(opaque);
    uint8_t id = 0;

    if(offset >= ADC_REG_ADC_IOFFx 
        && offset < ADC_REG_ADC_WDHT)
    {
        id = (offset - ADC_REG_ADC_IOFFx)/0x4;
        offset = ADC_REG_ADC_IOFFx;
    }

    if(offset >= ADC_REG_ADC_IDATAx 
        && offset < ADC_REG_ADC_RDATA)
    {
        id = (offset - ADC_REG_ADC_IDATAx)/0x4;
        offset = ADC_REG_ADC_IDATAx;
    }

    switch (offset)
    {
    case ADC_REG_ADC_STAT:
        s->adc_stat = value;
        break;
    case ADC_REG_ADC_CTL0:
        s->adc_ctl0 = value;
        break;
    case ADC_REG_ADC_CTL1:
        s->adc_ctl1 = value;
        break;
    case ADC_REG_ADC_SAMPT0:
        s->adc_sampt0 = value;
        break;
    case ADC_REG_ADC_SAMPT1:
        s->adc_sampt1 = value;
        break;
    case ADC_REG_ADC_IOFFx:
        s->adc_ioff[id] = value;
        break;
    case ADC_REG_ADC_WDHT:
        s->adc_wdht = value;
        break;
    case ADC_REG_ADC_WDLT:
        s->adc_wdlt = value;
        break;
    case ADC_REG_ADC_RSQ0:
        s->adc_rsq0 = value;
        break;
    case ADC_REG_ADC_RSQ1:
        s->adc_rsq1 = value;
        break;
    case ADC_REG_ADC_RSQ2:
        s->adc_rsq2 = value;
        break;
    case ADC_REG_ADC_ISQ:
        s->adc_isq = value;
        break;
    case ADC_REG_ADC_IDATAx:
        s->adc_idata[id] = value;
        break;
    case ADC_REG_ADC_RDATA:
        s->adc_rdata = value;
        break;
    case ADC_REG_ADC_OVSAMPCTL:
        s->adc_ovsampctl = value;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps gd32vf103_adc_ops = {
    .read = gd32vf103_adc_read,
    .write = gd32vf103_adc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void gd32vf103_adc_realize(DeviceState *dev, Error **errp)
{
    GD32VF103ADCState *s = GD32VF103_ADC(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &gd32vf103_adc_ops,
                          s,TYPE_GD32VF103_ADC, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void gd32vf103_adc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = gd32vf103_adc_realize;
    dc->reset = gd32vf103_adc_reset;
    dc->desc = "GD32VF103 ADC";
}

static const TypeInfo gd32vf103_adc_info = {
    .name = TYPE_GD32VF103_ADC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103ADCState),
    .class_init = gd32vf103_adc_class_init,
};

static void gd32vf103_adc_register_types(void)
{
    type_register_static(&gd32vf103_adc_info);
}
type_init(gd32vf103_adc_register_types);