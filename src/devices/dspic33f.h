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

class dspic33f : public Pic
{
  public:
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
    uint16_t read_data(void);

    /*
     * DEVICES SECTION
     *                       ID       NAME           	  MEMSIZE
     */
    pic_device piclist[140] = {
        {0x0C00,   "DSPIC33FJ06GS101", 0x000FFF},
        {0x0C01,   "DSPIC33FJ06GS102", 0x000FFF},
        {0x0C02,   "DSPIC33FJ06GS202", 0x000FFF},
        {0x0C04,   "DSPIC33FJ16GS402", 0x002BFF},
        {0x0C06,   "DSPIC33FJ16GS404", 0x002BFF},
        {0x0C03,   "DSPIC33FJ16GS502", 0x002BFF},
        {0x0C05,   "DSPIC33FJ16GS504", 0x002BFF},
        {0x0802,   "DSPIC33FJ12GP201", 0x001FFF},
        {0x0803,   "DSPIC33FJ12GP202", 0x001FFF},
        {0x0800,   "DSPIC33FJ12MC201", 0x001FFF},
        {0x0801,   "DSPIC33FJ12MC202", 0x001FFF},
        {0x080A,     "PIC24HJ12GP201", 0x001FFF},
        {0x080B,     "PIC24HJ12GP202", 0x001FFF},
        {0x0F07,   "DSPIC33FJ16GP304", 0x002BFF},
        {0x0F03,   "DSPIC33FJ16MC304", 0x002BFF},
        {0x0F17,     "PIC24HJ16GP304", 0x002BFF},
        {0x0F0D,   "DSPIC33FJ32GP202", 0x0057FF},
        {0x0F0F,   "DSPIC33FJ32GP204", 0x0057FF},
        {0x0F09,   "DSPIC33FJ32MC202", 0x0057FF},
        {0x0F0B,   "DSPIC33FJ32MC204", 0x0057FF},
        {0x0F1D,     "PIC24HJ32GP202", 0x0057FF},
        {0x0F1F,     "PIC24HJ32GP204", 0x0057FF},
        {0x00C1,   "DSPIC33FJ64GP206", 0x00ABFF},
        {0x00CD,   "DSPIC33FJ64GP306", 0x00ABFF},
        {0x00CF,   "DSPIC33FJ64GP310", 0x00ABFF},
        {0x00D5,   "DSPIC33FJ64GP706", 0x00ABFF},
        {0x00D6,   "DSPIC33FJ64GP708", 0x00ABFF},
        {0x00D7,   "DSPIC33FJ64GP710", 0x00ABFF},
        {0x0089,   "DSPIC33FJ64MC506", 0x00ABFF},
        {0x008A,   "DSPIC33FJ64MC508", 0x00ABFF},
        {0x008B,   "DSPIC33FJ64MC510", 0x00ABFF},
        {0x0091,   "DSPIC33FJ64MC706", 0x00ABFF},
        {0x0097,   "DSPIC33FJ64MC710", 0x00ABFF},
        {0x0041,     "PIC24HJ64GP206", 0x00ABFF},
        {0x0047,     "PIC24HJ64GP210", 0x00ABFF},
        {0x0049,     "PIC24HJ64GP506", 0x00ABFF},
        {0x004B,     "PIC24HJ64GP510", 0x00ABFF},
        {0x00D9,  "DSPIC33FJ128GP206", 0x0157FF},
        {0x00E5,  "DSPIC33FJ128GP306", 0x0157FF},
        {0x00E7,  "DSPIC33FJ128GP310", 0x0157FF},
        {0x00ED,  "DSPIC33FJ128GP706", 0x0157FF},
        {0x00EE,  "DSPIC33FJ128GP708", 0x0157FF},
        {0x00EF,  "DSPIC33FJ128GP710", 0x0157FF},
        {0x00A1,  "DSPIC33FJ128MC506", 0x0157FF},
        {0x00A3,  "DSPIC33FJ128MC510", 0x0157FF},
        {0x00A9,  "DSPIC33FJ128MC706", 0x0157FF},
        {0x00AE,  "DSPIC33FJ128MC708", 0x0157FF},
        {0x00AF,  "DSPIC33FJ128MC710", 0x0157FF},
        {0x005D,    "PIC24HJ128GP206", 0x0157FF},
        {0x005F,    "PIC24HJ128GP210", 0x0157FF},
        {0x0065,    "PIC24HJ128GP306", 0x0157FF},
        {0x0067,    "PIC24HJ128GP310", 0x0157FF},
        {0x0061,    "PIC24HJ128GP506", 0x0157FF},
        {0x0063,    "PIC24HJ128GP510", 0x0157FF},
        {0x00F5,  "DSPIC33FJ256GP506", 0x02ABFF},
        {0x00F7,  "DSPIC33FJ256GP510", 0x02ABFF},
        {0x00FF,  "DSPIC33FJ256GP710", 0x02ABFF},
        {0x00B7,  "DSPIC33FJ256MC510", 0x02ABFF},
        {0x00BF,  "DSPIC33FJ256MC710", 0x02ABFF},
        {0x0071,    "PIC24HJ256GP206", 0x02ABFF},
        {0x0073,    "PIC24HJ256GP210", 0x02ABFF},
        {0x007B,    "PIC24HJ256GP610", 0x02ABFF},
        {0x0605,   "DSPIC33FJ32GP302", 0x0057FF},
        {0x0607,   "DSPIC33FJ32GP304", 0x0057FF},
        {0x0601,   "DSPIC33FJ32MC302", 0x0057FF},
        {0x0603,   "DSPIC33FJ32MC304", 0x0057FF},
        {0x0615,   "DSPIC33FJ64GP202", 0x00ABFF},
        {0x0617,   "DSPIC33FJ64GP204", 0x00ABFF},
        {0x061D,   "DSPIC33FJ64GP802", 0x00ABFF},
        {0x061F,   "DSPIC33FJ64GP804", 0x00ABFF},
        {0x0611,   "DSPIC33FJ64MC202", 0x00ABFF},
        {0x0613,   "DSPIC33FJ64MC204", 0x00ABFF},
        {0x0619,   "DSPIC33FJ64MC802", 0x00ABFF},
        {0x061B,   "DSPIC33FJ64MC804", 0x00ABFF},
        {0x0625,  "DSPIC33FJ128GP202", 0x0157FF},
        {0x0627,  "DSPIC33FJ128GP204", 0x0157FF},
        {0x062D,  "DSPIC33FJ128GP802", 0x0157FF},
        {0x062F,  "DSPIC33FJ128GP804", 0x0157FF},
        {0x0621,  "DSPIC33FJ128MC202", 0x0157FF},
        {0x0623,  "DSPIC33FJ128MC204", 0x0157FF},
        {0x0629,  "DSPIC33FJ128MC802", 0x0157FF},
        {0x062B,  "DSPIC33FJ128MC804", 0x0157FF},
        {0x0645,     "PIC24HJ32GP302", 0x0057FF},
        {0x0647,     "PIC24HJ32GP304", 0x0057FF},
        {0x0655,     "PIC24HJ64GP202", 0x00ABFF},
        {0x0657,     "PIC24HJ64GP204", 0x00ABFF},
        {0x0675,     "PIC24HJ64GP502", 0x00ABFF},
        {0x0677,     "PIC24HJ64GP504", 0x00ABFF},
        {0x0665,    "PIC24HJ128GP202", 0x0157FF},
        {0x0667,    "PIC24HJ128GP204", 0x0157FF},
        {0x067D,    "PIC24HJ128GP502", 0x0157FF},
        {0x067F,    "PIC24HJ128GP504", 0x0157FF},
        {0x00C1,  "DSPIC33FJ64GP206A", 0x00ABFF},
        {0x00CD,  "DSPIC33FJ64GP306A", 0x00ABFF},
        {0x00CF,  "DSPIC33FJ64GP310A", 0x00ABFF},
        {0x00D5,  "DSPIC33FJ64GP706A", 0x00ABFF},
        {0x00D6,  "DSPIC33FJ64GP708A", 0x00ABFF},
        {0x00D7,  "DSPIC33FJ64GP710A", 0x00ABFF},
        {0x0089,  "DSPIC33FJ64MC506A", 0x00ABFF},
        {0x008A,  "DSPIC33FJ64MC508A", 0x00ABFF},
        {0x008B,  "DSPIC33FJ64MC510A", 0x00ABFF},
        {0x0091,  "DSPIC33FJ64MC706A", 0x00ABFF},
        {0x0097,  "DSPIC33FJ64MC710A", 0x00ABFF},
        {0x0041,    "PIC24HJ64GP206A", 0x00ABFF},
        {0x0047,    "PIC24HJ64GP210A", 0x00ABFF},
        {0x0049,    "PIC24HJ64GP506A", 0x00ABFF},
        {0x004B,    "PIC24HJ64GP510A", 0x00ABFF},
        {0x00D9, "DSPIC33FJ128GP206A", 0x0157FF},
        {0x00E5, "DSPIC33FJ128GP306A", 0x0157FF},
        {0x00E7, "DSPIC33FJ128GP310A", 0x0157FF},
        {0x00ED, "DSPIC33FJ128GP706A", 0x0157FF},
        {0x00EE, "DSPIC33FJ128GP708A", 0x0157FF},
        {0x00EF, "DSPIC33FJ128GP710A", 0x0157FF},
        {0x00A1, "DSPIC33FJ128MC506A", 0x0157FF},
        {0x00A3, "DSPIC33FJ128MC510A", 0x0157FF},
        {0x00A9, "DSPIC33FJ128MC706A", 0x0157FF},
        {0x00AE, "DSPIC33FJ128MC708A", 0x0157FF},
        {0x00AF, "DSPIC33FJ128MC710A", 0x0157FF},
        {0x005D,   "PIC24HJ128GP206A", 0x0157FF},
        {0x005F,   "PIC24HJ128GP210A", 0x0157FF},
        {0x0065,   "PIC24HJ128GP306A", 0x0157FF},
        {0x0067,   "PIC24HJ128GP310A", 0x0157FF},
        {0x0061,   "PIC24HJ128GP506A", 0x0157FF},
        {0x0063,   "PIC24HJ128GP510A", 0x0157FF},
        {0x07F5, "DSPIC33FJ256GP506A", 0x02ABFF},
        {0x07F7, "DSPIC33FJ256GP510A", 0x02ABFF},
        {0x07FF, "DSPIC33FJ256GP710A", 0x02ABFF},
        {0x07B7, "DSPIC33FJ256MC510A", 0x02ABFF},
        {0x07BF, "DSPIC33FJ256MC710A", 0x02ABFF},
        {0x0771,   "PIC24HJ256GP206A", 0x02ABFF},
        {0x0773,   "PIC24HJ256GP210A", 0x02ABFF},
        {0x077B,   "PIC24HJ256GP610A", 0x02ABFF},
        {0x4000,   "DSPIC33FJ32GS406", 0x0057FF},
        {0x4001,   "DSPIC33FJ64GS406", 0x00ABFF},
        {0x4002,   "DSPIC33FJ32GS606", 0x0057FF},
        {0x4003,   "DSPIC33FJ64GS606", 0x00ABFF},
        {0x4004,   "DSPIC33FJ32GS608", 0x0057FF},
        {0x4005,   "DSPIC33FJ64GS608", 0x00ABFF},
        {0x4006,   "DSPIC33FJ32GS610", 0x0057FF},
        {0x4007,   "DSPIC33FJ64GS610", 0x00ABFF}
    };
};
