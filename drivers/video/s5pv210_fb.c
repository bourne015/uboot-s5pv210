/*
* (C) Copyright 2006 by OpenMoko, Inc.
* Author: Harald Welte <laforge@openmoko.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/
#include <common.h>
#include <asm/io.h>
#include <video_fb.h>
#include <s5pc110.h>
#include "videomodes.h"
#include <video_fb.h>
#include <asm/arch/regs-fb.h>
#include <pmic.h>
#if defined(CONFIG_VIDEO_S5PV210)
/*
* Export Graphic Device
*/
GraphicDevice smi;
#define VIDEO_MEM_SIZE 0x200000  /* 240x320x16bit = 0x25800 bytes */

#if !UPDATER_TYPE
#define LCD_SPI_DELAY 1 
#undef msleep
#define msleep(x)	(udelay(x * 1000))

#define S5PV210_MP01_CON 	(ELFIN_GPIO_BASE + MP01CON_OFFSET)
#define S5PV210_MP01_DAT	(ELFIN_GPIO_BASE + MP01DAT_OFFSET)
#define S5PV210_MP02_CON 	(ELFIN_GPIO_BASE + MP02CON_OFFSET)
#define S5PV210_MP02_DAT	(ELFIN_GPIO_BASE + MP02DAT_OFFSET)
#define S5PV210_MP07_CON 	(ELFIN_GPIO_BASE + MP07CON_OFFSET)
#define S5PV210_MP07_DAT	(ELFIN_GPIO_BASE + MP07DAT_OFFSET)
#define S5PV210_GPJ3_CON 	(ELFIN_GPIO_BASE + GPJ3CON_OFFSET)
#define S5PV210_GPJ3_DAT	(ELFIN_GPIO_BASE + GPJ3DAT_OFFSET)
#define S5PV210_GPH2_CON 	(ELFIN_GPIO_BASE + GPH2CON_OFFSET)
#define S5PV210_GPH2_DAT	(ELFIN_GPIO_BASE + GPH2DAT_OFFSET)

#define S5P_FB_SPI_CLK_CON S5PV210_MP01_CON
#define S5P_FB_SPI_CLK_DAT S5PV210_MP01_DAT
#define S5P_FB_SPI_CLK_SHIFT 7

#define S5P_FB_SPI_SDO_CON S5PV210_MP01_CON
#define S5P_FB_SPI_SDO_DAT S5PV210_MP01_DAT
#define S5P_FB_SPI_SDO_SHIFT 6

#define S5P_FB_SPI_CS_CON S5PV210_MP02_CON
#define S5P_FB_SPI_CS_DAT S5PV210_MP02_DAT
#define S5P_FB_SPI_CS_SHIFT 0

#define S5P_FB_SPI_RESET_CON S5PV210_GPJ3_CON
#define S5P_FB_SPI_RESET_DAT S5PV210_GPJ3_DAT
#define S5P_FB_SPI_RESET_SHIFT 7

#define S5P_FB_SPI_PWREN_CON S5PV210_GPH2_CON
#define S5P_FB_SPI_PWREN_DAT S5PV210_GPH2_DAT
#define S5P_FB_SPI_PWREN_SHIFT 7

static void gpio_set_value(unsigned long gpio_group_con, unsigned long gpio_group_data, int value, int shift)
{
	writel((0x1<<(shift*4))|(readl(gpio_group_con)&((~0xf<<(shift*4)))), gpio_group_con);

	if(value){
		writel((0x1<<shift)|readl(gpio_group_data), gpio_group_data);
	}
	else{
		writel(~((0x1<<shift))&readl(gpio_group_data), gpio_group_data);
	}

}

inline void lq_spi_lcd_pwren(int value)
{
	gpio_set_value(S5P_FB_SPI_PWREN_CON, S5P_FB_SPI_PWREN_DAT, value, S5P_FB_SPI_PWREN_SHIFT);
}

inline void lq_spi_lcd_reset(int value)
{
	gpio_set_value(S5P_FB_SPI_RESET_CON, S5P_FB_SPI_RESET_DAT, value, S5P_FB_SPI_RESET_SHIFT);
}

inline void lq_spi_lcd_clk(int value)
{
        gpio_set_value(S5P_FB_SPI_CLK_CON, S5P_FB_SPI_CLK_DAT, value, S5P_FB_SPI_CLK_SHIFT);
}

inline void lq_spi_lcd_data(int value)
{
        gpio_set_value(S5P_FB_SPI_SDO_CON, S5P_FB_SPI_SDO_DAT, value, S5P_FB_SPI_SDO_SHIFT);
}

inline void lq_spi_lcd_cs(int value)
{
        gpio_set_value(S5P_FB_SPI_CS_CON, S5P_FB_SPI_CS_DAT, value, S5P_FB_SPI_CS_SHIFT);
}

