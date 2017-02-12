/*	iLink.c
 *	Purpose:	Contains the main functions of the iLinkman driver.
 *			It contains functions that are exported for use by other modules, and contains the entry point function.
 *
 *	Last Updated:	2012/02/29
 *	Programmer:	SP193
 */

#include <dmacman.h>
#include <errno.h>
#include <ioman.h>
#include <intrman.h>
#include <irx.h>
#include <loadcore.h>
#include <stdio.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#include <sysmem.h>
#include <sysclib.h>

#include "iLinkman.h"
#include "iLink_internal.h"

#define MODNAME "iLINK_HW_Manager"
IRX_ID(MODNAME, 0x00, 0x98);

/* Global variables. */
extern struct irx_export_table _exp_iLinkman;

struct ILINKMemMap *ILINKRegisterBase = (struct ILINKMemMap *)ILINK_REGISTER_BASE;
extern struct TransactionContextData TransactionContexts[MAX_CONCURRENT_TRANSACTIONS];
extern unsigned int *ConfigurationROM;
void (*CallBackFunction)(int reason, unsigned int offset, unsigned int size);

u64 ConsoleGUID;
char ConsoleModelName[32];

unsigned short int LocalNodeID;

extern int GenerationNumber;
extern int nNodes;
extern int NodeCapabilities;

int IntrEventFlag, UBUFTxSema, UBUFThreadID;

extern unsigned char IsBusRoot;

static iop_event_t EventFlagData = {
    2, /* evfp.attr=EA_MULTI */
    0, /* evfp.option */
    0  /* evfp.bits */
};

/* SP193: Get the console's BIOS version, and refuse to load if the console is a iLink-free console.
	Although Sony seems to have included the iLink controller in my SCPH-77006 console, it seems like DMAC #3 is missing from it's usual spot, and rom0:DMACMAN doesn't even know about the change.
	Doing anything with DMAC #3 resulted in bus errors. Strangely, DMAC #3 can be disabled at the startup of rom0:DMACMAN... I wonder why. Maybe I'm wrong, and DMACMAN doesn't disable the channels of DMAC #3 on my SCPH-77006 at bootup.
*/
#ifdef REQ_CHECK_CONSOLE_VERSION
static inline unsigned int GetBIOSVersion(void)
{
	char romver_str[5];
	int fd;

	fd = open("rom0:ROMVER", O_RDONLY);
	read(fd, romver_str, 4);
	close(fd);

	romver_str[4] = '\0';
	return strtoul(romver_str, NULL, 16);
}
#endif

int _start(int argc, char **argv)
{
	int i, result;
	iop_thread_t ThreadData;

	DEBUG_PRINTF("iLink driver version 0.98H\n");

#ifdef REQ_CHECK_CONSOLE_VERSION
	if (GetBIOSVersion() > 0x160) {
		printf("Unsupported console detected. Not loading.\n");
		return MODULE_NO_RESIDENT_END;
	}
#endif

	iLinkDisableIntr();

	if ((result = iLinkResetHW()) < 0) {
		DEBUG_PRINTF("Error occurred while resetting i.Link hardware. Code: %d\n", result);
		return MODULE_NO_RESIDENT_END;
	}

	iLinkHWInitialize();

	DEBUG_PRINTF("Initializing Configuration ROM...\n");

	memset(ConsoleModelName, 0, sizeof(ConsoleModelName));
	ConsoleGUID = 0;
	GetConsoleIDs(&ConsoleGUID, ConsoleModelName);
	DEBUG_PRINTF("Console GUID: 0x%08lx %08lx, ModelName: %s.\n", (u32)(ConsoleGUID >> 32), (u32)ConsoleGUID, ConsoleModelName);

	InitializeConfigurationROM();

	CallBackFunction = NULL;
	IntrEventFlag = CreateEventFlag(&EventFlagData);
	UBUFTxSema = CreateMutex(IOP_MUTEX_UNLOCKED);

	ThreadData.attr = TH_C;
	ThreadData.option = 0;
	ThreadData.thread = &UBUFThread;
	ThreadData.stacksize = 0x350;
	ThreadData.priority = 0x60;

	StartThread(UBUFThreadID = CreateThread(&ThreadData), NULL);

	RegisterIntrHandler(IOP_IRQ_ILINK, 1, &iLinkIntrHandler, NULL);
	EnableIntr(IOP_IRQ_ILINK);

	for (i = 0; i < MAX_CONCURRENT_TRANSACTIONS; i++)
		TransactionContexts[i].IsConnected = 0;

	DEBUG_PRINTF("Initialization complete.\n");

	return ((RegisterLibraryEntries(&_exp_iLinkman) != 0) ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END);
}

