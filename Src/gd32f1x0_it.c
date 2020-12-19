/*!
    \file  gd32f1x0_it.h
    \brief the header file of the ISR
*/

/*
    Copyright (C) 2017 GigaDevice

    2014-12-26, V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
    2017-06-19, V3.1.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0_it.h"
#include "systick.h"
#include "config.h"
#include "util.h"

float rpm_a[3];
volatile unsigned int rpm;
volatile unsigned long lastTick[3];
unsigned long currentTick[3];

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
	tick_count_increment();
  delay_decrement();
}

void EXTI4_15_IRQHandler(void)
{
  if(exti_interrupt_flag_get(EXTI_12)) {
    if (RESET != exti_interrupt_flag_get(EXTI_12)) {
          if(currentTick[0] == 0) get_tick_count_ms(&lastTick[0]);
          else lastTick[0] = currentTick[0];

          get_tick_count_ms(&currentTick[0]);

          // 15 ticks per rpm, 60000 milliseconds/rotation to rpm.
          rpm_a[0] = 4000/((currentTick[0]-lastTick[0]));

          exti_interrupt_flag_clear(EXTI_12);
      }
    } else if(exti_interrupt_flag_get(EXTI_8)) {
      if (RESET != exti_interrupt_flag_get(EXTI_8)) {
            if(currentTick[2] == 0) get_tick_count_ms(&lastTick[2]);
            else lastTick[2] = currentTick[2];

            get_tick_count_ms(&currentTick[2]);

            // 15 ticks per rpm, 60000 milliseconds/rotation to rpm.
            rpm_a[2] = 4000/((currentTick[2]-lastTick[2]));

            exti_interrupt_flag_clear(EXTI_8);
        }
    }

    rpm = average(&rpm_a, 3);
}

void EXTI2_3_IRQHandler(void)
{
  if (RESET != exti_interrupt_flag_get(EXTI_3)) {
        if(currentTick[1] == 0) get_tick_count_ms(&lastTick[1]);
        else lastTick[1] = currentTick[1];

        get_tick_count_ms(&currentTick[1]);

        // 15 ticks per rpm, 60000 milliseconds/rotation to rpm.
        rpm_a[1] = 4000/((currentTick[1]-lastTick[1]));

        exti_interrupt_flag_clear(EXTI_3);
    }

    rpm = average(&rpm_a, 3);
}
