/*
 * Nuclei machine interface
 *
 * Copyright (c) 2020 Nuclei, Inc.
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

#ifndef HW_RISCV_NUCLEI_H
#define HW_RISCV_NUCLEI_H

#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/nuclei_uart.h"
#include "hw/riscv/nuclei_timer.h"
#include "hw/riscv/sifive_gpio.h"
#include "hw/sysbus.h"

#define TYPE_NUCLEI_SOC "riscv.nuclei.soc"
#define RISCV_NUCLEI_SOC(obj) \
    OBJECT_CHECK(NucleiSoCState, (obj), TYPE_NUCLEI_SOC)

typedef struct NucleiSoCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
    DeviceState *eclic;
    MemoryRegion ilm;
    MemoryRegion dlm;
    MemoryRegion internal_rom;
    MemoryRegion xip_mem;

    NucLeiTIMERState timer;
    NucLeiUARTState uart;
    SIFIVEGPIOState gpio;
   

} NucleiSoCState;

typedef struct {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    NucleiSoCState soc;
} NucleiState;

enum {
    NUCLEI_DEBUG,
    NUCLEI_ROM,
    NUCLEI_TIMER,
    NUCLEI_ECLIC,
    NUCLEI_GPIO,
    NUCLEI_UART0,
    NUCLEI_QSPI0,
    NUCLEI_PWM0,
    NUCLEI_UART1,
    NUCLEI_QSPI1,
    NUCLEI_PWM1,
    NUCLEI_QSPI2,
    NUCLEI_PWM2,
    NUCLEI_XIP,
    NUCLEI_ILM,
    NUCLEI_DLM
};

enum {
    NUCLEI_Reserved0_IRQn            =   0,              /*!<  Internal reserved */
    NUCLEI_Reserved1_IRQn            =   1,              /*!<  Internal reserved */
    NUCLEI_Reserved2_IRQn            =   2,              /*!<  Internal reserved */
    NUCLEI_SysTimerSW_IRQn        =   3,              /*!<  System Timer SW interrupt */
    NUCLEI_Reserved3_IRQn            =   4,              /*!<  Internal reserved */
    NUCLEI_Reserved4_IRQn            =   5,              /*!<  Internal reserved */
    NUCLEI_Reserved5_IRQn            =   6,              /*!<  Internal reserved */
    NUCLEI_SysTimer_IRQn               =   7,              /*!<  System Timer Interrupt */
    NUCLEI_Reserved6_IRQn            =   8,              /*!<  Internal reserved */
    NUCLEI_Reserved7_IRQn            =   9,              /*!<  Internal reserved */
    NUCLEI_Reserved8_IRQn            =  10,              /*!<  Internal reserved */
    NUCLEI_Reserved9_IRQn            =  11,              /*!<  Internal reserved */
    NUCLEI_Reserved10_IRQn           =  12,              /*!<  Internal reserved */
    NUCLEI_Reserved11_IRQn           =  13,              /*!<  Internal reserved */
    NUCLEI_Reserved12_IRQn           =  14,              /*!<  Internal reserved */
    NUCLEI_Reserved13_IRQn           =  15,              /*!<  Internal reserved */
    NUCLEI_Reserved14_IRQn           =  16,              /*!<  Internal reserved */
    NUCLEI_BusError_IRQn             =  17,              /*!<  Bus Error interrupt */
    NUCLEI_PerfMon_IRQn              =  18,              /*!<  Performance Monitor */
    /* interruput numbers */
    NUCLEI_WWDGT_IRQn                   = 19,      /*!< window watchDog timer interrupt                          */
    NUCLEI_LVD_IRQn                     = 20,      /*!< LVD through EXTI line detect interrupt                   */
    NUCLEI_TAMPER_IRQn                  = 21,      /*!< tamper through EXTI line detect                          */
    NUCLEI_RTC_IRQn                     = 22,      /*!< RTC alarm interrupt                                      */
    NUCLEI_FMC_IRQn                     = 23,      /*!< FMC interrupt                                            */
    NUCLEI_RCU_CTC_IRQn                 = 24,      /*!< RCU and CTC interrupt                                    */
    NUCLEI_EXTI0_IRQn                   = 25,      /*!< EXTI line 0 interrupts                                   */
    NUCLEI_EXTI1_IRQn                   = 26,      /*!< EXTI line 1 interrupts                                   */
    NUCLEI_EXTI2_IRQn                   = 27,      /*!< EXTI line 2 interrupts                                   */
    NUCLEI_EXTI3_IRQn                   = 28,      /*!< EXTI line 3 interrupts                                   */
    NUCLEI_EXTI4_IRQn                   = 29,     /*!< EXTI line 4 interrupts                                   */
    NUCLEI_DMA0_Channel0_IRQn           = 30,     /*!< DMA0 channel0 interrupt                                  */
    NUCLEI_DMA0_Channel1_IRQn           = 31,     /*!< DMA0 channel1 interrupt                                  */
    NUCLEI_DMA0_Channel2_IRQn           = 32,     /*!< DMA0 channel2 interrupt                                  */
    NUCLEI_DMA0_Channel3_IRQn           = 33,     /*!< DMA0 channel3 interrupt                                  */
    NUCLEI_DMA0_Channel4_IRQn           = 34,     /*!< DMA0 channel4 interrupt                                  */
    NUCLEI_DMA0_Channel5_IRQn           = 35,     /*!< DMA0 channel5 interrupt                                  */
    NUCLEI_DMA0_Channel6_IRQn           = 36,     /*!< DMA0 channel6 interrupt                                  */
    NUCLEI_ADC0_1_IRQn                  = 37,     /*!< ADC0 and ADC1 interrupt                                  */
    NUCLEI_CAN0_TX_IRQn                 = 38,     /*!< CAN0 TX interrupts                                       */
    NUCLEI_CAN0_RX0_IRQn                = 39,     /*!< CAN0 RX0 interrupts                                      */
    NUCLEI_CAN0_RX1_IRQn                = 40,     /*!< CAN0 RX1 interrupts                                      */
    NUCLEI_CAN0_EWMC_IRQn               = 41,     /*!< CAN0 EWMC interrupts                                     */
    NUCLEI_EXTI5_9_IRQn                 = 42,     /*!< EXTI[9:5] interrupts                                     */
    NUCLEI_TIMER0_BRK_IRQn              = 43,     /*!< TIMER0 break interrupts                                  */
    NUCLEI_TIMER0_UP_IRQn               = 44,     /*!< TIMER0 update interrupts                                 */
    NUCLEI_TIMER0_TRG_CMT_IRQn          = 45,     /*!< TIMER0 trigger and commutation interrupts                */
    NUCLEI_TIMER0_Channel_IRQn          = 46,     /*!< TIMER0 channel capture compare interrupts                */
    NUCLEI_TIMER1_IRQn                  = 47,     /*!< TIMER1 interrupt                                         */
    NUCLEI_TIMER2_IRQn                  = 48,     /*!< TIMER2 interrupt                                         */
    NUCLEI_TIMER3_IRQn                  = 49,     /*!< TIMER3 interrupts                                        */
    NUCLEI_I2C0_EV_IRQn                 = 50,     /*!< I2C0 event interrupt                                     */
    NUCLEI_I2C0_ER_IRQn                 = 51,     /*!< I2C0 error interrupt                                     */
    NUCLEI_I2C1_EV_IRQn                 = 52,     /*!< I2C1 event interrupt                                     */
    NUCLEI_I2C1_ER_IRQn                 = 53,     /*!< I2C1 error interrupt                                     */
    NUCLEI_SPI0_IRQn                    = 54,     /*!< SPI0 interrupt                                           */
    NUCLEI_SPI1_IRQn                    = 55,     /*!< SPI1 interrupt                                           */
    NUCLEI_USART0_IRQn                  = 56,     /*!< USART0 interrupt                                         */
    NUCLEI_USART1_IRQn                  = 57,     /*!< USART1 interrupt                                         */
    NUCLEI_USART2_IRQn                  = 58,     /*!< USART2 interrupt                                         */
    NUCLEI_EXTI10_15_IRQn               = 59,     /*!< EXTI[15:10] interrupts                                   */
    NUCLEI_RTC_ALARM_IRQn               = 60,     /*!< RTC alarm interrupt EXTI                                 */
    NUCLEI_USBFS_WKUP_IRQn              = 61,     /*!< USBFS wakeup interrupt                                   */
    NUCLEI_EXMC_IRQn                    = 67,     /*!< EXMC global interrupt                                    */
    NUCLEI_TIMER4_IRQn                  = 69,     /*!< TIMER4 global interrupt                                  */
    NUCLEI_SPI2_IRQn                    = 70,     /*!< SPI2 global interrupt                                    */
    NUCLEI_UART3_IRQn                   = 71,     /*!< UART3 global interrupt                                   */
    NUCLEI_UART4_IRQn                   = 72,     /*!< UART4 global interrupt                                   */
    NUCLEI_TIMER5_IRQn                  = 73,     /*!< TIMER5 global interrupt                                  */
    NUCLEI_TIMER6_IRQn                  = 74,     /*!< TIMER6 global interrupt                                  */
    NUCLEI_DMA1_Channel0_IRQn           = 75,     /*!< DMA1 channel0 global interrupt                           */
    NUCLEI_DMA1_Channel1_IRQn           = 76,     /*!< DMA1 channel1 global interrupt                           */
    NUCLEI_DMA1_Channel2_IRQn           = 77,     /*!< DMA1 channel2 global interrupt                           */
    NUCLEI_DMA1_Channel3_IRQn           = 78,     /*!< DMA1 channel3 global interrupt                           */
    NUCLEI_DMA1_Channel4_IRQn           = 79,     /*!< DMA1 channel3 global interrupt                           */
    NUCLEI_CAN1_TX_IRQn                 = 82,     /*!< CAN1 TX interrupt                                        */
    NUCLEI_CAN1_RX0_IRQn                = 83,     /*!< CAN1 RX0 interrupt                                       */
    NUCLEI_CAN1_RX1_IRQn                = 84,     /*!< CAN1 RX1 interrupt                                       */
    NUCLEI_CAN1_EWMC_IRQn               = 85,     /*!< CAN1 EWMC interrupt                                      */
    NUCLEI_USBFS_IRQn                   = 86,     /*!< USBFS global interrupt                                   */
    NUCLEI_SOC_INT_MAX,
};

// TODO: Add riscv64 support
#if defined(TARGET_RISCV32)
#define NUCLEI_CPU TYPE_RISCV_CPU_NUCLEI_N307FD
#elif defined(TARGET_RISCV64)
#define NUCLEI64_CPU TYPE_RISCV_CPU_NUCLEI_NX600FD
#endif

#endif
