/**
  * This file is part of the hoverboard-sideboard-hack project.
  *
  * Copyright (C) 2020-2021 Emanuel FERU <aerdronix@gmail.com>
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
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Includes
#include "gd32f1x0.h"
#include "systick.h"
#include "setup.h"
#include "defines.h"
#include "config.h"

volatile adc_buf_t adc_buffer;

void gpio_config(void) {

	/* =========================== Configure LEDs GPIOs =========================== */
	/* enable the GPIO clock */
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);

	gpio_mode_set(BUZZER_PWR_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BUZZER_PWR_PIN);
	gpio_output_options_set(BUZZER_PWR_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BUZZER_PWR_PIN);

	// U HI & LO
	gpio_mode_set(TIM_UH_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TIM_UH_PIN);
	gpio_output_options_set(TIM_UH_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, TIM_UH_PIN);
	gpio_af_set(TIM_UH_PORT, GPIO_AF_2, TIM_UH_PIN);

	gpio_mode_set(TIM_UL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TIM_UL_PIN);
	gpio_output_options_set(TIM_UL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, TIM_UL_PIN);
	gpio_af_set(TIM_UL_PORT, GPIO_AF_2, TIM_UL_PIN);

	//V HI & LO
	gpio_mode_set(TIM_VH_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TIM_VH_PIN);
	gpio_output_options_set(TIM_VH_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, TIM_VH_PIN);
	gpio_af_set(TIM_VH_PORT, GPIO_AF_2, TIM_VH_PIN);

	gpio_mode_set(TIM_VL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TIM_VL_PIN);
	gpio_output_options_set(TIM_VL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, TIM_VL_PIN);
	gpio_af_set(TIM_VL_PORT, GPIO_AF_2, TIM_VL_PIN);

	// W HI & LO
	gpio_mode_set(TIM_WH_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TIM_WH_PIN);
	gpio_output_options_set(TIM_WH_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, TIM_WH_PIN);
	gpio_af_set(TIM_WH_PORT, GPIO_AF_2, TIM_WH_PIN);

	gpio_mode_set(TIM_WL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TIM_WL_PIN);
	gpio_output_options_set(TIM_WL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, TIM_WL_PIN);
	gpio_af_set(TIM_WL_PORT, GPIO_AF_2, TIM_WL_PIN);

	gpio_mode_set(BUZZER_PWR_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_PULLDOWN, GPIO_PIN_2);

	/* reset GPIO pin */
	gpio_bit_reset(BUZZER_PWR_PORT, BUZZER_PWR_PIN);
}

void usart_config(void) {
		dma_parameter_struct dma_init_struct;

		rcu_periph_clock_enable(RCU_USART1); //RCC->APB2ENR    |= RCC_APB2ENR_USART1EN;    // enable USART0 clock
		rcu_periph_clock_enable(RCU_DMA);

		//GPIOA->CRH &= ~(GPIO_CRH_CNF9  | GPIO_CRH_MODE9);   // reset PA9
		gpio_af_set(USART_GPIO_PORT, USART_AF, USART_TX_PIN);

		gpio_mode_set(USART_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_TX_PIN);
		gpio_output_options_set(USART_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, USART_TX_PIN); //GPIOA->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0;  // 0b11 50MHz output GPIOA->CRH |= GPIO_CRH_CNF9_1;    // PA9: output @ 50MHz - Alt-function Push-pull


		/* USART configure */
    usart_deinit(USART1);
		usart_baudrate_set(USART1, DEBUG_BAUD);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
		usart_invert_config(USART1, USART_SWAP_ENABLE);
    usart_enable(USART1);

		dma_deinit(DMA_CH3);
    dma_init_struct.direction           = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width        = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_addr         = USART1_TDATA_ADDRESS;
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width        = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority            = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA_CH3, dma_init_struct);

    /* USART DMA enable for transmission */
    usart_dma_transmit_config(USART1, USART_DENT_ENABLE);
}

