#include <common.h>
#include <malloc.h>
#include <video_fb.h>
#include <version.h>
#include <linux/types.h>
#include <devices.h>
#include <regs.h>

#define VIDEO_VISIBLE_COLS	(pGD->winSizeX)
#define VIDEO_VISIBLE_ROWS	(pGD->winSizeY)
#define VIDEO_PIXEL_SIZE	(pGD->gdfBytesPP)
#define VIDEO_DATA_FORMAT	(pGD->gdfIndex)
#define VIDEO_FB_ADRS		(pGD->frameAdrs)

#define BMP_LOGO_WIDTH		100
#define BMP_LOGO_HEIGHT		200
#define BMP_LOGO_COLORS		240
#define BMP_LOGO_OFFSET		16

#define VIDEO_LOGO_WIDTH	BMP_LOGO_WIDTH
#define VIDEO_LOGO_HEIGHT	BMP_LOGO_HEIGHT
#define VIDEO_LOGO_LUT_OFFSET	BMP_LOGO_OFFSET
#define VIDEO_LOGO_COLORS	BMP_LOGO_COLORS

#define VIDEO_COLS		VIDEO_VISIBLE_COLS
//#define VIDEO_ROWS		VIDEO_VISIBLE_ROWS
//#define VIDEO_SIZE		(VIDEO_ROWS*VIDEO_COLS*VIDEO_PIXEL_SIZE)
//#define VIDEO_PIX_BLOCKS	(VIDEO_SIZE >> 2)
//#define VIDEO_LINE_LEN		(VIDEO_COLS*VIDEO_PIXEL_SIZE)
//#define VIDEO_BURST_LEN		(VIDEO_COLS/8)

/* Macros */
#ifdef	VIDEO_FB_LITTLE_ENDIAN
#define SWAP16(x)	 ((((x) & 0x00ff) << 8) | ( (x) >> 8))
#define SWAP32(x)	 ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8)|\
			  (((x) & 0x00ff0000) >>  8) | (((x) & 0xff000000) >> 24) )
#define SHORTSWAP32(x)	 ((((x) & 0x000000ff) <<  8) | (((x) & 0x0000ff00) >> 8)|\
			  (((x) & 0x00ff0000) <<  8) | (((x) & 0xff000000) >> 8) )
#else
#define SWAP16(x)	 (x)
#define SWAP32(x)	 (x)
#if !defined(VIDEO_FB_16BPP_PIXEL_SWAP)
#define SHORTSWAP32(x)	 (x)
#else
#define SHORTSWAP32(x)	 ( ((x) >> 16) | ((x) << 16) )
#endif
#endif

/* Locals */
static GraphicDevice *pGD;	/* Pointer to Graphic array */
static void *video_fb_address;		/* frame buffer address */
static int charge_state = 0;

#undef readl
#define readl(addr) (*(volatile unsigned int*)(addr))
#undef writel
#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))

enum led_color {
	LED_BLUE,
	LED_YELLOW,
	LED_GREEN,
	LED_RED,
	ALL_LIGHT,
	ALL_DARK,
};

void led_light(int color)
{
	writel ((readl(GPJ3CON) & 0xffff0000), GPJ3CON);
	writel ((readl(GPJ3CON) | 0x1111), GPJ3CON);
	writel ((readl(GPJ3PUD) & 0xff00), GPJ3PUD);

	if (color == ALL_LIGHT) {
		writel((readl(GPJ3DAT) | (0xf)), GPJ3DAT);
	} else if (color == ALL_DARK) {
		writel((readl(GPJ3DAT) & ~(0xf)), GPJ3DAT);
	} else {
		writel((readl(GPJ3DAT) & ~(0xf)), GPJ3DAT);
		writel((readl(GPJ3DAT) | (0x1<<color)), GPJ3DAT);
	}
}

static int battery_volume(void)
{
        int bat_adc =0;
        int bat_vol;

        bat_adc = s3c_bat_get_adc_data();
	 bat_vol = (((bat_adc * 3300)/4095)*509)/200; 
        //printf("bat adc: %d, bat_voltage: %d\n", bat_adc, bat_vol);
	 
        if (bat_vol <= 7414) {
                charge_state = 0;
        } else if (bat_vol <= 7682) {
                charge_state = 1;
        } else if (bat_vol <= 8282) {
                charge_state = 2;
        } else {
                charge_state = 3;
        }
	 if (charge_state == 3)
	 	led_light(LED_GREEN);
	 else
	 	led_light(LED_YELLOW);

	 return 0;
}

