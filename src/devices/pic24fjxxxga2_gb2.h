/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2014 Francesco Valla
 * Copyright 2016 Enric Balletbo i Serra
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

#include <iostream>

#include "../common.h"
#include "device.h"

using namespace std;

class pic24fjxxxga2_gb2 : public Pic {

	public:
		void enter_program_mode(void);
		void exit_program_mode(void);
		bool setup_pe(void){return true;};
		bool read_device_id(void);
		void bulk_erase(void);
		void dump_configuration_registers(void);
		void read(char *outfile, uint32_t start, uint32_t count);
		void write(char *infile);
		uint8_t blank_check(void);

	protected:
		void send_cmd(uint32_t cmd);
		uint16_t read_data(void);

		/*
		 *                         ID       NAME             MEMSIZE
		 */
		pic_device piclist[8] = {{0x4C5B, "PIC24FJ128GB204", 0x0157F8},
					 {0x4C5A, "PIC24FJ128GB202", 0x0157F8},
					 {0x4C59, "PIC24FJ64GB204",  0x00ABF8},
					 {0x4C58, "PIC24FJ64GB202",  0x00ABF8},
					 {0x4C53, "PIC24FJ128GA204", 0x0157F8},
					 {0x4C52, "PIC24FJ128GA202", 0x0157F8},
					 {0x4C51, "PIC24FJ64GA204",  0x00ABF8},
					 {0x4C50, "PIC24FJ64GA202",  0x00ABF8}};
};
