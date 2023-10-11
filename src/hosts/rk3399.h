/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2016 Francesco Valla
 *
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

#include "wiringx.h"
#include <stdio.h>
/* GPIO registers address */
#define GPIO_BASE          0x00000000
#define PORTOFFSET         0

/* GPIO setup macros. Always use GPIO_IN(x) before using GPIO_OUT(x) */
#define GPIO_IN(g)    (pinMode(g, PINMODE_INPUT))
#define GPIO_OUT(g)   (pinMode(g, PINMODE_OUTPUT))

#define GPIO_SET(g)   (digitalWrite(g, HIGH))
#define GPIO_CLR(g)   (digitalWrite(g, LOW))
#define GPIO_LEV(g)   (digitalRead(g)) & 0x1

/* default GPIO <-> PIC connections */
// To override, give the CLI option: -g PGC,PGD,MCLR

// For RockPi4, these gpio #'s correspond to pin #'s 33, 35, 37.
// see: https://wiki.radxa.com/Rockpi4/hardware/gpio
#define DEFAULT_PIC_CLK    76    /* PGC - Output */
#define DEFAULT_PIC_DATA   133   /* PGD - I/O */
#define DEFAULT_PIC_MCLR   158   /* MCLR - Output */

#define WIRINGX_PLATFORM "rock4"
