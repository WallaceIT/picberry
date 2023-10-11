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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <fstream>

#include "common.h"
#include "devices/dspic33f.h"
#include "devices/dspic33e.h"
#include "devices/pic10f322.h"
#include "devices/pic18fj.h"
#include "devices/pic24fjxxxga0xx.h"
#include "devices/pic24fjxxxga3xx.h"
#include "devices/pic24fjxxga1xx_gb0xx.h"
#include "devices/pic32.h"
#include "devices/pic24fjxxxga1_gb1.h"
#include "devices/pic24fjxxxga2_gb2.h"
#include "devices/pic24fxxka1xx.h"


#if defined(BOARD_RK3399)
#include "wiringx.h"
#endif

int                 mem_fd;
void                *gpio_map;
volatile uint32_t   *gpio;

struct flags_struct flags;

int pic_clk  = DEFAULT_PIC_CLK;
int pic_data = DEFAULT_PIC_DATA;
int pic_mclr = DEFAULT_PIC_MCLR;
char pic_clk_port=0, pic_data_port=0, pic_mclr_port=0;

#define FXN_NULL        0b00000000
#define FXN_RESET       0b00000001
#define FXN_SERVER      0b00000010
#define FXN_READ        0b00000100
#define FXN_WRITE       0b00001000
#define FXN_ERASE       0b00010000
#define FXN_BLANKCHEK   0b00100000
#define FXN_REGDUMP     0b01000000

/* Hardware delay function by Gordon's Projects - WiringPi */
void delay_us (unsigned int howLong)
{
	struct timeval tNow, tLong, tEnd;

	gettimeofday (&tNow, 0);
	tLong.tv_sec  = howLong / 1000000;
	tLong.tv_usec = howLong % 1000000;
	timeradd (&tNow, &tLong, &tEnd);

	while (timercmp (&tNow, &tEnd, <))
		gettimeofday (&tNow, 0);
}

