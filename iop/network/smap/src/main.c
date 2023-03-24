#ifdef BUILDING_SMAP_NETMAN
#include <errno.h>
#endif
#include <stdio.h>
#ifdef BUILDING_SMAP_PS2IP
#include <sysclib.h>
#endif
#include <loadcore.h>
#include <thbase.h>
#ifdef BUILDING_SMAP_PS2IP
#include <thevent.h>
#include <thsemap.h>
#include <intrman.h>
#endif
#ifdef BUILDING_SMAP_PS2IP
#include <ps2ip.h>
#endif
#ifdef BUILDING_SMAP_NETMAN
#include <irx.h>
#include <netman.h>
#endif

#include "main.h"
#ifdef BUILDING_SMAP_NETMAN
#include "xfer.h"
#endif

// Last SDK 3.1.0 has INET family version "2.26.0"
// SMAP module is the same as "2.25.0"
IRX_ID("SMAP_driver", 0x2, 0x1A);

#ifdef BUILDING_SMAP_PS2IP

#define IFNAME0 's'
#define IFNAME1 'm'

typedef struct ip4_addr IPAddr;
typedef struct netif NetIF;
typedef struct SMapIF SMapIF;
typedef struct pbuf PBuf;

static struct pbuf *TxHead, *TxTail;
static void EnQTxPacket(struct pbuf *tx);

static NetIF NIF;

// From lwip/err.h and lwip/tcpip.h

#define ERR_OK   0   // No error, everything OK
#define ERR_MEM  -1  // Out of memory error.
#define ERR_CONN -6  // Not connected
#define ERR_IF   -11 // Low-level netif error

// SMapLowLevelOutput():

// This function is called by the TCP/IP stack when a low-level packet should be sent. It'll be invoked in the context of the
// tcpip-thread.

static err_t
SMapLowLevelOutput(NetIF *pNetIF, PBuf *pOutput)
{
    err_t result;
    struct pbuf *pbuf;

#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif

    (void)pNetIF;

    result = ERR_OK;
    if (pOutput->tot_len > pOutput->len) {
        pbuf_ref(pOutput);                                          // Increment reference count because LWIP must free the PBUF, not the driver!
        if ((pbuf = pbuf_coalesce(pOutput, PBUF_RAW)) != pOutput) { // No need to increase reference count because pbuf_coalesce() does it.
            EnQTxPacket(pbuf);
            SMAPXmit();
        } else
            result = ERR_MEM;
    } else {
        pbuf_ref(pOutput); // This will be freed later.
        EnQTxPacket(pOutput);
        SMAPXmit();
    }

#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return result;
}

// SMapOutput():

// This function is called by the TCP/IP stack when an IP packet should be sent. It'll be invoked in the context of the
// tcpip-thread, hence no synchronization is required.
//  For LWIP versions before v1.3.0.
#ifdef PRE_LWIP_130_COMPAT
static err_t
SMapOutput(NetIF *pNetIF, PBuf *pOutput, IPAddr *pIPAddr)
{
    err_t result;
    PBuf *pBuf;

#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif

    pBuf = etharp_output(pNetIF, pIPAddr, pOutput);

    result = pBuf != NULL ? SMapLowLevelOutput(pNetIF, pBuf) : ERR_OK;

#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return result;
}
#endif

// SMapIFInit():

// Should be called at the beginning of the program to set up the network interface.

static err_t
SMapIFInit(NetIF *pNetIF)
{
#if USE_GP_REGISTER
    void *OldGP;

    OldGP = SetModuleGP();
#endif

    TxHead = NULL;
    TxTail = NULL;

    pNetIF->name[0] = IFNAME0;
    pNetIF->name[1] = IFNAME1;
#ifdef PRE_LWIP_130_COMPAT
    pNetIF->output = &SMapOutput; // For LWIP versions before v1.3.0.
#else
    pNetIF->output = &etharp_output;                             // For LWIP 1.3.0 and later.
#endif
    pNetIF->linkoutput = &SMapLowLevelOutput;
    pNetIF->hwaddr_len = NETIF_MAX_HWADDR_LEN;
#ifdef PRE_LWIP_130_COMPAT
    pNetIF->flags |= (NETIF_FLAG_LINK_UP | NETIF_FLAG_BROADCAST); // For LWIP versions before v1.3.0.
#else
    pNetIF->flags |= (NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST); // For LWIP v1.3.0 and later.
#endif
    pNetIF->mtu = 1500;

    // Get MAC address.
    SMAPGetMACAddress(pNetIF->hwaddr);
    DEBUG_PRINTF("MAC address : %02x:%02x:%02x:%02x:%02x:%02x\n", pNetIF->hwaddr[0], pNetIF->hwaddr[1], pNetIF->hwaddr[2],
                 pNetIF->hwaddr[3], pNetIF->hwaddr[4], pNetIF->hwaddr[5]);

    // Enable sending and receiving of data.
    SMAPInitStart();

#if USE_GP_REGISTER
    SetGP(OldGP);
#endif

    return ERR_OK;
}

void SMapLowLevelInput(PBuf *pBuf)
{
    // When we receive data, the interrupt-handler will invoke this function, which means we are in an interrupt-context. Pass on
    // the received data to ps2ip.

    ps2ip_input(pBuf, &NIF);
}

static void EnQTxPacket(struct pbuf *tx)
{
    int OldState;

    CpuSuspendIntr(&OldState);

    if (TxHead != NULL)
        TxHead->next = tx;

    TxHead   = tx;
    tx->next = NULL;

    if (TxTail == NULL) // Queue empty
        TxTail = TxHead;

    CpuResumeIntr(OldState);
}