//************** write parameter
static  void write_lcdcom(unsigned char SSD2123_index)//,unsigned char NUMoFparameter)
  {
	 //unsigned char HX_WR_COM=0x74;//74
	 unsigned char  i;
	 
	 lq_spi_lcd_cs(1);   //SET_HX_CS;
	 lq_spi_lcd_clk(1);  //SET_HX_CLK;
	 lq_spi_lcd_data(1); //SET_HX_SDO;
	
	 lq_spi_lcd_cs(0);   // output "L" set CS
	 udelay(LCD_SPI_DELAY);
	 
	   lq_spi_lcd_clk(0); 
	  // udelay(LCD_SPI_DELAY);
   
	   lq_spi_lcd_data(0);//SDO transfer "0"(command)
       udelay(LCD_SPI_DELAY); 
        
	   lq_spi_lcd_clk(1);
	   udelay(LCD_SPI_DELAY); 

	   for(i=0;i<8;i++) // 8 Data
	   {
		    lq_spi_lcd_clk(0); //CLK "L"

		   if ( SSD2123_index& 0x80)
		    	lq_spi_lcd_data(1);
		   else
		    	lq_spi_lcd_data(0);

		    SSD2123_index<<= 1;
		    udelay(LCD_SPI_DELAY); 
		    
		    lq_spi_lcd_clk(1); //CLK "H"
		    udelay(LCD_SPI_DELAY); 
	   }
	   
	   //lq_spi_lcd_clk( 1);
	   //udelay(LCD_SPI_DELAY); 

	   lq_spi_lcd_cs(1); //output "H" release cs
       udelay(LCD_SPI_DELAY);
  }
/************************ write register ***************************/
static  void write_lcdregister(unsigned char SSD2123_data)//,unsigned char NUMoFparameter1)
{
	   //unsigned char HX_WR_COM=0x76;//76
	 unsigned char i;
	   
	 lq_spi_lcd_cs(1);   //SET_HX_CS;
	 lq_spi_lcd_clk(1);  //SET_HX_CLK;
	 lq_spi_lcd_data(1); //SET_HX_SDO;
	
	 lq_spi_lcd_cs(0);   // output "L" set CS
	 udelay(LCD_SPI_DELAY); 

	  lq_spi_lcd_clk(0);  

	  lq_spi_lcd_data(1); //SDO transfer "1"(data)
	    
	  udelay(LCD_SPI_DELAY); 

	  lq_spi_lcd_clk(1);
	  udelay(LCD_SPI_DELAY); 

	  for(i=0;i<8;i++) // 8 Data
	  {
		  lq_spi_lcd_clk(0);

		  if ( SSD2123_data& 0x80)
	   			lq_spi_lcd_data(1);
	   		else
	    		lq_spi_lcd_data(0);
	    	SSD2123_data<<= 1;
	    	udelay(LCD_SPI_DELAY);

            //CLK from "L" to "H"
	    	lq_spi_lcd_clk(1);
		  	udelay(LCD_SPI_DELAY);
	    }
	    
	   	//lq_spi_lcd_clk( 1);
	  	udelay(LCD_SPI_DELAY);  

	   	//SET_HX_CS;
		lq_spi_lcd_cs(1);   //SET_HX_CS;
	   	udelay(LCD_SPI_DELAY);
  }

//power on

static void ili9806e_reset_lcd(void)
{
	extern int v70_hw_ver;
	/*in hw3.0 power on is: 0, in hw3.1 power on is : 1*/
	if (v70_hw_ver == 30) {
		lq_spi_lcd_pwren(0);
	} else {
		lq_spi_lcd_pwren(1);
	}
	msleep(1); 
	lq_spi_lcd_reset(0);
	msleep(1);  
	lq_spi_lcd_reset(1);
	msleep(120);  
}

