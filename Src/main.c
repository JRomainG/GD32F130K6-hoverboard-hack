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

#include "gd32f1x0.h"
#include "systick.h"
#include "defines.h"
#include "setup.h"
#include "config.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern volatile adc_buf_t adc_buffer;

int cmd1;  // normalized input values. -1000 to 1000
int cmd2;
int cmd3;

typedef struct{
	uint16_t start_of_frame;
	int16_t  steer;
	int16_t  speed;
	uint16_t checksum;
} Serialcommand;

volatile Serialcommand command;

uint8_t button1, button2;

int steer; // global variable for steering. -1000 to 1000
int speed; // global variable for speed. -1000 to 1000

extern volatile int pwm_m;  // global variable for pwm left. -1000 to 1000
extern volatile int weak_m; // global variable for field weakening left. -1000 to 1000

extern uint8_t buzzerFreq;    // global variable for the buzzer pitch. can be 1, 2, 3, 4, 5, 6, 7...
extern uint8_t buzzerPattern; // global variable for the buzzer pattern. can be 1, 2, 3, 4, 5, 6, 7...

extern uint8_t enable; // global variable for motor enable

volatile uint32_t timeout; // global variable for timeout
extern float batteryVoltage; // global variable for battery voltage

extern volatile unsigned int rpm;
extern volatile unsigned long lastTick[3];

uint32_t inactivity_timeout_counter;
uint32_t main_loop_counter;

int32_t motor_test_direction = 1;

int milli_vel_error_sum = 0;

void poweroff() {
	#ifdef AUXBOARD
		for (int i = 0; i < 8; i++) {
			buzzerFreq = i;
			delay_1ms(100);
		}
	#endif

	#ifdef MAINBOARD
		buzzerPattern = 0;
		enable = 0;
		delay_1ms(800);
		gpio_bit_write(BUZZER_PWR_PORT, BUZZER_PWR_PIN, 0);
		while(1) {}
	#endif
}

