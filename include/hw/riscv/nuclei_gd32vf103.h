/*
 * Nuclei machine interface
 *
 * Copyright (c) 2020Nuclei Limited. All rights reserved.
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


#ifndef HW_RISCV_NUCLEI_H
#define HW_RISCV_NUCLEI_H

#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/nuclei_uart.h"
#include "hw/riscv/nuclei_systimer.h"
#include "hw/riscv/nuclei_gpio.h"
#include "hw/riscv/nuclei_rcu.h"
#include "hw/riscv/nuclei_eclic.h"
#include "hw/sysbus.h"

#define TYPE_NUCLEI_GD32VF103_SOC "riscv.nuclei.gd32vf103.soc"
#define RISCV_NUCLEI_GD32VF103_SOC(obj) \
    OBJECT_CHECK(NucleiGDSoCState, (obj), TYPE_NUCLEI_GD32VF103_SOC)

typedef struct NucleiGDSoCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
    NucLeiECLICState *eclic;
    
     MemoryRegion internal_rom;
    MemoryRegion main_flash;
    MemoryRegion boot_loader;
    MemoryRegion ob;
    MemoryRegion xip_mem;
    MemoryRegion sram;

    NucLeiRCUState rcu;
    NucLeiSYSTIMERState systimer;
    NucLeiSYSTIMERState timer;

    NucLeiGPIOState gpioa;
    NucLeiGPIOState gpiob;
    NucLeiGPIOState gpioc;
    NucLeiGPIOState gpiod;
    NucLeiGPIOState gpioe;

    NucLeiUARTState uart;
   
} NucleiGDSoCState;

typedef struct {
    /*< private >*/
    SysBusDevice parent_obj;
    
    /*< public >*/
    NucleiGDSoCState soc;
} NucleiGDState;

enum {
    GD32VF103_EXMC_SWREG,
    GD32VF103_EXMC_NPS,
    GD32VF103_USBFS,
    GD32VF103_CRC,
    GD32VF103_FMC,
    GD32VF103_RCU,
    GD32VF103_DMA1,
    GD32VF103_DMA0,
    GD32VF103_USART0,
    GD32VF103_SRAM,
    GD32VF103_OB,
    GD32VF103_BL,
    GD32VF103_MAINFLASH,
    GD32VF103_MFOL,
    GD32VF103_ECLIC,
    GD32VF103_SYSTIMER,
    GD32VF103_SPI0,
    GD32VF103_TIMER0,
    GD32VF103_ADC1,
    GD32VF103_ADC0,
    GD32VF103_GPIOE,
    GD32VF103_GPIOD,
    GD32VF103_GPIOC,
    GD32VF103_GPIOB,
    GD32VF103_GPIOA,
    GD32VF103_EXTI,
    GD32VF103_AFIO,
    GD32VF103_DAC,
    GD32VF103_PMU,
    GD32VF103_BKP,
    GD32VF103_CAN1,
    GD32VF103_CAN0,
    GD32VF103_SUCSRAM,
    GD32VF103_USBDEVFS,
    GD32VF103_I2C1,
    GD32VF103_I2C0,
    GD32VF103_UART4,
    GD32VF103_UART3,
    GD32VF103_USART2,
    GD32VF103_USART1,
    GD32VF103_SPI2,
    GD32VF103_SPI1,
    GD32VF103_FWDGT,
    GD32VF103_WWDGT,
    GD32VF103_RTC,
    GD32VF103_TIMER6,
    GD32VF103_TIMER5,
    GD32VF103_TIMER4,
    GD32VF103_TIMER3,
    GD32VF103_TIMER2,
    GD32VF103_TIMER1,
};

