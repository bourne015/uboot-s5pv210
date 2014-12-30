#ifndef __PMIC_H__
#define __PMIC_H__

#define GPD1CON_PMIC		*(volatile unsigned long *)(0xE02000C0)
#define GPD1DAT_PMIC		*(volatile unsigned long *)(0xE02000C4)
#define GPD1PUD_PMIC		*(volatile unsigned long *)(0xE02000C8)

#if 1
#define IIC_ESCL_Hi	GPD1DAT_PMIC |= (0x1<<5)
#define IIC_ESCL_Lo	GPD1DAT_PMIC &= ~(0x1<<5)
#define IIC_ESDA_Hi	GPD1DAT_PMIC |= (0x1<<4)
#define IIC_ESDA_Lo	GPD1DAT_PMIC &= ~(0x1<<4)

#define IIC_RSDA	       GPD1DAT_PMIC& (0x1<<4)
#define IIC_ESCL_INP	GPD1CON_PMIC &= ~(0xf<<20)
#define IIC_ESCL_OUTP	GPD1CON_PMIC = (GPD1CON_PMIC & ~(0xf<<20))|(0x1<<20)

#define IIC_ESDA_INP	GPD1CON_PMIC &= ~(0xf<<16)
#define IIC_ESDA_OUTP	GPD1CON_PMIC = (GPD1CON_PMIC & ~(0xf<<16))|(0x1<<16)
#else 
#define IIC_ESCL_Hi	GPD1DAT_PMIC |= (0x1<<1)
#define IIC_ESCL_Lo	GPD1DAT_PMIC &= ~(0x1<<1)
#define IIC_ESDA_Hi	GPD1DAT_PMIC |= (0x1<<0)
#define IIC_ESDA_Lo	GPD1DAT_PMIC &= ~(0x1<<0)

#define IIC_ESCL_INP	GPD1CON_PMIC &= ~(0xf<<4)
#define IIC_ESCL_OUTP	GPD1CON_PMIC = (GPD1CON_PMIC & ~(0xf<<4))|(0x1<<4)

#define IIC_ESDA_INP	GPD1CON_PMIC &= ~(0xf<<0)
#define IIC_ESDA_OUTP	GPD1CON_PMIC = (GPD1CON_PMIC & ~(0xf<<0))|(0x1<<0)
#endif

#define DELAY			100

#define MAX8698_ADDR	0x66	// when SRAD pin = 0, CC/CDh is selected
#define MAX8998_ADDR	0x66	// when SRAD pin = 0, CC/CDh is selected

#define WM8310_ADDR		0x34


extern void PMIC_InitIp(void);
extern void IIC_EWrite (unsigned char ChipId, unsigned short IicAddr, unsigned short IicData);

#endif /*__PMIC_H__*/

