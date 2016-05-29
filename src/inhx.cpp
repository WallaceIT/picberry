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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include "common.h"

using namespace std;

/*
 * Read a file in Intel HEX8M or HEX32 format and fill the memory structure
 * Returns the number of filled locations
 *
 */
unsigned int read_inhx(char *infile, memory *mem, uint32_t offset)
{
        FILE *fp;
        int linenum;
        char line[256], *ptr;
        size_t linelen;
        int nread;

        unsigned int filled_locations=0;

        uint16_t i=0;
        uint8_t  byte_count;
        uint16_t base_address = 0x0000;
        uint16_t address;
        uint32_t extended_address;
        uint8_t  record_type;
        uint16_t data, tmp;
        uint8_t  checksum_calculated;
        uint8_t  checksum_read;

        fp = fopen(infile, "r");
        if (fp == NULL) {
        	cerr << "Error: cannot open source file " << infile << endl;
        	return 0;
        }

        if(flags.debug) cerr << "Reading hex file..." << endl;

        linenum = 0;
        while (1) {
        	ptr = fgets(line, 256, fp);

            if (ptr != NULL) {
            	linenum++;
                linelen = strlen(line);
                if (flags.debug) {
                	fprintf(stderr, "  line %d (%zd bytes): '", linenum, linelen);
                    for (i = 0; i < linelen; i++) {
                    	if (line[i] == '\n')
                    		cerr << "\\n";
                    	else if (line[i] == '\r')
                    		cerr << "\\r";
                    	else
                    		fprintf(stderr, "%c", line[i]);
                    }
                    cerr << "'\n";
                }

                if (line[0] != ':') {
                	cerr << "Error: invalid start code."  << endl;
                	return 0;
                }

                nread = sscanf(&line[1], "%2hhx", &byte_count);
                if (nread != 1) {
                	cerr << "Error: cannot read byte count." << endl;
                	return 0;
                }
                if (flags.debug) fprintf(stderr, "  byte_count  = 0x%02X\n", byte_count);

                nread = sscanf(&line[3], "%4hx", &address);
                if (nread != 1) {
                	cerr << "Error: cannot read address." << endl;
                	return 0;
                }
                        
                nread = sscanf(&line[7], "%2hhx", &record_type);
                if (nread != 1) {
                	cerr << "Error: cannot read record type." << endl;
                	return 0;
                }

                if (flags.debug && record_type != 0x04) fprintf(stderr, "  address     = 0x%04X\n", address);

                if (flags.debug)
                	fprintf(stderr, "  record_type = 0x%02X (%s)\n",
                			record_type, record_type == 0 ? "data" :
                				(record_type == 1 ? "EOF" :
                					(record_type == 0x04 ? "Extended Linear Address" : "Unknown")));

                if (record_type != 0 && record_type != 1 && record_type != 0x04) {
                	cerr << "Error: unknown record type." << endl;
                	return 0;
                }

                checksum_calculated  = byte_count;
                checksum_calculated += (address >> 8) & 0xFF;
                checksum_calculated += address & 0xFF;
                checksum_calculated += record_type;

                if(record_type == 0x04){
                	nread = sscanf(&line[9], "%4hx", &base_address);
                	if (flags.debug) fprintf(stderr, "  NEW BASE ADDRESS     = 0x%04X\n", base_address);
                	checksum_calculated += (base_address >> 8) & 0xFF;
                	checksum_calculated += base_address & 0xFF;
                	i = 1;
                }
                else
                	for (i = 0; i < byte_count/2; i++) {
                		nread = sscanf(&line[9+4*i], "%4hx", &data);
                		if (nread != 1) {
                			cerr << "Error: cannot read data." << endl;
                			return 0;
                		}
                		tmp = data;
                		data = (data >> 8) | (tmp << 8);
                		if (flags.debug) fprintf(stderr, "  data        = 0x%04X", data);
                		checksum_calculated += (data >> 8) & 0xFF;
                		checksum_calculated += data & 0xFF;

                		extended_address = ( ((uint32_t)base_address << 16) | address);
                		if (flags.debug)
                			fprintf(stderr, " @0x%08X\n", extended_address/2+i);
						
                		mem->location[extended_address/2 + i - offset/2] = data;
                		mem->filled[extended_address/2 + i - offset/2] = 1;
                		filled_locations++;
                	}

                checksum_calculated = (checksum_calculated ^ 0xFF) + 1;

                nread = sscanf(&line[9+4*i], "%2hhx", &checksum_read);
                if (nread != 1) {
                	cerr << "Error: cannot read checksum." << endl;
                	return 0;
                }
                if (flags.debug) fprintf(stderr, "  checksum    = 0x%02X\n", checksum_read);

                if (checksum_calculated != checksum_read) {
                	cerr << "Error: checksum does not match. ";

                	if(flags.debug)
                		fprintf(stderr, "Calculated = 0x%02X, Read = 0x%02X\n", checksum_calculated, checksum_read);
                	return 0;
                }

                if (flags.debug)
                	cerr << "\n";

                if (record_type == 0x01)
                	break;
            }
            else {
            	cerr << "Error: unexpected EOF." << endl;
            	return 0;
            }
	}

    fclose(fp);
	
    if(flags.debug)
		cerr << "DONE! " << filled_locations << " memory locations read." << endl;

    return filled_locations;
}