enum {
    GD32VF103_WWDGT_IRQn                = 19,      /*!< window watchDog timer interrupt                          */
    GD32VF103_LVD_IRQn                  = 20,      /*!< LVD through EXTI line detect interrupt                   */
    GD32VF103_TAMPER_IRQn               = 21,      /*!< tamper through EXTI line detect                          */
    GD32VF103_RTC_IRQn                  = 22,      /*!< RTC alarm interrupt                                      */
    GD32VF103_FMC_IRQn                  = 23,      /*!< FMC interrupt                                            */
    GD32VF103_RCU_CTC_IRQn              = 24,      /*!< RCU and CTC interrupt                                    */
    GD32VF103_EXTI0_IRQn                = 25,      /*!< EXTI line 0 interrupts                                   */
    GD32VF103_EXTI1_IRQn                = 26,      /*!< EXTI line 1 interrupts                                   */
    GD32VF103_EXTI2_IRQn                = 27,      /*!< EXTI line 2 interrupts                                   */
    GD32VF103_EXTI3_IRQn                = 28,      /*!< EXTI line 3 interrupts                                   */
    GD32VF103_EXTI4_IRQn                = 29,     /*!< EXTI line 4 interrupts                                   */
    GD32VF103_DMA0_Channel0_IRQn        = 30,     /*!< DMA0 channel0 interrupt                                  */
    GD32VF103_DMA0_Channel1_IRQn        = 31,     /*!< DMA0 channel1 interrupt                                  */
    GD32VF103_DMA0_Channel2_IRQn        = 32,     /*!< DMA0 channel2 interrupt                                  */
    GD32VF103_DMA0_Channel3_IRQn        = 33,     /*!< DMA0 channel3 interrupt                                  */
    GD32VF103_DMA0_Channel4_IRQn        = 34,     /*!< DMA0 channel4 interrupt                                  */
    GD32VF103_DMA0_Channel5_IRQn        = 35,     /*!< DMA0 channel5 interrupt                                  */
    GD32VF103_DMA0_Channel6_IRQn        = 36,     /*!< DMA0 channel6 interrupt                                  */
    GD32VF103_ADC0_1_IRQn               = 37,     /*!< ADC0 and ADC1 interrupt                                  */
    GD32VF103_CAN0_TX_IRQn              = 38,     /*!< CAN0 TX interrupts                                       */
    GD32VF103_CAN0_RX0_IRQn             = 39,     /*!< CAN0 RX0 interrupts                                      */
    GD32VF103_CAN0_RX1_IRQn             = 40,     /*!< CAN0 RX1 interrupts                                      */
    GD32VF103_CAN0_EWMC_IRQn            = 41,     /*!< CAN0 EWMC interrupts                                     */
    GD32VF103_EXTI5_9_IRQn              = 42,     /*!< EXTI[9:5] interrupts                                     */
    GD32VF103_TIMER0_BRK_IRQn           = 43,     /*!< TIMER0 break interrupts                                  */
    GD32VF103_TIMER0_UP_IRQn            = 44,     /*!< TIMER0 update interrupts                                 */
    GD32VF103_TIMER0_TRG_CMT_IRQn       = 45,     /*!< TIMER0 trigger and commutation interrupts                */
    GD32VF103_TIMER0_Channel_IRQn       = 46,     /*!< TIMER0 channel capture compare interrupts                */
    GD32VF103_TIMER1_IRQn               = 47,     /*!< TIMER1 interrupt                                         */
    GD32VF103_TIMER2_IRQn               = 48,     /*!< TIMER2 interrupt                                         */
    GD32VF103_TIMER3_IRQn               = 49,     /*!< TIMER3 interrupts                                        */
    GD32VF103_I2C0_EV_IRQn              = 50,     /*!< I2C0 event interrupt                                     */
    GD32VF103_I2C0_ER_IRQn              = 51,     /*!< I2C0 error interrupt                                     */
    GD32VF103_I2C1_EV_IRQn              = 52,     /*!< I2C1 event interrupt                                     */
    GD32VF103_I2C1_ER_IRQn              = 53,     /*!< I2C1 error interrupt                                     */
    GD32VF103_SPI0_IRQn                 = 54,     /*!< SPI0 interrupt                                           */
    GD32VF103_SPI1_IRQn                 = 55,     /*!< SPI1 interrupt                                           */
    GD32VF103_USART0_IRQn               = 56,     /*!< USART0 interrupt                                         */
    GD32VF103_USART1_IRQn               = 57,     /*!< USART1 interrupt                                         */
    GD32VF103_USART2_IRQn               = 58,     /*!< USART2 interrupt                                         */
    GD32VF103_EXTI10_15_IRQn            = 59,     /*!< EXTI[15:10] interrupts                                   */
    GD32VF103_RTC_ALARM_IRQn            = 60,     /*!< RTC alarm interrupt EXTI                                 */
    GD32VF103_USBFS_WKUP_IRQn           = 61,     /*!< USBFS wakeup interrupt                                   */
    GD32VF103_EXMC_IRQn                 = 67,     /*!< EXMC global interrupt                                    */
    GD32VF103_TIMER4_IRQn               = 69,     /*!< TIMER4 global interrupt                                  */
    GD32VF103_SPI2_IRQn                 = 70,     /*!< SPI2 global interrupt                                    */
    GD32VF103_UART3_IRQn                = 71,     /*!< UART3 global interrupt                                   */
    GD32VF103_UART4_IRQn                = 72,     /*!< UART4 global interrupt                                   */
    GD32VF103_TIMER5_IRQn               = 73,     /*!< TIMER5 global interrupt                                  */
    GD32VF103_TIMER6_IRQn               = 74,     /*!< TIMER6 global interrupt                                  */
    GD32VF103_DMA1_Channel0_IRQn        = 75,     /*!< DMA1 channel0 global interrupt                           */
    GD32VF103_DMA1_Channel1_IRQn        = 76,     /*!< DMA1 channel1 global interrupt                           */
    GD32VF103_DMA1_Channel2_IRQn        = 77,     /*!< DMA1 channel2 global interrupt                           */
    GD32VF103_DMA1_Channel3_IRQn        = 78,     /*!< DMA1 channel3 global interrupt                           */
    GD32VF103_DMA1_Channel4_IRQn        = 79,     /*!< DMA1 channel3 global interrupt                           */
    GD32VF103_CAN1_TX_IRQn              = 82,     /*!< CAN1 TX interrupt                                        */
    GD32VF103_CAN1_RX0_IRQn             = 83,     /*!< CAN1 RX0 interrupt                                       */
    GD32VF103_CAN1_RX1_IRQn             = 84,     /*!< CAN1 RX1 interrupt                                       */
    GD32VF103_CAN1_EWMC_IRQn            = 85,     /*!< CAN1 EWMC interrupt                                      */
    GD32VF103_USBFS_IRQn                = 86,     /*!< USBFS global interrupt                                   */
    GD32VF103_SOC_INT_MAX,
};


#define NUCLEI_CPU TYPE_RISCV_CPU_NUCLEI_N205

#endif
