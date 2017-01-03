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

class pic24fjxxxga0xx : public Pic {

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
		pic_device piclist[17] = {{0x0444, "PIC24FJ16GA002", 0x002BFC},
					  {0x044C, "PIC24FJ16GA004", 0x002BFC},
					  {0x0445, "PIC24FJ32GA002", 0x0057FC},
					  {0x044D, "PIC24FJ32GA004", 0x0057FC},
					  {0x0446, "PIC24FJ48GA002", 0x0083FC},
					  {0x044E, "PIC24FJ48GA004", 0x0083FC},
					  {0x0447, "PIC24FJ64GA002", 0x00ABFC},
					  {0x044F, "PIC24FJ64GA004", 0x00ABFC},
					  {0x0405, "PIC24FJ64GA006", 0x00ABFC},
					  {0x0408, "PIC24FJ64GA008", 0x00ABFC},
					  {0x040B, "PIC24FJ64GA010", 0x00ABFC},
					  {0x0406, "PIC24FJ96GA006", 0x00FFFC},
					  {0x0409, "PIC24FJ96GA008", 0x00FFFC},
					  {0x040C, "PIC24FJ96GA010", 0x00FFFC},
					  {0x0407, "PIC24FJ128GA006", 0x0157FC},
					  {0x040A, "PIC24FJ128GA008", 0x0157FC},
					  {0x040D, "PIC24FJ128GA010", 0x0157FC}};
};
