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

#if defined(BOARD_A10)
#include "hosts/a10.h"
#elif defined(BOARD_RPI)
#include "hosts/rpi.h"
#elif defined(BOARD_AM335X)
#include "hosts/am335x.h"
#endif

#include "devices/device.h"

using namespace std;

#define VERSION "0.1"

/* Low-level functions */
void delay_us(unsigned int howLong);
void setup_io(void);
void close_io(void);

/* inhx.cpp functions */
unsigned int read_inhx(char *infile, memory *mem, uint32_t offset=0);
void write_inhx(memory *mem, char *outfile, uint32_t offset=0);

/* Runtime Functions */
void pic_reset(bool silent = false);

/* main functions */
void usage(void);
void server_mode(int port);
uint8_t send_file(char * filename);
uint8_t receive_file(int sock, char * filename);

extern volatile uint32_t *gpio;
extern int pic_clk, pic_data, pic_mclr;

struct flags_struct {
   int debug = 0;
   int client = 0;
   int noverify = 0;
   int boot_only = 0;
   int program_only = 0;
};

extern struct flags_struct flags;

#endif /* COMMON_H_ */