static void lq_init_ldi(void)  
{
	printf("Lcd init DDI\n");

	ili9806e_reset_lcd();
	
	 // ****************************** EXTC Command Set enable register ******************************//
	write_lcdcom(0xFF);
	write_lcdregister(0xFF);
	write_lcdregister(0x98);
	write_lcdregister(0x06);
	write_lcdregister(0x04);
	write_lcdregister(0x01);

	write_lcdregister(0x08);
	write_lcdregister(0x10);

	write_lcdregister(0x21);
	write_lcdregister(0x01);

	write_lcdregister(0x30);
	write_lcdregister(0x02);

	write_lcdregister(0x31);
	write_lcdregister(0x02);

	write_lcdregister(0x40);
	write_lcdregister(0x15);
		msleep(1);
	write_lcdregister(0x41);
	write_lcdregister(0x33);
		msleep(1);
	write_lcdregister(0x42);
	write_lcdregister(0x01);
		msleep(1);
	write_lcdregister(0x43);
	write_lcdregister(0x02);
		msleep(1);
	write_lcdregister(0x44);
	write_lcdregister(0x02);
		msleep(1);

	//write_lcdregister(0x46);
	//write_lcdregister(0x22);

	//write_lcdregister(0x50);
	//write_lcdregister(0x78);
	//
	//write_lcdregister(0x51);
	//write_lcdregister(0x78);
	//
	//write_lcdregister(0x52);
	//write_lcdregister(0x00);
	//
	//write_lcdregister(0x53);
	//write_lcdregister(0xbc);
	//
	//write_lcdregister(0x54);
	//write_lcdregister(0x00);
	//
	//write_lcdregister(0x55);
	//write_lcdregister(0xbc);

	write_lcdregister(0x60);
	write_lcdregister(0x03);

	write_lcdregister(0x61);
	write_lcdregister(0x00);

	write_lcdregister(0x62);
	write_lcdregister(0x08);

	write_lcdregister(0x63);
	write_lcdregister(0x04);

	write_lcdregister(0x52);
	write_lcdregister(0x00);

	write_lcdregister(0x53);
	write_lcdregister(0x6e);
	//****************************** Gamma Setting ******************************//
	//write_lcdcom(0xFF);
	//write_lcdregister(0xFF);
	//write_lcdregister(0x98);
	//write_lcdregister(0x06);
	//write_lcdregister(0x04);
	//write_lcdregister(0x01);

	write_lcdcom(0xA0);
	write_lcdregister(0x00);

	write_lcdcom(0xA1);
	write_lcdregister(0x03);

	write_lcdcom(0xA2);
	write_lcdregister(0x0a);

	write_lcdcom(0xA3);
	write_lcdregister(0x0f);

	write_lcdcom(0xA4);
	write_lcdregister(0x09);

	write_lcdcom(0xA5);
	write_lcdregister(0x18);

	write_lcdcom(0xA6);
	write_lcdregister(0x0A);

	write_lcdcom(0xA7);
	write_lcdregister(0x09);

	write_lcdcom(0xA8);
	write_lcdregister(0x03);

	write_lcdcom(0xA9);
	write_lcdregister(0x07);

	write_lcdcom(0xAA);
	write_lcdregister(0x05);

	write_lcdcom(0xAB);
	write_lcdregister(0x03);

	write_lcdcom(0xAC);
	write_lcdregister(0x0b);

	write_lcdcom(0xAD);
	write_lcdregister(0x2e);

	write_lcdcom(0xAE);
	write_lcdregister(0x28);

	write_lcdcom(0xAF);
	write_lcdregister(0x00);
	//******************************Nagitive ******************************//
	write_lcdcom(0xC0);
	write_lcdregister(0x00);

	write_lcdcom(0xC1);
	write_lcdregister(0x02);

	write_lcdcom(0xC2);
	write_lcdregister(0x09);

	write_lcdcom(0xC3);
	write_lcdregister(0x0f);

	write_lcdcom(0xC4);
	write_lcdregister(0x08);

	write_lcdcom(0xC5);
	write_lcdregister(0x17);

	write_lcdcom(0xC6);
	write_lcdregister(0x0b);

	write_lcdcom(0xC7);
	write_lcdregister(0x08);

	write_lcdcom(0xC8);
	write_lcdregister(0x03);

	write_lcdcom(0xC9);
	write_lcdregister(0x08);

	write_lcdcom(0xCA);
	write_lcdregister(0x06);

	write_lcdcom(0xCB);
	write_lcdregister(0x04);

	write_lcdcom(0xCC);
	write_lcdregister(0x0a);

	write_lcdcom(0xCD);
	write_lcdregister(0x2d);

	write_lcdcom(0xCE);
	write_lcdregister(0x29);

	write_lcdcom(0xCF);
	write_lcdregister(0x00);
	//****************************** Page 6 Command ******************************//
	write_lcdcom(0xFF);
	write_lcdregister(0xFF);
	write_lcdregister(0x98);
	write_lcdregister(0x06);
	write_lcdregister(0x04);
	write_lcdregister(0x06);

	write_lcdcom(0x00);
	write_lcdregister(0x21);

	write_lcdcom(0x01);
	write_lcdregister(0x06);

	write_lcdcom(0x02);
	write_lcdregister(0x20);

	write_lcdcom(0x03);
	write_lcdregister(0x00);

	write_lcdcom(0x04);
	write_lcdregister(0x01);

	write_lcdcom(0x05);
	write_lcdregister(0x01);

	write_lcdcom(0x06);
	write_lcdregister(0x80);

	write_lcdcom(0x07);
	write_lcdregister(0x02);

	write_lcdcom(0x08);
	write_lcdregister(0x05);

	write_lcdcom(0x09);
	write_lcdregister(0x00);

	write_lcdcom(0x0A);
	write_lcdregister(0x00);

	write_lcdcom(0x0B);
	write_lcdregister(0x00);

	write_lcdcom(0x0C);
	write_lcdregister(0x01);

	write_lcdcom(0x0D);
	write_lcdregister(0x01);

	write_lcdcom(0x0E);
	write_lcdregister(0x00);

	write_lcdcom(0x0F);
	write_lcdregister(0x00);

	write_lcdcom(0x10);
	write_lcdregister(0xF0);

	write_lcdcom(0x11);
	write_lcdregister(0xF4);

	write_lcdcom(0x12);
	write_lcdregister(0x00);

	write_lcdcom(0x13);
	write_lcdregister(0x00);

	write_lcdcom(0x14);
	write_lcdregister(0x00);

	write_lcdcom(0x15);
	write_lcdregister(0xC0);

	write_lcdcom(0x16);
	write_lcdregister(0x08);

	write_lcdcom(0x17);
	write_lcdregister(0x00);

	write_lcdcom(0x18);
	write_lcdregister(0x00);

	write_lcdcom(0x19);
	write_lcdregister(0x00);

	write_lcdcom(0x1A);
	write_lcdregister(0x00);

	write_lcdcom(0x1B);
	write_lcdregister(0x00);

	write_lcdcom(0x1C);
	write_lcdregister(0x00);

	write_lcdcom(0x1D);
	write_lcdregister(0x00);

	write_lcdcom(0x20);
	write_lcdregister(0x02);

	write_lcdcom(0x21);
	write_lcdregister(0x23);

	write_lcdcom(0x22);
	write_lcdregister(0x45);

	write_lcdcom(0x23);
	write_lcdregister(0x67);

	write_lcdcom(0x24);
	write_lcdregister(0x01);

	write_lcdcom(0x25);
	write_lcdregister(0x23);

	write_lcdcom(0x26);
	write_lcdregister(0x45);

	write_lcdcom(0x27);
	write_lcdregister(0x67);

	write_lcdcom(0x30);
	write_lcdregister(0x13);

	write_lcdcom(0x31);
	write_lcdregister(0x22);

	write_lcdcom(0x32);
	write_lcdregister(0x22);

	write_lcdcom(0x33);
	write_lcdregister(0x22);

	write_lcdcom(0x34);
	write_lcdregister(0x22);

	write_lcdcom(0x35);
	write_lcdregister(0xbb);

	write_lcdcom(0x36);
	write_lcdregister(0xaa);

	write_lcdcom(0x37);
	write_lcdregister(0xdd);

	write_lcdcom(0x38);
	write_lcdregister(0xcc);

	write_lcdcom(0x39);
	write_lcdregister(0x66);

	write_lcdcom(0x3A);
	write_lcdregister(0x77);

	write_lcdcom(0x3B);
	write_lcdregister(0x22);

	write_lcdcom(0x3C);
	write_lcdregister(0x22);

	write_lcdcom(0x3D);
	write_lcdregister(0x22);

	write_lcdcom(0x3E);
	write_lcdregister(0x22);

	write_lcdcom(0x3F);
	write_lcdregister(0x22);

	write_lcdcom(0x40);
	write_lcdregister(0x22);
	//****************************** Page 0 Command ******************************//
	write_lcdcom(0xFF);
	write_lcdregister(0xFF);
	write_lcdregister(0x98);
	write_lcdregister(0x06);
	write_lcdregister(0x04);
	write_lcdregister(0x00);

	write_lcdcom(0x3a);
	write_lcdregister(0x77);

	write_lcdcom(0x11);
	msleep(220);
	write_lcdcom(0x29);
	printf("LCD Init End\n");	
}
#endif

