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

#ifndef COMMON_H_
#define COMMON_H_

using namespace std;

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

#define VERSION 0.1

struct memory{
		uint32_t	program_memory_size;   	// size in WORDS (16bits each)
		uint32_t	code_memory_size;		// size in WORDS (16bits each)
		uint16_t	*location;		// 16-bit data
		bool		*filled;		// 1 if the corresponding location is used
};

struct pic_device{
	uint16_t    device_id;
	char        name[25];
	int			code_memory_size;	/* size in WORDS (16bits each)  */
};

class Pic{

	public:
		uint16_t 		device_id;
		char			name[25];
		memory mem;

		Pic(){device_id=0;};	// Constructor
		virtual ~Pic(){};		// Destructor

		virtual void enter_program_mode(void) = 0;
		virtual void exit_program_mode(void) = 0;
		virtual void read_device_id(void) = 0;
		virtual void bulk_erase(void) = 0;
		virtual void dump_configuration_registers(void) = 0;
		virtual void read(char *outfile, uint32_t start, uint32_t count) = 0;
		virtual void write(char *infile) = 0;
		virtual void blank_check(void) = 0;
};

/* Low-level functions */
void delay_us (unsigned int howLong);
void setup_io(void);
void close_io(void);

/* inhx.cpp functions */
void read_inhx(char *infile, memory *mem);
void write_inhx(memory *mem, char *outfile);

/* Runtime Functions */
void pic_reset(void);

/* main functions */
void usage(void);

extern volatile uint32_t *gpio;
extern int pic_clk, pic_data, pic_mclr;
extern bool debug;

#endif /* COMMON_H_ */
