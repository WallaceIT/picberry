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

#include "../common.h"
#include "device.h"

using namespace std;

#define SF_DSPIC33E 0x00
#define SF_PIC24FJ  0x01

class dspic33e : public Pic
{
  public:
    dspic33e(uint8_t sf)
    {
        subfamily = sf;
    };
    void enter_program_mode(void);
    void exit_program_mode(void);
    bool setup_pe(void)
    {
        return true;
    };
    bool read_device_id(void);
    void bulk_erase(void);
    void dump_configuration_registers(void);
    void read(char *outfile, uint32_t start, uint32_t count);
    void write(char *infile);
    uint8_t blank_check(void);

  protected:
    void send_cmd(uint32_t cmd);
    inline void send_prog_nop(void);
    uint16_t read_data(void);

    /*
     * DEVICES SECTION
     *                       ID       NAME           	  MEMSIZE
     */
    pic_device piclist[28] = {
        {0x1861, "dsPIC33EP256MU806", 0x02ABFF},
        {0x1862, "dsPIC33EP256MU810", 0x02ABFF},
        {0x1863, "dsPIC33EP256MU814", 0x02ABFF},
        {0x1826,   "PIC24EP256GU810", 0x02ABFF},
        {0x1827,   "PIC24EP256GU814", 0x02ABFF},
        {0x187D, "dsPIC33EP512GP806", 0x02ABFF},
        {0x1879, "dsPIC33EP512MC806", 0x0557FF},
        {0x1872, "dsPIC33EP512MU810", 0x0557FF},
        {0x1873, "dsPIC33EP512MU814", 0x0557FF},
        {0x183D,   "PIC24EP512GP806", 0x0557FF},
        {0x1836,   "PIC24EP512GU810", 0x0557FF},
        {0x1837,   "PIC24EP512GU814", 0x0557FF},
        {0x6000,   "PIC24FJ128GA606", 0x015FFF},
        {0x6008,   "PIC24FJ256GA606", 0x02AFFF},
        {0x6010,   "PIC24FJ512GA606", 0x055FFF},
        {0x6018,  "PIC24FJ1024GA606", 0x0ABFFF},
        {0x6001,   "PIC24FJ128GA610", 0x015FFF},
        {0x6009,   "PIC24FJ256GA610", 0x02AFFF},
        {0x6011,   "PIC24FJ512GA610", 0x055FFF},
        {0x6019,  "PIC24FJ1024GA610", 0x0ABFFF},
        {0x6004,   "PIC24FJ128GB606", 0x015FFF},
        {0x600C,   "PIC24FJ256GB606", 0x02AFFF},
        {0x6014,   "PIC24FJ512GB606", 0x055FFF},
        {0x601C,  "PIC24FJ1024GB606", 0x0ABFFF},
        {0x6005,   "PIC24FJ128GB610", 0x015FFF},
        {0x600D,   "PIC24FJ256GB610", 0x02AFFF},
        {0x6015,   "PIC24FJ512GB610", 0x055FFF},
        {0x601D,  "PIC24FJ1024GB610", 0x0ABFFF}
    };
};
