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

class dspic33e : public Pic{

	public:

		void enter_program_mode(void);
		void exit_program_mode(void);
		bool read_device_id(void);
		void bulk_erase(void);
		void dump_configuration_registers(void);
		void read(char *outfile, uint32_t start, uint32_t count);
		void write(char *infile);
		void blank_check(void);

	protected:
		void send_cmd(uint32_t cmd);
		inline void send_prog_nop(void);
		uint16_t read_data(void);

		/*
		* DEVICES SECTION
		*                       ID       NAME           	  MEMSIZE
		*/
		pic_device piclist[1] = {{0x1861, "DSPIC33EP256MU806", 0x02ABFF}};
};