int main(int argc, char *argv[])
{
	int opt, function = 0;
    char *infile = 0;
    char *outfile = 0;
    bool log = false;
    char *logfile = 0;
    char *pins = 0;
    char *family = 0;
    uint32_t count = 0, start = 0;
    int option_index = 0;
    int server_port = 15000;
    uint8_t retval = 0;

    static struct option long_options[] = {
            {"help",        no_argument,       0,           'h'},
            {"server",      required_argument, 0,           'S'},
            {"gpio",        required_argument, 0,           'g'},
            {"family",      required_argument, 0,           'f'},
            {"read",        required_argument, 0,           'r'},
            {"write",       no_argument,       0,           'w'},
            {"erase",       no_argument,       0,           'e'},
            {"blankcheck",  no_argument,       0,           'b'},
            {"regdump",     no_argument,       0,           'd'},
            {"reset",       no_argument,       0,           'R'},
            {"log",         required_argument, 0,           'l'},
            {"debug",       no_argument,       &flags.debug,        1},
            {"noverify",    no_argument,       &flags.noverify,     1},
            {"boot-only",   no_argument,       &flags.boot_only,    1},
            {"program-only",no_argument,       &flags.program_only, 1},
	    {"fulldump",    no_argument,       &flags.fulldump,     1},
            {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hS:l:g:c:s:f:r:w:ebdR",
                              long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                break;
            case 'h':
                usage();
                exit(0);
                break;
            case 'S':
                flags.client = 1;
                server_port = atoi(optarg);
                function = FXN_SERVER;
                break;
            case 'f':
                family = optarg;
                break;
            case 'g':
                pins = optarg;
                break;
            case 'l':
                log = true;
                logfile = optarg;
                break;
            case 'r':
                outfile = optarg;
                function |= FXN_READ;
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case 's':
                start = atoi(optarg);
                break;
            case 'w':
                infile = optarg;
                function |= FXN_WRITE;
                break;
            case 'e':
                function |= FXN_ERASE;
                break;
            case 'b':
                function |= FXN_BLANKCHEK;
                break;
            case 'd':
                function |= FXN_REGDUMP;
                break;
            case 'R':
                function = FXN_RESET;
                break;
            default:
                cout << endl;
                usage();
                exit(1);
        }
    }

    if (function & FXN_WRITE && !infile) {
        cout << "Please specify an input file!" << endl;
        exit(1);
    }

    /* if not in log mode, disable stdout line buffering */
    if(!log){
        setvbuf(stdout, NULL, _IONBF, 1024);
    }

    /* If required, setup log file redirecting stdout */
    if(log){
        cout << "picberry PIC Programmer v" << VERSION << " - Output to log file." << endl;
        freopen(logfile?logfile:"picberry-log.txt","w",stdout);
    }

    cout << "picberry PIC Programmer v" << VERSION << endl;

    /* Configure GPIOs */
    if(pins != 0){       // if GPIO connections are specified in the options...
        if(!strchr(&pins[0],':'))   // port not specified
            sscanf(&pins[0], "%d,%d,%d", &pic_clk, &pic_data, &pic_mclr);
        else{                       // port specified
            if(!sscanf(&pins[0],
                    "%[A-Z]:%d,%[A-Z]:%d,%[A-Z]:%d",
                    &pic_clk_port, &pic_clk,
                    &pic_data_port, &pic_data,
                    &pic_mclr_port, &pic_mclr)){
                        cout << "GPIO selection string not correctly formatted!"
                             << endl;
                        exit(0);
                    }
#if defined(BOARD_RK3399)
            pic_clk = (pic_clk_port-'A');
            pic_data = (pic_data_port-'A');
            pic_mclr = (pic_mclr_port-'A');
#else
            pic_clk |= ((pic_clk_port-'A')*PORTOFFSET)<<8;
            pic_data |= ((pic_data_port-'A')*PORTOFFSET)<<8;
            pic_mclr |= ((pic_mclr_port-'A')*PORTOFFSET)<<8;
#endif
        }
    }
    
    if(flags.debug){
        cout << "PGC <=> pin " << pic_clk_port << (pic_clk&0xFF)
             << endl;
        cout << "PGD <=> pin " << pic_data_port << (pic_data&0xFF)
             << endl;
        cout << "MCLR <=> pin " << pic_mclr_port << (pic_mclr&0xFF)
             << endl;
    }

    /* Setup gpio pointer for direct register access */
    if(flags.debug) cout << "Setting up I/O..." << endl;
    setup_io();

    if(function == FXN_RESET)
        pic_reset();
    else if(function == FXN_SERVER)
        server_mode(server_port);
    else{

        Pic *pic = new dspic33f();

        if(family == 0 || strcmp(family, "dspic33f") == 0);
            //pic = new dspic33f(); default case, nothing to do
        else if(strcmp(family,"dspic33e") == 0)
            pic = new dspic33e(SF_DSPIC33E);
        else if(strcmp(family,"pic24fj") == 0)
            pic = new dspic33e(SF_PIC24FJ);
        else if(strcmp(family,"pic10f322") == 0)
            pic = new pic10f322();
        else if(strcmp(family,"pic18fj") == 0)
            pic = new pic18fj();
        else if(strcmp(family,"pic24fjxxxga0xx") == 0)
            pic = new pic24fjxxxga0xx();
        else if(strcmp(family,"pic24fjxxxga3xx") == 0)
            pic = new pic24fjxxxga3xx();
        else if(strcmp(family,"pic24fjxxga1xx") == 0)
            pic = new pic24fjxxga1xx_gb0xx();
        else if(strcmp(family,"pic24fjxxgb0xx") == 0)
            pic = new pic24fjxxga1xx_gb0xx();
        else if(strcmp(family,"pic24fjxxxga1xx") == 0)
            pic = new pic24fjxxxga1_gb1();
        else if(strcmp(family,"pic24fjxxxga2xx") == 0)
            pic = new pic24fjxxxga2_gb2();
        else if(strcmp(family,"pic24fjxxxgb1xx") == 0)
            pic = new pic24fjxxxga1_gb1();
        else if(strcmp(family,"pic24fjxxxgb2xx") == 0)
            pic = new pic24fjxxxga2_gb2();
	else if(strcmp(family,"pic24fxxka1xx") == 0)
            pic = new pic24fxxka1xx();
	else if(strcmp(family,"pic32mx1") == 0)
            pic = new pic32(SF_PIC32MX1);
        else if(strcmp(family,"pic32mx2") == 0)
            pic = new pic32(SF_PIC32MX2);
        else if(strcmp(family,"pic32mx3") == 0)
            pic = new pic32(SF_PIC32MX3);
        else if(strcmp(family,"pic32mz") == 0)
            pic = new pic32(SF_PIC32MZ);
        else if(strcmp(family,"pic32mk") == 0)
            pic = new pic32(SF_PIC32MK);   
        else{
            cerr << "ERROR: PIC family not correctly chosen." << endl;
            cerr << "Available families:" << endl
                 << "- dspic33e" << endl
                 << "- pic24fj" << endl
                 << "- pic24fjxxxga0xx" << endl
                 << "- pic24fjxxxga3xx" << endl
                 << "- pic24fjxxga1xx" << endl
                 << "- pic24fjxxgb0xx" << endl 
                 << "- pic24fjxxxga1xx" << endl
                 << "- pic24fjxxxga2xx" << endl
                 << "- pic24fjxxxgb1xx" << endl
                 << "- pic24fjxxxgb2xx" << endl
		 << "- pic24fxxka1xx" << endl 
		 << "- pic10f322" << endl
                 << "- pic18fj" << endl
                 << "- pic32mx1" << endl
                 << "- pic32mx2" << endl
                 << "- pic32mx3" << endl
                 << "- pic32mz" << endl
                 << "- pic32mk" << endl;
            goto clean;
        }

        /* ENTER PROGRAM MODE */
        pic -> enter_program_mode();
        pic -> setup_pe();

        if(pic -> read_device_id()){  // Read devide ID and setup memory
        
            fprintf(stdout,"Device Name: %s\n", pic->name);
		    fprintf(stdout,"Device ID: 0x%08x\n", pic->device_id);
            fprintf(stderr,"Revision: 0x%08x\n", pic->device_rev);

            switch (function){
                case FXN_NULL:          // no function selected, exit
                    break;
                case FXN_READ:
                    cout << "Reading chip...";
                    pic->read(outfile,start,count);
                    cout << "DONE! " << endl;
                    break;
                case FXN_WRITE:
                    cout << "Writing chip...";
                    pic->write(infile);
                    cout << "DONE! " << endl;
                    break;
                case FXN_ERASE:
                	cout << "Bulk Erase...";
                    pic->bulk_erase();
                    cout << "DONE!" << endl;
                    break;
                case FXN_BLANKCHEK:
                    cout << "Blank check...";
                    retval = pic->blank_check();
                    if(retval == 0)
                        cout << "chip is blank." << endl;
                    else
                        cout << "chip is not blank." << endl;
                    break;
                case FXN_REGDUMP:
                    pic->dump_configuration_registers();
                    break;
                default:
                    cout << endl << endl << "Please select only one option" <<
                    "between -d, -b, -r, -w, -e." << endl;
                    break;
            };
        }
        else{
		    fprintf(stdout,"Device ID: 0x%x\n", pic ->device_id);
            cout << "ERROR: unknown/unsupported device "
                    "or programmer not connected." << endl;
        }
            

        pic->exit_program_mode();
        
        if(!log){
            cout << "Press ENTER to exit program mode...";
            fgetc(stdin);
        }
        
        /* Free memory */
        free(pic->mem.location);
        free(pic->mem.filled);
    }

clean:
    /* Release the MCLR pin and clean up I\O structures */
    close_io();

    fclose(stderr);
    fclose(stdout);
    return 0;
}