void init_clock(void)
{
    // Conf clock : 72MHz using IRC8M 8MHz crystal w/ PLL X 18 (8MHz/2 x 18 = 72MHz)

    fmc_wscnt_set(WS_WSCNT_2); // Two wait states, per datasheet
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);    // prescale AHB1 = HCLK/2

		rcu_pll_config(RCU_PLLSRC_IRC8M_DIV2, RCU_PLL_MUL18); // set PLL source to HSE, multiply by 18
    rcu_osci_on(RCU_PLL_CK);//RCU->CR         |= RCU_CR_PLLON;        // enable the PLL
    rcu_osci_stab_wait(RCU_PLL_CK);//while( !(RCU->CR & RCU_CR_PLLRDY) );    // wait for the PLLRDY flag

    rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);//RCU->CFGR       |= RCU_CFGR_SW_PLL;     // set clock source to pll
		rcu_adc_clock_config(RCU_ADCCK_IRC14M);
		rcu_osci_on(RCU_IRC14M);

    SystemCoreClockUpdate();                // calculate the SYSCLOCK value
}

void adc_config(void) {
	rcu_periph_clock_enable(RCU_ADC);
	rcu_periph_clock_enable(RCU_DMA);

	dma_deinit(DMA_CH0);
	dma_periph_address_config(DMA_CH0,ADC_RDATA_ADDRESS);
	dma_memory_address_config(DMA_CH0,(uint32_t)(&adc_buffer));
	dma_transfer_direction_config(DMA_CH0,DMA_PERIPHERAL_TO_MEMORY);
	dma_memory_width_config(DMA_CH0,DMA_MEMORY_WIDTH_16BIT);
	dma_periph_width_config(DMA_CH0,DMA_PERIPHERAL_WIDTH_16BIT);
	dma_priority_config(DMA_CH0,DMA_PRIORITY_HIGH);
	dma_transfer_number_config(DMA_CH0,3);
	dma_periph_increase_disable(DMA_CH0);
	dma_memory_increase_enable(DMA_CH0);
	dma_circulation_enable(DMA_CH0);
	dma_interrupt_enable(DMA_CH0, DMA_INT_FTF);
	dma_channel_enable(DMA_CH0);

	 adc_tempsensor_vrefint_enable();
	 /* ADC channel length config */
	 adc_channel_length_config(ADC_REGULAR_CHANNEL,3);
	 /* ADC regular channel config */
	 adc_regular_channel_config(0,ADC_CHANNEL_2,ADC_SAMPLETIME_7POINT5);
	 /* ADC regular channel config */
	 adc_regular_channel_config(1,ADC_CHANNEL_16,ADC_SAMPLETIME_239POINT5);
	 /* ADC regular channel config */
	 adc_regular_channel_config(2,ADC_CHANNEL_17,ADC_SAMPLETIME_13POINT5);
	 /* ADC external trigger enable */
	 adc_external_trigger_config(ADC_REGULAR_CHANNEL,ENABLE);
	 /* ADC external trigger source config */
	 adc_external_trigger_source_config(ADC_REGULAR_CHANNEL,ADC_EXTTRIG_REGULAR_T2_TRGO);
	 /* ADC data alignment config */
	 adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
	 /* enable ADC interface */
	 adc_enable();
	 /* ADC DMA function enable */
	 adc_dma_mode_enable();
	 /* ADC contineous function enable */
	 adc_special_function_config(ADC_SCAN_MODE,ENABLE);
	 /* ADC software trigger enable */
	 adc_software_trigger_enable(ADC_REGULAR_CHANNEL);

	 nvic_irq_enable(DMA_Channel0_IRQn, 0, 0);     // Enable USART0 interrupt
}

