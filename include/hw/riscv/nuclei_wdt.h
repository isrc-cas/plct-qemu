/*
 *  NUCLEI WDT interface
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


#ifndef HW_NUCLEI_WDT_H
#define HW_NUCLEI_WDT_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_FWDGT "riscv.nuclei.fwdgt"
#define TYPE_NUCLEI_WWDGT "riscv.nuclei.wwdgt"

#define NUCLEI_FWDGT(obj) \
    OBJECT_CHECK(NucLeiFWDGTState, (obj), TYPE_NUCLEI_FWDGT)

#define NUCLEI_WWDGT(obj) \
    OBJECT_CHECK(NucLeiWWDGTState, (obj), TYPE_NUCLEI_WWDGT)

#define NUCLEI_FWDGT_REG_FWDGT_CTL   0x00
#define NUCLEI_FWDGT_REG_FWDGT_PSC   0x04
#define NUCLEI_FWDGT_REG_FWDGT_RLD   0x08
#define NUCLEI_FWDGT_REG_FWDGT_STAT  0x0C

typedef struct NucLeiFWDGTState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t fwdgt_ctl;
    uint32_t fwdgt_psc;
    uint32_t fwdgt_rld;
    uint32_t fwdgt_stat;
} NucLeiFWDGTState;

#define NUCLEI_WWDGT_REG_WWDGT_CTL   0x00
#define NUCLEI_WWDGT_REG_WWDGT_CFG    0x04
#define NUCLEI_WWDGT_REG_WWDGT_STAT   0x08

typedef struct NucLeiWWDGTState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;

    uint32_t wwdgt_ctl;
    uint32_t wwdgt_cfg;
    uint32_t wwdgt_stat;

} NucLeiWWDGTState;

#endif
