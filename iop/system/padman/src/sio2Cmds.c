/*
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

/**
 * @file
 * IOP pad driver
 */

#include "irx.h"
#include "types.h"
#include "sio2Cmds.h"

typedef struct
{
	u8 id;
	u8 pad[3];
	void (*readdata)(u8 *);
	u32 (*getportctrl1)(u32, u32);
	u32 (*getportctrl2)(u32);
	u32 (*reg_data)(void);
	u32 (*size1)(void);
	u32 (*size2)(void);
	u32 (*enterconfigmode)(u8 *);
	u32 (*exitconfigmode)(u8 *);
	u32 (*querymodel)(u8 *);
	u32 (*queryact)(u8 *);
	u32 (*querycomb)(u8 *);
	u32 (*querymode)(u8 *);
	u32 (*querybuttonmask)(u8 *);
	u32 (*setbuttoninfo)(u8 *);
	u32 (*setvrefparam)(u8 *);
	u32 (*setmainmode)(u8 *);
	u32 (*setactalign)(u8 *);
} sio2Cmds_t;

static sio2Cmds_t sio2Cmds[SIO2_CMD_MAX];
static int numControllers;

void sio2cmdReset(void)
{
	int i;

	for(i=0; i < SIO2_CMD_MAX; i++)
		sio2Cmds[i].id = 0;

	numControllers = 0;
}

static int SetupCmds(const sio2Cmds_t *s)
{
	if( ((numControllers+1) < SIO2_CMD_MAX) && (s != NULL) )
	{
		if(numControllers > 0)
		{
			int i = 0;

			// Check if the controller was already registered (if so, do nothing).
			do
			{
				if(s->id == sio2Cmds[i].id)
					return 0;

				i++;
			} while(numControllers > i);
		}

		//Register new controller
		int i = numControllers;

		sio2Cmds[i].id = s->id;
		sio2Cmds[i].readdata = s->readdata;
		sio2Cmds[i].getportctrl1 = s->getportctrl1;
		sio2Cmds[i].getportctrl2 = s->getportctrl2;
		sio2Cmds[i].reg_data = s->reg_data;
		sio2Cmds[i].size1 = s->size1;
		sio2Cmds[i].size2 = s->size2;
		sio2Cmds[i].enterconfigmode = s->enterconfigmode;
		sio2Cmds[i].exitconfigmode = s->exitconfigmode;
		sio2Cmds[i].querymodel = s->querymodel;
		sio2Cmds[i].queryact = s->queryact;
		sio2Cmds[i].querycomb = s->querycomb;
		sio2Cmds[i].querymode = s->querymode;
		sio2Cmds[i].querybuttonmask = s->querybuttonmask;
		sio2Cmds[i].setbuttoninfo = s->setbuttoninfo;
		sio2Cmds[i].setvrefparam = s->setvrefparam;
		sio2Cmds[i].setmainmode = s->setmainmode;
		sio2Cmds[i].setactalign = s->setactalign;

		numControllers++;

		return 1;
	}
	else
	{
		return 0;
	}
}

/*******************************************************************************
 FindPads
*******************************************************************************/

static void FindPadsReadData(u8 *a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
}

static u32 FindPadsGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 FindPadsGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x2000A;
	else
		return 0x2012C;
}

static u32 FindPadsGetSize1(void)
{
	return 0x5;
}

static u32 FindPadsGetSize2(void)
{
	return 0x5;
}

static u32 FindPadsRegData(void)
{
	u32 res1, res2;

	res1 = FindPadsGetSize1();

	res1 = ((res1 & 0x1FF) << 8) | 0x40;

	res2 = FindPadsGetSize2();

	res2 = (res2 & 0x1FF) << 18;

	return (res1 | res2);
}

static u32 FindPadsEnterConfigMode(u8 *a)
{
	return 0;
}

