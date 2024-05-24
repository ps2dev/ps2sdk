/**
 * @file
 * AIF and RTC registers
 */

#ifndef __AIFREGS_H__
#define __AIFREGS_H__

#include <tamtypes.h>

// clang-format off
/*
 * Slightly modified from "Basic AIF driver set" by sp193 ("[140524]AIF.7z")
 * The Sony PlayStation 2 TOOL DTL-T10000(H) units have a slightly different SSBUSC design from the SCPH-10000 unit:
 * its expansion interface (the PC CARD slot) seems to be connected and managed from an interface known as the "AIF".
 * Nothing too much is known about this interface, other than the fact that it's only present on TOOL units.
 * 
 * Every TOOL unit has two ATA disks: one which is connected to the SBC PCI card,
 * while the other is connected to a PCB that has the PC CARD slot on it.
 * 
 * While the HDD that is connected to the SBC card obviously has a purpose (it contains the PC-side Linux installation),
 * the purpose of the other unit is not very clear because it was never used for any official applications.
 * 
 * That HDD unit, which is accessible via the AIF on the PlayStation 2 side of the TOOL,
 * was reported to have contained a pristine copy of the PC-side Linux installation.
 * However, it does not seem possible to restore the SBC HDD unit without external help because the AIF HDD is not directly accessible from the SBC.
 * 
 * The AIF is known to have support for only three devices: ATA HDD unit 0, a Motorola MC146818A RTC, and the PC CARD interface.
 * 
 * While the AIF seems to have support for ATA HDD unit 1, there isn't a physical port to connect a second HDD unit to.
 * 
 * The lack of DMA support in the official Sony code (within the PS2 Linux kernel) and the apparent lack of hardware registers for DMA support
 * suggests that it might be indeed incapable of any DMA transfer modes.
 *
 * Memory seems to be mirrored after offset 0x200.
 * 
 * Legend:
 *  R   Read-only
 *  W   Write-only
 *  X   R/W
 *  .   Unused
 * 
 * Offset:      Register:   Detected bits:      Default value:
 * 0x0000       IDENT       RRRRRRRRRRRRRRRR    0x0061
 * 0x0002       REVISION    RRRRRRRRRRRRRRRR    0x0003
 * 0x0004       INTSR/INTCL .............XXX    0x0002
 * 0x0006       INTEN       .............XXX    0x0000
 * 0x0008       TIMCFG      ...............X    0x0000
 * 0x000A       unknown     ...............R    0x0001
 * 0x000C       unknown     ...............R    0x0001
 * 0x0010       COUNT_L     RRRRRRRRRRRRRRRR    ??????
 * 0x0012       COUNT_H     RRRRRRRRRRRRRRRR    ??????
 * 0x0014-0x003E    unknown     ...............R    0x0001
 * 0x0040       ATA_TCFG    ........XXXXXXXX    0x0000
 * 0x0042-0x005E    unknown     ...............R    0x0001
 * 0x0060       ATA_DATA    ........XXXXXXXX    0xFF50
 * 0x0062       ATA_FEATURE ........XXXXXXXX    0xFF00
 * 0x0064       ATA_NSECTOR ........XXXXXXXX    0xFF00
 * 0x0066       ATA_SECTOR  ........XXXXXXXX    0xFF01
 * 0x0068       ATA_LCYL    ........XXXXXXXX    0xFF01
 * 0x006A       ATA_HCYL    ........XXXXXXXX    0xFF01
 * 0x006C       ATA_SELECT  ........XXXXXXXX    0xFF00
 * 0x006E       ATA_COMMAND ........XXXXXXXX    0xFF00
 * 0x0070       unknown     ...............X    0xFF00
 * 0x0072       unknown     ...............X    0xFF00
 * 0x0074       unknown     ...............X    0xFF00
 * 0x0076       unknown     ...............X    0xFF01
 * 0x0078       unknown     ...............X    0xFF01
 * 0x007A       unknown     ...............X    0xFF00
 * 0x007C       ATA_CONTROL ........XXXXXXXX    0xFF00
 * 0x007E       unknown     ........XXXXXXXX    0xFF50
 * 0x0080-0x00DE    unknown     ...............R    0x0001
 * 0x00E0-0x00FE    A second ATA port? The power-on defaults appear to be the same as the ATA registers above.
 * 0x0100-0x011A    MC146818A RTC registers.
 * 0x011C-0x01FE    unknown     Probably the RTC's user RAM region. Seems to be fully readable from and writable to.
 * 
 * Reading from all the unknown seems to be wonky, as it takes the second read from the register to read in the written value.
 * Sometimes, it even feels as if the value that is read was from elsewhere. Are they possibly write-only or unmapped?
 * 
 * The purpose of TIMCFG, COUNT_L and COUNT_H is not known because the official Sony code within the PS2 Linux kernel does not seem to use these registers anywhere.
 * 
 * INTEN, INTSR and INTCL bits:
 * Bit 1: ATA0
 * Bit 2: RTC? The purpose of the bit here isn't known because it isn't used, but it seems likely to be the RTC's because the RTC doesn't have a known interrupt even bit for itself.
 * Bit 3: PCMCIA interrupt.
*/
// clang-format on

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
