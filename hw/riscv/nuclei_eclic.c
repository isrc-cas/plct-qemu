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
#include "hw/riscv/nuclei_eclic.h"

#define RISCV_DEBUG_ECLIC 0

static void nuclei_eclic_update_intmth(NucLeiECLICState *eclic, int irq, int mth);
static void nuclei_eclic_update_intip(NucLeiECLICState *eclic, int irq, int new_intip);
static void nuclei_eclic_update_intie(NucLeiECLICState *eclic, int irq, int new_intie);
static void nuclei_eclic_update_intattr(NucLeiECLICState *eclic, int irq, int new_intattr);
static void nuclei_eclic_update_intctl(NucLeiECLICState *eclic, int irq, int new_intctl);

qemu_irq nuclei_eclic_get_irq(DeviceState *dev, int irq)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(dev);
    return eclic->irqs[irq];
}

static uint64_t nuclei_eclic_read(void *opaque, hwaddr offset, unsigned size)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(opaque);
    uint64_t value = 0;
    uint32_t id = 0;

    if( offset >= NUCLEI_ECLIC_REG_CLICINTIP_BASE){
        if( (offset - 0x1000)%4 == 0 )
        {
            id =  (offset - 0x1000) /4;
        }else if ((offset - 0x1001)%4 == 0)
        {
            id =  (offset - 0x1001)/4;
        }else if ((offset - 0x1002)%4 == 0)
        {
            id =  (offset - 0x1002)/4;
        }else if ((offset - 0x1003)%4 == 0)
        {
            id =  (offset - 0x1003)/4;
        }
        offset = offset - 4 * id;

    } else {

    }

    switch (offset)
    {
    case NUCLEI_ECLIC_REG_CLICCFG:
        value =  eclic->cliccfg & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINFO:
        value =  CLICINTCTLBITS << 21 & 0xFFFF;
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
    uint32_t id = 0;

    if(offset >= NUCLEI_ECLIC_REG_CLICINTIP_BASE) {

         if( (offset - 0x1000)%4 == 0 )
        {
            id =  (offset - 0x1000)/4;
        }else if ((offset - 0x1001)%4 == 0)
        {
            id =  (offset - 0x1001)/4;
        }else if ((offset - 0x1002)%4 == 0)
        {
            id =  (offset - 0x1002)/4;
        }else if ((offset - 0x1003)%4 == 0)
        {
            id =  (offset - 0x1003)/4;
        }
        offset = offset - 4 * id;
    } else {

    }

    switch (offset)
    {
    case NUCLEI_ECLIC_REG_CLICCFG:
        eclic->cliccfg = value & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_MTH:
        nuclei_eclic_update_intmth(eclic, id, value & 0xFF);
        break;
    case NUCLEI_ECLIC_REG_CLICINTIP_BASE:
        nuclei_eclic_update_intip(eclic, id, value & 0xFF);
        break;
    case NUCLEI_ECLIC_REG_CLICINTIE_BASE:
        nuclei_eclic_update_intie(eclic, id, value & 0xFF);
        break;
    case NUCLEI_ECLIC_REG_CLICINTATTR_BASE:
        nuclei_eclic_update_intattr(eclic, id, value & 0xFF);
        break;
    case NUCLEI_ECLIC_REG_CLICINTCTL_BASE:
        nuclei_eclic_update_intctl(eclic, id, value & 0xFF);
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

static void reg_decode(NucLeiECLICState *eclic, int irq, int *level, int *priority, int *enable, int *trigger)
{
    int level_width = (eclic->cliccfg >> 1) & 0xF; // cliccfg.nlbits
    int prio_width = CLICINTCTLBITS - level_width;

    if (level_width == 0) *level = 255;
    else if (level_width <= CLICINTCTLBITS) 
        *level = (eclic->clicintctl[irq] >> (8 - level_width)) & \
             ~((char)0x80 >> (8 - level_width));
    else if (level_width > CLICINTCTLBITS)
         *level = ((eclic->clicintctl[irq] >> (8 - level_width)) & \
             ~((char)0x80 >> (8 - level_width))) | \
             ~((char)0x80 >> (8 - (CLICINTCTLBITS - level_width)));

    // TODO: implement priority decode logic when width > CLICINTCTLBITS or zeros
    if (prio_width == 0) *priority = 0;
    else if (prio_width > 0)
        *priority = (eclic->clicintctl[irq] >> (8 - level_width)) &
                ~(0x80 >> (8 - prio_width));
    else *priority = 0;

    if (enable)
        *enable = eclic->clicintie[irq] & 0x1;
    // 0, level triggered; 2, rising edge; 3, falling edge
    if (trigger)
        *trigger = (eclic->clicintattr[irq] >> 1) & 0x3;
}

static void nuclei_eclic_next_interrupt(NucLeiECLICState *eclic, int old_intip, int irq)
{
    RISCVCPU *cpu = RISCV_CPU(qemu_get_cpu(0));

    ECLICPendingInterrupt *active;// = eclic->pending_list;
    int level, priority, enable, trigger, new_intip = 0;
    QLIST_FOREACH(active, &eclic->pending_list, next) {
        new_intip = eclic->clicintip[active->irq] & 0x1;
        reg_decode(eclic, active->irq, &level, &priority, &enable, &trigger);

        if (enable) {
             if (((trigger == 0) && new_intip) ||
                 ((trigger == 2) && (irq == active->irq) && !old_intip && new_intip) ||
                 ((trigger == 3) && (irq == active->irq) && old_intip && !new_intip)) {
                 if (level >= eclic->mth) {
                    uint32_t shv =  eclic->clicintattr[active->irq] & 0x1;
                    eclic->active_count++;
                    QLIST_REMOVE(active, next);
                    riscv_cpu_eclic_interrupt(cpu, (active->irq & 0xFFF) | (shv << 12));
                    free(active);
                    return;
                 }
             }
        }
        if (trigger == 0 && new_intip == 0) {
             eclic->active_count++;
             QLIST_REMOVE(active, next);
             free(active);
        }
    }
    riscv_cpu_eclic_interrupt(cpu, -1);
}

static int level_compare(NucLeiECLICState *eclic, ECLICPendingInterrupt *irq1, ECLICPendingInterrupt *irq2) 
{
    reg_decode(eclic, irq1->irq, &irq1->level, &irq1->prio, &irq1->enable, &irq1->trigger);
    reg_decode(eclic, irq2->irq, &irq2->level, &irq2->prio, &irq2->enable, &irq2->trigger);

    if (irq1->level == irq2->level) {
	if (irq1->prio == irq2->prio) {
            if (irq1->irq >= irq2->irq) {
                // put irq2 behind
                return 0;
	    } else {
                // irq2 before irq1
                return 1; 
            }
	} else if (irq1->prio > irq2->level) {
	    return 0;
	} else {
            return 1;
	}
    } else if (irq1->level > irq2->level) {
        return 0;
    } else {
        return 1;
    }
}

static void nuclei_eclic_irq(void *opaque, int id, int new_intip)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(opaque);
    nuclei_eclic_update_intip(eclic, id, new_intip);
}

/* cliccfg not supposed to be modified
static void nuclei_eclic_update_intcfg(NucLeiECLICState *eclic, int irq)
{
    nuclei_eclic_next_interrupt(eclic, irq, eclic->clicintip[irq]);
}*/

static void nuclei_eclic_update_intmth(NucLeiECLICState *eclic, int irq, int mth)
{
    eclic->mth = mth;
    nuclei_eclic_next_interrupt(eclic, eclic->clicintip[irq], irq);
}

static void nuclei_eclic_update_intip(NucLeiECLICState *eclic, int irq, int new_intip)
{
    ECLICPendingInterrupt *newActiveIRQ = (ECLICPendingInterrupt *)calloc(sizeof(ECLICPendingInterrupt), 1);
    ECLICPendingInterrupt *node;
   
    int old_intip = eclic->clicintip[irq];

    newActiveIRQ->irq = irq;
    eclic->clicintip[irq] = new_intip & 0x1;
    eclic->sources_id = irq;

    if (QLIST_EMPTY(&eclic->pending_list)) {
        QLIST_INSERT_HEAD(&eclic->pending_list, newActiveIRQ, next);
    } else {
        QLIST_FOREACH(node, &eclic->pending_list, next) {
            if (level_compare(eclic, node, newActiveIRQ)) {
                QLIST_INSERT_BEFORE(node, newActiveIRQ, next);
            } else {
                QLIST_INSERT_AFTER(node, newActiveIRQ, next);
            }
        }
    }

    nuclei_eclic_next_interrupt(eclic, old_intip & 0x1, irq);
}

static void nuclei_eclic_update_intie(NucLeiECLICState *eclic, int irq, int new_intie)
{
    eclic->clicintie[irq] = new_intie;
    nuclei_eclic_next_interrupt(eclic, eclic->clicintip[irq], irq);
}

// TODO: intattr not supposed to be changed during runtime?
static void nuclei_eclic_update_intattr(NucLeiECLICState *eclic, int irq, int new_intattr)
{
    eclic->clicintattr[irq] = new_intattr;
    nuclei_eclic_next_interrupt(eclic, eclic->clicintip[irq], irq);
}

// TODO: intctl not supposed to be changed during runtime?
static void nuclei_eclic_update_intctl(NucLeiECLICState *eclic, int irq, int new_intctl)
{
    eclic->clicintctl[irq] = new_intctl;
    nuclei_eclic_next_interrupt(eclic, eclic->clicintip[irq], irq);
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
    QLIST_INIT(&eclic->pending_list);
    eclic->active_count = 0;

    /* Init ECLIC IRQ */
    eclic->irqs[Internal_SysTimerSW_IRQn] = qemu_allocate_irq(nuclei_eclic_irq,
                        eclic, Internal_SysTimerSW_IRQn);
    eclic->irqs[Internal_SysTimer_IRQn] = qemu_allocate_irq(nuclei_eclic_irq,
                        eclic, Internal_SysTimer_IRQn);
                        
    for (id = Internal_Reserved_Max_IRQn; id < eclic->num_sources; id++) {
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

static void nuclei_eclic_mtimecmp_cb(void *cpu) {
    CPURISCVState *env = &((RISCVCPU *)cpu)->env;
    nuclei_eclic_irq(((RISCVCPU *)cpu)->env.eclic, Internal_SysTimer_IRQn, 1);
    timer_del(env->mtimer);
}


NucLeiECLICState *nuclei_eclic_create(hwaddr addr,  uint32_t aperture_size,  uint32_t num_sources)
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
    return OBJECT_CHECK(NucLeiECLICState, dev, TYPE_NUCLEI_ECLIC);
}
