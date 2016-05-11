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
 
#define SW_PORTC_IO_BASE  	0x01c20800
#define GPIO_BASE		  	0x01c20000
#define OFFSET			  	0x00000800
#define BLOCK_SIZE			0x00002000

/* PORT OFFSETs */
#define PA  0x00
#define PB  0x24
#define PC  0x48
#define PD  0x6C
#define PE  0x90
#define PF  0xB4
#define PG  0xD8
#define PH  0xFC
#define PI  0x120

#define PORTOFFSET  0x24

#define SET         0x10
#define PULL        0x1C

/* GPIO setup macros. Always use GPIO_IN(x) before using GPIO_OUT(x) */
#define GPIO_IN(g)    *(int*)((char*)gpio+OFFSET+(g>>8)+(((int)(g&0xFF)/8)*4)) &= ~(0x07<<(((int)(g&0xFF)%8)*4))
#define GPIO_OUT(g)   *(int*)((char*)gpio+OFFSET+(g>>8)+(((int)(g&0xFF)/8)*4)) |= (0x01<<(((int)(g&0xFF)%8)*4))

#define GPIO_SET(g)   *(int*)((char*)gpio+OFFSET+(g>>8)+SET) |= 1<<(int)(g&0xFF)
#define GPIO_CLR(g)   *(int*)((char*)gpio+OFFSET+(g>>8)+SET) &= ~(1<<(int)(g&0xFF))
#define GPIO_LEV(g)   (*(int*)((char*)gpio+OFFSET+(g>>8)+SET) >> (int)(g&0xFF)) & 0x1

/* default GPIO <-> PIC connections */
#define DEFAULT_PIC_CLK    (int)((PB<<8)|15)   /* PGC - Output - PB15 */
#define DEFAULT_PIC_DATA   (int)((PB<<8)|17)   /* PGD - I/O - PB17 */
#define DEFAULT_PIC_MCLR   (int)((PI<<8)|15)   /* MCLR - Output - PB12 */
