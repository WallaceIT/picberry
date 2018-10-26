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

class pic24fjxxxga1_gb1 : public Pic {

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
		pic_device piclist[21] = {{0x1008, "PIC24FJ128GA106", 0x0157FA},
					  {0x1010, "PIC24FJ192GA106", 0x020BFA},
					  {0x1018, "PIC24FJ256GA106", 0x02ABFA},
					  {0x100A, "PIC24FJ128GA100", 0x0157FA},
					  {0x1012, "PIC24FJ192GA108", 0x020BFA},
					  {0x101A, "PIC24FJ256GA108", 0x02ABFA},
					  {0x100E, "PIC24FJ128GA110", 0x0157FA},
					  {0x1016, "PIC24FJ192GA110", 0x020BFA},
					  {0x101E, "PIC24FJ256GA110", 0x02ABFA},
					  {0x1001, "PIC24FJ64GB106", 0x00ABFA},
					  {0x1009, "PIC24FJ128GB106", 0x0157FA},
					  {0x1011, "PIC24FJ192GB106", 0x020BFA},
					  {0x1019, "PIC24FJ256GB106", 0x02ABFA},
					  {0x1003, "PIC24FJ64GB108", 0x00ABFA},
					  {0x100B, "PIC24FJ128GB108", 0x0157FA},
					  {0x1013, "PIC24FJ192GB108", 0x020BFA},
					  {0x101B, "PIC24FJ256GB108", 0x02ABFA},
					  {0x1007, "PIC24FJ64GB110", 0x00ABFA},
					  {0x100F, "PIC24FJ128GB110", 0x0157FA},
					  {0x1017, "PIC24FJ192GB110", 0x020BFA},
					  {0x101F, "PIC24FJ256GB110", 0x02ABFA}};
};
