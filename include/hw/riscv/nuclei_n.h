/*
 * Nuclei machine interface
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


#ifndef HW_RISCV_NUCLEI_HBIRD_H
#define HW_RISCV_NUCLEI_HBIRD_H

#include "hw/riscv/riscv_hart.h"
#include "hw/char/nuclei_uart.h"
#include "hw/intc/nuclei_systimer.h"
#include "hw/gpio/sifive_gpio.h"
#include "hw/intc/nuclei_eclic.h"
#include "hw/sysbus.h"

#define TYPE_NUCLEI_HBIRD_SOC "riscv.nuclei.hbird.soc"
#define RISCV_NUCLEI_HBIRD_SOC(obj) \
    OBJECT_CHECK(NucleiHBSoCState, (obj), TYPE_NUCLEI_HBIRD_SOC)


typedef struct NucleiHBSoCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;

    DeviceState *eclic;
    MemoryRegion ilm;
    MemoryRegion dlm;
    MemoryRegion internal_rom;
    MemoryRegion xip_mem;

    DeviceState *timer;
    // NucLeiSYSTIMERState *timer;
    NucLeiUARTState uart;
    SIFIVEGPIOState gpio;

} NucleiHBSoCState;

typedef struct {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    NucleiHBSoCState soc;
} NucleiHBState;

enum {
    HBIRD_DEBUG,
    HBIRD_ROM,
    HBIRD_TIMER,
    HBIRD_ECLIC,
    HBIRD_GPIO,
    HBIRD_UART0,
    HBIRD_QSPI0,
    HBIRD_PWM0,
    HBIRD_UART1,
    HBIRD_QSPI1,
    HBIRD_PWM1,
    HBIRD_QSPI2,
    HBIRD_PWM2,
    HBIRD_XIP,
    HBIRD_ILM,
    HBIRD_DLM
};

enum {
    HBIRD_SOC_INT19_IRQn           = 19,                /*!< Device Interrupt */
    HBIRD_SOC_INT20_IRQn           = 20,                /*!< Device Interrupt */
    HBIRD_SOC_INT21_IRQn           = 21,                /*!< Device Interrupt */
    HBIRD_SOC_INT22_IRQn           = 22,                /*!< Device Interrupt */
    HBIRD_SOC_INT23_IRQn           = 23,                /*!< Device Interrupt */
    HBIRD_SOC_INT24_IRQn           = 24,                /*!< Device Interrupt */
    HBIRD_SOC_INT25_IRQn           = 25,                /*!< Device Interrupt */
    HBIRD_SOC_INT26_IRQn           = 26,                /*!< Device Interrupt */
    HBIRD_SOC_INT27_IRQn           = 27,                /*!< Device Interrupt */
    HBIRD_SOC_INT28_IRQn           = 28,                /*!< Device Interrupt */
    HBIRD_SOC_INT29_IRQn           = 29,                /*!< Device Interrupt */
    HBIRD_SOC_INT30_IRQn           = 30,                /*!< Device Interrupt */
    HBIRD_SOC_INT31_IRQn           = 31,                /*!< Device Interrupt */
    HBIRD_SOC_INT32_IRQn           = 32,                /*!< Device Interrupt */
    HBIRD_SOC_INT33_IRQn           = 33,                /*!< Device Interrupt */
    HBIRD_SOC_INT34_IRQn           = 34,                /*!< Device Interrupt */
    HBIRD_SOC_INT35_IRQn           = 35,                /*!< Device Interrupt */
    HBIRD_SOC_INT36_IRQn           = 36,                /*!< Device Interrupt */
    HBIRD_SOC_INT37_IRQn           = 37,                /*!< Device Interrupt */
    HBIRD_SOC_INT38_IRQn           = 38,                /*!< Device Interrupt */
    HBIRD_SOC_INT39_IRQn           = 39,                /*!< Device Interrupt */
    HBIRD_SOC_INT40_IRQn           = 40,                /*!< Device Interrupt */
    HBIRD_SOC_INT41_IRQn           = 41,                /*!< Device Interrupt */
    HBIRD_SOC_INT42_IRQn           = 42,                /*!< Device Interrupt */
    HBIRD_SOC_INT43_IRQn           = 43,                /*!< Device Interrupt */
    HBIRD_SOC_INT44_IRQn           = 44,                /*!< Device Interrupt */
    HBIRD_SOC_INT45_IRQn           = 45,                /*!< Device Interrupt */
    HBIRD_SOC_INT46_IRQn           = 46,                /*!< Device Interrupt */
    HBIRD_SOC_INT47_IRQn           = 47,                /*!< Device Interrupt */
    HBIRD_SOC_INT48_IRQn           = 48,                /*!< Device Interrupt */
    HBIRD_SOC_INT49_IRQn           = 49,                /*!< Device Interrupt */
    HBIRD_SOC_INT50_IRQn           = 50,                /*!< Device Interrupt */
    HBIRD_SOC_INT_MAX,
};

#define NUCLEI_CPU TYPE_RISCV_CPU_NUCLEI_N307FD

#endif