int SMapTxPacketNext(void **payload)
{
    int len;

    if (TxTail != NULL) {
        *payload = TxTail->payload;
        len      = TxTail->len;
    } else
        len = 0;

    return len;
}

void SMapTxPacketDeQ(void)
{
    struct pbuf *toFree;
    int OldState;

    toFree = NULL;

    CpuSuspendIntr(&OldState);
    if (TxTail != NULL) {
        toFree = TxTail;

        if (TxTail == TxHead) {
            // Last in queue.
            TxTail = NULL;
            TxHead = NULL;
        } else {
            TxTail = TxTail->next;
        }
    }
    CpuResumeIntr(OldState);

    if (toFree != NULL) {
        toFree->next = NULL;
        pbuf_free(toFree);
    }
}

static inline int SMapInit(IPAddr *IP, IPAddr *NM, IPAddr *GW, int argc, char *argv[])
{
    if (smap_init(argc, argv) != 0) {
        return 0;
    }
    DEBUG_PRINTF("SMapInit: SMap initialized\n");

    netif_add(&NIF, IP, NM, GW, &NIF, &SMapIFInit, tcpip_input);
    netif_set_default(&NIF);
    netif_set_up(&NIF);
    DEBUG_PRINTF("SMapInit: NetIF added to ps2ip\n");

    // Return 1 (true) to indicate success.

    return 1;
}

static void
PrintIP(struct ip4_addr const *pAddr)
{
    printf("%d.%d.%d.%d", (u8)pAddr->addr, (u8)(pAddr->addr >> 8), (u8)(pAddr->addr >> 16), (u8)(pAddr->addr >> 24));
}

void PS2IPLinkStateUp(void)
{
    tcpip_callback((void *)&netif_set_link_up, &NIF);
}

void PS2IPLinkStateDown(void)
{
    tcpip_callback((void *)&netif_set_link_down, &NIF);
}
#endif

#ifdef BUILDING_SMAP_NETMAN
// While the header of the export table is small, the large size of the export table (as a whole) places it in data instead of sdata.
extern struct irx_export_table _exp_smap __attribute__((section("data")));
#endif

#ifdef BUILDING_SMAP_MODULAR
extern struct irx_export_table _exp_smapmodu;
#endif

int _start(int argc, char *argv[])
{
#ifdef BUILDING_SMAP_PS2IP
    IPAddr IP;
    IPAddr NM;
    IPAddr GW;
    int numArgs;
    char **pArgv;
#endif
#ifdef BUILDING_SMAP_NETMAN
    int result;
#endif

#ifdef BUILDING_SMAP_NETMAN
    if (RegisterLibraryEntries(&_exp_smap) != 0) {
        DEBUG_PRINTF("module already loaded\n");
        return MODULE_NO_RESIDENT_END;
    }
#endif

#ifdef BUILDING_SMAP_MODULAR
    if (RegisterLibraryEntries(&_exp_smapmodu) != 0) {
        DEBUG_PRINTF("module already loaded\n");
        return MODULE_NO_RESIDENT_END;
    }
#endif

    DisplayBanner();

    // This code was present in SMAP, but cannot be implemented with the default IOP kernel due to MODLOAD missing these functions.
    // It may be necessary to prevent SMAP from linking with an old DEV9 module.

    /* if ((ModuleID = SearchModuleByName("dev9")) < 0) {
        sceInetPrintf("dev9 module not found\n");
        return MODULE_NO_RESIDENT_END;
    }
    if (ReferModuleStatus(ModuleID, &ModStatus) < 0) {
        sceInetPrintf("can't get dev9 module status\n");
        return MODULE_NO_RESIDENT_END;
    }

    if (ModStatus.version < 0x204) {
        sceInetPrintf("dev9 module version must be 2.4 or later\n");
        return MODULE_NO_RESIDENT_END;
    } */

#ifdef BUILDING_SMAP_PS2IP
    // Parse IP args.
    DEBUG_PRINTF("argc %d\n", argc);

    if (argc >= 4) {
        DEBUG_PRINTF("%s %s %s\n", argv[1], argv[2], argv[3]);
        IP.addr = inet_addr(argv[1]);
        NM.addr = inet_addr(argv[2]);
        GW.addr = inet_addr(argv[3]);

        numArgs = argc - 4;
        pArgv   = &argv[4];
    } else {
        // Set some defaults.

        IP4_ADDR(&IP, 192, 168, 0, 80);
        IP4_ADDR(&NM, 255, 255, 255, 0);
        IP4_ADDR(&GW, 192, 168, 0, 1);

        numArgs = argc - 1;
        pArgv   = &argv[1];
    }
#endif

#ifdef BUILDING_SMAP_NETMAN
    if ((result = smap_init(argc, argv)) < 0) {
        DEBUG_PRINTF("smap_init -> %d\n", result);
        ReleaseLibraryEntries(&_exp_smap);
        return MODULE_NO_RESIDENT_END;
    }
#endif

#ifdef BUILDING_SMAP_PS2IP
    if (!SMapInit(&IP, &NM, &GW, numArgs, pArgv)) {

        // Something went wrong, return 1 to indicate failure.
        return MODULE_NO_RESIDENT_END;
    }

    printf("Initialized OK, IP: ");
    PrintIP(&IP);
    printf(", NM: ");
    PrintIP(&NM);
    printf(", GW: ");
    PrintIP(&GW);
    printf("\n");

    // Initialized ok.
#endif

    return MODULE_RESIDENT_END;
}