/* Set up a memory regions to access GPIO */
void setup_io(void)
{
    
#if defined(BOARD_RK3399)
    wiringXSetup(WIRINGX_PLATFORM, NULL);
#else
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

#endif
        
    GPIO_IN(pic_clk);   // NOTE: MUST use GPIO_IN before GPIO_OUT
    GPIO_OUT(pic_clk);
    
    GPIO_IN(pic_data);
    GPIO_OUT(pic_data);
    
    GPIO_IN(pic_mclr);      // MCLR as input, puts the output driver in Hi-Z

    GPIO_CLR(pic_clk);
    GPIO_CLR(pic_data);

    delay_us(1);        // sleep for 1us after GPIO configuration
}

/* Release GPIO memory region */
void close_io(void)
{       
    #if defined(BOARD_RK3399)
        GPIO_IN(pic_mclr);
    return;
    #else    
        int ret;

        /* MCLR as input, puts the output driver in Hi-Z */
        GPIO_IN(pic_mclr);
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
    #endif
}

/* reset the device */
void pic_reset(bool silent)
{
    GPIO_OUT(pic_mclr);

    GPIO_CLR(pic_mclr);     // remove VDD from MCLR pin
    delay_us(1500);
    if(!flags.client && !silent){
        cout << "Press any key to release the reset...";
        fgetc(stdin);
        cout << endl;
    }
    GPIO_IN(pic_mclr);      // MCLR as input, puts the output driver in Hi-Z
}

