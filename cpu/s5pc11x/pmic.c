#include <config.h>
#include <pmic.h>
#include <common.h>
void Delay(void)
{
	unsigned long i,j;
	for(i=0;i<DELAY;i++);
}

void SCLH_SDAH()
{
	IIC_ESCL_Hi;
	IIC_ESDA_Hi;
	Delay();
}

void SCLH_SDAL()
{
	IIC_ESCL_Hi;
	IIC_ESDA_Lo;
	Delay();
}

void SCLL_SDAH()
{
	IIC_ESCL_Lo;
	IIC_ESDA_Hi;
	Delay();
}

void SCLL_SDAL()
{
	IIC_ESCL_Lo;
	IIC_ESDA_Lo;
	Delay();
}

void IIC_ELow()
{
	SCLL_SDAL();
	SCLH_SDAL();
	SCLH_SDAL();
	SCLL_SDAL();
}

void IIC_EHigh()
{
	SCLL_SDAH();
	SCLH_SDAH();
	SCLH_SDAH();
	SCLL_SDAH();
}

unsigned char IIC_RData()
{
	unsigned char data = 0;
	IIC_ESCL_Lo;
	Delay();
	IIC_ESCL_Hi;
	Delay();
	data  = IIC_RSDA;
	IIC_ESCL_Hi;
	Delay();
	IIC_ESCL_Lo;
	Delay();
	return data;
}



void IIC_EStart()
{
	SCLH_SDAH();
	SCLH_SDAL();
	Delay();
	SCLL_SDAL();
}

void IIC_EEnd()
{
	SCLL_SDAL();
	SCLH_SDAL();
	Delay();
	SCLH_SDAH();
}

void IIC_RAck()
{
	SCLL_SDAL();
	SCLH_SDAL();
	SCLH_SDAL();
	SCLL_SDAL();
}

void IIC_RNAck()
{
	SCLL_SDAH();
	SCLH_SDAH();
	SCLH_SDAH();
	SCLL_SDAH();
}



void IIC_EAck()
{
	unsigned long ack;

	IIC_ESDA_INP;			// Function <- Input

	IIC_ESCL_Lo;
	Delay();
	IIC_ESCL_Hi;
	Delay();
	ack = GPD1DAT_PMIC;
	IIC_ESCL_Hi;
	Delay();
	IIC_ESCL_Hi;
	Delay();

	IIC_ESDA_OUTP;			// Function <- Output (SDA)

	ack = (ack>>4)&0x1;
	while(ack!=0);

	SCLL_SDAL();
}

void IIC_ESetport(void)
{
	GPD1PUD_PMIC&= ~(0xf<<8);	// Pull Up/Down Disable	SCL, SDA

	IIC_ESCL_Hi;
	IIC_ESDA_Hi;

	IIC_ESCL_OUTP;		// Function <- Output (SCL)
	IIC_ESDA_OUTP;		// Function <- Output (SDA)

	Delay();
}

