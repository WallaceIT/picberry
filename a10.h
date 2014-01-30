#define SW_PORTC_IO_BASE  	0x01c20800
#define GPIO_BASE		  	0x01c20000
#define OFFSET			  	0x00000800
#define BLOCK_SIZE			0x00002000

/* PORT OFFSET:
 *
 * PA -> 0x00
 * PB -> 0x24
 * PC -> 0x48
 * PD -> 0x6C
 * PE -> 0x90
 * PF -> 0xB4
 * PG -> 0xD8
 * PH -> 0xFC
 * PI -> 0x120
 *
 * Only pins on the same port are allowed!
 *
 */
#define PORTOFFSET	0x24

#define SET			0x10
#define PULL		0x1C


/* GPIO setup macros. Always use GPIO_IN(x) before using GPIO_OUT(x) */
#define GPIO_IN(g)    *(int*)((char*)gpio+OFFSET+PORTOFFSET+(((int)g/8)*4)) &= ~(0x00000007<<(((int)g%8)*4))
#define GPIO_OUT(g)   *(int*)((char*)gpio+OFFSET+PORTOFFSET+(((int)g/8)*4)) |= (0x00000001<<(((int)g%8)*4))

#define GPIO_SET(g)   *(int*)((char*)gpio+OFFSET+PORTOFFSET+SET) |= 1<<(g)
#define GPIO_CLR(g)   *(int*)((char*)gpio+OFFSET+PORTOFFSET+SET) &= ~(1<<(g))
#define GPIO_LEV(g)   (*(int*)((char*)gpio+OFFSET+PORTOFFSET+SET) >> g) & 0x1

/* default GPIO <-> PIC connections */
#define DEFAULT_PIC_CLK    15	/* PGC - Output - PB15 */
#define DEFAULT_PIC_DATA   17	/* PGD - I/O - PB17 */
#define DEFAULT_PIC_MCLR   12	/* MCLR - Output - PB12 */