/* print the help */
void usage(void)
{
    cout <<
            "Usage: picberry [options] \n"
            "\n"
            "   Programming Options \n"
            "\n"
            "       --help,             -h                print help\n"
            "       --server=port,      -S port           server mode, listening on given port\n"
            "       --log=[file],       -l [file]         redirect the output to log file(s)\n"
            "       --gpio=PGC,PGD,MCLR -g PGC,PGD,MCLR   GPIO selection in form [PORT:]NUM (optional)\n"
            "       --family=[family],  -f [family]       PIC family [default: dspic33f]\n"
            "       --read=[file.hex],  -r [file.hex]     read chip to file [defaults to ofile.hex]\n"
            "       --write=file.hex,   -w file.hex       bulk erase and write chip\n"
            "       --erase,            -e                bulk erase chip\n"
            "       --blankcheck,       -b                blank check of the chip\n"
            "       --regdump,          -d                read configuration registers\n"
            "       --noverify                            skip memory verification after writing\n"
            "       --debug                               turn ON debug\n"
            "       --fulldump                            don't detect empty sections, make complete dump (PIC32)\n"
            "       --program-only                        read/write only program section (PIC32)\n"
            "       --boot-only                           read/write only boot section (PIC32)\n"
            "\n"
            "\n"
            "   Runtime Options\n"
            "\n"
            "       --reset, -R                           reset\n"
            "\n"
            "\n"
            "   Available PIC families:\n"
            "\n"
            "       dspic33e    \n"
            "       dspic33f    \n"
            "       pic10f322   \n"
            "       pic18fj     \n"
            "       pic24fj     \n"
            "       pic24fjxxxga0xx \n"
            "       pic24fjxxxga3xx \n"
            "       pic24fjxxga1xx \n"
            "       pic24fjxxgb0xx \n"
            "       pic24fjxxxga1xx \n"
            "       pic24fjxxxga2xx \n"
            "       pic24fjxxxgb1xx \n"
            "       pic24fjxxxgb2xx \n"
            "       pic32mx1    \n"
            "       pic32mx2    \n"
            "       pic32mx3    \n"
            "       pic32mz     \n"
            "       pic32mk     \n";
}

#define BUFFSIZE 32

enum srv_command : char{
    SRV_PB_VER      = '0',
    SRV_RESET       = '1',
    SRV_ENTER       = '2',
    SRV_EXIT        = '3',
    SRV_DEV_ID      = '4',
    SRV_ERASE       = '5',
    SRV_READ        = '6',
    SRV_WRITE       = '7',
    SRV_BLANKCHECK  = '8',
    SRV_REGDUMP     = '9',
    SRV_SET_FAMILY  = 'A'
};

enum srv_families : char{
    SRV_FAM_DSPIC33E = '0',
    SRV_FAM_DSPIC33F = '1',
    SRV_FAM_PIC18FJ  = '2',
    SRV_FAM_PIC24FJ  = '3',
    SRV_FAM_PIC32MX1 = '4',
    SRV_FAM_PIC32MX2 = '5',
    SRV_FAM_PIC32MX3 = '6',
    SRV_FAM_PIC32MZ  = '7',
    SRV_FAM_PIC32MK  = '8'
};