void IIC_EWrite (unsigned char ChipId, unsigned short IicAddr, unsigned short IicData)
{
	unsigned long i;
       unsigned char data;
	IIC_EStart();

////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_ELow();	// write 'W'

	IIC_EAck();	// ACK
	
////////////////// write reg. addr. //////////////////
data = ((IicAddr & 0xff00) >> 8);
	for(i = 8; i>0; i--)
	{
		if((data >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK
////////////////// write reg. addr. //////////////////
data = ((IicAddr & 0x00ff));
	for(i = 8; i>0; i--)
	{
		if((data >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK
	
////////////////// write reg. data. //////////////////
data = ((IicData & 0xff00) >> 8);
	for(i = 8; i>0; i--)
	{
		if((data >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK
////////////////// write reg. data. //////////////////
data = ((IicData & 0x00ff));
	for(i = 8; i>0; i--)
	{
		if((data >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK

	IIC_EEnd();
}

unsigned short  IIC_ERead (unsigned char ChipId, unsigned short IicAddr)
{
	unsigned long i;
       unsigned char data;
	IIC_EStart();
	unsigned short IicData;
////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_ELow();	// write 'W'

	IIC_EAck();	// ACK
	
////////////////// write reg. addr. //////////////////
data = ((IicAddr & 0xff00) >> 8);
	for(i = 8; i>0; i--)
	{
		if((data >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK
////////////////// write reg. addr. //////////////////
data = ((IicAddr & 0x00ff));
	for(i = 8; i>0; i--)
	{
		if((data >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK
	IIC_EEnd();
//read 
	IIC_EStart();
////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}
	IIC_EHigh();	// write 'R'
	IIC_EAck();	// ACK
	
data = 0;
	IIC_ESDA_INP;
	for(i = 8; i>0; i--)	
	{
		if(IIC_RData())
			data |= (0x1<<(i-1));
		else
			data &= (~(0x1<<(i-1)));
	}
	IicData = (data&0xff)<<8;

	IIC_ESDA_OUTP;
	IIC_RAck();	// ACK
	IIC_ESDA_INP;
data = 0;
	for(i = 8; i>0; i--)	
	{
		if(IIC_RData())
			data |= (0x1<<(i-1));
		else
			data &= (~(0x1<<(i-1)));
	}
	IicData =(IicData&0xff00) | (data&0xff);
	IIC_ESDA_OUTP;
	IIC_RAck();	// ACK
	IIC_ESDA_INP;
	IIC_EEnd();
	IIC_ESDA_OUTP;
	return IicData;
}

void PMIC_InitIp(void)
{
	int i,k=0;
	IIC_ESetport();

	#if 1
	//bl
	//IIC_EWrite(WM8310_ADDR,0x404E,0xc535);//CS1_ENA  and CS1_drive set to 1 , CS1_ISEL[5:0]=0x35
	//IIC_EWrite(WM8310_ADDR,0x4050,0x00ff); //DCn_ENA set to 1	
	//DC1
	IIC_EWrite(WM8310_ADDR,0x4059,0x4300); 
	//IIC_EWrite(WM8310_ADDR,0x405a,0x1024);//dvs
	//DC2
	IIC_EWrite(WM8310_ADDR,0x405e,0x4300); 
	//IIC_EWrite(WM8310_ADDR,0x405f,0x1828);//dvs
	//DC3
	IIC_EWrite(WM8310_ADDR,0x4062,(IIC_ERead(WM8310_ADDR, 0x4062)&(~0x7f))|0x24); // 1.80v in the standby mode
	IIC_EWrite(WM8310_ADDR,0x4063,0x324); // 1.8v in sleep mode
	//EPE1
	//IIC_EWrite(WM8310_ADDR,0x4066,0x6060); 
	//ldo1
	IIC_EWrite(WM8310_ADDR,0x406a,0x11F); //sleep 3.3v 
	//ldo2
	IIC_EWrite(WM8310_ADDR,0x406c,0x801a); // 2.8v  in the standby mode
	IIC_EWrite(WM8310_ADDR,0x406d,0x8100); 
	//ldo3 
	IIC_EWrite(WM8310_ADDR,0x406f,0x0000); //no use
	IIC_EWrite(WM8310_ADDR,0x4070,0x0000); //no use
	//ldo4
	IIC_EWrite(WM8310_ADDR,0x4072,0x0000); //no use
	IIC_EWrite(WM8310_ADDR,0x4073,0x0000); //no use
	//ldo5
	IIC_EWrite(WM8310_ADDR,0x4076,0x4100);  //VDD_PLL
	//IIC_EWrite(WM8310_ADDR,0x4076,0x0104); 
	//ldo6
	IIC_EWrite(WM8310_ADDR,0x4078,0x0000); //no use
	IIC_EWrite(WM8310_ADDR,0x4079,0x0000); //no use
	//ldo7
	IIC_EWrite(WM8310_ADDR,0x407c,0x8100); 
	//IIC_EWrite(WM8310_ADDR,0x407c,0x0102);
	//ldo8
	IIC_EWrite(WM8310_ADDR,0x407f,0xa100); 
	//IIC_EWrite(WM8310_ADDR,0x407f,0x011d); 
	//ldo9
	IIC_EWrite(WM8310_ADDR,0x4082,0x8100); 
	//IIC_EWrite(WM8310_ADDR,0x4082,0x011d); 
	//ldo10
//	IIC_EWrite(WM8310_ADDR,0x4085,0x8100);//disable in sleep mode 
	IIC_EWrite(WM8310_ADDR,0x4085,0x011d);//enable in sleep mode 
	//ldo11
	IIC_EWrite(WM8310_ADDR,0x4087,0x3006); 
	IIC_EWrite(WM8310_ADDR,0x4088,0x0006);//in sleep mode  1.1v

	IIC_EWrite(WM8310_ADDR,0x4008,0x9716);
	IIC_EWrite(WM8310_ADDR,0x4006,0x8463);
	IIC_EWrite(WM8310_ADDR,0x4008,0x0000);
	#endif
}
void Main_PMIC_InitIp(void)
{
#if 0
	int i,k=0;
	IIC_ESetport();
//dc1
	printf("dc1 0x4056 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4056));
	printf("dc1 0x4057 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4057));
	printf("dc1 0x4058 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4058));
	printf("dc1 0x4059 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4059));
	printf("dc1 0x405a = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x405a));

//dc2
	printf("dc2 0x405b = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x405b));
	printf("dc2 0x405c = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x405c));
	printf("dc2 0x405d = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x405d));
	printf("dc2 0x405e = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x405e));
	printf("dc2 0x405f = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x405f));

//dc3
	printf("dc3 0x4060 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4060));
	printf("dc3 0x4061 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4061));
	printf("dc3 0x4062 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4062));
	printf("dc3 0x4063 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4063));
	k=0;
	for(i=0x4068;i<=0x4085;i++)
	{
		
		if((k%3) == 0 || k==0)
			printf("ldo%d : \n", (k/3)+1);
		printf("0x%x = 0x%x\n",  i, IIC_ERead(WM8310_ADDR, i));
		k++;
	}
	printf("ld11 0x4087 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4087));
	printf("ld11 0x4088 = 0x%x\n", IIC_ERead(WM8310_ADDR, 0x4088));
#endif
}