int main(void)
{
	systick_config();
	init_clock();
	gpio_config();
	usart_config();
	timer_config();
	adc_config();
	exti_config();

	#ifdef MAINBOARD
		gpio_bit_write(BUZZER_PWR_PORT, BUZZER_PWR_PIN, 1);
		delay_1ms(800);
	#endif

	#ifdef AUXBOARD
		for (int i = 8; i >= 0; i--) {
    	buzzerFreq = i;
    	delay_1ms(100);
  	}
		buzzerFreq = 0;
	#endif

	int lastSpeed_m = 0;
  int speed_m = 0;
  //float direction = 1;

	float board_temp_adc_filtered = (float)adc_buffer.temp;
	float board_temp_deg_c;

	enable = 1;  // enable motors

	while(1) {
		delay_1ms(DELAY_IN_MAIN_LOOP);

		#ifdef CONTROL_ADC
      // ADC values range: 0-4095, see ADC-calibration in config.h
      //cmd1 = CLAMP(adc_buffer.adc - ADC1_MIN, 0, ADC1_MAX) / (ADC1_MAX / 1000.0f);  // ADC1
      cmd2 = CLAMP(adc_buffer.adc - ADC_MIN, 0, ADC_MAX) / (ADC_MAX / 1000.0f);  // ADC2

      // use ADCs as button inputs:
      //button1 = (uint8_t)(adc_buffer.l_tx2 > 2000);  // ADC1
      //button2 = (uint8_t)(adc_buffer.l_rx2 > 2000);  // ADC2

      timeout = 0;
    #endif

		#ifdef CONTROL_MOTOR_TEST
			if (rpm < CONTROL_MOTOR_TEST_RPM-CONTROL_MOTOR_TEST_PADDING) cmd2 += 1;
			if (rpm > CONTROL_MOTOR_TEST_RPM+CONTROL_MOTOR_TEST_PADDING) cmd2 -= 1;

			timeout = 0;
		#endif

		// ####### LOW-PASS FILTER #######
    speed = speed * (1.0 - FILTER) + cmd2 * FILTER;

		// ####### MIXER #######
    //speed_m = CLAMP(speed * SPEED_COEFFICIENT +  steer * STEER_COEFFICIENT, -1000, 1000);
		speed_m = CLAMP(speed * SPEED_COEFFICIENT, -1000, 1500);

		// ####### SET OUTPUTS #######
    if ((speed_m < lastSpeed_m + 50 && speed_m > lastSpeed_m - 50) && timeout < TIMEOUT) {
	    #ifdef INVERT_DIRECTION
	      pwm_m = speed_m;
	    #else
	      pwm_m = -speed_m;
	    #endif
		}

    lastSpeed_m = speed_m;

		unsigned long t_ms = 0;
		get_tick_count_ms(&t_ms);
		if((t_ms - lastTick[0]) > 1000) {
			rpm = 0;
		}

		if (main_loop_counter % 25 == 0) {
      // ####### CALC BOARD TEMPERATURE #######
      board_temp_adc_filtered = board_temp_adc_filtered * 0.99 + (float)adc_buffer.temp * 0.01;
      board_temp_deg_c = ((float)TEMP_CAL_HIGH_DEG_C - (float)TEMP_CAL_LOW_DEG_C) / ((float)TEMP_CAL_HIGH_ADC - (float)TEMP_CAL_LOW_ADC) * (board_temp_adc_filtered - (float)TEMP_CAL_LOW_ADC) + (float)TEMP_CAL_LOW_DEG_C;

			// ####### DEBUG SERIAL OUT #######
      setScopeChannel(0, (int)0);
    	setScopeChannel(1, (int)main_loop_counter);
      setScopeChannel(2, (int)cmd2);  // 3: output speed: 0-1000
      setScopeChannel(3, (int)rpm);  // 4: output speed: 0-1000
      setScopeChannel(4, (int)adc_buffer.batt);  // 5: for battery voltage calibration
      setScopeChannel(5, (int)(batteryVoltage * 100.0f));  // 6: for verifying battery voltage calibration
      setScopeChannel(6, (int)board_temp_adc_filtered);  // 7: for board temperature calibration
      setScopeChannel(7, (int)board_temp_deg_c);  // 8: for verifying board temperature calibration
      consoleScope();
    }

		#ifdef MAINBOARD
			// ####### POWEROFF BY POWER-BUTTON #######
			if (gpio_input_bit_get(PWR_BUTTON_PORT, PWR_BUTTON_PIN) == 1 && weak_m == 0) {
				enable = 0;
				while (gpio_input_bit_get(PWR_BUTTON_PORT, PWR_BUTTON_PIN)) {}
				poweroff();
			}

			// ####### INACTIVITY TIMEOUT #######
	    if (abs(speed_m) > 50) {
	      inactivity_timeout_counter = 0;
	    } else {
	      inactivity_timeout_counter ++;
	    }
	    if (inactivity_timeout_counter > (INACTIVITY_TIMEOUT * 60 * 1000) / (DELAY_IN_MAIN_LOOP + 1)) {  // rest of main loop needs maybe 1ms
	      poweroff();
	    }
		#endif

		#ifdef AUXBOARD
			// ####### BEEP AND EMERGENCY POWEROFF #######
	    if ((TEMP_POWEROFF_ENABLE && board_temp_deg_c >= TEMP_POWEROFF && abs(speed) < 20) || (batteryVoltage < ((float)BAT_LOW_DEAD * (float)BAT_NUMBER_OF_CELLS) && abs(speed) < 20)) {  // poweroff before mainboard burns OR low bat 3
	      poweroff();
	    } else if (TEMP_WARNING_ENABLE && board_temp_deg_c >= TEMP_WARNING) {  // beep if mainboard gets hot
	      buzzerFreq = 4;
	      buzzerPattern = 1;
	    } else if (batteryVoltage < ((float)BAT_LOW_LVL1 * (float)BAT_NUMBER_OF_CELLS) && batteryVoltage > ((float)BAT_LOW_LVL2 * (float)BAT_NUMBER_OF_CELLS) && BAT_LOW_LVL1_ENABLE) {  // low bat 1: slow beep
	      buzzerFreq = 5;
	      buzzerPattern = 42;
	    } else if (batteryVoltage < ((float)BAT_LOW_LVL2 * (float)BAT_NUMBER_OF_CELLS) && batteryVoltage > ((float)BAT_LOW_DEAD * (float)BAT_NUMBER_OF_CELLS) && BAT_LOW_LVL2_ENABLE) {  // low bat 2: fast beep
	      buzzerFreq = 5;
	      buzzerPattern = 6;
	    } else if (BEEPS_BACKWARD && speed < -50) {  // backward beep
	      buzzerFreq = 5;
	      buzzerPattern = 1;
	    } else {  // do not beep
	      buzzerFreq = 0;
	      buzzerPattern = 0;
	    }
		#endif

		main_loop_counter++;
		timeout++;
	}
}
