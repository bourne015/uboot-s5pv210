#include <common.h>
#include <malloc.h>
#include <version.h>
#include <linux/types.h>
#include <devices.h>
#include <regs.h>

#define S5PV2XX_PA_ADC        	(0xE1700000)
#define S3C_ADCCON                     (0x00)
#define S3C_ADCDLY                      (0x08)
#define S3C_ADCDAT0            		(0x0C)
#define S3C_ADCMUX                     (0x1C)
#define S3C_ADCCON_RESSEL_12BIT         (0x1<<16)
#define S3C_ADCCON_PRSCEN                   (1<<14)
#define S3C_ADCCON_PRSCVL(x)               (((x)&0xFF)<<6)
#define S3C_ADCCON_STDBM                     (1<<2)

#define S3C_ADCCON_ENABLE_START         (1<<0)
#define S3C_ADCCON_ECFLG                       (1<<15)
#define S3C_ADCDAT0_XPDATA_MASK_12BIT   (0x0FFF)
#define ADC_DATA_ARR_SIZE       6

#define ADC_DELAY	10000
#define ADC_PRESC	49
//#define ADC_RESOLUT	12
#define ADC_CHANNEL 	1

#undef readl
#define readl(addr) (*(volatile unsigned int*)(addr))
#undef writel
#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))

static unsigned int base_addr = S5PV2XX_PA_ADC;

#if 0
static unsigned int s3c_adc_convert(int channel, int prescaler, int delay)
{
	unsigned int adc_return = 0;
	unsigned long data0;
	unsigned long data1;
	int adc_port = channel;
	
	writel((readl(base_addr + S3C_ADCCON) | S3C_ADCCON_PRSCEN) & ~S3C_ADCCON_STDBM,
		base_addr + S3C_ADCCON);

	writel((adc_port & 0xF), base_addr + S3C_ADCMUX);

	udelay(10);

	writel(readl(base_addr + S3C_ADCCON) | S3C_ADCCON_ENABLE_START,
		base_addr + S3C_ADCCON);

	do {
		data0 = readl(base_addr + S3C_ADCCON);
	} while (!(data0 & S3C_ADCCON_ECFLG));

	data1 = readl(base_addr + S3C_ADCDAT0);

	writel((readl(base_addr + S3C_ADCCON) | S3C_ADCCON_STDBM) & ~S3C_ADCCON_PRSCEN,
		base_addr + S3C_ADCCON);

	adc_return = data1 & S3C_ADCDAT0_XPDATA_MASK_12BIT;

	return adc_return;
}
#else
static unsigned int s3c_adc_get_value(int channel, int prescaler, int delay)
{
        int i = 5000;
        unsigned long ctrl, val;
	 
        ctrl = S3C_ADCCON_PRSCEN | S3C_ADCCON_PRSCVL(prescaler);
        writel(ctrl, base_addr + S3C_ADCCON);

        writel((delay & 0xffff), base_addr + S3C_ADCDLY);
        writel((channel & 0x000f), base_addr + S3C_ADCMUX);

        ctrl |= S3C_ADCCON_RESSEL_12BIT;
        writel(ctrl, base_addr + S3C_ADCCON);

        //start adc
        ctrl |= S3C_ADCCON_ENABLE_START;
        writel(ctrl, base_addr + S3C_ADCCON);

        //wait for adc done
        do {
        	val = readl(base_addr + S3C_ADCCON);
        } while (i-- > 0 && !(val & S3C_ADCCON_ECFLG));

        val = readl(base_addr + S3C_ADCDAT0);

        ctrl |= S3C_ADCCON_STDBM;
        ctrl &= ~(S3C_ADCCON_PRSCEN | S3C_ADCCON_ENABLE_START);
        writel(ctrl, base_addr + S3C_ADCCON);

        return (val & S3C_ADCDAT0_XPDATA_MASK_12BIT);
}
#endif

int s3c_bat_get_adc_data(int channel)
{
        int adc_data;
        int adc_max = 0;
        int adc_min = 0;
        int adc_total = 0;
        int i;

        for (i = 0; i < ADC_DATA_ARR_SIZE; i++) {
                adc_data = s3c_adc_get_value(channel, ADC_PRESC, ADC_DELAY); //adc: 3855
		   //adc_data = s3c_adc_convert(1,49,1000); //adc: 3895
		   //printf("adc data: %d\n", adc_data);
                if (i != 0) {
                        if (adc_data > adc_max)
                                adc_max = adc_data;
                        else if (adc_data < adc_min)
                                adc_min = adc_data;
                } else {
                        adc_max = adc_data;
                        adc_min = adc_data;
                }
                adc_total += adc_data;
        }

        return (adc_total - adc_max - adc_min) / (ADC_DATA_ARR_SIZE - 2);
       // return (adc_total - adc_max - adc_min) >> 2;
}

void s5p_adc_setup(void)
{
        //delay  = 10000,
        //presc  = 49,
        //resolution = 12,
        writel(S3C_ADCCON_PRSCEN | S3C_ADCCON_PRSCVL(ADC_PRESC),
                        base_addr + S3C_ADCCON);
        writel(ADC_DELAY, base_addr + S3C_ADCDLY);
        writel(readl(base_addr + S3C_ADCCON) |S3C_ADCCON_RESSEL_12BIT,
                        base_addr + S3C_ADCCON);

        writel((readl(base_addr + S3C_ADCCON) | S3C_ADCCON_STDBM) &
                        ~S3C_ADCCON_PRSCEN, base_addr + S3C_ADCCON);
}