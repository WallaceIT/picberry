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
#include <getopt.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <iostream>
#include <fstream>

#include "common.h"
#include "dspic.h"
#include "pic18fj.h"

int                	mem_fd;
void              	*gpio_map;
volatile uint32_t	*gpio;

bool debug=0, verify=1, client=0, log=0;

int pic_clk  = DEFAULT_PIC_CLK;
int pic_data = DEFAULT_PIC_DATA;
int pic_mclr = DEFAULT_PIC_MCLR;

/* Hardware delay function by Gordon's Projects - WiringPi */
void delay_us (unsigned int howLong)
{
   struct timeval tNow, tLong, tEnd ;

   gettimeofday (&tNow, 0) ;
   tLong.tv_sec  = howLong / 1000000 ;
   tLong.tv_usec = howLong % 1000000 ;
   timeradd (&tNow, &tLong, &tEnd) ;

   while (timercmp (&tNow, &tEnd, <))
     gettimeofday (&tNow, 0) ;
}

int main(int argc, char *argv[])
{
	int opt, function = 0;
	char *infile = 0;
	char *outfile = 0;
	char *logfile = 0;
	char *pins = 0;
	char *family = 0;
	short family_index=1;
	uint32_t count = 0, start = 0;
	int option_index = 0;

	static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"debug", 0, 0, 'D'},
			{"noverify", 0, 0, 'n'},
			{"gpio", 1, 0, 'g'},
			{"family", 1, 0, 'f'},
			{"read", 1, 0, 'r'},
			{"write", 0, 0, 'w'},
			{"erase", 0, 0, 'e'},
			{"blankcheck", 0, 0, 'b'},
			{"registerdump", 0, 0, 'd'},
			{"reset", 0, 0, 'R'},
			{"silent", 0, 0, 'x'},
			{"log", 1, 0, 'l'}
	};

	while ((opt = getopt_long(argc, argv, "hDl:ng:c:s:f:r:w:ebdRx",long_options, &option_index)) != -1) {
		switch (opt) {
		case 'h':
			usage();
			exit(0);
			break;
		case 'x':
			client = 1;
			break;
		case 'D':
			debug = 1;
			break;
		case 'n':
			verify = 0;
			break;
		case 'f':
			family = optarg;
			break;
		case 'g':
			pins = optarg;
		    break;
		case 'l':
			log = 1;
			logfile = optarg;
			break;
		case 'r':
			outfile = optarg;
			function |= 0x01;
			break;
		case 'c':
			count = atoi(optarg);
			break;
		case 's':
			start = atoi(optarg);
			break;
		case 'w':
			infile = optarg;
			function |= 0x02;
			break;
		case 'e':
			function |= 0x04;
			break;
		case 'b':
			function |= 0x08;
			break;
		case 'd':
			function |= 0x10;
			break;
        case 'R':
			function = 0x80;
			break;
		default:
			cout << endl;
			usage();
			exit(1);
		}
	}

	if (function == 0x02 && !infile) {
		cout << "Please specify an input file!" << endl;
		exit(1);
	}

	/* if not in log mode, disable stdout line buffering */
	if(!log){
		setvbuf(stdout, NULL, _IONBF, 1024);
	}

	/* If required, setup log file redirecting stdout */
	if(log){
		if(!client) cout << "picberry PIC Programmer v" << VERSION <<
							" - Output to log file." << endl;
		freopen(logfile?logfile:"picberry-log.txt","w",stdout);
	}

	cout << "picberry PIC Programmer v" << VERSION << endl;

	/* Setup gpio pointer for direct register access */
	if(debug) cout << "Setting up I/O...\n";

	setup_io();

	/* Configure GPIOs */
	if(pins != 0)    	// if GPIO connections are specified in the options...
	   sscanf(&pins[0], "%d,%d,%d", &pic_clk, &pic_data, &pic_mclr);
	if(debug){
	   cout << "PGC connected to pin " << pic_clk << endl;
	   cout << "PGD connected to pin " << pic_data << endl;
	   cout << "MCLR connected to pin " << pic_mclr << endl;
	}

	GPIO_IN(pic_clk); 	// NOTE: MUST use GPIO_IN before GPIO_OUT
	GPIO_OUT(pic_clk);

	GPIO_IN(pic_data);
	GPIO_OUT(pic_data);

	GPIO_IN(pic_mclr);      // MCLR as input, puts the output driver in Hi-Z

	GPIO_CLR(pic_clk);
	GPIO_CLR(pic_data);

	delay_us(1);      	// sleep for 1us after GPIO configuration

	if(function == 0x80){
		pic_reset();
		exit(0);
	}

	Pic *pic[2] = {new dspic(), new pic18fj()};

	if(family == 0 || strcmp(family, "dspic") == 0)
		family_index=0;
	else if(strcmp(family,"18fj") == 0)
		family_index=1;
	else{
		cout << "ERROR: PIC family not correctly chosen." << endl;
		goto clean;
	}

	/* ENTER PROGRAM MODE */
	pic[family_index] -> enter_program_mode();

	if(pic[family_index] -> read_device_id()){	// Read devide ID and setup memory

		switch (function){
			case 0x00:			// no function selected, exit
				break;
			case 0x01:
				pic[family_index]->read(outfile,start,count);
				break;
			case 0x02:
				pic[family_index]->write(infile);
				break;
			case 0x04:
				pic[family_index]->bulk_erase();
				break;
			case 0x08:
				pic[family_index]->blank_check();
				break;
			case 0x10:
				pic[family_index]->dump_configuration_registers();
				break;
			default:
				cout << endl << endl << "Please select only one option" <<
				"between -d, -b, -r, -w, -e." << endl;
				break;
		};
	}
	else{
		if(client) cerr << "MSG@";
			cerr << "ERROR: unknown/unsupported device "
					"or programmer not connected." << endl;
	}

	pic[family_index]->exit_program_mode();
	if(client) cerr << "END";