void server_mode(int port){
    int serversock, clientsock;
    struct sockaddr_in pbserver, pbclient;
    char buffer[BUFFSIZE];
    int received = -1;
    bool program_mode = false;
    char current_family = 0;
    
    /* Set picberry to work in "client" mode */
    flags.client = 1;

    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        cerr << "Failed to create socket";
        exit(1);
    }
    /* Construct the server sockaddr_in structure */
    memset(&pbserver, 0, sizeof(pbserver));       /* Clear struct */
    pbserver.sin_family = AF_INET;                /* Internet/IP */
    pbserver.sin_addr.s_addr = htonl(INADDR_ANY); /* Incoming addr */
    pbserver.sin_port = htons(port);              /* server port */
    
     /* Bind the server socket */      
    if (bind(serversock, (struct sockaddr *) &pbserver, sizeof(pbserver)) < 0) {
       cerr << "Failed to bind the server socket";
       exit(1);
    }
    /* Listen on the server socket (single connection) */
    if (listen(serversock, 1) < 0) {
        cerr << "Failed to listen on server socket";
        exit(1);
    }
    
    /* Setup picberry operation */
    Pic *pic = new dspic33f();
    current_family = SRV_FAM_DSPIC33F;
    
    /* Run until cancelled */
    while (1) {
        unsigned int clientlen = sizeof(pbclient);
        /* Wait for client connection */
        if ((clientsock = accept(serversock, (struct sockaddr *) &pbclient,
                                 &clientlen)) < 0) {
            cerr << "Failed to accept client connection";
            exit(1);
        }
        /* redirect stdout to socket */
        setbuf(stdout, NULL);
        dup2(clientsock, STDOUT_FILENO);
        cerr << "Client connected: " << inet_ntoa(pbclient.sin_addr) << endl;
        /* Receive message */
        if ((received = recv(clientsock, buffer, BUFFSIZE, 0)) < 0)
            cerr << "Failed to receive initial bytes from client";
        /* Send bytes and check for more incoming data in loop */
        while (received > 0) {
            if(flags.debug){
                buffer[received] = '\0';
                cerr << "Command received: " << buffer;   
            }
                
            switch(buffer[0]){
                case SRV_PB_VER:
                    cerr << "[CMD] Get picberry version" << endl;
                    send(clientsock, VERSION, strlen(VERSION), 0);
                    break;
                case SRV_RESET:
                    cerr << "[CMD] Reset" << endl;
                    pic_reset();
                    break;
                case SRV_ENTER:
                    if(!program_mode){
                        cerr << "[CMD] Enter Program Mode" << endl;
                        pic -> enter_program_mode();
                        if(pic -> setup_pe())
                            program_mode = true;
                        else
                            pic -> exit_program_mode();
                    }
                    break;
                case SRV_EXIT:
                    if(program_mode){
                        cerr << "[CMD] Exit Program Mode" << endl;
                        pic -> exit_program_mode();
                        program_mode = false;   
                    }
                    break;
                case SRV_SET_FAMILY:
                    cerr << "[CMD] Set Family ";
                    
                    if(current_family != buffer[1]){
                        current_family = buffer[1];
                    
                        switch(buffer[1]){
                            case SRV_FAM_DSPIC33E:
                                cerr << "DSPIC33E" << endl;
                                pic = new dspic33e(SF_DSPIC33E);
                                break;
                            case SRV_FAM_DSPIC33F:
                                cerr << "DSPIC33F" << endl;
                                pic = new dspic33f();
                                break;
                            case SRV_FAM_PIC18FJ:
                                cerr << "PIC18FJ" << endl;
                                pic = new pic18fj();
                                break;
                            case SRV_FAM_PIC24FJ:
                                cerr << "PIC24FJ" << endl;
                                pic = new dspic33e(SF_PIC24FJ);
                                break;
                            case SRV_FAM_PIC32MX1:
                                cerr << "PIC32MX1" << endl;
                                pic = new pic32(SF_PIC32MX1);
                                break;
                            case SRV_FAM_PIC32MX2:
                                cerr << "PIC32MX2" << endl;
                                pic = new pic32(SF_PIC32MX2);
                                break;
                            case SRV_FAM_PIC32MX3:
                                cerr << "PIC32MX3" << endl;
                                pic = new pic32(SF_PIC32MX3);
                                break;
                            case SRV_FAM_PIC32MZ:
                                cerr << "PIC32MZ" << endl;
                                pic = new pic32(SF_PIC32MZ);
                                break;
                            case SRV_FAM_PIC32MK:
                                cerr << "PIC32MK" << endl;
                                pic = new pic32(SF_PIC32MK);
                                break;
                        }
                    }
                    else{
                        cerr << "not needed." << endl;
                    }
                    fprintf(stdout, "K%c", buffer[1]);
                    break;
                case SRV_DEV_ID:
                    if(program_mode){
                        if(flags.debug) cerr << "[CMD] Read Device ID" << endl;
                        if(pic -> read_device_id())
                            fprintf(stdout,
                                    "{\"DevName\" : \"%s\", \"DevID\" : \"0x%08X\", \"DevRev\" : \"0x%08X\"}",
                                    pic->name,
                                    pic->device_id,
                                    pic->device_rev);
                        else{
                            fprintf(stdout, "NC");
                            pic -> exit_program_mode();
                            program_mode = false;
                        }
                    }
                    break;
                case SRV_BLANKCHECK:
                    if(program_mode){
                        cerr << "[CMD] Blank Check" << endl;
                        pic->blank_check();
                    }
                    break;
                case SRV_READ:
                    if(program_mode){
                        cerr << "[CMD] Read" << endl;
                        pic->read((char *)"/var/tmp/tmpr.hex", 0, 0);
                        send_file((char *)"/var/tmp/tmpr.hex");
                    }
                    break;
                case SRV_WRITE:
                    if(program_mode){
                        cerr << "[CMD] Write" << endl;
                        if(receive_file(clientsock, (char *)"/var/tmp/tmpw.hex")){
                            cerr << "File transfer failed!" << endl;
                            fprintf(stdout, "@ERR");
                            break;
                        }
                        pic->write((char *)"/var/tmp/tmpw.hex");
                    }
                    break;
                case SRV_ERASE:
                    if(program_mode){
                        cerr << "[CMD] Erase" << endl;
                        pic->bulk_erase();
                    }
                    break;
                default:
                    break;
            }
            
            /* Check for more data */
            if ((received = recv(clientsock, buffer, BUFFSIZE, 0)) < 0)
                cerr << "Error." << endl;
        }
        cerr << "Client disconnected." << endl;
        if(program_mode){
            pic -> exit_program_mode();
            program_mode = false;   
        }
        close(clientsock);
    }
}

