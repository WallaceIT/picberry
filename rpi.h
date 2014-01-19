/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2014 Francesco Valla
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


/* GPIO registers address */
#define BCM2708_PERI_BASE  0x20000000
#define GPIO_BASE          (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define BLOCK_SIZE         (256)

/* GPIO setup macros. Always use GPIO_IN(x) before using GPIO_OUT(x) */
#define GPIO_IN(g)    *(gpio+((g)/10))   &= ~(7<<(((g)%10)*3))
#define GPIO_OUT(g)   *(gpio+((g)/10))   |=  (1<<(((g)%10)*3))

#define GPIO_SET(g)   *(gpio+7)  = 1<<(g)
#define GPIO_CLR(g)   *(gpio+10) = 1<<(g)
#define GPIO_LEV(g)   (*(gpio+13) >> (g)) & 0x1	/* reads pin level */

/* default GPIO <-> PIC connections */
#define DEFAULT_PIC_CLK    23	/* PGC - Output */
#define DEFAULT_PIC_DATA   24	/* PGD - I/O */
#define DEFAULT_PIC_MCLR   18	/* MCLR - Output */
