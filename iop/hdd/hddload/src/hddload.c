#include <dev9.h>
#include <atad.h>
#include <cdvdman.h>
#include <intrman.h>
#include <loadcore.h>
#include <secrman.h>
#include <sifman.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>

static int DMATxBuffer[4];
static int DMATransferID;
static u32 OSDAddr;
static u32 OSDStatAddr;
static u32 SectorBuffer[128];

//Function prototypes
static void SendResultCode(int result);
static void HDDBootError(int result);
static void FinishHDDLoad(int result);
static int ReadMBR(unsigned char *buffer, unsigned int lba, unsigned int count);
static void HDDLOADThread(void *arg);

int _start(int argc, char *argv[])
{
	int i, result, ThreadID;
	iop_thread_t ThreadData;

	FlushDcache();
	OSDStatAddr = 0;
	OSDAddr = 0;

	if(argc>0)
	{
		i=0;

		do{
			printf("arg %d %s %s\n", i, argv[0], argv[1]);

			if(strcmp(argv[0], "-osd") == 0)
			{
				i++;
				argv++;
				OSDAddr=strtol(*argv, NULL, 0);
				printf("osd addr %lx\n", OSDAddr);
			}
			else if(strcmp(argv[0], "-stat") == 0)
			{
				i++;
				argv++;
				OSDStatAddr=strtol(*argv, NULL, 0);
				printf("stat addr %lx\n", OSDStatAddr);
			}

			i++;
			argv++;
		}while(i<argc);
	}

	if((OSDAddr == 0) || (OSDStatAddr == 0))
	{
		printf("osd_addr or stat addr dont exist\n");
		result = MODULE_NO_RESIDENT_END;
	}
	else
	{
		CpuEnableIntr();
		ThreadData.attr = TH_C;
		ThreadData.thread = &HDDLOADThread;
		ThreadData.priority = 0x32;
		ThreadData.stacksize = 0x2000;
		ThreadData.option = 0;
		if((ThreadID = CreateThread(&ThreadData)) > 0)
		{
			StartThread(ThreadID, NULL);
			result = MODULE_RESIDENT_END;
		}
		else
		{
			dev9Shutdown();
			result = MODULE_NO_RESIDENT_END;
		}
	}

	return result;
}

static void SendResultCode(int result)
{
	int OldState;
	SifDmaTransfer_t dmat;

	while(sceSifDmaStat(DMATransferID) >= 0){};

	DMATxBuffer[0] = result;

	dmat.src=DMATxBuffer;
	dmat.size=0x10;
	dmat.attr=0;
	dmat.dest=(void*)OSDStatAddr;

	CpuSuspendIntr(&OldState);
	DMATransferID = sceSifSetDma(&dmat, 1);
	CpuResumeIntr(OldState);
}

static void HDDBootError(int result)
{
	dev9Shutdown();
	SendResultCode(result);
	SleepThread();
}

static void FinishHDDLoad(int result)
{
	u8 value;

	value=*(*(vu8 **)0x000003c0);	//Access the system configuration storage, which is set up by EECONF.

	if(!(value&2)) dev9Shutdown();	//If HDD support is disabled, switch off the DEV9 interface.
	SendResultCode(result);
	SleepThread();
}

static int ReadMBR(unsigned char *buffer, unsigned int lba, unsigned int count)
{
	int NumSectorsToRead, BufferOffset;

	BufferOffset=0;
	while(count>0)
	{
		NumSectorsToRead=(count<0x81)?count:0x80;

		if(ata_device_sector_io(0, buffer+BufferOffset, lba, NumSectorsToRead, ATA_DIR_READ)!=0)
		{
			printf("cannot read sector 0\n");
			return -1;
		}

		count-=NumSectorsToRead;
		lba+=NumSectorsToRead;
		buffer+=(NumSectorsToRead * 512);
	}
	
	return 0;
}

static void HDDLOADThread(void *arg)
{
	ata_devinfo_t *devinfo;
	int i, OldState, BufferSize;
	u32 stat;
	u8 iLinkID[0x20];
	u8 *buffer;
	void *DecryptedKELF;

	printf("HDL:hdd load thread start.(01/06/10)\n");

	SendResultCode(0);

	printf("HDL:init ata\n");
	if((devinfo = ata_get_devinfo(0)) == NULL){
		printf("HDL:ATA initialization failed.\n");
		HDDBootError(-1);
	}

	for(i=0x1F; i>=0; i--) iLinkID[i] = 0;

	sceCdRI(iLinkID, &stat);

	if(ata_device_sce_sec_unlock(0, iLinkID) != 0)
	{
		printf("cannot unlock password\n");
		HDDBootError(-2);
	}

	printf("HDL:read 0 sec\n");
	if(ata_device_sector_io(0, SectorBuffer, 0, 1, ATA_DIR_READ) != 0)
	{
		printf("cannot read sector 0\n");
		HDDBootError(-3);
	}

	printf("HDL:osdstart %lx osdsize %lx\n", SectorBuffer[76], SectorBuffer[77]);

	BufferSize=SectorBuffer[77]<<9;

	CpuSuspendIntr(&OldState);
	buffer=AllocSysMemory(ALLOC_LAST, BufferSize, NULL);
	CpuResumeIntr(OldState);

	if(buffer==NULL)
	{
		printf("cannot alloc memory\n");
		HDDBootError(-6);
	}

	if(ReadMBR(buffer, SectorBuffer[76], SectorBuffer[77]) < 0)
	{
		printf("cannot read data \n");
		CpuSuspendIntr(&OldState);
		FreeSysMemory(buffer);
		CpuResumeIntr(OldState);
		HDDBootError(-4);
	}

	if((buffer[0] == 1) && (buffer[3] & 0x04))
	{
		printf("HDL:securiyt decript.\n");

		if((DecryptedKELF=SecrDiskBootFile(buffer))==NULL)
		{
			printf("cannot secur data \n");
			CpuSuspendIntr(&OldState);
			FreeSysMemory(buffer);
			CpuResumeIntr(OldState);
			HDDBootError(-5);
		}

		SifDmaTransfer_t dmat;

		dmat.src=DecryptedKELF;
		dmat.size=BufferSize;
		dmat.attr=0;
		dmat.dest=(void*)OSDAddr;

		CpuSuspendIntr(&OldState);
		sceSifSetDma(&dmat, 1);
		CpuResumeIntr(OldState);

		CpuSuspendIntr(&OldState);
		FreeSysMemory(buffer);
		CpuResumeIntr(OldState);

		printf("HDL:END ver.1.00\n");

		FinishHDDLoad(1);
	}
	else{
		printf("HDL:security contents error.\n");
		CpuSuspendIntr(&OldState);
		FreeSysMemory(buffer);
		CpuResumeIntr(OldState);
		HDDBootError(-5);
	}
}