void sio2cmdInitFindPads(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI(PAD_ID_FINDPADS);

	s.readdata = &FindPadsReadData;
	s.getportctrl1 = &FindPadsGetPortCtrl1;
	s.getportctrl2 = &FindPadsGetPortCtrl2;
	s.reg_data = &FindPadsRegData;
	s.size1 = &FindPadsGetSize1;
	s.size2 = &FindPadsGetSize2;
	s.enterconfigmode = &FindPadsEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Mouse
*******************************************************************************/

static void MouseReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
}

static u32 MouseGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 MouseGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 MouseRegData(void)
{
	return 0x1c0740;
}

static u32 MouseSize1(void)
{
	return 0x7;
}

static u32 MouseSize2(void)
{
	return 0x7;
}

static u32 MouseEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;

	return 0x7;
}

void sio2cmdInitMouse(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI(PAD_ID_MOUSE);

	s.readdata = &MouseReadData;
	s.getportctrl1 = &MouseGetPortCtrl1;
	s.getportctrl2 = &MouseGetPortCtrl2;
	s.reg_data = &MouseRegData;
	s.size1 = &MouseSize1;
	s.size2 = &MouseSize2;
	s.enterconfigmode = &MouseEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;


	SetupCmds(&s);
}

/*******************************************************************************
 Negicon
*******************************************************************************/

static void NegiconReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;
}

static u32 NegiconGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}


static u32 NegiconGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 NegiconRegData(void)
{
	return 0x240940;
}

static u32 NegiconSize1(void)
{
	return 0x9;
}

static u32 NegiconSize2(void)
{
	return 0x9;
}

static u32 NegiconEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;

	return 0x9;
}

void sio2cmdInitNegicon(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI(PAD_ID_NEGICON);

	s.readdata = &NegiconReadData;
	s.getportctrl1 = &NegiconGetPortCtrl1;
	s.getportctrl2 = &NegiconGetPortCtrl2;
	s.reg_data = &NegiconRegData;
	s.size1 = &NegiconSize1;
	s.size2 = &NegiconSize2;
	s.enterconfigmode = &NegiconEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Konami Gun
*******************************************************************************/

static void KonamiGunReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
}

static u32 KonamiGunGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 KonamiGunGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 KonamiGunRegData(void)
{
	return 0x140540;
}

static u32 KonamiGunSize1(void)
{
	return 0x5;
}

static u32 KonamiGunSize2(void)
{
	return 0x5;
}

static u32 KonamiGunEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;

	return 0x5;
}

void sio2cmdInitKonamiGun(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI( PAD_ID_KONAMIGUN );

	s.readdata = &KonamiGunReadData;
	s.getportctrl1 = &KonamiGunGetPortCtrl1;
	s.getportctrl2 = &KonamiGunGetPortCtrl2;
	s.reg_data = &KonamiGunRegData;
	s.size1 = &KonamiGunSize1;
	s.size2 = &KonamiGunSize2;
	s.enterconfigmode = &KonamiGunEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Digital
*******************************************************************************/

static void DigitalReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
}

static u32 DigitalGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 DigitalGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 DigitalRegData(void)
{
	return 0x140540;
}

static u32 DigitalSize1(void)
{
	return 0x5;
}

static u32 DigitalSize2(void)
{
	return 0x5;
}

static u32 DigitalEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;

	return 0x5;
}

void sio2cmdInitDigital(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI(PAD_ID_DIGITAL);

	s.readdata = &DigitalReadData;
	s.getportctrl1 = &DigitalGetPortCtrl1;
	s.getportctrl2 = &DigitalGetPortCtrl2;
	s.reg_data = &DigitalRegData;
	s.size1 = &DigitalSize1;
	s.size2 = &DigitalSize2;
	s.enterconfigmode = &DigitalEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Joystick
*******************************************************************************/

static void JoystickReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;
}

static u32 JoystickGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 JoystickGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 JoystickRegData(void)
{
	return 0x240940;
}

static u32 JoystickSize1(void)
{
	return 0x9;
}

static u32 JoystickSize2(void)
{
	return 0x9;
}

static u32 JoystickEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;

	return 0x9;
}

