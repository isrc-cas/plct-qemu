/*
 *  NUCLEI TIMER (Timer Unit) interface
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
#include "hw/riscv/nuclei_systimer.h"
#include "hw/riscv/nuclei_eclic.h"
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

static uint64_t cpu_riscv_read_rtc(uint64_t timebase_freq)
{
    return muldiv64(qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL),
        timebase_freq, NANOSECONDS_PER_SECOND);
}

static void nuclei_timer_update_compare(NucLeiSYSTIMERState *s)
{
    CPUState *cpu = qemu_get_cpu(0);
    CPURISCVState *env = cpu ? cpu->env_ptr : NULL;
    uint64_t cmp, real_time;
    int64_t diff;

    real_time =  s->mtime_lo | ((uint64_t)s->mtime_hi << 32);

    env->mtimer->expire_time  = real_time;

    cmp = (uint64_t)s->mtimecmp_lo | ((uint64_t)s->mtimecmp_hi <<32);
    env->mtimecmp =  cmp;

    diff = cmp - real_time;

    if ( real_time >= cmp) {
        qemu_set_irq(*(s->timer_irq), 1);
    }
    else {
    	qemu_set_irq(*(s->timer_irq), 0);

        if (s->mtimecmp_hi != 0xffffffff) {
            // set up future timer interrupt
            uint64_t next_ns = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) +
                muldiv64(diff, NANOSECONDS_PER_SECOND, s->timebase_freq);
            timer_mod(env->mtimer, next_ns);
	    }
    }
}

static void nuclei_timer_reset(DeviceState *dev)
{
    NucLeiSYSTIMERState *s = NUCLEI_SYSTIMER(dev);
    s->mtime_lo = 0x0;
    s->mtime_hi = 0x0;
    s->mtimecmp_lo = 0xFFFFFFFF;
    s->mtimecmp_hi = 0xFFFFFFFF;
    s->mstop = 0x0;
    s->mstop = 0x0;
}

static uint64_t nuclei_timer_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiSYSTIMERState *s = NUCLEI_SYSTIMER(opaque);
    CPUState *cpu = qemu_get_cpu(0);
    CPURISCVState *env = cpu ? cpu->env_ptr : NULL;
    uint64_t value = 0;

    switch (offset) {
    case NUCLEI_SYSTIMER_REG_MTIMELO:
        value = cpu_riscv_read_rtc(s->timebase_freq);
        s->mtime_lo =  value & 0xffffffff;
        s->mtime_hi = (value >> 32) & 0xffffffff;
        value = s->mtime_lo;
        break;
    case NUCLEI_SYSTIMER_REG_MTIMEHI:
        value =  s->mtime_hi;
        break;
    case NUCLEI_SYSTIMER_REG_MTIMECMPLO:
        s->mtimecmp_lo = (env->mtimecmp) & 0xFFFFFFFF;
        value = s->mtimecmp_lo;
        break;
    case NUCLEI_SYSTIMER_REG_MTIMECMPHI:
        s->mtimecmp_hi = (env->mtimecmp >> 32) & 0xFFFFFFFF;
        value = s->mtimecmp_hi;
        break;
    case NUCLEI_SYSTIMER_REG_MSFTRST:
        break;
    case NUCLEI_SYSTIMER_REG_MSTOP:
        value = s->mstop;
        break;
    case NUCLEI_SYSTIMER_REG_MSIP:
        value = s->msip;
        break;
    default:
        break;
    }

    return (value & 0xFFFFFFFF);
}

static void nuclei_timer_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiSYSTIMERState *s = NUCLEI_SYSTIMER(opaque);
    CPUState *cpu = qemu_get_cpu(0);
    CPURISCVState *env = cpu ? cpu->env_ptr : NULL;

    value = value & 0xFFFFFFFF;
    switch (offset) {
    case NUCLEI_SYSTIMER_REG_MTIMELO:
        s->mtime_lo = value;
        env->mtimer->expire_time |= (value &0xFFFFFFFF);
        break;
    case NUCLEI_SYSTIMER_REG_MTIMEHI:
        s->mtime_hi = value;
        env->mtimer->expire_time |= ((value << 32)&0xFFFFFFFF);
        break;
    case NUCLEI_SYSTIMER_REG_MTIMECMPLO:
        s->mtimecmp_lo = value;
        s->mtimecmp_hi = 0xFFFFFFFF;
        env->mtimecmp  |= (value &0xFFFFFFFF); 
        nuclei_timer_update_compare(s);
        break;
    case NUCLEI_SYSTIMER_REG_MTIMECMPHI:
        s->mtimecmp_hi = value;
        env->mtimecmp  |= ((value << 32)&0xFFFFFFFF);
        nuclei_timer_update_compare(s);
        break;
    case NUCLEI_SYSTIMER_REG_MSFTRST:
	if (!(value & 0x80000000) == 0)
            nuclei_timer_reset((DeviceState *)s);
        break;
    case NUCLEI_SYSTIMER_REG_MSTOP:
        s->mstop = value;
        break;
    case NUCLEI_SYSTIMER_REG_MSIP:
        s->msip = value;
        if ((s->msip & 0x1) == 1) {
            qemu_set_irq(*(s->soft_irq), 1);
        }else{
            //riscv_cpu_eclic_interrupt(riscv_cpu, -1);
            qemu_set_irq(*(s->soft_irq), 0);
        }

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
    NucLeiSYSTIMERState *s = NUCLEI_SYSTIMER(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_timer_ops,
                          s,TYPE_NUCLEI_SYSTIMER, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void nuclei_timer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_timer_realize;
    dc->reset = nuclei_timer_reset;
    dc->desc = "NucLei Timer";
}

static const TypeInfo nuclei_timer_info = {
    .name = TYPE_NUCLEI_SYSTIMER,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiSYSTIMERState),
    .class_init = nuclei_timer_class_init,
};

static void nuclei_timer_register_types(void)
{
    type_register_static(&nuclei_timer_info);
}
type_init(nuclei_timer_register_types);
