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

class pic24fxxka1xx : public Pic {

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
		pic_device piclist[16] = {{0x0D08, "PIC24F08KA101", 0x0015FF},
					  {0x0D01, "PIC24F16KA101", 0x002BFF},
					  {0x0D0A, "PIC24F08KA102", 0x0015FF},
					  {0x0D03, "PIC24F16KA102", 0x002BFF},
					  {0x4509, "PIC24FV16KA301", 0x002BFF},
					  {0x4508, "PIC24F16KA301", 0x002BFF},
					  {0x4503, "PIC24FV16KA302", 0x002BFF},
					  {0x4502, "PIC24F16KA302", 0x002BFF},
					  {0x4507, "PIC24FV16KA304", 0x002BFF},
					  {0x4506, "PIC24F16KA304", 0x002BFF},
					  {0x4519, "PIC24FV32KA301", 0x0057FF},
					  {0x4518, "PIC24F32KA301", 0x0057FF},
					  {0x4513, "PIC24FV32KA302", 0x0057FF},
					  {0x4512, "PIC24F32KA302", 0x0057FF},
					  {0x4517, "PIC24FV32KA304", 0x0057FF},
					  {0x4516, "PIC24F32KA304", 0x0057FF} };
};
