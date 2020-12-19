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

volatile int pos_m = 0;
volatile int pwm_m = 0;
volatile int weak_m = 0;

extern volatile int speed;

extern volatile adc_buf_t adc_buffer;

extern volatile uint32_t timeout;

uint32_t buzzerFreq = 0;
uint32_t buzzerPattern = 0;

uint8_t enable = 0;
uint8_t buzOn = 0;

const int pwm_res = 64000000 / 2 / PWM_FREQ; // = 2000

const uint8_t hall_to_pos[8] = {
    0,
    0,
    2,
    1,
    4,
    5,
    3,
    0,
};

void blockPWM(int pwm, int pos, int *u, int *v, int *w) {
  switch(pos) {
    case 0:
      *u = 0;
      *v = pwm;
      *w = -pwm;
      break;
    case 1:
      *u = -pwm;
      *v = pwm;
      *w = 0;
      break;
    case 2:
      *u = -pwm;
      *v = 0;
      *w = pwm;
      break;
    case 3:
      *u = 0;
      *v = -pwm;
      *w = pwm;
      break;
    case 4:
      *u = pwm;
      *v = -pwm;
      *w = 0;
      break;
    case 5:
      *u = pwm;
      *v = 0;
      *w = -pwm;
      break;
    default:
      *u = 0;
      *v = 0;
      *w = 0;
  }
}

void blockPhaseCurrent(int pos, int u, int v, int *q) {
  switch(pos) {
    case 0:
      *q = u - v;
      // *u = 0;
      // *v = pwm;
      // *w = -pwm;
      break;
    case 1:
      *q = u;
      // *u = -pwm;
      // *v = pwm;
      // *w = 0;
      break;
    case 2:
      *q = u;
      // *u = -pwm;
      // *v = 0;
      // *w = pwm;
      break;
    case 3:
      *q = v;
      // *u = 0;
      // *v = -pwm;
      // *w = pwm;
      break;
    case 4:
      *q = v;
      // *u = pwm;
      // *v = -pwm;
      // *w = 0;
      break;
    case 5:
      *q = -(u - v);
      // *u = pwm;
      // *v = 0;
      // *w = -pwm;
      break;
    default:
      *q = 0;
      // *u = 0;
      // *v = 0;
      // *w = 0;
  }
}

uint32_t buzzerTimer = 0;

int offsetcount = 0;
int offsetr1   = 2000;
int offsetr2   = 2000;
int offsetdc   = 2000;

float batteryVoltage = BAT_NUMBER_OF_CELLS * 4.0;

int cur = 0;

int last_pos = 0;
int timer = 0;
const int max_time = PWM_FREQ / 10;

volatile int vel = 0;

//scan 8 channels with 2ADCs @ 20 clk cycles per sample
//meaning ~80 ADC clock cycles @ 8MHz until new DMA interrupt =~ 100KHz
//=640 cpu cycles
void DMA_Channel0_IRQHandler() {
  dma_interrupt_flag_clear(DMA_CH0, DMA_INT_FLAG_FTF);

  if(offsetcount < 1000) {
    offsetcount++;
    offsetr1 = (adc_buffer.r1 + offsetr1) / 2;
    offsetr2 = (adc_buffer.r2 + offsetr2) / 2;
    offsetdc = (adc_buffer.dc + offsetdc) / 2;
    return;
  }

  if (buzzerTimer % 1000 == 0) {
    batteryVoltage = batteryVoltage * 0.99 + ((float)adc_buffer.batt * ((float)BAT_CALIB_REAL_VOLTAGE / (float)BAT_CALIB_ADC)) * 0.01;
  }

  //if(ABS((adc_buffer.dcl - offsetdcl) * MOTOR_AMP_CONV_DC_AMP) > DC_CUR_LIMIT || timeout > TIMEOUT || enable == 0) {
    //LEFT_TIM->BDTR &= ~TIM_BDTR_MOE;
  //} else {
      //timer_primary_output_config(TIMER0, ENABLE);
  //}

  int u, v, w;

  uint8_t hall_u = !(gpio_input_bit_get(HALL_U_PORT, HALL_U_PIN));
  uint8_t hall_v = !(gpio_input_bit_get(HALL_V_PORT, HALL_V_PIN));
  uint8_t hall_w = !(gpio_input_bit_get(HALL_W_PORT, HALL_W_PIN));

  uint8_t hall_m   = hall_u * 1 + hall_v * 2 + hall_w * 4;
  pos_m = hall_to_pos[hall_m];
  pos_m += 2;
  pos_m %= 6;

  //blockPhaseCurrent(pos_m, adc_buffer.r1 - offsetr1, adc_buffer.r2 - offsetr2, &cur);

  #ifdef AUXBOARD
    //create square wave for buzzer
    buzzerTimer++;
    if (buzzerFreq != 0 && (buzzerTimer / 7000) % (buzzerPattern + 1) == 0) {
      if (buzzerTimer % buzzerFreq == 0) {
        if(buzOn == TRUE) {
           buzOn = FALSE;
           gpio_bit_reset(BUZZER_PWR_PORT, BUZZER_PWR_PIN);
        } else {
          buzOn = TRUE;
          gpio_bit_set(BUZZER_PWR_PORT, BUZZER_PWR_PIN);
        }
      }
    } else {
        gpio_bit_reset(BUZZER_PWR_PORT, BUZZER_PWR_PIN);
    }
  #endif

  blockPWM(pwm_m, pos_m, &u, &v, &w);

  int weaku, weakv, weakw;
  if (pwm_m > 0) {
    blockPWM(weak_m, (pos_m+5) % 6, &weaku, &weakv, &weakw);
  } else {
    blockPWM(-weak_m, (pos_m+1) % 6, &weaku, &weakv, &weakw);
  }
  u += weaku;
  v += weakv;
  w += weakw;

  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0,CLAMP(u + pwm_res / 2, 10, pwm_res-10));
  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1,CLAMP(v + pwm_res / 2, 10, pwm_res-10));
  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_2,CLAMP(w + pwm_res / 2, 10, pwm_res-10));

}