void board_video_gpio_init(void) 
{
	writel(0x10000000, GPBCON);     //GPBCON [31:28]:output [27:0]:input
	writel(0x1555, GPBPUD);//GPBPUD GPBPUD[7]:Pull-up/ down disabled GPBPUD[6:0]:Pull-down enabled
	writel(0xc000, GPBDRV_SR);//GPBDRV GPBDRV[7]:4x  GPBDRV[6:0]:1x
	writel(0x10010000, GPBCON);//GPBCON [31:28],[19:16]:output [27:20],[15:0]:input
	writel(0x1455, GPBPUD);//GPBPUD GPBPUD[7],[4]:Pull-up/ down disabled ,GPBPUD[6:5][3:0]:Pull-down enabled
	writel(0xc300, GPBDRV_SR);//GPBDRV GPBDRV[7],[4]:4x  GPBDRV[6:5][3:0]:1x
	writel(0x10110000, GPBCON);//GPBCON [31:28],[23:20],[19:16]:output [27:24],[15:0]:input
	writel(0x1055, GPBPUD);//GPBPUD GPBPUD[7],[5][4]:Pull-up/ down disabled ,GPBPUD[6][3:0]:Pull-down enabled
	writel(0xcf00, GPBDRV_SR);//GPBDRV GPBDRV[7],[5],[4]:4x  GPBDRV[6][3:0]:1x
	writel(0x1|(readl(GPD1CON)&(~0xf)), GPD1CON);//GPD1CON [23:4]:input [3:0]:output
	writel(0x54, GPD1PUD);//GPD1PUD GPD1PUD[5:4],[0]:Pull-up/ down disabled ,GPBPUD[3:1]:Pull-down enabled
	writel(0x3, GPD1DRV);//GPD1DRV GPD1DRV[0]:4x  GPBDRV[5:1]:1x
	writel(0x11|(readl(GPD1CON)&(~0xff)), GPD1CON);//GPD1CON [23:8]:input [7:0]:output
	writel(0x50, GPD1PUD);//GPD1PUD GPD1PUD[5:4],[1:0]:Pull-up/ down disabled ,GPBPUD[3:2]:Pull-down enabled
	writel(0xf, GPD1DRV);//GPD1DRV GPD1DRV[1:0]:4x  GPBDRV[5:2]:1x
	writel(0x0011, GPD0CON);//GPD0CON GPD0CON[3]£¬[0]:output GPD0CON[2:1]:input  
	writel(0x51, GPD0PUD);//GPD0PUD GPD0PUD[3]:Pull-up/ down disabled,GPD0PUD[2:0]:Pull-down enabled
	writel(0x0c, GPD0DRV);//GPD0DRV GPD0DRV[3]:4x,GPD0DRV[2:0]:1x
	writel(0x1000010, GPH0CON);// GPH0CON GPH0CON[6],[1]:output,GPH0CON[7],[5:2],[0]:input  
	writel(0x4455, GPH0PUD);// GPH0PUD GPH0PUD[6],[4]:Pull-up/ down disabled GPH0PUD[7],[5],[4:0]:Pull-down enabled
	writel(0x3000, GPH0DRV);// GPH0DRV GPH0DRV[6]:4x GPH0DRV[7],[5:0]:1x
	writel(0x11110000, GPBCON);//GPBCON [31:16]:output [15:0]:input
	writel(0x55, GPBPUD);//GPBPUD GPBPUD[7:4]:Pull-up/ down disabled GPBPUD[3:0]:Pull-down enabled
	writel(0xff00, GPBDRV_SR);//GPBDRV GPBDRV[7:4]:4x  GPBDRV[3:0]:1x
	writel(0x11110100, GPBCON);//GPBCON [31:16],[11:8]:output [15:12],[7:0]:input
	writel(0x55, GPBPUD);//GPBPUD GPBPUD[7:4]:Pull-up/ down disabled GPBPUD[3:0]:Pull-down enabled
	writel(0xff00, GPBDRV_SR);//GPBDRV GPBDRV[7:4]:4x  GPBDRV[3:0]:1x
	writel(0x80, GPBDAT);//GPBDAT GPBDAT[7]=1,GPBDAT[6:0]=0  
	writel(0x98, GPBDAT);//GPBDAT GPBDAT[7],[4:3]=1,GPBDAT[6:5],[2:0]=0
	writel(0xb9, GPBDAT);//GPBDAT GPBDAT[7],[5:3],[0]=1,GPBDAT[6],[2:1]=0
	writel(0xbb, GPBDAT);//GPBDAT GPBDAT[7],[5:3],[1:0]=1,GPBDAT[6],[2]=0
	writel(0xbb, GPBDAT);//GPBDAT GPBDAT[7],[5:3],[1:0]=1,GPBDAT[6],[2]=0
	//writel(0x7, GPD0DAT);//GPD0DAT GPD0DAT[3:2],[0]=1,GPD0DAT[1]=0  
	writel(0xd1, GPH0DAT);//GPH0DAT[7:6],[4],[0]=1,GPH0DAT[5],[3:1]=0  
	writel(0xfb, GPBDAT);//GPBDAT GPBDAT[7:3],[1:0]=1,GPBDAT[2]=0
	writel(0xff, GPBDAT);//GPBDAT GPBDAT[7:0]=1
	writel(0x91, GPH0DAT);//GPH0DAT[7],[4],[0]=1,GPH0DAT[6:5],[3:1]=0
	writel(0xd1, GPH0DAT);//GPH0DAT[7:6],[4],[0]=1,GPH0DAT[5],[3:1]=0
	writel(0xd3, GPH0DAT);//GPH0DAT[7:6],[4],[1:0]=1,GPH0DAT[5],[3:2]=0

	writel(0x22222222, GPF0CON);    //GPF0CON set GPF0[0:7] as HSYNC,VSYNC,VDEN,VCLK,VD[0:3]
	writel(0x0, GPF0PUD);      //GPF0PUD set pull-up,down disable
	writel(0x22222222, GPF1CON);    //set GPF1CON[7:0] as VD[11:4]
	writel(0x0, GPF1PUD);      //GPF1PUD set pull-up,down disable
	writel(0x22222222, GPF2CON);    //set GPF2CON[7:0] as VD[19:12]
	writel(0x0, GPF2PUD);      //GPF2PUD set pull-up,down disable
	writel(0x00002222, GPF3CON);    //set GPF3CON[3:0] as VD[23:20]
	writel(0x0, GPF3PUD);      //GPF3PUD set pull-up,down disable
	//--------- S5PC110 EVT0 needs MAX drive strength---------//
	writel(0xffffffff, GPF0DRV);    //set GPF0DRV drive strength max by WJ.KIM(09.07.17)
	writel(0xffffffff, GPF1DRV);    //set GPF1DRV drive strength max by WJ.KIM(09.07.17)
	writel(0xffffffff, GPF2DRV);    //set GPF2DRV drive strength max by WJ.KIM(09.07.17)
	writel(0x3ff, GPF3DRV);     //set GPF3DRV drive strength max by WJ.KIM(09.07.17)
	//init gpio func for MMC
	/* Set Initial global variables */
	//s5pc110_gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;
	//smc9115_pre_init();
	//pwm_pre_init();
}

