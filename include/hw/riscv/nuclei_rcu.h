/*
 *  NUCLEI RCU (RCU Unit) interface
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


#ifndef HW_NUCLEI_RCU_H
#define HW_NUCLEI_RCU_H

#include "hw/sysbus.h"
#include "hw/irq.h"

#define TYPE_NUCLEI_RCU "riscv.nuclei.rcu"

#define NUCLEI_RCU(obj) \
    OBJECT_CHECK(NucLeiRCUState, (obj), TYPE_NUCLEI_RCU)

#define RCU_CTL_HXTALEN                 BIT(16)
#define RCU_CTL_HXTALSTB                BIT(17) 
#define RCU_CTL_PLLEN                   BIT(24)
#define RCU_CTL_PLLSTB                  BIT(25)
#define RCU_CTL_PLL1EN                  BIT(26)
#define RCU_CTL_PLL1STB                 BIT(27) 
#define RCU_CTL_PLL2EN                  BIT(28) 
#define RCU_CTL_PLL2STB                 BIT(29)

#define NUCLEI_RCU_REG_RCU_CTL   0x00
#define NUCLEI_RCU_REG_RCU_CFG0    0x04
#define NUCLEI_RCU_REG_RCU_INT   0x08
#define NUCLEI_RCU_REG_RCU_APB2RST   0x0C
#define NUCLEI_RCU_REG_RCU_APB1RST   0x10
#define NUCLEI_RCU_REG_RCU_AHBEN   0x14
#define NUCLEI_RCU_REG_RCU_APB2EN   0x18
#define NUCLEI_RCU_REG_RCU_APB1EN   0x1C
#define NUCLEI_RCU_REG_RCU_BDCTL   0x20
#define NUCLEI_RCU_REG_RCU_RSTSCK   0x24
#define NUCLEI_RCU_REG_RCU_AHBRST   0x28
#define NUCLEI_RCU_REG_RCU_CFG1   0x2C
#define NUCLEI_RCU_REG_RCU_DSV   0x34

typedef struct NucLeiRCUState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    qemu_irq rcu_irq;

    uint32_t rcu_ctl;
    uint32_t rcu_cfg0;
    uint32_t rcu_int;
    uint32_t rcu_apb2rst;
    uint32_t rcu_apb1rst;
    uint32_t rcu_ahben;
    uint32_t rcu_apb2en;
    uint32_t rcu_apb1en;
    uint32_t rcu_bdctl;
    uint32_t rcu_rstsck;
    uint32_t rcu_ahbrst;
    uint32_t rcu_cfg1;
    uint32_t rcu_dsv;

} NucLeiRCUState;

#endif
