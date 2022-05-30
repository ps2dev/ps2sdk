/**
 * @file
 * AIF and RTC registers
 */

#ifndef __AIFREGS_H__
#define __AIFREGS_H__

#include <tamtypes.h>

#define AIF_REGBASE (SPD_REGBASE + 0x4000000)

#define USE_AIF_REGS volatile u16 *aif_regs = \
                         (volatile u16 *)AIF_REGBASE

enum AIF_REGS {
    AIF_IDENT = 0x00,
    AIF_REVISION,
    AIF_INTSR,
    AIF_INTEN,
    AIF_TIMCFG,
    AIF_COUNT_L = 0x08,
    AIF_COUNT_H,
    AIF_ATA_TCFG = 0x20,
    AIF_ATA      = 0x30, // ATA register base.
    AIF_ATACTL   = 0x3C,
    AIF_RTC      = 0x80 // RTC register base.
};

#define AIF_INTCL AIF_INTSR

// AIF interrupt management
enum AIF_INUM {
    AIF_INUM_ATA0 = 0,
    AIF_INUM_RTC, // I don't know what this interrupt event is, but it should be for the RTC because it doesn't have a known bit for itself.
    AIF_INUM_PCMCIA,

    AIF_INUM_COUNT
};

#define AIF_INTR_ATA0   (1 << AIF_INUM_ATA0)
#define AIF_INTR_RTC    (1 << AIF_INUM_RTC)
#define AIF_INTR_PCMCIA (1 << AIF_INUM_PCMCIA)

// Motorola MC146818 RTC management
#define USE_AIF_RTC_REGS volatile u16 *aif_rtc_regs = \
                             (&aif_regs[AIF_RTC])

enum RTC_REGS {
    RTC_SECONDS = 0x00,
    RTC_SECONDS_ALARM,
    RTC_MINUTES,
    RTC_MINUTES_ALARM,
    RTC_HOURS,
    RTC_HOURS_ALARM,
    RTC_DAY_OF_WEEK,
    RTC_DAY_OF_MONTH,
    RTC_MONTH,
    RTC_YEAR,
    RTC_REG_A,
    RTC_REG_B,
    RTC_REG_C,
    RTC_REG_D
};

/**********************************************************************
 * RTC register details, taken from mc146818rtc.h
 **********************************************************************/
#define RTC_FREQ_SELECT RTC_REG_A

/**
 * update-in-progress  - set to "1" 244 microsecs before RTC goes off the bus,
 * reset after update (may take 1.984ms @ 32768Hz RefClock) is complete,
 * totalling to a max high interval of 2.228 ms.
 */
#define RTC_UIP            0x80
#define RTC_DIV_CTL        0x70
/** divider control: refclock values 4.194 / 1.049 MHz / 32.768 kHz */
#define RTC_REF_CLCK_4MHZ  0x00
#define RTC_REF_CLCK_1MHZ  0x10
#define RTC_REF_CLCK_32KHZ 0x20
/** 2 values for divider stage reset, others for "testing purposes only" */
#define RTC_DIV_RESET1     0x60
#define RTC_DIV_RESET2     0x70
/** Periodic intr. / Square wave rate select. 0=none, 1=32.8kHz,... 15=2Hz */
#define RTC_RATE_SELECT    0x0F

/**********************************************************************/
#define RTC_CONTROL   RTC_REG_B
/** disable updates for clock setting */
#define RTC_SET       0x80
/** periodic interrupt enable */
#define RTC_PIE       0x40
/** alarm interrupt enable */
#define RTC_AIE       0x20
/** update-finished interrupt enable */
#define RTC_UIE       0x10
/** enable square-wave output */
#define RTC_SQWE      0x08
/** all time/date values are BCD if clear */
#define RTC_DM_BINARY 0x04
/** 24 hour mode - else hours bit 7 means pm */
#define RTC_24H       0x02
/** auto switch DST - works f. USA only */
#define RTC_DST_EN    0x01


/**********************************************************************/
#define RTC_INTR_FLAGS RTC_REG_C
/* caution - cleared by read */
/** any of the following 3 is active */
#define RTC_IRQF       0x80
#define RTC_PF         0x40
#define RTC_AF         0x20
#define RTC_UF         0x10

/**********************************************************************/
#define RTC_VALID RTC_REG_D
/** valid RAM and time */
#define RTC_VRT   0x80
/**********************************************************************/

#endif /* __AIFREGS_H__ */
