/*
 * NUCLEI ECLIC(Enhanced Core Local Interrupt Controller)
 *
 * Copyright (c) 2020 NucLei, Inc.
 *
 * This provides a parameterizable interrupt controller based on NucLei's ECLIC.
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
#include "qemu/log.h"
#include "qemu/module.h"
#include "qemu/error-report.h"
#include "hw/sysbus.h"
#include "hw/pci/msi.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "target/riscv/cpu.h"
#include "sysemu/sysemu.h"
#include "hw/riscv/nuclei.h"
#include "hw/riscv/nuclei_eclic.h"

#define RISCV_DEBUG_ECLIC 0

qemu_irq nuclei_eclic_get_irq(DeviceState *dev, int irq)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(dev);
    return eclic->irqs[irq];
}

static uint64_t nuclei_eclic_read(void *opaque, hwaddr offset, unsigned size)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(opaque);
    uint64_t value = 0;
    uint32_t id = eclic->sources_id;

    if( offset >= NUCLEI_ECLIC_REG_CLICINTIP_BASE)
        offset = offset - 4 * id;

    switch (offset)
    {
    case NUCLEI_ECLIC_REG_CLICCFG:
        value =  eclic->cliccfg & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINFO:
        value =  eclic->clicinfo & 0xFFFF;
        break;
    case NUCLEI_ECLIC_REG_MTH:
        value =  eclic->mth & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTIP_BASE:
        value =  eclic->clicintip[id] & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTIE_BASE:
        value =  eclic->clicintie[id] & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTATTR_BASE:
        value =  eclic->clicintattr[id] & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTCTL_BASE:
        value =  eclic->clicintctl[id] & 0xFF;
        break;
    default:
        break;
    }

    return value;
}

static void nuclei_eclic_write(void *opaque, hwaddr offset, uint64_t value,
        unsigned size)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(opaque);
    uint32_t id = eclic->sources_id;

    if( offset >= NUCLEI_ECLIC_REG_CLICINTIP_BASE)
        offset = offset - 4 * id;

    switch (offset)
    {
    case NUCLEI_ECLIC_REG_CLICCFG:
        eclic->cliccfg = value & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINFO:
        eclic->clicinfo = value & 0xFFFF;
        break;
    case NUCLEI_ECLIC_REG_MTH:
        eclic->mth =  value & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTIP_BASE:
        eclic->clicintip[id] = value & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTIE_BASE:
        eclic->clicintie[id] = value  & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTATTR_BASE:
        eclic->clicintattr[id] = value  & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINTCTL_BASE:
        eclic->clicintctl[id] = value & 0xFF;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps nuclei_eclic_ops = {
    .read = nuclei_eclic_read,
    .write = nuclei_eclic_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4
    }
};

static Property nuclei_eclic_properties[] = {
    DEFINE_PROP_UINT32("aperture-size", NucLeiECLICState, aperture_size, 0),
    DEFINE_PROP_UINT32("num-sources", NucLeiECLICState, num_sources, 0),
    DEFINE_PROP_END_OF_LIST(),
};

static void clicintctl_decode(NucLeiECLICState *eclic, uint8_t clicintctl, int *level, int *priority)
{
    //TODO: implement register decoding
    *level = 0;
    *priority = 0;
}

static void nuclei_eclic_next_interrupt(NucLeiECLICState *eclic)
{
    RISCVCPU *cpu = RISCV_CPU(qemu_get_cpu(0));

    ECLICActiveInterrupt *active = eclic->active_list;
    size_t active_count = eclic->active_count;
    int level = 0, priority = 0;

    while (active_count) {
        clicintctl_decode(eclic, active->clicintctl, &level, &priority);
        if (eclic->clicintip[active->irq]) {
            riscv_cpu_eclic_interrupt(cpu, active->irq  | level<<12);
            return;
        }
        /* check next enabled interrupt */
        active_count--;
        active++;
    }

    /* clear pending interrupt for this hart */
    riscv_cpu_eclic_interrupt(cpu, -1);
}

