/*
 * QEMU RISC-V PLCT  stone machine interface
 *
 * Copyright (c) 2021 PLCT, lab.
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

#ifndef HW_PLCT_STONE_H
#define HW_PLCT_STONE_H

#include "hw/riscv/riscv_hart.h"
#include "hw/sysbus.h"
#include "hw/block/flash.h"
#include "qom/object.h"

#include "hw/riscv/sifive_cpu.h"
#include "hw/gpio/sifive_gpio.h"

#define STONE_CPUS_MAX 8
#define STONE_SOCKETS_MAX 8

#define TYPE_PLCT_STONE_MACHINE \
                            MACHINE_TYPE_NAME("plct_stone")

typedef struct PLCTStoneState PLCTStoneState;
DECLARE_INSTANCE_CHECKER(PLCTStoneState, PLCT_STONE_MACHINE,
                         TYPE_PLCT_STONE_MACHINE)

#define TYPE_RISCV_PLCT_MACHINE_SOC "riscv.plct.machine.soc"
#define RISCV_PLCT_MACHINE_SOC(obj) \
    OBJECT_CHECK(PlctMachineSoCState, (obj), TYPE_RISCV_PLCT_MACHINE_SOC)

#define TYPE_RISCV_PLCT_STONE_SOC "riscv.plct.stone.soc"
#define RISCV_PLCT_STONE_SOC(obj) \
    OBJECT_CHECK(PLCTStoneSoCState, (obj), TYPE_RISCV_PLCT_STONE_SOC)

typedef struct PLCTStoneSoCState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus[STONE_SOCKETS_MAX];
    DeviceState *plic[STONE_CPUS_MAX];
    DeviceState *mmio_plic;
    DeviceState *virtio_plic;
    DeviceState *pcie_plic;

} PLCTStoneSoCState;

struct PLCTStoneState {
    /*< private >*/
    MachineState parent;

    /*< public >*/
    PLCTStoneSoCState soc;

    void *fdt;
    int fdt_size;
};

typedef struct PlctMachineSoCState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
    DeviceState *plic;
    SIFIVEGPIOState gpio;
    MemoryRegion xip_mem;
    MemoryRegion mask_rom;
} PlctMachineSoCState;

typedef struct PlctMachineState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    PlctMachineSoCState soc;
    bool revb;
} PlctMachineState;

#define TYPE_RISCV_PLCT_MACHINE_MACHINE MACHINE_TYPE_NAME("plct_machine")
#define RISCV_PLCT_MACHINE_MACHINE(obj) \
    OBJECT_CHECK(PlctMachineState, (obj), TYPE_RISCV_PLCT_MACHINE_MACHINE)

enum {
    PLCT_STONE_DEBUG,
    PLCT_STONE_MROM,
    PLCT_STONE_TEST,
    PLCT_STONE_RTC,
    PLCT_STONE_CLINT,
    PLCT_STONE_PLIC,
    PLCT_STONE_UART0,
    PLCT_STONE_VIRTIO,
    PLCT_STONE_DRAM,
    PLCT_STONE_PCIE_MMIO,
    PLCT_STONE_PCIE_PIO,
    PLCT_STONE_PCIE_ECAM
};

enum {
    PLCT_MACHINE_DEV_DEBUG,
    PLCT_MACHINE_DEV_MROM,
    PLCT_MACHINE_DEV_OTP,
    PLCT_MACHINE_DEV_CLINT,
    PLCT_MACHINE_DEV_PLIC,
    PLCT_MACHINE_DEV_AON,
    PLCT_MACHINE_DEV_PRCI,
    PLCT_MACHINE_DEV_OTP_CTRL,
    PLCT_MACHINE_DEV_GPIO0,
    PLCT_MACHINE_DEV_UART0,
    PLCT_MACHINE_DEV_QSPI0,
    PLCT_MACHINE_DEV_PWM0,
    PLCT_MACHINE_DEV_UART1,
    PLCT_MACHINE_DEV_QSPI1,
    PLCT_MACHINE_DEV_PWM1,
    PLCT_MACHINE_DEV_QSPI2,
    PLCT_MACHINE_DEV_PWM2,
    PLCT_MACHINE_DEV_XIP,
    PLCT_MACHINE_DEV_DTIM
};

enum {
    UART0_IRQ = 10,
    RTC_IRQ = 11,
    VIRTIO_IRQ = 1, /* 1 to 8 */
    VIRTIO_COUNT = 8,
    PCIE_IRQ = 0x20, /* 32 to 35 */
    VIRTIO_NDEV = 0x35 /* Arbitrary maximum number of interrupts */
};

enum {
    PLCT_MACHINE_UART0_IRQ  = 3,
    PLCT_MACHINE_UART1_IRQ  = 4,
    PLCT_MACHINE_GPIO0_IRQ0 = 8
};

#define PLCT_STONE_PLIC_HART_CONFIG "MS"
#define PLCT_MACHINE_PLIC_HART_CONFIG "M"
#define PLCT_STONE_PLIC_NUM_SOURCES 127
#define PLCT_STONE_PLIC_NUM_PRIORITIES 7
#define PLCT_STONE_PLIC_PRIORITY_BASE 0x04
#define PLCT_STONE_PLIC_PENDING_BASE 0x1000
#define PLCT_STONE_PLIC_ENABLE_BASE 0x2000
#define PLCT_STONE_PLIC_ENABLE_STRIDE 0x80
#define PLCT_STONE_PLIC_CONTEXT_BASE 0x200000
#define PLCT_STONE_PLIC_CONTEXT_STRIDE 0x1000
#define PLCT_STONE_PLIC_SIZE(__num_context) \
    (PLCT_STONE_PLIC_CONTEXT_BASE + (__num_context) * PLCT_STONE_PLIC_CONTEXT_STRIDE)

#define FDT_PCI_ADDR_CELLS    3
#define FDT_PCI_INT_CELLS     1
#define FDT_PLIC_ADDR_CELLS   0
#define FDT_PLIC_INT_CELLS    1
#define FDT_INT_MAP_WIDTH     (FDT_PCI_ADDR_CELLS + FDT_PCI_INT_CELLS + 1 + \
                               FDT_PLIC_ADDR_CELLS + FDT_PLIC_INT_CELLS)


//PLCT N cpu For  MCU SDK  or  RTOS(no mmu)test
//PLCT U cpu For Linux test
#if defined(TARGET_RISCV32)
#define PLCT_N_CPU TYPE_RISCV_CPU_PLCT_N32
#define PLCT_U_CPU TYPE_RISCV_CPU_PLCT_U32
#elif defined(TARGET_RISCV64)
#define PLCT_N_CPU TYPE_RISCV_CPU_PLCT_N64
#define PLCT_U_CPU TYPE_RISCV_CPU_PLCT_U64
#endif

#endif
