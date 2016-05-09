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

#include <iostream>

#include "common.h"

using namespace std;

class pic18fj: public Pic{

	public:

		void enter_program_mode(void);
		void exit_program_mode(void);
		bool read_device_id(void);
		void bulk_erase(void);
		void dump_configuration_registers(void);
		void read(char *outfile, uint32_t start, uint32_t count);
		void write(char *infile);
		uint8_t blank_check(void);

	protected:
		void send_cmd(uint8_t cmd);
		uint16_t read_data(void);
		void write_data(uint16_t data);
		void goto_mem_location(uint32_t data);

		/*
		* DEVICES SECTION
		*                    	ID       NAME           MEMSIZE
		*/
		pic_device piclist[22] = {{0x1D20,  "PIC18F44J10", 0x2000},
								{0x1C20,  "PIC18F45J10", 0x4000},
								{0x4D80,  "PIC18F24J11", 0x2000},
								{0x4DA0,  "PIC18F25J11", 0x4000},
								{0x4DC0,  "PIC18F26J11", 0x8000},
								{0x4DE0,  "PIC18F44J11", 0x2000},
								{0x4E00,  "PIC18F45J11", 0x4000},
								{0x4E20,  "PIC18F46J11", 0x8000},
								{0x4C00,  "PIC18F24J50", 0x2000},
								{0x4C20,  "PIC18F25J50", 0x4000},
								{0x4C40,  "PIC18F26J50", 0x8000},
								{0x4C60,  "PIC18F44J50", 0x2000},
								{0x4C80,  "PIC18F45J50", 0x4000},
								{0x4CA0,  "PIC18F46J50", 0x8000},
								{0x5920,  "PIC18F26J13", 0x8000},
								{0x59A0,  "PIC18F46J13", 0x8000},
								{0x5820,  "PIC18F26J53", 0x8000},
								{0x58A0,  "PIC18F46J53", 0x8000},
								{0x5960,  "PIC18F27J13", 0x10000},
								{0x59E0,  "PIC18F47J13", 0x10000},
								{0x5860,  "PIC18F27J53", 0x10000},
								{0x58E0,  "PIC18F47J53", 0x10000}};
};
