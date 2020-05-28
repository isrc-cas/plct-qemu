/*
 *  NUCLEI TIMER (Timer Unit) interface
 *
 * Copyright (c) 2020 Nuclei, Inc.
 *
 * This provides a NUCLEI RISC-V TIMER device
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
#include "qemu/timer.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/nuclei_timer.h"
#include "hw/riscv/nuclei_eclic.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static void nuclei_timer_update_irq(NucLeiTIMERState *s)
{
    qemu_set_irq(s->timer_irq, 1);
}

static void nuclei_soft_update_irq(NucLeiTIMERState *s)
{
    bool enable = !!s->msip ;
    qemu_set_irq(s->soft_irq, enable);
}

static void nuclei_timer_update_compare(NucLeiTIMERState *s)
{
    nuclei_timer_update_irq(s);
}

static uint64_t nuclei_timer_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(opaque);
    CPUState *cpu = qemu_get_cpu(0);
    CPURISCVState *env = cpu ? cpu->env_ptr : NULL;
    uint64_t value = 0;

    switch (offset) {
    case NUCLEI_TIMER_REG_MTIMELO:
        value= qemu_clock_get_us(QEMU_CLOCK_VIRTUAL);
        value >>= 8 * (offset - NUCLEI_TIMER_REG_MTIMELO);
        value &= UINT32_MAX;
        break;
    case NUCLEI_TIMER_REG_MTIMEHI:
        value= qemu_clock_get_us(QEMU_CLOCK_VIRTUAL);
        value >>= 8 * (offset - NUCLEI_TIMER_REG_MTIMELO);
        value &= UINT32_MAX;
        break;
    case NUCLEI_TIMER_REG_MTIMECMPLO:
        s->mtimecmp_lo = (env->mtimecmp) & 0xFFFFFFFF;
        value = s->mtimecmp_lo;
        break;
    case NUCLEI_TIMER_REG_MTIMECMPHI:
        s->mtimecmp_hi = (env->mtimecmp >> 32) & 0xFFFFFFFF;
        value = s->mtimecmp_hi;
        break;
    case NUCLEI_TIMER_REG_MSFTRST:
        break;
    case NUCLEI_TIMER_REG_MSTOP:
        value = s->mstop;
        break;
    case NUCLEI_TIMER_REG_MSIP:
        value = s->msip;
        break;
    default:
        break;
    }

    return value;
}

static void nuclei_timer_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(opaque);
    CPUState *cpu = qemu_get_cpu(0);
    CPURISCVState *env = cpu ? cpu->env_ptr : NULL;

    qemu_log(">>nuclei_timer_write\n");

    value = value & 0xFFFFFFFF;
    switch (offset) {
    case NUCLEI_TIMER_REG_MTIMELO:
        s->mtime_lo = value;
        env->mtimer->expire_time |= (value &0xFFFFFFFF);
        break;
    case NUCLEI_TIMER_REG_MTIMEHI:
        s->mtime_hi = value;
        env->mtimer->expire_time |= ((value << 32)&0xFFFFFFFF);
        break;
    case NUCLEI_TIMER_REG_MTIMECMPLO:
        s->mtimecmp_lo = value;
        env->mtimecmp  |= (value &0xFFFFFFFF); 
        nuclei_timer_update_compare(s);
        break;
    case NUCLEI_TIMER_REG_MTIMECMPHI:
        s->mtimecmp_hi = value;
        env->mtimecmp  |= ((value << 32)&0xFFFFFFFF);
        nuclei_timer_update_compare(s);
        break;
    case NUCLEI_TIMER_REG_MSFTRST:
        break;
    case NUCLEI_TIMER_REG_MSTOP:
        s->mstop = value;
        break;
    case NUCLEI_TIMER_REG_MSIP:
        s->msip = value;
        nuclei_soft_update_irq(s);
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

static void nuclei_timer_reset(DeviceState *dev)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(dev);
    s->mtime_lo = 0x0;
    s->mtime_hi = 0x0;
    s->mtimecmp_lo = 0xFFFFFFFF;
    s->mtimecmp_hi = 0xFFFFFFFF;
    s->mstop = 0x0;
    s->mstop = 0x0;
}

static void nuclei_timer_realize(DeviceState *dev, Error **errp)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_timer_ops,
                          s,TYPE_NUCLEI_TIMER, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->timer_irq);
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->soft_irq);
}

static void nuclei_timer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_timer_realize;
    dc->reset = nuclei_timer_reset;
    dc->desc = "NucLei Timer";
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
