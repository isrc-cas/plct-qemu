/*
 *  GD32VF103 WDT interface
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
#ifndef HW_WDT_GD32VF103_H
#define HW_WDT_GD32VF103_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_GD32VF103_FWDGT "gd32vf103-fwdgt"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103FWDGTState, GD32VF103_FWDGT)
#define TYPE_GD32VF103_WWDGT "gd32vf103-wwdgt"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103WWDGTState, GD32VF103_WWDGT)

#define FWDGT_REG_FWDGT_CTL   0x00
#define FWDGT_REG_FWDGT_PSC   0x04
#define FWDGT_REG_FWDGT_RLD   0x08
#define FWDGT_REG_FWDGT_STAT  0x0C

typedef struct GD32VF103FWDGTState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t fwdgt_ctl;
    uint32_t fwdgt_psc;
    uint32_t fwdgt_rld;
    uint32_t fwdgt_stat;
} GD32VF103FWDGTState;

#define WWDGT_REG_WWDGT_CTL   0x00
#define WWDGT_REG_WWDGT_CFG    0x04
#define WWDGT_REG_WWDGT_STAT   0x08

typedef struct GD32VF103WWDGTState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t wwdgt_ctl;
    uint32_t wwdgt_cfg;
    uint32_t wwdgt_stat;

} GD32VF103WWDGTState;

#endif