void lcd_logo_empty(void)
{
	int i, j;
	unsigned char *dest = (unsigned char *)video_fb_address +
			      ((300 * VIDEO_COLS * VIDEO_PIXEL_SIZE) +
			       190 * VIDEO_PIXEL_SIZE);
	int skip   = (VIDEO_COLS - VIDEO_LOGO_WIDTH) * VIDEO_PIXEL_SIZE;
	
	for (i = 0; i < 190; i++) {
		for (j = 0; j < 100; j++) {
			*(unsigned short *) dest = SWAP16(unsigned short)(0x00);
			dest += VIDEO_PIXEL_SIZE;
		}
		dest += skip;
	}
}

void lcd_logo_head(int x, int y)
{
	int i, j;
	unsigned char *dest = (unsigned char *)video_fb_address +
			      ((y * VIDEO_COLS * VIDEO_PIXEL_SIZE) +
			       x * VIDEO_PIXEL_SIZE);
	int skip   = (VIDEO_COLS - 45) * VIDEO_PIXEL_SIZE;
	
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 45; j++) {
			*(unsigned short *) dest = SWAP16(unsigned short)(0xffffff);
			dest += VIDEO_PIXEL_SIZE;
		}
		dest += skip;
	}
}

void lcd_logo_power(int x, int y)
{
	int i, j;
	unsigned char *dest = (unsigned char *)video_fb_address +
			      ((y * VIDEO_COLS * VIDEO_PIXEL_SIZE) +
			       x * VIDEO_PIXEL_SIZE);
	int skip   = (VIDEO_COLS - 100) * VIDEO_PIXEL_SIZE;
	
	for (i = 0; i < 60; i++) {
		for (j = 0; j < 100; j++) {
			*(unsigned short *) dest = SWAP16(unsigned short)(0x7e0);
			dest += VIDEO_PIXEL_SIZE;
		}
		dest += skip;
	}
}

void lcd_logo_frame(void)
{
	int i, j;
	unsigned char *dest;
	int skip   = (VIDEO_COLS - VIDEO_LOGO_WIDTH) * VIDEO_PIXEL_SIZE;

//up
	dest = (unsigned char *)video_fb_address +
			      ((297 * VIDEO_COLS * VIDEO_PIXEL_SIZE) +
			       188 * VIDEO_PIXEL_SIZE);
	for (i = 0; i < 104; i++) {
		*(unsigned short *) dest = SWAP16(unsigned short)(0xffffff);
		dest += VIDEO_PIXEL_SIZE;
	}
//bottom
	dest = (unsigned char *)video_fb_address +
			      ((490 * VIDEO_COLS * VIDEO_PIXEL_SIZE) +
			       188 * VIDEO_PIXEL_SIZE);
	for (i = 0; i < 104; i++) {
		*(unsigned short *) dest = SWAP16(unsigned short)(0xffffff);
		dest += VIDEO_PIXEL_SIZE;
	}

//left
	dest = (unsigned char *)video_fb_address +
			      ((298 * VIDEO_COLS * VIDEO_PIXEL_SIZE) +
			       188 * VIDEO_PIXEL_SIZE);
	for (i = 0; i < 193; i++) {
		*(unsigned short *) dest = SWAP16(unsigned short)(0xffffff);
		dest += VIDEO_PIXEL_SIZE*(VIDEO_COLS);
	}

//right
	dest = (unsigned char *)video_fb_address +
			      ((298 * VIDEO_COLS * VIDEO_PIXEL_SIZE) +
			       291 * VIDEO_PIXEL_SIZE);
	for (i = 0; i < 193; i++) {
		*(unsigned short *) dest = SWAP16(unsigned short)(0xffffff);
		dest += VIDEO_PIXEL_SIZE*(VIDEO_COLS);
	}
}

int charge_disp (void)
{
	int bat_adc;
	
	bat_adc = s3c_bat_get_adc_data(1);//channel = 1
	if(bat_adc < 450) { //no battery
		lcd_logo_empty();
		return 1;
	}

	lcd_logo_empty();
	if (charge_state == 0) {
		led_light(LED_YELLOW);
		lcd_logo_frame();
		lcd_logo_head(217, 287);
		charge_state++;
	} else if (charge_state == 1) {
		lcd_logo_power(190, 426);
		charge_state++;
	} else if (charge_state == 2) {
		lcd_logo_power(190, 426);
		lcd_logo_power(190, 363);
		charge_state++;
	} else if (charge_state == 3) {
		lcd_logo_power(190, 426);
		lcd_logo_power(190, 363);
		lcd_logo_power(190, 300);
		battery_volume();
	}

	return 0;
}