void sio2cmdInitJoystick(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI( PAD_ID_JOYSTICK );

	s.readdata = &JoystickReadData;
	s.getportctrl1 = &JoystickGetPortCtrl1;
	s.getportctrl2 = &JoystickGetPortCtrl2;
	s.reg_data = &JoystickRegData;
	s.size1 = &JoystickSize1;
	s.size2 = &JoystickSize2;
	s.enterconfigmode = &JoystickEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Namco Gun
*******************************************************************************/

static void NamcoGunReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;
}

static u32 NamcoGunGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 NamcoGunGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 NamcoGunRegData(void)
{
	return 0x240940;
}

static u32 NamcoGunSize1(void)
{
	return 0x9;
}

static u32 NamcoGunSize2(void)
{
	return 0x9;
}

static u32 NamcoGunEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;

	return 0x9;
}

void sio2cmdInitNamcoGun(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI(PAD_ID_NAMCOGUN);

	s.readdata = &NamcoGunReadData;
	s.getportctrl1 = &NamcoGunGetPortCtrl1;
	s.getportctrl2 = &NamcoGunGetPortCtrl2;
	s.reg_data = &NamcoGunRegData;
	s.size1 = &NamcoGunSize1;
	s.size2 = &NamcoGunSize2;
	s.enterconfigmode = &NamcoGunEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Analog
*******************************************************************************/

static void AnalogReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;
	a[9] = 0;
	a[10] = 0;
	a[11] = 0;
	a[12] = 0;
	a[13] = 0;
	a[14] = 0;
}

static u32 AnalogGetPortCtrl1(u32 a, u32 b)
{
	if(a != 0)
		return 0xFF060505;
	else
	{
		u32 val1, val2, val3 ;

		if((b & 0x2) == 0)
			val1 = 0x5;
		else
			val1 = 0xA;

		if((b & 0x2) == 0)
			val2 = 0x5;
		else
			val2 = 0xA;

		val1 &= 0xFFFF00FF;

		val3 = val1 | ( val2 << 8);

		if((b & 0x2) == 0)
			val1 = 0xC0;
		else
			val1 = 0x60;

		val3 &= 0xFF00FFFF;
		val3 = val3 | (val1 << 16) | 0xFF000000;

		return val3;
	}
}

static u32 AnalogGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 AnalogEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;
	a[9] = 0;
	a[10] = 0;
	a[11] = 0;
	a[12] = 0;
	a[13] = 0;
	a[14] = 0;

	return 0x15;
}

void sio2cmdInitAnalog(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI(PAD_ID_ANALOG);

	s.readdata = &AnalogReadData;
	s.getportctrl1 = &AnalogGetPortCtrl1;
	s.getportctrl2 = &AnalogGetPortCtrl2;
	s.reg_data = NULL;
	s.size1 = NULL;
	s.size2 = NULL;
	s.enterconfigmode = &AnalogEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Jogcon
*******************************************************************************/

static void JogconReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;
	a[9] = 0;
	a[10] = 0;
	a[11] = 0;
	a[12] = 0;
}

static u32 JogconGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 JogconGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 JogconRegData(void)
{
	return 0x340d40;
}

static u32 JogconSize1(void)
{
	return 0xD;
}

static u32 JogconSize2(void)
{
	return 0xD;
}

static u32 JogconEnterConfigMode(u8 *a)
{
	a[0] = 0x1;
	a[1] = 0x43;
	a[2] = 0;
	a[3] = 0x1;
	a[4] = 0;
	a[5] = 0;
	a[6] = 0;
	a[7] = 0;
	a[8] = 0;
	a[9] = 0;
	a[10] = 0;
	a[11] = 0;
	a[12] = 0;

	return 0xD;
}

void sio2cmdInitJogcon(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI(PAD_ID_JOGCON);

	s.readdata = &JogconReadData;
	s.getportctrl1 = &JogconGetPortCtrl1;
	s.getportctrl2 = &JogconGetPortCtrl2;
	s.reg_data = &JogconRegData;
	s.size1 = &JogconSize1;
	s.size2 = &JogconSize2;
	s.enterconfigmode = &JogconEnterConfigMode;
	s.exitconfigmode = NULL;
	s.querymodel = NULL;
	s.queryact = NULL;
	s.querycomb = NULL;
	s.querymode = NULL;
	s.querybuttonmask = NULL;
	s.setbuttoninfo = NULL;
	s.setvrefparam = NULL;
	s.setmainmode = NULL;
	s.setactalign = NULL;

	SetupCmds(&s);
}

