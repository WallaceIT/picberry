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
 
 /* PORT OFFSETs */
#define GPIO0_BASE 0x44E07000
#define GPIO1_BASE 0x4804C000
#define GPIO2_BASE 0x481AC000
#define GPIO3_BASE 0x481AE000
#define GPIO_BASE GPIO0_BASE
#define PORTOFFSET  0

#define BANK_SIZE  0x00000FFF
#define BLOCK_SIZE (GPIO3_BASE+BANK_SIZE-GPIO0_BASE)

/* OE: 0 is output, 1 is input */
#define GPIO_OE_REG 0x4d
#define GPIO_IN_REG 0x4e
#define GPIO_OUT_REG 0x4f

#define OFFSET(g) ((int)((bool)(g/32))*(GPIO1_BASE-GPIO0_BASE)+(int)((bool)(g/64))*(GPIO2_BASE-GPIO1_BASE)+(int)((bool)(g/96))*(GPIO3_BASE-GPIO2_BASE))/4

/* GPIO setup macros. Always use GPIO_IN(x) before using GPIO_OUT(x) */
#define GPIO_OUT(g)   *(gpio+OFFSET(g)+GPIO_OE_REG) &= ~(0x01<<(g%32))
#define GPIO_IN(g)    *(gpio+OFFSET(g)+GPIO_OE_REG) |= (0x01<<(g%32))

#define GPIO_SET(g)   *(gpio+OFFSET(g)+GPIO_OUT_REG) |= (0x01<<(g%32))
#define GPIO_CLR(g)   *(gpio+OFFSET(g)+GPIO_OUT_REG) &= ~(0x01<<(g%32))
#define GPIO_LEV(g)   (*(gpio+OFFSET(g)+GPIO_IN_REG) >> (g%32)) & 0x01

/* default GPIO <-> PIC connections */
#define DEFAULT_PIC_CLK    60   /* PGC  - Output - gpio1_28 */
#define DEFAULT_PIC_DATA   49   /* PGD  - I/O    - gpio1_17 */
#define DEFAULT_PIC_MCLR   48   /* MCLR - Output - gpio1_16 */
