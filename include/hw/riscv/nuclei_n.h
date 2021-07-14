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
#include "hw/intc/nuclei_systimer.h"
#include "hw/char/nuclei_uart.h"

#define TYPE_NUCLEI_N_SOC "riscv.nuclei.n.soc"
#define NUCLEI_N_SOC(obj) \
    OBJECT_CHECK(NucLeiNSoCState, (obj), TYPE_NUCLEI_N_SOC)

typedef struct NucLeiNSoCState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;

    MemoryRegion internal_rom;
    MemoryRegion ilm;
    MemoryRegion dlm;
    MemoryRegion xip_mem;

    NucLeiSYSTIMERState timer;
    NucLeiUARTState uart0;
    NucLeiUARTState uart1;

    DeviceState *eclic;

} NucLeiNSoCState;

#define TYPE_NUCLEI_MCU_FPGA_MACHINE MACHINE_TYPE_NAME("mcu_200t")
#define MCU_FPGA_MACHINE(obj) \
    OBJECT_CHECK(NucLeiNState, (obj), TYPE_NUCLEI_MCU_FPGA_MACHINE)

typedef struct NucLeiNState {
    /*< private >*/
   MachineState parent_obj;

    /*< public >*/
    NucLeiNSoCState soc;

    uint32_t msel;

} NucLeiNState;

enum
{
    MSEL_ILM = 1,
    MSEL_FLASH = 2,
    MSEL_FLASHXIP = 3,
    MSEL_DDR = 4
};

enum
{
    NUCLEI_N_DEBUG,
    NUCLEI_N_ROM,
    NUCLEI_N_TIMER,
    NUCLEI_N_ECLIC,
    NUCLEI_N_GPIO,
    NUCLEI_N_UART0,
    NUCLEI_N_QSPI0,
    NUCLEI_N_PWM0,
    NUCLEI_N_UART1,
    NUCLEI_N_QSPI1,
    NUCLEI_N_PWM1,
    NUCLEI_N_QSPI2,
    NUCLEI_N_PWM2,
    NUCLEI_N_XIP,
    NUCLEI_N_DRAM,
    NUCLEI_N_ILM,
    NUCLEI_N_DLM
};

#if defined(TARGET_RISCV32)
#define NUCLEI_N_CPU TYPE_RISCV_CPU_NUCLEI_N600
#elif defined(TARGET_RISCV64)
#define NUCLEI_N_CPU TYPE_RISCV_CPU_NUCLEI_NX600
#endif

#endif