void board_video_init(GraphicDevice *pGD) 
{
	//DISPLAY_CONTROL_REG = 0x2; //DISPLAY_CONTROL output path RGB=FIMD I80=FIMD ITU=FIMD
	*(volatile unsigned long *)(0xE0107008) = 0x2;
	
	//CLK_SRC1_REG = 0x700000;  //CLK_SRC1 fimdclk = EPLL
	*(volatile unsigned long *)(0xE0100204) = 0x700000;
} 

void board_video_reset(void)
{
	S5PC11X_FB * const fb = S5PC11X_GetBase_FB();
	fb->WINCON1 &= ~(S3C_WINCON_BPPMODE_16BPP_565 | S3C_WINCON_ENWIN_ENABLE |
	S3C_WINCON_HAWSWP_ENABLE);
} 

#if !UPDATER_TYPE
void backlight_on(void)
{
#if 0
	IIC_EWrite(WM8310_ADDR,0x404E,0xc535);//CS1_ENA  and CS1_drive set to 1 , CS1_ISEL[5:0]=0x35
	IIC_EWrite(WM8310_ADDR,0x4050,0xff); //DCn_ENA set to 1	
#else
	writel ((readl(GPD0CON) & ~(0xf<<4)), GPD0CON);
	writel ((readl(GPD0CON) | (0x1<<4)), GPD0CON); //Output
	writel ((readl(GPD0DAT) | (0x1<<1)), GPD0DAT);
	writel ((readl(GPD0PUD) & ~(0x3<<2)), GPD0PUD); //PULL_NONE
	writel ((readl(GPD0CON) & ~(0xf<<4)), GPD0CON);
	writel ((readl(GPD0CON) | (0x2<<4)), GPD0CON); //TOUT_1
#endif
}