void timer_config(void) {
	timer_parameter_struct timer_init_struct;
	timer_oc_parameter_struct timer_oc_struct;
	timer_break_parameter_struct timer_brk_config;

	rcu_periph_clock_enable(RCU_TIMER2);
	timer_deinit(TIMER2);
	timer_init_struct.prescaler           = 0;
	timer_init_struct.alignedmode         = TIMER_COUNTER_CENTER_DOWN;
	timer_init_struct.counterdirection    = TIMER_COUNTER_UP;
	timer_init_struct.period              = 64000000 / 2 / PWM_FREQ;
	timer_init_struct.clockdivision       = TIMER_CKDIV_DIV1;
	timer_init_struct.repetitioncounter   = 0;
	timer_init(TIMER2, &timer_init_struct);

	timer_master_slave_mode_config(TIMER2, TIMER_MASTER_SLAVE_MODE_ENABLE);
	timer_slave_mode_select(TIMER2, TIMER_SLAVE_MODE_PAUSE);
	timer_input_trigger_source_select(TIMER2, TIMER_SMCFG_TRGSEL_ITI0);
	timer_master_output_trigger_source_select(TIMER2, TIMER_TRI_OUT_SRC_UPDATE);
	timer_primary_output_config(TIMER2, ENABLE);
	timer_enable(TIMER2);

	rcu_periph_clock_enable(RCU_TIMER0);
	timer_deinit(TIMER0);
	timer_init_struct.prescaler           = 0;
	timer_init_struct.alignedmode         = TIMER_COUNTER_CENTER_DOWN;
	timer_init_struct.counterdirection    = TIMER_COUNTER_UP;
	timer_init_struct.period              = 62000000 / 2 / PWM_FREQ;
	timer_init_struct.clockdivision       = TIMER_CKDIV_DIV1;
	timer_init_struct.repetitioncounter   = 0;
	timer_init(TIMER0, &timer_init_struct);

	timer_oc_struct.outputstate = TIMER_CCX_ENABLE;
	timer_oc_struct.outputnstate = TIMER_CCXN_ENABLE;
	timer_oc_struct.ocpolarity = TIMER_OC_POLARITY_HIGH;
	timer_oc_struct.ocnpolarity = TIMER_OCN_POLARITY_LOW;
	timer_oc_struct.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
	timer_oc_struct.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;
	timer_channel_output_config(TIMER0, TIMER_CH_0, &timer_oc_struct);
	timer_channel_output_config(TIMER0, TIMER_CH_1, &timer_oc_struct);
	timer_channel_output_config(TIMER0, TIMER_CH_2, &timer_oc_struct);

	timer_brk_config.runoffstate = TIMER_ROS_STATE_ENABLE;
	timer_brk_config.ideloffstate = TIMER_IOS_STATE_ENABLE;
	timer_brk_config.deadtime = DEAD_TIME;
	timer_brk_config.breakpolarity = TIMER_BREAK_POLARITY_LOW;
	timer_brk_config.outputautostate = TIMER_OUTAUTO_DISABLE;
	timer_brk_config.protectmode = TIMER_CCHP_PROT_OFF;
	timer_brk_config.breakstate = TIMER_BREAK_DISABLE;
	timer_break_config(TIMER0, &timer_brk_config);

	timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM1);
	timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM1);
	timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER_OC_MODE_PWM1);

	timer_master_output_trigger_source_select(TIMER0, TIMER_TRI_OUT_SRC_ENABLE);
	timer_primary_output_config(TIMER0, ENABLE);
	timer_enable(TIMER0);
}

void exti_config(void) {
	rcu_periph_clock_enable(RCU_CFGCMP);
	syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN12);
	exti_init(EXTI_12, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_interrupt_enable(EXTI_12);

	syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN3);
	exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_interrupt_enable(EXTI_3);

	syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN8);
	exti_init(EXTI_8, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_interrupt_enable(EXTI_8);

	nvic_irq_enable(EXTI4_15_IRQn, 0, 0);
	nvic_irq_enable(EXTI2_3_IRQn, 0, 0);
}
