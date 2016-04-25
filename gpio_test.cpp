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

int                 mem_fd;
void                *gpio_map;
volatile uint32_t   *gpio;

void setup_io(void);
void close_io(void);

bool debug=0;

int tested_gpio = DEFAULT_PIC_CLK;
char tested_gpio_port = 0;

int main(int argc, char *argv[])
{
    char *pins = 0;
    int opt = 0, option_index = 0;

    static struct option long_options[] = {
            {"debug", 0, 0, 'D'},
            {"gpio", 1, 0, 'g'}
    };
    
    setvbuf(stdout, NULL, _IONBF, 1024);

    while ((opt = getopt_long(argc, argv, "Dg:",long_options, &option_index)) != -1) {
        switch (opt) {
        case 'D':
            debug = 1;
            break;
        case 'g':
            pins = optarg;
            break;
        default:
            cout << endl;
            exit(1);
        }
    }
    
    /* Configure GPIOs */
    if(pins != 0){       // if GPIO connections are specified in the options...
        if(!strchr(&pins[0],':'))   // port not specified
            sscanf(&pins[0], "%d", &tested_gpio);
        else{                       // port specified
            if(!sscanf(&pins[0], "%[A-Z]:%d", &tested_gpio_port, &tested_gpio)){
                        cout << "GPIO selection string not correctly formatted!" << endl;
                        exit(0);
                    }
            tested_gpio |= ((tested_gpio_port-'A')*PORTOFFSET)<<8;
        }
    }
    
    cout << "Testing GPIO " << tested_gpio_port << (tested_gpio&0xFF) << endl;
    cout << "BASE ADDRESS " << hex << GPIO_BASE << " + OFFSET " << hex << OFFSET(tested_gpio) << " = FINAL ADDRESS " << hex << GPIO_BASE + OFFSET(tested_gpio) << endl;

    setup_io();
    
    
    GPIO_IN(tested_gpio);   // NOTE: MUST use GPIO_IN before GPIO_OUT
    cout << "Read Test: level = " << (int)(GPIO_LEV(tested_gpio)) << endl;
    
    GPIO_OUT(tested_gpio);
    cout << "Blink Test";
    
    for(int k=0;k<10;k++){
        cout << ".";
        GPIO_SET(tested_gpio);
        usleep(500000L);
        GPIO_CLR(tested_gpio);
        usleep(500000L);
    }
    
    cout << "OK" << endl;
    
    GPIO_IN(tested_gpio);
    cout << "Read Test: ";
    for(int k=0;k<10;k++){
        cout << (int)(GPIO_LEV(tested_gpio)) << " ";
        GPIO_SET(tested_gpio);
        usleep(500000L);
        GPIO_CLR(tested_gpio);
        usleep(500000L);
    }
    
    cout << "OK" << endl;
    
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
        gpio_map = mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, GPIO_BASE);
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
