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
#include <stdio.h>
#include <string.h>
#include "systick.h"
#include "gd32f1x0.h"
#include "defines.h"
#include "config.h"
#include "setup.h"
#include "util.h"

/* =========================== General Functions =========================== */
float average(float array[], uint8_t len) {
  float avg = 0;
    for(int i = 0; i < len; i++) {
      avg += array[i];
    }

    avg /= (len);

    return avg-1;
}

void consoleLog(char *message)
{
  #ifdef SERIAL_DEBUG
	log_i("%s", message);
  #endif
}
