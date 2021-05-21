/*
 * Nuclei N series  SOC machine interface
 *
 * Copyright (c) 2020 Gao ZhiYuan <alapha23@gmail.com>
 * Copyright (c) 2020-2021 PLCT Lab.All rights reserved.
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

#ifndef HW_RISCV_NUCLEI_N_H
#define HW_RISCV_NUCLEI_N_H

#include "hw/sysbus.h"
#include "hw/riscv/riscv_hart.h"

#define TYPE_NUCLEI_N_SOC "riscv.nuclei.n.soc"
#define NUCLEI_N_SOC(obj) \
    OBJECT_CHECK(NucLeiNSoCState, (obj), TYPE_NUCLEI_N_SOC)

typedef struct NucLeiNSoCState {
    /*< private >*/
    DeviceState parent_obj;

    RISCVHartArrayState cpus;

    /*< public >*/

} NucLeiNSoCState;

#define TYPE_NUCLEI_MCU_FPGA_MACHINE MACHINE_TYPE_NAME("mcu_200t")
#define MCU_FPGA_MACHINE(obj) \
    OBJECT_CHECK(NucLeiNState, (obj), TYPE_NUCLEI_MCU_FPGA_MACHINE)

typedef struct NucLeiNState {
    /*< private >*/
   MachineState parent_obj;

    /*< public >*/
    NucLeiNSoCState soc;

} NucLeiNState;

#if defined(TARGET_RISCV32)
#define NUCLEI_N_CPU TYPE_RISCV_CPU_NUCLEI_N600
#elif defined(TARGET_RISCV64)
#define NUCLEI_N_CPU TYPE_RISCV_CPU_NUCLEI_NX600
#endif

#endif