/*******************************************************************************
 Config
*******************************************************************************/

static void ConfigReadData(u8* a)
{
	a[0] = 1;
	a[1] = 0x42;
	a[2] = 0;
	a[3] = 0x5A;
	a[4] = 0x5A;
	a[5] = 0x5A;
	a[6] = 0x5A;
	a[7] = 0x5A;
	a[8] = 0x5A;
}

static u32 ConfigGetPortCtrl1(u32 a, u32 b)
{
	if(a == 0)
		return 0xFFC00505;
	else
		return 0xFF060505;
}

static u32 ConfigGetPortCtrl2(u32 a)
{
	if(a == 0)
		return 0x20014;
	else
		return 0x2012C;
}

static u32 ConfigRegData(void)
{
	return 0x240940;
}

static u32 ConfigSize1(void)
{
	return 0x9;
}

static u32 ConfigSize2(void)
{
	return 0x9;
}

static u32 ConfigExitConfigMode(u8 *a)
{
	a[0] = 1;
	a[1] = 0x43;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x5a;
	a[5] = 0x5a;
	a[6] = 0x5a;
	a[7] = 0x5a;
	a[8] = 0x5a;

	return 9;
}

static u32 ConfigQueryModel(u8 *a)
{
	a[0] = 1;
	a[1] = 0x45;
	a[2] = 0x0;
	a[3] = 0x5a;
	a[4] = 0x5a;
	a[5] = 0x5a;
	a[6] = 0x5a;
	a[7] = 0x5a;
	a[8] = 0x5a;

	return 9;
}

static u32 ConfigQueryAct(u8 *a)
{
	a[0] = 1;
	a[1] = 0x46;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x5a;
	a[5] = 0x5a;
	a[6] = 0x5a;
	a[7] = 0x5a;
	a[8] = 0x5a;

	return 9;
}

static u32 ConfigQueryComb(u8 *a)
{
	a[0] = 1;
	a[1] = 0x47;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x5a;
	a[5] = 0x5a;
	a[6] = 0x5a;
	a[7] = 0x5a;
	a[8] = 0x5a;

	return 9;
}

static u32 ConfigQueryMode(u8 *a)
{
	a[0] = 1;
	a[1] = 0x4c;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x5a;
	a[5] = 0x5a;
	a[6] = 0x5a;
	a[7] = 0x5a;
	a[8] = 0x5a;

	return 9;
}

static u32 ConfigQueryButtonMask(u8 *a)
{
	a[0] = 1;
	a[1] = 0x41;
	a[2] = 0x0;
	a[3] = 0x5a;
	a[4] = 0x5a;
	a[5] = 0x5a;
	a[6] = 0x5a;
	a[7] = 0x5a;
	a[8] = 0x5a;

	return 9;
}

static u32 ConfigSetButtonInfo(u8 *a)
{
	a[0] = 1;
	a[1] = 0x4F;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x0;
	a[5] = 0x0;
	a[6] = 0x0;
	a[7] = 0x0;
	a[8] = 0x0;

	return 9;
}

static u32 ConfigSetVrefParam(u8 *a)
{
	a[0] = 1;
	a[1] = 0x40;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x0;
	a[5] = 0x0;
	a[6] = 0x0;
	a[7] = 0x0;
	a[8] = 0x0;

	return 9;
}

static u32 ConfigSetMainMode(u8 *a)
{
	a[0] = 1;
	a[1] = 0x44;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x0;
	a[5] = 0x0;
	a[6] = 0x0;
	a[7] = 0x0;
	a[8] = 0x0;

	return 9;
}