/* Write the filled cells in given memory struct
 * to an Intel HEX8M or HEX32 file */
void write_inhx(memory *mem, char *outfile, uint32_t offset)
{
	FILE *fp;
	uint32_t base, j, k, start, stop;
	uint8_t  byte_count;
	uint32_t address;
	uint16_t base_address = 0x0000;
	uint8_t  record_type;
	uint16_t data, tmp;
	uint8_t  checksum;

	fp = fopen(outfile?outfile:"ofile.hex", "w");
	if (fp == NULL) {
		cerr << "Error: cannot open destination file " << outfile << endl;
		return;
	}

	if(flags.debug)
		cerr << "Writing hex file...";

	/* Write the program memory bytes */

	for (base = 0; base < mem -> program_memory_size; ){

		for (j = 0 ; j < (mem -> program_memory_size - base); j++)
			if (mem -> filled[base+j]) break;

		start = j;

		for ( ; j < (mem -> program_memory_size - base); j++)
			if (!mem -> filled[base+j] || (j-start == 8) ) break;

		stop = j;

		byte_count  = (stop - start)*2;

		if (byte_count > 0) {
			address = (base + start)*2+offset;
			record_type = 0x00;

			if(mem -> program_memory_size >= 0x10000 && (address >> 16) != base_address){  //extended linear address
				base_address = (address >> 16);
				fprintf(fp, ":02000004%04x", base_address);
				checksum = 0x06 + ((base_address>>8) & 0xFF) + (base_address & 0xFF);
				checksum = (checksum ^ 0xFF) + 1;
				fprintf(fp, "%02x\n", checksum);
			}

			fprintf(fp, ":%02x%04x%02x", byte_count, (address & 0x0000FFFF), record_type);

			checksum  = byte_count;
			checksum += ((address & 0x0000FFFF) >> 8) & 0xFF;
			checksum += (address & 0x0000FFFF) & 0xFF;
			checksum += record_type;

			for (k = start; k < stop; k++) {
				data = mem -> location[base+k];
				tmp = data;
				data = (data >> 8) | (tmp << 8);
				fprintf(fp, "%04x", data);
				checksum += (data >> 8) & 0xFF;
				checksum += data & 0xFF;
			}

			checksum = (checksum ^ 0xFF) + 1;
			fprintf(fp, "%02x\n", checksum);

		}
		base += stop;
	}

	fprintf(fp, ":00000001FF\n");
	fclose(fp);
	if(flags.debug)
		cerr << "DONE!" << endl;
}
