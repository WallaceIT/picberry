
#define SW_PORTC_IO_BASE  	0x01c20800
#define GPIO_BASE		  	0x01c20000
#define OFFSET			  	0x00000800
#define BLOCK_SIZE			0x00002000

#define DIR_OFFSET			0x6C	// PORT D
#define SET_OFFSET			0x7C	// PORT D
#define PULL_OFFSET			0x88	// PORT D

/* GPIO setup macros. Always use GPIO_IN(x) before using GPIO_OUT(x) */
#define GPIO_IN(g)    *(int*)((char*)gpio+OFFSET+DIR_OFFSET) &= ~(0x00000007<<(g*4))
#define GPIO_OUT(g)   *(int*)((char*)gpio+OFFSET+DIR_OFFSET) |= (0x00000001<<(g*4))

#define GPIO_SET(g)   *(int*)((char*)gpio+OFFSET+SET_OFFSET) |= 1<<(g)
#define GPIO_CLR(g)   *(int*)((char*)gpio+OFFSET+SET_OFFSET) &= ~(1<<(g))
#define GPIO_LEV(g)   (*(int*)((char*)gpio+OFFSET+SET_OFFSET) >> g) & 0x1	/* reads pin level */

/* default GPIO <-> PIC connections */
#define DEFAULT_PIC_CLK    4	/* PGC - Output - PD6 */
#define DEFAULT_PIC_DATA   5	/* PGD - I/O - PD7 */
#define DEFAULT_PIC_MCLR   1	/* MCLR - Output - PD1 */
