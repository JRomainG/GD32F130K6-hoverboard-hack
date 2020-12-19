#include "gd32f1x0.h"
#include "defines.h"
#include "setup.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

volatile uint8_t uart_buf[100];
volatile int16_t ch_buf[8];

void setScopeChannel(uint8_t ch, int16_t val) {
  ch_buf[ch] = val;
}

void consoleScope() {
    memset(uart_buf, 0, sizeof(uart_buf));
    sprintf(uart_buf, "1:%i 2:%i 3:%i 4:%i 5:%i 6:%i 7:%i 8:%i\r\n", ch_buf[0], ch_buf[1], ch_buf[2], ch_buf[3], ch_buf[4], ch_buf[5], ch_buf[6], ch_buf[7]);

    #ifdef DEBUG_SERIAL
      if(usart_flag_get(USART1, USART_FLAG_TBE)) {
        dma_channel_disable(DMA_CH3);
    		DMA_CH3CNT = strlen(uart_buf);
    		DMA_CH3MADDR = (uint32_t)uart_buf;
    		dma_channel_enable(DMA_CH3);
      }
    #endif
}