void backlight_off(void)
{
#if 0
//	IIC_EWrite(WM8310_ADDR,0x404E,0xc535);//CS1_ENA  and CS1_drive set to 1 , CS1_ISEL[5:0]=0x35
	IIC_EWrite(WM8310_ADDR,0x4050,0xf7); //DCn_ENA set to 1	
#else
	writel ((readl(GPD0CON) & ~(0xf<<4)), GPD0CON);
	writel ((readl(GPD0CON) | (0x1<<4)), GPD0CON);
	writel ((readl(GPD0DAT) & ~(0x1<<1)), GPD0DAT);
#endif
}
#endif

/*******************************************************************************
*
* Init video chip with common Linux graphic modes (lilo)
*/
void *video_hw_init (void)
{
	S5PC11X_FB * const fb = S5PC11X_GetBase_FB();
	GraphicDevice *pGD = (GraphicDevice *)&smi;
	int videomode;
	unsigned long t1, hsynch, vsynch;
	char *penv;
	int tmp, i, bits_per_pixel;
	struct ctfb_res_modes *res_mode;
	struct ctfb_res_modes var_mode;
	int clkval;
	/* Search for video chip */
	printf("Video: ");
	tmp = 0;
	videomode = CFG_SYS_DEFAULT_VIDEO_MODE;
	/* get video mode via environment */
	if ((penv = getenv ("videomode")) != NULL) {
		/* deceide if it is a string */
		if (penv[0] <= '9') {
			videomode = (int) simple_strtoul (penv, NULL, 16);
			tmp = 1;
		}
	} else {
		tmp = 1;
	}
	
	if (tmp) {
		/* parameter are vesa modes */
		/* search params */
		for (i = 0; i < VESA_MODES_COUNT; i++) {
			if (vesa_modes[i].vesanr == videomode)
			break;
		}
		if (i == VESA_MODES_COUNT) {
			printf ("no VESA Mode found, switching to mode 0x%x ", CFG_SYS_DEFAULT_VIDEO_MODE);
			i = 0;
		}
		res_mode = (struct ctfb_res_modes *)&res_mode_init[vesa_modes[i].resindex];
		bits_per_pixel = vesa_modes[i].bits_per_pixel;
	} else {
		res_mode = (struct ctfb_res_modes *) &var_mode;
		bits_per_pixel = video_get_params (res_mode, penv);
	}

	/* calculate hsynch and vsynch freq (info only) */
	t1 = (res_mode->left_margin + res_mode->xres + res_mode->right_margin + res_mode->hsync_len) / 8;
	t1 *= 8;
	t1 *= res_mode->pixclock;
	t1 /= 1000;
	hsynch = 1000000000L / t1;
	t1 *= (res_mode->upper_margin + res_mode->yres + res_mode->lower_margin + res_mode->vsync_len);
	t1 /= 1000;
	vsynch = 1000000000L / t1;

	/* fill in Graphic device struct */ 
	sprintf (pGD->modeIdent, "%dx%dx%d %ldkHz %ldHz", res_mode->xres, res_mode->yres, bits_per_pixel, (hsynch /
	1000), (vsynch / 1000));
	printf ("%s\n", pGD->modeIdent);
	pGD->winSizeX = res_mode->xres;
	pGD->winSizeY = res_mode->yres;
	pGD->plnSizeX = res_mode->xres;
	pGD->plnSizeY = res_mode->yres;

	switch (bits_per_pixel) {
		case 8:
			pGD->gdfBytesPP = 1;
			pGD->gdfIndex = GDF__8BIT_INDEX;
			break;
		case 15:
			pGD->gdfBytesPP = 2;
			pGD->gdfIndex = GDF_15BIT_555RGB;
			break;
		case 16:
			pGD->gdfBytesPP = 2;
			pGD->gdfIndex = GDF_16BIT_565RGB;
			break;
		case 24:
			pGD->gdfBytesPP = 3;
			pGD->gdfIndex = GDF_24BIT_888RGB;
			break;
		case 32:
			pGD->gdfBytesPP = 4;
			pGD->gdfIndex = GDF_32BIT_X888RGB;
			break;
	}
	
#if 0
	/* statically configure settings */
	pGD->winSizeX = pGD->plnSizeX = 240;
	pGD->winSizeY = pGD->plnSizeY = 320;
	pGD->gdfBytesPP = 2;
	pGD->gdfIndex = GDF_16BIT_565RGB;
#endif
	pGD->frameAdrs = CFG_LCD_FBUFFER;
	pGD->memSize = VIDEO_MEM_SIZE;
	
	/* Clear video memory */
	memset((void *)pGD->frameAdrs, 0x00, pGD->memSize);
	board_video_init(pGD);
	t1 = res_mode->pixclock;
	t1 /= 1000;
	t1 = 1000000000L / t1;
	clkval = (CONFIG_SYS_VIDEO_VCLOCK_HZ / t1) - 1;
	
	/* 配置视频输出格式和显示使能/禁止。*/
	fb->VIDCON0 = ( S3C_VIDCON0_VIDOUT_RGB | S3C_VIDCON0_PNRMODE_RGB_P |
	S3C_VIDCON0_CLKVALUP_ALWAYS | S3C_VIDCON0_CLKVAL_F(clkval)|
	S3C_VIDCON0_VCLKEN_NORMAL | S3C_VIDCON0_CLKDIR_DIVIDED|
	S3C_VIDCON0_CLKSEL_HCLK );
	
	/* RGB I/F控制信号。*/
	fb->VIDCON1 = ( S3C_VIDCON1_IVSYNC_INVERT | S3C_VIDCON1_IHSYNC_INVERT);
	
	/* 配置视频输出时序和显示尺寸。*/
	fb->VIDTCON0 = ( S3C_VIDTCON0_VBPD(res_mode->upper_margin) |
	S3C_VIDTCON0_VFPD(res_mode->lower_margin) |
	S3C_VIDTCON0_VSPW(res_mode->vsync_len));         
	fb->VIDTCON1 = ( S3C_VIDTCON1_HBPD(res_mode->left_margin) |
	S3C_VIDTCON1_HFPD(res_mode->right_margin) |
	S3C_VIDTCON1_HSPW(res_mode->hsync_len));
	fb->VIDTCON2 = ( S3C_VIDTCON2_LINEVAL(pGD->winSizeY - 1) |
	S3C_VIDTCON2_HOZVAL(pGD->winSizeX - 1));
	
	#if defined(LCD_VIDEO_BACKGROUND)
		fb->WINCON0 = (S3C_WINCON_BPPMODE_16BPP_565 | S3C_WINCON_ENWIN_ENABLE |
		S3C_WINCON_HAWSWP_ENABLE); 
		fb->VIDOSD0A = ( S3C_VIDOSD_LEFT_X(0) | S3C_VIDOSD_TOP_Y(0));
		fb->VIDOSD0B = ( S3C_VIDOSD_RIGHT_X(pGD->winSizeX - 1) |
		S3C_VIDOSD_BOTTOM_Y(pGD->winSizeY - 1));
		/* 指定视频窗口0的大小控制寄存器。*/
		fb->VIDOSD0C = S3C_VIDOSD_SIZE(pGD->winSizeY * pGD->winSizeX);
	#endif
	/* 窗口格式设置 */
	fb->WINCON1 = (S3C_WINCON_BPPMODE_16BPP_565 | S3C_WINCON_ENWIN_ENABLE |
	S3C_WINCON_HAWSWP_ENABLE);
	
	/* 指定OSD图像的左上角像素的横向屏幕坐标。*/
	fb->VIDOSD1A = ( S3C_VIDOSD_LEFT_X(0) | S3C_VIDOSD_TOP_Y(0));
	
	/* 指定横屏右下角的OSD图像的像素坐标。*/
	fb->VIDOSD1B = ( S3C_VIDOSD_RIGHT_X(pGD->winSizeX - 1) |
	S3C_VIDOSD_BOTTOM_Y(pGD->winSizeY - 1));
	#if defined(LCD_VIDEO_BACKGROUND)
		fb->VIDOSD1C = ( S3C_VIDOSD_ALPHA0_R(LCD_VIDEO_BACKGROUND_ALPHA) |
		S3C_VIDOSD_ALPHA0_G(LCD_VIDEO_BACKGROUND_ALPHA) | S3C_VIDOSD_ALPHA0_B(LCD_VIDEO_BACKGROUND_ALPHA) );
	#endif
	
	/* 指定视频窗口1的大小控制寄存器。*/
	fb->VIDOSD1D = S3C_VIDOSD_SIZE(pGD->winSizeY * pGD->winSizeX);
	fb->SHADOWCON = S3C_WINSHMAP_CH_ENABLE(1); //Enables Channel 1
	
	#if defined(LCD_VIDEO_BACKGROUND)
		/* config Display framebuffer addr for background*/
		/* 指定窗口0的缓冲区起始地址寄存器，缓冲器0。*/
		fb->VIDW00ADD0B0 = LCD_VIDEO_BACKGROUND_ADDR;
		/* This marks the end of the frame buffer. */
		/* 指定窗口0的缓冲区，缓冲区结束地址寄存器0。*/
		fb->VIDW00ADD1B0 = (S3C_VIDW00ADD0B0 &0xffffffff) + (pGD->winSizeX+0) * pGD->winSizeY * (pGD->gdfBytesPP);
		/* 指定窗口0的缓冲区大小寄存器。*/
		fb->VIDW00ADD2= ((pGD->winSizeX * pGD->gdfBytesPP) & 0x1fff);
	#endif
	
	/* config Display framebuffer addr for console*/
	fb->VIDW01ADD0B0 = pGD->frameAdrs;
	/* This marks the end of the frame buffer. */
	fb->VIDW01ADD1B0 = (S3C_VIDW01ADD0B0 &0xffffffff) + (pGD->winSizeX+0) * pGD->winSizeY * (pGD->gdfBytesPP);
	fb->VIDW01ADD2= ((pGD->winSizeX * pGD->gdfBytesPP) & 0x1fff);
	/* Enable  Display */
	fb->VIDCON0 |= (S3C_VIDCON0_ENVID_ENABLE | S3C_VIDCON0_ENVID_F_ENABLE);  /* ENVID = 1  ENVID_F = 1*/
	fb->TRIGCON = 3;//(TRGMODE_I80 | SWTRGCMD_I80);TRIGCON = 3
	
	/* Enable  Display  */
	//VIDCON0 |= (VIDCON0_ENVID_ENABLE | VIDCON0_ENVID_F_ENABLE);   /* ENVID = 1     ENVID_F = 1*/ 
	//TRIGCON = (TRGMODE_I80 | SWTRGCMD_I80);  //TRIGCON = 3

	board_video_gpio_init();

#if !UPDATER_TYPE
	lq_init_ldi();
	msleep(50);
	backlight_on();
#endif

	printf("Video: video_hw_init complete \n");

	return ((void*)&smi);
}

void video_set_lut(unsigned int index,  /* color number */
unsigned char r,     /* red */
unsigned char g,     /* green */
unsigned char b     /* blue */
) 
{

}
#endif /* CONFIG_VIDEO_S5PV210 */
