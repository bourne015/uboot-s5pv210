/*
 * ds1302.c - Support for the Dallas Semiconductor DS1302 Timekeeping Chip
 *
 * Rex G. Feany <rfeany@zumanetworks.com>
 *
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <hush.h>
#include "trigger.h"
#include "unlocked.h"
#include "bmp_lock.h"
#include "bmp_logo2.h"

#if 0
	#define TRIGGER_DEBUG	printf
#else
	#define TRIGGER_DEBUG(...)
#endif

extern int clear_20_blks();

unsigned long get_endtime(unsigned long usec)
{
	unsigned long tmo;
	unsigned long endtime;

	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= get_tbclk();
		tmo /= 1000;
	}
	else {
		tmo = usec * get_tbclk();
		tmo /= (1000 * 1000);
	}

	endtime = get_timer_masked() + tmo;

	return endtime;

}

#define PIXEL_SIZE	2
#define LCD_FRAME_WIDTH		480
#define LCD_FRAME_HEIGHT		800

static void set_black(void)
{
	u32 i;
	u32* pBuffer = (u32*)CFG_LCD_FBUFFER;

	for (i=0; i < 800*480; i++)
		*pBuffer++ = 0x0;
}

static void show_logo_plot (unsigned char *bitmap, unsigned short *palette, 
							int x, int y, int logo_width, int logo_hight, int logo_color)
{
	int xcount, i;
	int skip   = (LCD_FRAME_WIDTH - logo_width) * PIXEL_SIZE;
	int ycount = logo_hight;
	unsigned char r, g, b, *logo_red, *logo_blue, *logo_green;
	unsigned char *source;

	unsigned char *dest = (unsigned char *)(CFG_LCD_FBUFFER +
			     			 ((y * LCD_FRAME_WIDTH * PIXEL_SIZE) +
			       		x * PIXEL_SIZE));

	source = bitmap;

	backlight_on();
	//set_black();
	// Allocate temporary space for computing colormap			
	logo_red = malloc (logo_color);
	logo_green = malloc (logo_color);
	logo_blue = malloc (logo_color);
	// Compute color map
	for (i = 0; i < logo_color; i++) {
		logo_red[i] = (palette[i] & 0x0f00) >> 4;
		logo_green[i] = (palette[i] & 0x00f0);
		logo_blue[i] = (palette[i] & 0x000f) << 4;
	}

	while (ycount--) {
		xcount = logo_width;
		while (xcount--) {
			r = logo_red[*source - BMP_LOGO_OFFSET];
			g = logo_green[*source - BMP_LOGO_OFFSET];
			b = logo_blue[*source - BMP_LOGO_OFFSET];

			*(unsigned short *)dest = (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));

			source++;
			dest += PIXEL_SIZE;
		}
		dest += skip;
	}
	free (logo_red);
	free (logo_green);
	free (logo_blue);
}

//#define OPEN_UNLOCKED_TIME_LIMIT
#define COUNT 10
int check_k21_lock(void)
{
	long i;
	int j = 0, k = 0;
	unsigned char unlock_key[11]={0x02,0x0c,0x12,0x00,0x01,0x00,0x01,0x1f,0x00,0x00,0x00};
	unsigned char reset_key[11]={0x02,0x0c,0x14,0x00,0x01,0x00,0x01,0x19,0x00,0x00,0x00};

	video_hw_init();
 	show_logo_plot (t_bmp_logo_bitmap, t_bmp_logo_palette, 
					(LCD_FRAME_WIDTH-t_BMP_LOGO_WIDTH)/2, 
					(LCD_FRAME_HEIGHT - t_BMP_LOGO_HEIGHT)/2, 
					t_BMP_LOGO_WIDTH, t_BMP_LOGO_HEIGHT, t_BMP_LOGO_COLORS);
	while (k++ < 1000)
		udelay(2000);
#ifdef CONFIG_K21_TRIGGER_ON
	if(k21_security_check(100)) {
		printf("\nsecurity module shut down or tamper detected\n");
		//do_reset (NULL, 0, 0, NULL);
		///clear_20_blks();
		//video_hw_init();
		//while(1);
	} else {
		printf("\nsecurity module check done! run as normal\n");
		board_video_reset();
		backlight_off();
		return 0;
	}
#else
	board_video_reset();
	backlight_off();
	return -1;
#endif

	printf("k21 locked\n");
	set_black();
 	show_logo_plot (bmp_lock_bitmap, bmp_lock_palette, 
					(LCD_FRAME_WIDTH-BMP_LOGO_WIDTH)/2, 
					(LCD_FRAME_HEIGHT - BMP_LOGO_HEIGHT)/2, 
					BMP_LOGO_WIDTH, BMP_LOGO_HEIGHT, BMP_LOGO_COLORS);

	for(i = 0; i < COUNT; i++){
#ifdef OPEN_UNLOCKED_TIME_LIMIT
			unsigned long endtime;
			signed long diff;
#endif

#ifdef OPEN_UNLOCKED_TIME_LIMIT
			printf("Waiting for unlocked the mmk , 1 minute only!!!\n");
			endtime = get_endtime(1000);//unit:usec
#endif
			do {
#ifdef OPEN_UNLOCKED_TIME_LIMIT
				unsigned long now = get_timer_masked();
				diff = endtime - now;

				if(diff < 0)
						i++;
#endif
				if(!s_WaitForHostCmd()) {
					//char *cmd = "setenv bootargs console=/dev/null; boot";
					char *cmd = "reset";

					printf("try to unlock\n");
					for(i=0;i<11;i++)
						serial3_putc(unlock_key[i]);
					udelay(100000);
					for(i=0;i<11;i++)
						serial3_putc(reset_key[i]);
					board_video_reset();
					backlight_off();
					parse_string_outer(cmd, FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP);
				} else {
#ifdef OPEN_UNLOCKED_TIME_LIMIT
					if(i%(1000*1000) == 0){
						j++;
						//printf("Waiting for unlocked the mmk ... %d\n", j);
					}
#endif
					continue;
				}
			}
#ifdef OPEN_UNLOCKED_TIME_LIMIT
			while (i<24*1000*1000);
			printf("Waiting for unlocked the mmk timeout!Please restart the machine...\n");
			while(1) ;
#else
			while(1);
#endif
	}

	printf("####check k21 OK####\n");
	return 0;
}