int _exit(int argc, char **argv)
{
	int result;

	/* Shutdown the hardware and unregister all registered library exports. */
	ReleaseLibraryEntries(&_exp_iLinkman);
	DisableIntr(IOP_IRQ_ILINK, &result);
	ReleaseIntrHandler(IOP_IRQ_ILINK);
	TerminateThread(UBUFThreadID);
	DeleteThread(UBUFThreadID);
	DeleteSema(UBUFTxSema);
	DeleteEventFlag(IntrEventFlag);
	iLinkShutdownHW();

	FreeSysMemory(ConfigurationROM); /* Free the memory allocated for storing the configuration ROM. */

	return 0;
}

int iLinkTrAlloc(unsigned short int NodeID, unsigned char speed)
{
	int result;
	unsigned int i;

	if (NodeID != LocalNodeID) {
		for (i = 0; i < MAX_CONCURRENT_TRANSACTIONS; i++) {
			if (TransactionContexts[i].IsConnected == 0) {
				TransactionContexts[i].NodeID = NodeID;
				TransactionContexts[i].GenerationNumber = GenerationNumber;
				TransactionContexts[i].IsConnected = 1;
				iLinkSetNodeTrSpeed(i, speed);
				break;
			}
		}

		result = (i == MAX_CONCURRENT_TRANSACTIONS) ? -1002 : i;
	} else
		result = -1;

	return result;
}

void iLinkTrFree(int trContext)
{
	TransactionContexts[trContext].IsConnected = 0;
}

int iLinkTrWrite(int trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes)
{
	return ((nBytes == 4) ? iLinkWriteReq(&TransactionContexts[trContext], offset_high, offset_low, buffer, nBytes) : iLinkWritePHTReq(&TransactionContexts[trContext], offset_high, offset_low, buffer, nBytes));
}

int iLinkTrRead(int trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes)
{
	return ((nBytes == 4) ? iLinkReadReq(&TransactionContexts[trContext], offset_high, offset_low, buffer, nBytes) : iLinkReadPHTReq(&TransactionContexts[trContext], offset_high, offset_low, buffer, nBytes));
}

unsigned int iLinkSetNodeCapabilities(unsigned int capabilities)
{
	NodeCapabilities = capabilities;

	/* Enable the enabled iLink features. */
	if (NodeCapabilities & iLink_NODE_CMC)
		iLinkEnableCMaster();

	InitializeConfigurationROM();
	return (NodeCapabilities);
}

int iLinkGetGenerationNumber(void)
{
	return (GenerationNumber);
}

int iLinkGetLocalNodeID(void)
{
	return (LocalNodeID);
}

int iLinkGetNodeCount(void)
{
	return (nNodes);
}

void iLinkEnableSBus(void)
{
	iLinkPHY_SetLCTRL(1);
	iLinkBusEnable();
	iLinkResetSBus();
}

void iLinkDisableSBus(void)
{
	ILINKRegisterBase->ctrl0 &= ~(iLink_CTRL0_RxEn | iLink_CTRL0_TxEn); /* CTL0: ~(TxEn | RxEn) */
}

void iLinkResetSBus(void)
{
	ClearEventFlag(IntrEventFlag, ~(iLinkEventBusReady | iLinkEventGotSELFIDs));

	DEBUG_PRINTF("-=Reseting bus=-\n");

	iLinkPHYBusReset();

	DEBUG_PRINTF("-=Waiting for the serial bus to be ready=-\n");
	WaitEventFlag(IntrEventFlag, iLinkEventBusReady | iLinkEventGotSELFIDs, WEF_AND, NULL); /* Wait for the bus to be completely reset. */
	DEBUG_PRINTF("-=Bus reset and ready=-\n");
}

void *iLinkSetTrCallbackHandler(void *function)
{
	void *cb_function;

	cb_function = CallBackFunction;
	CallBackFunction = function;

	return cb_function;
}

static const unsigned char speeds[3] = {
    0, /* S100 */
    2, /* S200 */
    4  /* S400 */
};

int iLinkGetNodeTrSpeed(int trContext)
{
	int CurrentSpeed, i;

	CurrentSpeed = -1;
	for (i = 0; i < 3; i++) {
		if (speeds[i] == TransactionContexts[trContext].speed) {
			CurrentSpeed = i;
			break;
		}
	}

	return CurrentSpeed;
}

int iLinkSetNodeTrSpeed(int trContext, unsigned char speed)
{
	int result;

	result = iLinkGetNodeTrSpeed(trContext);

	/* !!HACK!! Due to a chip bug (?) in the iLink hardware of most consoles, limit the speed to S100 mode whenever the console is the root node
		(Read the README file for more details on this bug). */
	if (IsBusRoot)
		speed = S100;

	/* Assign the specified speed to the specified transaction. Assign the maxium allowed speed if the specified speed is above the maximum value. */
	TransactionContexts[trContext].speed = (speed > (sizeof(speeds) - 1)) ? speeds[sizeof(speeds) - 1] : speeds[speed];

	return result;
}

u64 iLinkGetLocalNodeEUI64(void)
{
	return ConsoleGUID;
}
