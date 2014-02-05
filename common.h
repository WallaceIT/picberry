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

/*
 * Define BOARD_A10 for Allwinner A10, or BOARD_RPI for RaspberryPi
 * (normally commented out because defined in makefile)
 */
//#define BOARD_RPI
//#define BOARD_A10

#if defined(BOARD_A10)
#include "a10.h"
#elif defined(BOARD_RPI)
#include "rpi.h"
#endif

using namespace std;

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
		virtual bool read_device_id(void) = 0;
		virtual void bulk_erase(void) = 0;
		virtual void dump_configuration_registers(void) = 0;
		virtual void read(char *outfile, uint32_t start, uint32_t count) = 0;
		virtual void write(char *infile) = 0;
		virtual void blank_check(void) = 0;
};

/* Low-level functions */
void delay_us(unsigned int howLong);
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
extern bool verify;

#endif /* COMMON_H_ */