uint8_t send_file(char * filename){
    
    FILE *fp;
    char line[256], *ptr;
        
    fp = fopen(filename, "r");
    if (fp == NULL) {
      	cerr << "Error: cannot open source file " << filename << "." << endl;
       	return 1;
    }
    
    while (1) {
        ptr = fgets(line, 256, fp);

        if (ptr != NULL) 
            cout << line;
        else
            break;
    }
    fclose(fp);
    
    fprintf(stdout, "@FIN");
    
    return 0;
}

uint8_t receive_file(int sock, char * filename){
    
    FILE *fp;
    char line[45];
    char buffer;
    bool need_reading = true;
    int received = -1;
    int k=0;
        
    fp = fopen(filename, "w");
    if (fp == NULL) {
      	cerr << "Error: cannot open destination file " << filename << "." << endl;
       	return 1;
    }
    
    while (need_reading) {
        for(k=0; k<46; k++){
            if ((received = recv(sock, &buffer, 1, 0)) < 0)
                cerr << "Failed to receive bytes from client";
            // Check if last line
            if(!received || buffer == '@'){
                need_reading = false;
                break;
            }    
            else{
                line[k] = buffer;
                if(buffer == '\n')
                break;
            }
        }
        // If not last line...
        if(need_reading){
            line[k+1] = '\0';
            fprintf(fp, line);
        } 
    }
    fclose(fp);
    
    return 0;
}
