/*
 * NUCLEI ECLIC (Enhanced Core Local Interrupt Controller) interface
 *
 * Copyright (c) 2020 Nuclei, Inc.
 *
 * This provides a NUCLEI RISC-V ECLIC device
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

#ifndef HW_NUCLEI_ECLIC_H
#define HW_NUCLEI_ECLIC_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_ECLIC "riscv.nuclei.eclic"

#define  INTERRUPT_SOURCE_MIN_ID         (18)
#define  INTERRUPT_SOURCE_MAX_ID         (4096)

#define NUCLEI_ECLIC(obj) \
    OBJECT_CHECK(NucLeiECLICState, (obj), TYPE_NUCLEI_ECLIC)

typedef struct ECLICActiveInterrupt {
    uint8_t clicintctl;
    uint16_t irq;
} ECLICActiveInterrupt;

#define NUCLEI_ECLIC_REG_CLICCFG   0x0000
#define NUCLEI_ECLIC_REG_CLICINFO   0x0004
#define NUCLEI_ECLIC_REG_MTH   0x000b
#define NUCLEI_ECLIC_REG_CLICINTIP_BASE   0x1000
#define NUCLEI_ECLIC_REG_CLICINTIE_BASE   0x1001
#define NUCLEI_ECLIC_REG_CLICINTATTR_BASE   0x1002
#define NUCLEI_ECLIC_REG_CLICINTCTL_BASE   0x1003

typedef struct NucLeiECLICState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion mmio;

    uint32_t num_addrs;
    uint32_t *source_priority;
    uint32_t *target_priority;
    uint32_t *pending;
    uint32_t *claimed;
    uint32_t *enable;

   uint32_t num_sources;  /* 4-1024 */

    /* config */
    uint32_t sources_id;
    uint8_t  cliccfg;
    uint32_t clicinfo;
    uint8_t  mth;
    uint8_t  *clicintip;
    uint8_t  *clicintie;
    uint8_t  *clicintattr;
    uint8_t  *clicintctl;

     uint32_t aperture_size;

    ECLICActiveInterrupt *active_list;
    size_t active_count;

    /* CLIC IRQ handlers */
    qemu_irq *irqs;

} NucLeiECLICState;

DeviceState *nuclei_eclic_create(hwaddr addr, uint32_t aperture_size, uint32_t num_sources);

qemu_irq nuclei_eclic_get_irq(DeviceState *dev,  int irq);

#endif
