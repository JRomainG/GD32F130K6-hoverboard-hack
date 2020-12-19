/**
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

// Define to prevent recursive inclusion
#ifndef DEFINES_H
#define DEFINES_H

// Includes
#include "gd32f1x0.h"
#include "config.h"

/* =========================== Defines USART =========================== */
#define USART_GPIO_PORT     		  GPIOA
#define USART_AF            	  	GPIO_AF_1
#define USART_TX_PIN   		      	GPIO_PIN_15
#define USART_RX_PIN    	      	GPIO_PIN_7

#define USART1_TDATA_ADDRESS    	((uint32_t)0x40004428)
#define ADC_RDATA_ADDRESS       	((uint32_t)0x4001244C)

/* =========================== Defines General =========================== */
#define HALL_V_PIN GPIO_PIN_12
#define HALL_W_PIN GPIO_PIN_3
#define HALL_U_PIN GPIO_PIN_8
#define HALL_V_PORT GPIOA
#define HALL_W_PORT GPIOB
#define HALL_U_PORT GPIOB

#define TIM_M TIM0
#define TIM_U CCR1
#define TIM_UH_PIN GPIO_PIN_8
#define TIM_UH_PORT GPIOA
#define TIM_UL_PIN GPIO_PIN_7
#define TIM_UL_PORT GPIOA
#define TIM_V CCR2
#define TIM_VH_PIN GPIO_PIN_9
#define TIM_VH_PORT GPIOA
#define TIM_VL_PIN GPIO_PIN_0
#define TIM_VL_PORT GPIOB
#define TIM_W CCR3
#define TIM_WH_PIN GPIO_PIN_10
#define TIM_WH_PORT GPIOA
#define TIM_WL_PIN GPIO_PIN_1
#define TIM_WL_PORT GPIOB

//#define LEFT_DC_CUR_PIN GPIO_PIN_0
#define LEFT_U_CUR_PIN GPIO_PIN_4
#define LEFT_V_CUR_PIN GPIO_PIN_5

//#define LEFT_DC_CUR_PORT GPIOC
#define LEFT_U_CUR_PORT GPIOA
#define LEFT_V_CUR_PORT GPIOA

//#define DCLINK_PIN GPIO_PIN_2
//#define DCLINK_PORT GPIOC

#define BUZZER_PWR_PIN GPIO_PIN_11
#define BUZZER_PWR_PORT GPIOA

//#define SWITCH_PIN GPIO_PIN_1
//#define SWITCH_PORT GPIOA

#define PWR_BUTTON_PIN GPIO_PIN_2
#define PWR_BUTTON_PORT GPIOB

//#define CHARGER_PIN GPIO_PIN_12
//#define CHARGER_PORT GPIOA

typedef struct {
  uint16_t adc;
  uint16_t temp;
	uint16_t batt;
	uint16_t r1;
	uint16_t r2;
	uint16_t dc;
} adc_buf_t;

#define DELAY_TIM_FREQUENCY_US 1000000

#define MOTOR_AMP_CONV_DC_AMP 0.02  // A per bit (12) on ADC.

#define MILLI_R (R * 1000)
#define MILLI_PSI (PSI * 1000)
#define MILLI_V (V * 1000)
#define M_PI 3.14159265358979323846

#define NO 0
#define YES 1
#define ABS(a) (((a) < 0.0) ? -(a) : (a))
#define LIMIT(x, lowhigh) (((x) > (lowhigh)) ? (lowhigh) : (((x) < (-lowhigh)) ? (-lowhigh) : (x)))
#define SAT(x, lowhigh) (((x) > (lowhigh)) ? (1.0) : (((x) < (-lowhigh)) ? (-1.0) : (0.0)))
#define SAT2(x, low, high) (((x) > (high)) ? (1.0) : (((x) < (low)) ? (-1.0) : (0.0)))
#define STEP(from, to, step) (((from) < (to)) ? (MIN((from) + (step), (to))) : (MAX((from) - (step), (to))))
#define DEG(a) ((a)*M_PI / 180.0)
#define RAD(a) ((a)*180.0 / M_PI)
#define SIGN(a) (((a) < 0.0) ? (-1.0) : (((a) > 0.0) ? (1.0) : (0.0)))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define SCALE(value, high, max) MIN(MAX(((max) - (value)) / ((max) - (high)), 0.0), 1.0)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN3(a, b, c) MIN(a, MIN(b, c))
#define MAX3(a, b, c) MAX(a, MAX(b, c))
#define ARRAY_LEN(x) 				(uint32_t)(sizeof(x) / sizeof(*(x)))
#define FALSE 0
#define TRUE 1
#define delay_ms    				delay_1ms
#define delay    				    delay_1ms
#define get_ms      				get_tick_count_ms
#define log_i       				printf			// redirect the log_i debug function to printf

#if defined(PRINTF_FLOAT_SUPPORT) && defined(SERIAL_DEBUG) && defined(__GNUC__)
	asm(".global _printf_float"); 		// this is the magic trick for printf to support float. Warning: It will increase code considerably! Better to avoid!
#endif

#endif //  DEFINES_H
