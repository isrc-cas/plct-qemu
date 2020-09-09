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
static void eclic_insert_pending_list(NucLeiECLICState *eclic, int irq);
static void eclic_remove_pending_list(NucLeiECLICState *eclic, int irq);
static void nuclei_eclic_next_interrupt(void *eclic);
static void update_eclic_int_info(NucLeiECLICState *eclic, int irq);
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
    }

    switch (offset)
    {
    case NUCLEI_ECLIC_REG_CLICCFG:
        value =  eclic->cliccfg & 0xFF;
        break;
    case NUCLEI_ECLIC_REG_CLICINFO:
        value =  (CLICINTCTLBITS << 21) & 0xFFFFFFFF;
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
    }
    switch (offset)
    {
    case NUCLEI_ECLIC_REG_CLICCFG:
        eclic->cliccfg = value & 0xFF;
        for (id = 0; id < eclic->num_sources; id++) {
            update_eclic_int_info(eclic, id);
        }
        break;
    case NUCLEI_ECLIC_REG_MTH:
        nuclei_eclic_update_intmth(eclic, id, value & 0xFF);
        break;
    case NUCLEI_ECLIC_REG_CLICINTIP_BASE:
        if ((eclic->clicintlist[id].trigger & 0x1) != 0) {
            if ((eclic->clicintip[id] == 0) && (value & 0x1) == 1) {
                eclic->clicintip[id] = 1;
                eclic_insert_pending_list(eclic, id);
            } else if ((eclic->clicintip[id] == 1) && (value & 0x1) == 0) {
                eclic->clicintip[id] = 0;
                eclic_remove_pending_list(eclic, id);
            }

        }
        nuclei_eclic_next_interrupt(eclic);
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

static void update_eclic_int_info(NucLeiECLICState *eclic, int irq)
{
    int level_width = (eclic->cliccfg >> 1) & 0xF; // cliccfg.nlbits
    if (level_width > CLICINTCTLBITS)
        level_width = CLICINTCTLBITS;
    int prio_width = CLICINTCTLBITS - level_width;

    if (level_width == 0) eclic->clicintlist[irq].level = 255;
    else
    	eclic->clicintlist[irq].level = (((eclic->clicintctl[irq] >> (8 - level_width)) & \
    	     ~((char)0x80 >> (8 - level_width))) << (8 - level_width)) | (0xff >> level_width);


    // TODO: implement priority decode logic when width > CLICINTCTLBITS or zeros
    if (prio_width == 0) eclic->clicintlist[irq].prio = 0;
    else
        eclic->clicintlist[irq].prio = (eclic->clicintctl[irq] >> (8 - level_width)) &
                ~(0x80 >> (8 - prio_width));

    eclic->clicintlist[irq].enable = eclic->clicintie[irq] & 0x1;
    // 0, level triggered; 2, rising edge; 3, falling edge
    eclic->clicintlist[irq].trigger = (eclic->clicintattr[irq] >> 1) & 0x3;

}

static void nuclei_eclic_next_interrupt(void *eclic_ptr)
{
    RISCVCPU *cpu = RISCV_CPU(qemu_get_cpu(0));
    NucLeiECLICState *eclic = (NucLeiECLICState *)eclic_ptr;
    ECLICPendingInterrupt *active;
    int shv;

    QLIST_FOREACH(active, &eclic->pending_list, next) {
         if(active->enable) {
             if (active->level >= eclic->mth) {

                shv = eclic->clicintattr[active->irq] & 0x1;
                eclic->active_count++;
                riscv_cpu_eclic_interrupt(cpu, (active->irq & 0xFFF) | (shv << 12) | (active->level << 13));
                return;
             }
        }
    }
    riscv_cpu_eclic_interrupt(cpu, -1);
}

void riscv_cpu_eclic_int_handler_start(void *eclic_ptr, int irq) {
    NucLeiECLICState *eclic = (NucLeiECLICState *)eclic_ptr;
    if ((eclic->clicintlist[irq].trigger & 0x1) != 0) {
        eclic->clicintip[irq] = 0;
        eclic_remove_pending_list(eclic, irq);
    }
    nuclei_eclic_next_interrupt(eclic);
}

static int level_compare(NucLeiECLICState *eclic, ECLICPendingInterrupt *irq1, ECLICPendingInterrupt *irq2) 
{
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

static void nuclei_eclic_update_intmth(NucLeiECLICState *eclic, int irq, int mth)
{
    eclic->mth = mth;
    nuclei_eclic_next_interrupt(eclic);
}

static void eclic_insert_pending_list(NucLeiECLICState *eclic, int irq) {
    ECLICPendingInterrupt *node;
    if (QLIST_EMPTY(&eclic->pending_list)) {
        QLIST_INSERT_HEAD(&eclic->pending_list, &eclic->clicintlist[irq], next);
    } else {
        QLIST_FOREACH(node, &eclic->pending_list, next) {
            if (level_compare(eclic, node, &eclic->clicintlist[irq])) {
                QLIST_INSERT_BEFORE(node, &eclic->clicintlist[irq], next);
                break;
            } else if (node->next.le_next == NULL) {
                QLIST_INSERT_AFTER(node, &eclic->clicintlist[irq], next);
                break;
            }
        }
    }
}

static void eclic_remove_pending_list(NucLeiECLICState *eclic, int irq) {
    QLIST_REMOVE(&eclic->clicintlist[irq], next);
}

static void nuclei_eclic_update_intip(NucLeiECLICState *eclic, int irq, int new_intip)
{
   
    int old_intip = eclic->clicintlist[irq].sig;
    int trigger = (eclic->clicintattr[irq] >> 1) & 0x3;
    if (((trigger == 0) && new_intip) ||
        ((trigger == 1) && !old_intip && new_intip) ||
        ((trigger == 3) && old_intip && !new_intip)) {
        eclic->clicintip[irq] = 1;
        eclic->clicintlist[irq].sig = new_intip;
        eclic_insert_pending_list(eclic, irq);
    }
    else {
        if (eclic->clicintip[irq])
            eclic_remove_pending_list(eclic, irq);
        eclic->clicintip[irq] = 0;
        eclic->clicintlist[irq].sig = new_intip;
    }

    nuclei_eclic_next_interrupt(eclic);
}

static void nuclei_eclic_update_intie(NucLeiECLICState *eclic, int irq, int new_intie)
{
    eclic->clicintie[irq] = new_intie;
    update_eclic_int_info(eclic, irq);
    nuclei_eclic_next_interrupt(eclic);
}

// TODO: intattr not supposed to be changed during runtime?
static void nuclei_eclic_update_intattr(NucLeiECLICState *eclic, int irq, int new_intattr)
{
    eclic->clicintattr[irq] = new_intattr;
    update_eclic_int_info(eclic, irq);
    nuclei_eclic_next_interrupt(eclic);
}

// TODO: intctl not supposed to be changed during runtime?
static void nuclei_eclic_update_intctl(NucLeiECLICState *eclic, int irq, int new_intctl)
{
    eclic->clicintctl[irq] = new_intctl;
    update_eclic_int_info(eclic, irq);
    nuclei_eclic_next_interrupt(eclic);
}

static void nuclei_eclic_realize(DeviceState *dev, Error **errp)
{
    NucLeiECLICState *eclic = NUCLEI_ECLIC(dev);
    int id;

    memory_region_init_io(&eclic->mmio, OBJECT(dev), &nuclei_eclic_ops, eclic,
                          TYPE_NUCLEI_ECLIC, eclic->aperture_size);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &eclic->mmio);

    eclic->clicintip = g_new0(uint8_t, eclic->num_sources);
    eclic->clicintlist = g_new0(ECLICPendingInterrupt, eclic->num_sources);
    eclic->clicintie = g_new0(uint8_t, eclic->num_sources);
    eclic->clicintattr = g_new0(uint8_t, eclic->num_sources);
    eclic->clicintctl = g_new0(uint8_t, eclic->num_sources);
    eclic->irqs = g_new0(qemu_irq, eclic->num_sources);
    QLIST_INIT(&eclic->pending_list);
    for (id = 0; id < eclic->num_sources; id++) {
        eclic->clicintlist[id].irq = id;
        update_eclic_int_info(eclic, id);
    }
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