int charge_hw_init (void)
{
	if ((pGD = video_hw_init ()) == NULL)
		return -1;

	video_fb_address = (void *) VIDEO_FB_ADRS;

	//adc_setup();

	/*light red led if battery isn't full*/
	//led_light(LED_YELLOW);
}


/*
 *press power button for 2 seconds to power on the machine
 *pwr_state/GPH1_0: 1=no button pressed; 0=button pressed
 *reboot_info: 0=normal boot; 2=user reboot; 3=user power off when dc in
 */
 
#define S5P_INFORM6 0xE010F018
#define S5PV210_PS_HOLD_CONTROL_REG 0xE010E81c

void wait_powerkey(void)
{
	unsigned int pwr_state, reboot_info, charging, bat_err;
	unsigned int lcd_state = 1, delay_cnt = 0, lcd_timeout = 0;
	unsigned int loop_c = 0, init_c = 0;
//#if !UPDATER_TYPE
//	backlight_off();
//#endif
	writel((readl(GPH1CON) & 0xfffffff0), GPH1CON);
	writel((readl(GPH1PUD) & ~(0x3)), GPH1PUD);
	writel((readl(GPH0CON) & ~(0xf<<12)), GPH0CON);
	writel((readl(GPH0PUD) & ~(0x3<<6)), GPH0PUD);
	writel((readl(GPH3CON) & ~(0xf<<16)), GPH0CON);
	writel((readl(GPH3PUD) & ~(0x3<<8)), GPH0PUD);
	
	reboot_info = readl(S5P_INFORM6);
	charging = readl(GPH0DAT) & 0x8;

	if (reboot_info == 2) //normal reboot, do nothing
		goto out2;

	//charge_hw_init();	

	printf("please press power button!\n");
	do {
		pwr_state = readl(GPH1DAT) & 0x1;
		charging = readl(GPH0DAT) & 0x8;
		if (!pwr_state) {
			while (loop_c++ < 6) {
				udelay(50000);
				pwr_state = readl(GPH1DAT) & 0x1;
				if (pwr_state)
					break;
			}
			loop_c = 0;
			if (!pwr_state) {
				//long press, boot system
				goto out1;
			} else if (pwr_state && !charging) {
				//short press, backlight on/off
				if (lcd_state)
					backlight_off();
				else
					backlight_on();
				lcd_state = !lcd_state;
				lcd_timeout = 0;
			}
		}
		//printf("reboot info %d, pwr state: %d, charging: %d\n",
		//	reboot_info, pwr_state,charging);
		if (reboot_info != 2 && pwr_state && charging) {
			//writel ((readl(GPJ2DAT) & 0xbf), GPJ2DAT);
			printf("wrong press, power down!\n");
			udelay(2000000);
			writel(readl(S5PV210_PS_HOLD_CONTROL_REG) & 0xFFFFFEFF,
                             S5PV210_PS_HOLD_CONTROL_REG);
			while(1);
		}
		if (delay_cnt == 0) {
			bat_err = readl(GPH3DAT) & 0x10;
			//printf("battery test0 %d\n", bat_err);
		}
		udelay(50000);
		delay_cnt++;
		if (delay_cnt == 20) {
			if (init_c == 0)
				charge_hw_init();
			init_c = 1;
			if (!charging && lcd_state && pwr_state) {
				if (charge_disp()) {
					backlight_off(); //no battery
					lcd_state = 0;
				}
				lcd_timeout++;
			}

			//printf("battery test2 %d\n\n\n", (readl(GPH3DAT) & 0x10));
			if (bat_err != (readl(GPH3DAT) & 0x10)) {
				backlight_off();
				lcd_state = 0;
				led_light(ALL_DARK);
				//printf("battery error or no battery!!!\n");
			}
			
			delay_cnt = 0;
		}
		if (lcd_timeout == 30) {//screen timeout: 50ms*20*30
			backlight_off();
			lcd_state = 0;
			lcd_timeout = 0;
		}
	} while ((reboot_info == 3) || (!charging));

out1:
	//lcd_logo_flush();
	board_video_reset(); //reset to make sure linux logo print out
	backlight_off();
out2:
	writel(0, S5P_INFORM6);
}