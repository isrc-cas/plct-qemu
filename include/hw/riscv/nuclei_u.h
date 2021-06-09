/*
 * Nuclei U series  SOC machine interface
 *
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

#ifndef HW_NUCLEI_U_H
#define HW_NUCLEI_U_H

#include "hw/sysbus.h"
#include "hw/riscv/riscv_hart.h"

#define TYPE_NUCLEI_U_SOC "riscv.nuclei.u.soc"
#define NUCLEI_U_SOC(obj) \
    OBJECT_CHECK(NucLeiUSoCState, (obj), TYPE_NUCLEI_U_SOC)

typedef struct NucLeiUSoCState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;

} NucLeiUSoCState;

#define TYPE_NUCLEI_DDR_FPGA_MACHINE MACHINE_TYPE_NAME("ddr_200t")
#define DDR_FPGA_MACHINE(obj) \
    OBJECT_CHECK(NucLeiUState, (obj), TYPE_NUCLEI_DDR_FPGA_MACHINE)

typedef struct NucLeiUState {
    /*< private >*/
   MachineState parent_obj;

    /*< public >*/
    NucLeiUSoCState soc;

    void *fdt;
    int fdt_size;

    uint32_t msel;

} NucLeiUState;

enum
{
    MSEL_FLASH = 1,
    MSEL_SD = 2
};
enum
{
    NUCLEI_U_MROM,
    NUCLEI_U_TIMER,
    NUCLEI_U_CLINT,
    NUCLEI_U_PLIC,
    NUCLEI_U_UART0,
    NUCLEI_U_UART1,
    NUCLEI_U_GPIO,
    NUCLEI_U_FLASH0,
    NUCLEI_U_DRAM,
    NUCLEI_U_SPI0,
    NUCLEI_U_SPI2
};

#define NUCLEI_U_CPU TYPE_RISCV_CPU_NUCLEI_UX600

#endif