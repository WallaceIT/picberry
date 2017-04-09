/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2014 Francesco Valla
 * Copyright 2016 Enric Balletbo i Serra
 * Copyright 2017 Nicola Chiesa
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

class pic24fjxxxga3xx : public Pic {

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
		pic_device piclist[24] = {{0x4100, "PIC24FJ128GB206", 0x0157F8},
															{0x4102, "PIC24FJ128GB210", 0x0157F8},
															{0x4104, "PIC24FJ256GB206", 0x02AFF8},
															{0x4106, "PIC24FJ256GB210", 0x02AFF8},
															{0x4108, "PIC24FJ128DA206", 0x0157F8},
															{0x4109, "PIC24FJ128DA106", 0x0157F8},
															{0x410A, "PIC24FJ128DA210", 0x0157F8},
															{0x410B, "PIC24FJ128DA110", 0x0157F8},
															{0x410C, "PIC24FJ256DA206", 0x02AFF8},
															{0x410D, "PIC24FJ256DA106", 0x02AFF8},
															{0x410E, "PIC24FJ256DA210", 0x02AFF8},
															{0x410F, "PIC24FJ256DA110", 0x02AFF8},
															{0x46C0, "PIC24FJ64GA306", 0x00ABF8},
															{0x46C2, "PIC24FJ128GA306", 0x0157F8},
															{0x46C4, "PIC24FJ64GA308", 0x00ABF8},
															{0x46C6, "PIC24FJ128GA308", 0x00ABF8},
															{0x46C8, "PIC24FJ64GA310", 0x00ABF8},
															{0x46CA, "PIC24FJ128GA310", 0x00ABF8},
															{0x4884, "PIC24FJ64GC010", 0x00ABF8},
															{0x4885, "PIC24FJ128GC010", 0x00ABF8},
															{0x4888, "PIC24FJ64GC006", 0x00ABF8},
															{0x4889, "PIC24FJ128GC006", 0x00ABF8},
															{0x488A, "PIC24FJ64GC008", 0x00ABF8},
															{0x488B, "PIC24FJ128GC008", 0x00ABF8}};
};