static u32 ConfigSetSetActAlign(u8 *a)
{
	a[0] = 1;
	a[1] = 0x4d;
	a[2] = 0x0;
	a[3] = 0x0;
	a[4] = 0x0;
	a[5] = 0x0;
	a[6] = 0x0;
	a[7] = 0x0;
	a[8] = 0x0;

	return 9;
}

void sio2cmdInitConfig(void)
{
	sio2Cmds_t s;

	s.id = PAD_ID_HI( PAD_ID_CONFIG );

	s.readdata = &ConfigReadData;
	s.getportctrl1 = &ConfigGetPortCtrl1;
	s.getportctrl2 = &ConfigGetPortCtrl2;
	s.reg_data = &ConfigRegData;
	s.size1 = &ConfigSize1;
	s.size2 = &ConfigSize2;
	s.enterconfigmode = NULL;
	s.exitconfigmode = &ConfigExitConfigMode;
	s.querymodel = &ConfigQueryModel;
	s.queryact = &ConfigQueryAct;
	s.querycomb = &ConfigQueryComb;
	s.querymode = &ConfigQueryMode;
	s.querybuttonmask = &ConfigQueryButtonMask;
	s.setbuttoninfo = &ConfigSetButtonInfo;
	s.setvrefparam = &ConfigSetVrefParam;
	s.setmainmode = &ConfigSetMainMode;
	s.setactalign = &ConfigSetSetActAlign;

	SetupCmds(&s);
}

/*******************************************************************************
*******************************************************************************/

u32 sio2cmdCheckId(u8 id)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
			return 1;
	}

	return 0;
}


u32 sio2CmdGetPortCtrl1(u8 id, u32 b, u8 c)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if(sio2Cmds[i].id == id)
		{
			if(sio2Cmds[i].getportctrl1 != NULL)
				return sio2Cmds[i].getportctrl1(b, c);
		}
	}

	return 0;
}

u32 sio2CmdGetPortCtrl2(u32 id, u32 b)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if(sio2Cmds[i].id == id)
		{
			if(sio2Cmds[i].getportctrl2 != NULL)
				return sio2Cmds[i].getportctrl2(b);
		}
	}

	return 0;
}

void sio2CmdSetReadData(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if(sio2Cmds[i].id == id)
		{
			if(sio2Cmds[i].readdata != NULL)
			{
				sio2Cmds[i].readdata(buf);
				return;
			}
		}
	}
}

u32 sio2CmdSetEnterConfigMode(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if(sio2Cmds[i].id == id)
		{
			if(sio2Cmds[i].enterconfigmode != NULL)
				return sio2Cmds[i].enterconfigmode(buf);
		}

	}

	return 0;
}

u32 sio2CmdSetQueryModel(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].querymodel != NULL)
				return sio2Cmds[i].querymodel(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetSetMainMode(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].setmainmode != NULL)
				return sio2Cmds[i].setmainmode(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetQueryAct(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].queryact != NULL)
				return sio2Cmds[i].queryact(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetQueryComb(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].querycomb != NULL)
				return sio2Cmds[i].querycomb(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetQueryMode(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].querymode != NULL)
				return sio2Cmds[i].querymode(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetExitConfigMode(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].exitconfigmode != NULL)
				return sio2Cmds[i].exitconfigmode(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetSetActAlign(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].setactalign != NULL)
				return sio2Cmds[i].setactalign(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetQueryButtonMask(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].querybuttonmask != NULL)
				return sio2Cmds[i].querybuttonmask(buf);
		}
	}

	return 0;
}

u32 sio2CmdSetSetVrefParam(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].setvrefparam != NULL)
				return sio2Cmds[i].setvrefparam(buf);
		}
	}

	return 0;

}

u32 sio2CmdSetSetButtonInfo(u32 id, u8 *buf)
{
	int i;
	id = PAD_ID_HI(id);

	for(i=0; i < numControllers; i++)
	{
		if( sio2Cmds[i].id == id )
		{
			if( sio2Cmds[i].setbuttoninfo != NULL)
				return sio2Cmds[i].setbuttoninfo(buf);
		}
	}

	return 0;

}
