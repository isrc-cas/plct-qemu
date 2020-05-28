/*
<<<<<<< HEAD
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
=======
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

>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/timer.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/nuclei_timer.h"
<<<<<<< HEAD
=======
#include "hw/riscv/nuclei_eclic.h"
>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
#include "hw/registerfields.h"
#include "migration/vmstate.h"
#include "trace.h"

<<<<<<< HEAD
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

=======
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
>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
}

static uint64_t nuclei_timer_read(void *opaque, hwaddr offset,
                                    unsigned size)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(opaque);
<<<<<<< HEAD
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
=======
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
>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
        break;
    default:
        break;
    }

<<<<<<< HEAD
    return (value & 0xFFFFFFFF);
=======
    return value;
>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
}

static void nuclei_timer_write(void *opaque, hwaddr offset,
                                 uint64_t value, unsigned size)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(opaque);
<<<<<<< HEAD

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
=======
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
>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
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

<<<<<<< HEAD
<<<<<<< HEAD
=======
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

>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
=======


>>>>>>> 796be3c83e... 323
static void nuclei_timer_realize(DeviceState *dev, Error **errp)
{
    NucLeiTIMERState *s = NUCLEI_TIMER(dev);

    memory_region_init_io(&s->iomem, OBJECT(dev), &nuclei_timer_ops,
                          s,TYPE_NUCLEI_TIMER, 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
<<<<<<< HEAD
=======
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->timer_irq);
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->soft_irq);
>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
}

static void nuclei_timer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = nuclei_timer_realize;
    dc->reset = nuclei_timer_reset;
<<<<<<< HEAD
    dc->desc = "NucLei HW TIMER";
=======
    dc->desc = "NucLei Timer";
>>>>>>> a37d2e24e1... Support Nuclei HummingBird RISC-V SoC emulation:
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