static void nuclei_eclic_update_intip(NucLeiECLICState *eclic, int irq, int new_intip)
{
    eclic->clicintip[irq] = !!new_intip;
    nuclei_eclic_next_interrupt(eclic);
}

static void sifive_clic_timer_irq(RISCVCPU *cpu, int irq, int level)
{
    CPURISCVState *env = &cpu->env;
    NucLeiECLICState *eclic = env->eclic;
    nuclei_eclic_update_intip(eclic,  irq, level);
}

static void nuclei_eclic_mtimecmp_cb(void *cpu)
{
    sifive_clic_timer_irq(cpu, MIP_MTIP, 1);
}

static void nuclei_eclic_irq(void *opaque, int id, int level)
{
   NucLeiECLICState *eclic = NUCLEI_ECLIC(opaque);
   nuclei_eclic_update_intip(eclic, id, level);
}

static void nuclei_eclic_realize(DeviceState *dev, Error **errp)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(dev);
    int id;

    memory_region_init_io(&eclic->mmio, OBJECT(dev), &nuclei_eclic_ops, eclic,
                          TYPE_NUCLEI_ECLIC, eclic->aperture_size);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &eclic->mmio);

    eclic->clicintip = g_new0(uint8_t, eclic->num_sources);
    eclic->clicintie = g_new0(uint8_t, eclic->num_sources);
    eclic->clicintattr = g_new0(uint8_t, eclic->num_sources);
    eclic->clicintctl = g_new0(uint8_t, eclic->num_sources);
    eclic->irqs = g_new0(qemu_irq, eclic->num_sources);
    eclic->active_list = g_new0(ECLICActiveInterrupt, eclic->num_sources);
    eclic->active_count =0;

    /* Init ECLIE IRQ */
    eclic->irqs[NUCLEI_SysTimerSW_IRQn] = qemu_allocate_irq(nuclei_eclic_irq,
                        eclic, NUCLEI_SysTimerSW_IRQn);
    eclic->irqs[NUCLEI_SysTimer_IRQn] = qemu_allocate_irq(nuclei_eclic_irq,
                        eclic, NUCLEI_SysTimer_IRQn);
    for (id = NUCLEI_WWDGT_IRQn; id < eclic->num_sources; id++) {
            eclic->irqs[id] = qemu_allocate_irq(nuclei_eclic_irq,
                        eclic, id);
    }

    RISCVCPU *cpu = RISCV_CPU(qemu_get_cpu(0));
    cpu->env.eclic = eclic;
}

static void nuclei_eclic_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    device_class_set_props(dc, nuclei_eclic_properties);
    dc->realize = nuclei_eclic_realize;
}

static const TypeInfo nuclei_eclic_info = {
    .name          = TYPE_NUCLEI_ECLIC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiECLICState),
    .class_init    = nuclei_eclic_class_init,
};

static void nuclei_eclic_register_types(void)
{
    type_register_static(&nuclei_eclic_info);
}

type_init(nuclei_eclic_register_types);

DeviceState *nuclei_eclic_create(hwaddr addr,  uint32_t aperture_size,  uint32_t num_sources)
{
    RISCVCPU *cpu = RISCV_CPU(qemu_get_cpu(0));
    CPURISCVState *env = &cpu->env;
    env->features |= (1ULL << RISCV_FEATURE_ECLIC);
    env->mtimer = timer_new_ns(QEMU_CLOCK_VIRTUAL,
                                   &nuclei_eclic_mtimecmp_cb, cpu);
    env->mtimecmp = 0;

    DeviceState *dev = qdev_create(NULL, TYPE_NUCLEI_ECLIC);
    qdev_prop_set_uint32(dev, "aperture-size", aperture_size);
    qdev_prop_set_uint32(dev, "num-sources", num_sources);
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
    return dev;
}