clean:

	if(!client && !log){
		cout << "Press ENTER to exit program mode...";
		fgetc(stdin);
	}

	GPIO_IN(pic_mclr);      /* MCLR as input, puts the output driver in Hi-Z */

	close_io();
	free(pic[family_index]->mem.location);
	free(pic[family_index]->mem.filled);

	fclose(stderr);
	fclose(stdout);
	return 0;
}

/* Set up a memory regions to access GPIO */
void setup_io(void)
{
        /* open /dev/mem */
        mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
        if (mem_fd == -1) {
                perror("Cannot open /dev/mem");
                exit(1);
        }

        /* mmap GPIO */
        gpio_map = mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE,
        				MAP_SHARED, mem_fd, GPIO_BASE);
        if (gpio_map == MAP_FAILED) {
                perror("mmap() failed");
                exit(1);
        }

        /* Always use volatile pointer! */
        gpio = (volatile uint32_t *) gpio_map;

}

/* Release GPIO memory region */
void close_io(void)
{
        int ret;

        /* munmap GPIO */
        ret = munmap(gpio_map, BLOCK_SIZE);
        if (ret == -1) {
            perror("munmap() failed");
            exit(1);
        }

        /* close /dev/mem */
        ret = close(mem_fd);
        if (ret == -1) {
        	perror("Cannot close /dev/mem");
            exit(1);
        }
}

/* reset the device */
void pic_reset(void)
{
    GPIO_OUT(pic_mclr);

    GPIO_CLR(pic_mclr);		// remove VDD from MCLR pin
	delay_us(1500);
	if(!client && !log){
		cout << "Press any key to release the reset...";
		fgetc(stdin);
		cout << endl;
	}
    GPIO_IN(pic_mclr);		// MCLR as input, puts the output driver in Hi-Z
}

/* print the help */
void usage(void)
{
	cout <<
"Usage: picberry [options]" << endl << endl <<
"   Programming Options" << endl << endl <<
"       -h                print help" << endl <<
"       -x                silent mode" << endl <<
"       -l [file]         redirect the output to log file(s)" << endl <<
"       -D                turn ON debug" << endl <<
"       -g PGC,PGD,MCLR   GPIO selection (optional)" << endl <<
"       -f family         PIC family (dspic or 18fj) [defaults to dsPIC33F]" << endl <<
"       -r [file.hex]     read chip to file [defaults to ofile.hex]" << endl <<
"       -w file.hex       bulk erase and write chip" << endl <<
"       -n                skip memory verification after writing" << endl <<
"       -e                bulk erase chip" << endl <<
"       -b                blank check of the chip" << endl <<
"       -d                read configuration registers" << endl << endl <<
"   Runtime Options" << endl << endl <<
"       -R                reset" << endl << endl;
}
