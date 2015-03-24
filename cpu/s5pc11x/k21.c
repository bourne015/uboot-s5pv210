#include <config.h>
#include <common.h>
#include <s5pc110.h>
#include <s5pc11x.h>

#define GPA1CON		*(volatile unsigned long *)(0xE0200020)
#define GPA1DAT		*(volatile unsigned long *)(0xE0200024)
#define GPA1PUD		*(volatile unsigned long *)(0xE0200028)

#define GPH1CON		*(volatile unsigned long *)(0xE0200C20)
#define GPH1DAT		*(volatile unsigned long *)(0xE0200C24)
#define GPH1PUD		*(volatile unsigned long *)(0xE0200C28)


#define UART_SECU S5PC11X_UART3

#define baudrate  115200

static void s_serial_setbrg(void)
{
	S5PC11X_UART * const uart = S5PC11X_GetBase_UART(UART_SECU);
	unsigned int reg = 0;
	int i;

	/* value is calculated so : (int)(PCLK/16./baudrate) -1 */
	reg = get_PCLK();
	printf("PCLK=%8x\n",reg);
	reg /= (16 * baudrate) ;
	reg-=1;
	

	uart->UBRDIV = reg;
	for (i = 0; i < 1000; i++);
}

static int s_serial_io_init(void)
{
	GPA1CON = (GPA1CON &(~(0xff<<8)))|(0x22<<8);
	GPA1PUD = (GPA1PUD &(~(0xf<<4)));
}
static int s_serial_init(void)
{
	S5PC11X_UART * const uart = S5PC11X_GetBase_UART(UART_SECU);
	printf("uart IO = %08x \r\n",uart);
	/* FIFO enable, Tx/Rx FIFO clear */

	s_serial_io_init();
	
	uart->UFCON = 0x07;
	uart->UMCON = 0x0;

	/* Normal,No parity,1 stop,8 bit */
	uart->ULCON = 0x3;
	//uart->ULCON = 0x7;
	/*
	 * tx=level,rx=edge,disable timeout int.,enable rx error int.,
	 * normal,interrupt or polling
	 */
	uart->UCON = 0x245;

	s_serial_setbrg();

	printf("uart UFCON = %08x \r\n",uart->UFCON);
	printf("uart UMCON = %08x \r\n",uart->UMCON);
	printf("uart ULCON = %08x \r\n",uart->ULCON);
	printf("uart UCON  = %08x \r\n",uart->UCON);
	printf("uart UBRDIV = %08x \r\n",uart->UBRDIV);
	printf("uart CLK_SRC4_REG = %08x \r\n",CLK_SRC4_REG);	
	printf("uart CLK_DIV4_REG = %08x \r\n",CLK_DIV4_REG);	
	printf("uart CLK_GATE_IP3_REG = %08x \r\n",CLK_GATE_IP3_REG);	
	
;
	return (0);
}

int s_serial_getc(void)
{
	//S5PC1XX_UART *const uart = S5PC1XX_GetBase_UART(UART_NR);
	S5PC11X_UART *const uart = S5PC11X_GetBase_UART(UART_SECU);

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	return uart->URXH & 0xff;
}



/*
 * Output a single byte to the serial port.
 */
void s_serial_putc(const char c)
{
	S5PC11X_UART *const uart = S5PC11X_GetBase_UART(UART_SECU);

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

	uart->UTXH = c;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');
}

/*
 * Test whether a character is in the RX buffer
 */
int s_serial_tstc(void)
{
	S5PC11X_UART *const uart = S5PC11X_GetBase_UART(UART_SECU);

	return uart->UTRSTAT & 0x1;
}

void s_serial_puts(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

void serial_send(const unsigned char data)
{
	S5PC11X_UART *const uart = S5PC11X_GetBase_UART(UART_SECU);

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

	uart->UTXH = data;


}

void serial_sends(const unsigned char *data , int datalen)
{
	int i;
	for(i=0;i<datalen;i++){
		serial_send(data[i]);
		printf(" ");
	}
}
void s_Delay(void)
{
	unsigned long i,j;
	for(i=0;i<10;i++);
		for(j=0;j<1650;j++);
}
void s_uDelay(int delay)
{
	unsigned long i,j;
	for(i=0;i<delay;i++);
		for(j=0;j<1650;j++);
}

int serial_receive(unsigned char *recv_data,int wait)
{
	int i;
	for(i=0;i<wait;i++){
		if(s_serial_tstc())
		{
			recv_data[0]= s_serial_getc();
			return 0;
		}
		s_uDelay(2);
	}
	return -1;
}

int serial_receives(unsigned char *recv_data,int max_len,int *recv_len,int wait)
{
	int i=0,j=0,rv=0;

	if(wait==0)
		
	
	while(j++)
	{
		if(i>=max_len)
			break;
		if(j>=wait){
			rv= -1;
			break;
		}		
		if(!serial_receive(recv_data+i,1))
			i++;
		*recv_len = i;

	
	}
	return rv;

}



unsigned char  GetLRC(unsigned char *dat ,  int len)
{
     int i;
     unsigned char Edc;  
     Edc=0;
     for(i=0;i<len;i++)
          Edc ^= dat[i];
     return Edc;
}

int k21_security_check_serial(int time)
{
	int i,recv_len,rv;
	unsigned char recv_data[4]={0,0,0,0};
	unsigned char cmd[65+10] = 
	{
		0x02,
		0x6F,0x3D,
		0x00,65,
	 	0x6F,0x3D,0x84,0x0E,0x32,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,0x2E,0x44,0x44,0x46,
	 	0x30,0x31,0xA5,0x2B,0xBF,0x0C,0x28,0x61,0x0C,0x4F,0x07,0xA0,0x00,0x00,0x00,0x04,
	 	0x10,0x10,0x87,0x01,0x01,0x61,0x0C,0x4F,0x07,0xA0,0x00,0x00,0x00,0x04,0x30,0x60,
	 	0x87,0x01,0x03,0x61,0x0A,0x4F,0x05,0xB0,0x12,0x34,0x56,0x78,0x87,0x01,0x09,0x90,
	 	0x00,
	 	0x01,
	 	0x00,0x00,0x00,0x00
	 };
	memset(cmd,0x55,75);
	//printf("k21_security_check\r\n");
	s_serial_init();

	cmd[65+6] = GetLRC(cmd+1,65+4);
	for(i=0;i<time;i++){
		s_Delay();
		printf(" ");
		//printf("serial_sends\r\n");
		serial_sends(cmd,75);
		rv= serial_receives(recv_data,4,&recv_len,100);
		if(rv==0)
			break;
	}
	printf("\r\n");
	printf("recv_data:%02x%02x%02x%02x\n",
			recv_data[0],recv_data[1],recv_data[2],recv_data[3]);
	if(rv)
		return rv;
	rv = memcmp(recv_data,"ok21",4);
	
	return rv;
}

int k21_security_check_io(int time)
{//GPH1(5)
	int i;
	GPH1CON = (GPH1CON &(~(0xf<<20)));
	GPH1PUD = (GPH1PUD &(~(0x3<<10)));
	for(i=0;i<time;i++){
		s_uDelay(500);
		if(GPH1DAT& (1<<5))
			return 0;
	}
	return 1;

}
int k21_security_check(int time)
{
	int rv;
	rv = k21_security_check_io(time);
	if(rv) return rv;
	return 0;
	return k21_security_check_serial(time);

}
