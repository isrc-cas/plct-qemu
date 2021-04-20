/*
 * SiFive U series machine interface
 *
 * Copyright (c) 2017 SiFive, Inc.
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

#ifndef HW_NUCLEI_U_H
#define HW_NUCLEI_U_H

#include "hw/dma/sifive_pdma.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/sifive_cpu.h"
#include "hw/gpio/sifive_gpio.h"
#include "hw/misc/sifive_u_otp.h"
#include "hw/misc/sifive_u_prci.h"
#include "hw/intc/nuclei_systimer.h"
#include "hw/sd/sd.h"
#include "hw/ssi/sifive_spi.h"

#define TYPE_RISCV_NUCLEI_U_SOC "riscv.nuclei.u.soc"
#define RISCV_NUCLEI_U_SOC(obj) \
    OBJECT_CHECK(NucLeiUSoCState, (obj), TYPE_RISCV_NUCLEI_U_SOC)

#define NUCLEI_NUM_SPIS       3

typedef struct NucLeiUSoCState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    CPUClusterState e_cluster;
    CPUClusterState u_cluster;
    RISCVHartArrayState e_cpus;
    RISCVHartArrayState u_cpus;
    DeviceState *plic;
    SIFIVEGPIOState gpio;
    // SiFiveSPIState spi[NUCLEI_NUM_SPIS];
    SiFivePDMAState dma;
    NucLeiSYSTIMERState timer;
    // DeviceState *timer;
    SiFiveSPIState spi0;
    SiFiveSPIState spi2;

    uint32_t serial;
    char *cpu_type;
} NucLeiUSoCState;

#define TYPE_RISCV_NUCLEI_U_MACHINE MACHINE_TYPE_NAME("nuclei_u")
#define RISCV_NUCLEI_U_MACHINE(obj) \
    OBJECT_CHECK(NucLeiUState, (obj), TYPE_RISCV_NUCLEI_U_MACHINE)

typedef struct NucLeiUState {
    /*< private >*/
    MachineState parent_obj;

    /*< public >*/
    NucLeiUSoCState soc;

    void *fdt;
    int fdt_size;

    bool start_in_flash;
    uint32_t msel;
    uint32_t serial;
} NucLeiUState;

enum {
    NUCLEI_U_DEV_MROM,
    NUCLEI_U_DEV_TIMER,
    NUCLEI_U_DEV_CLINT,
    NUCLEI_U_DEV_PLIC,
    NUCLEI_U_DEV_UART0,
    NUCLEI_U_DEV_UART1,
    NUCLEI_U_DEV_GPIO,
    NUCLEI_U_DEV_FLASH0,
    NUCLEI_U_DEV_DRAM,
    NUCLEI_U_SPI0,
    NUCLEI_U_SPI2
};

enum {
    NUCLEI_U_UART0_IRQ = 4,
    NUCLEI_U_UART1_IRQ = 5,
    NUCLEI_U_GPIO_IRQ0 = 7,
    NUCLEI_U_GPIO_IRQ1 = 8,
    NUCLEI_U_GPIO_IRQ2 = 9,
    NUCLEI_U_GPIO_IRQ3 = 10,
    NUCLEI_U_GPIO_IRQ4 = 11,
    NUCLEI_U_GPIO_IRQ5 = 12,
    NUCLEI_U_GPIO_IRQ6 = 13,
    NUCLEI_U_GPIO_IRQ7 = 14,
    NUCLEI_U_GPIO_IRQ8 = 15,
    NUCLEI_U_GPIO_IRQ9 = 16,
    NUCLEI_U_GPIO_IRQ10 = 17,
    NUCLEI_U_GPIO_IRQ11 = 18,
    NUCLEI_U_GPIO_IRQ12 = 19,
    NUCLEI_U_GPIO_IRQ13 = 20,
    NUCLEI_U_GPIO_IRQ14 = 21,
    NUCLEI_U_GPIO_IRQ15 = 22,
    NUCLEI_U_SPI0_IRQ = 34,
    NUCLEI_U_SPI2_IRQ = 36
};

enum {
    NUCLEI_U_HFCLK_FREQ = 8000000,
    NUCLEI_U_RTCCLK_FREQ = 1000000
};

enum {
    MSEL_MEMMAP_QSPI0_FLASH = 1,
    MSEL_L2LIM_QSPI0_FLASH = 6,
    MSEL_L2LIM_QSPI2_SD = 11
};

enum {
    NUCLEI_CLINT_TIMEBASE_FREQ = 32768
};

#define NUCLEI_U_MANAGEMENT_CPU_COUNT   1
#define NUCLEI_U_COMPUTE_CPU_COUNT      1

#define NUCLEI_U_PLIC_HART_CONFIG "MS"
#define NUCLEI_U_PLIC_NUM_SOURCES 54
#define NUCLEI_U_PLIC_NUM_PRIORITIES 7
#define NUCLEI_U_PLIC_PRIORITY_BASE 0x04
#define NUCLEI_U_PLIC_PENDING_BASE 0x1000
#define NUCLEI_U_PLIC_ENABLE_BASE 0x2000
#define NUCLEI_U_PLIC_ENABLE_STRIDE 0x80
#define NUCLEI_U_PLIC_CONTEXT_BASE 0x200000
#define NUCLEI_U_PLIC_CONTEXT_STRIDE 0x1000

#if defined(TARGET_RISCV32)
#define NUCLEI_U_CPU TYPE_RISCV_CPU_NUCLEI_UX600FD
#elif defined(TARGET_RISCV64)
#define NUCLEI_U_CPU TYPE_RISCV_CPU_NUCLEI_UX600FD
#endif

#endif