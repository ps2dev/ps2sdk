#ifndef __USBHW_H__
#define __USBHW_H__

#include <types.h>
#include "usbd.h"

#define USB_REGBASE 0xBF801600
#define USB_INTR 0x16

#define UNCACHED_SEG(x) ((u32) (x) | 0x20000000)
#define CACHED_SEG(x) ((u32) (x) & 0x0FFFFFFF)

/* OHCI Hardware Registers */
typedef struct _ohci_regs

{
  volatile u32 HcRevision;         /* 0 */
  volatile u32 HcControl;          /* 4 */
  volatile u32 HcCommandStatus;    /* 8 */
  volatile u32 HcInterruptStatus;  /* C */
  volatile u32 HcInterruptEnable;  /* 10 */
  volatile u32 HcInterruptDisable; /* 14 */
  volatile u32 HcHCCA;             /* 18 */
  volatile u32 HcPeriodCurrentED;  /* 1C */
  volatile u32 HcControlHeadED;    /* 20 */
  volatile u32 HcControlCurrentED; /* 24 */
  volatile u32 HcBulkHeadED;       /* 28 */
  volatile u32 HcBulkCurrentED;    /* 2C */
  volatile u32 HcDoneHead;         /* 30 */
  volatile u32 HcFmInterval;       /* 34 */
  volatile u32 HcFmRemaining;      /* 38 */
  volatile u32 HcFmNumber;         /* 3C */
  volatile u32 HcPeriodicStart;    /* 40 */
  volatile u32 HcLSThreshold;      /* 44 */
  volatile u32 HcRHDescriptorA;    /* 48 */
  volatile u32 HcRHDescriptorB;    /* 4C */
  volatile u32 HcRHStatus;         /* 50 */
  volatile u32 HcRHPortStatus[16]; /* Should only be two on PS2 */
} ohci_regs_t;
   
/* Structure for the HCCA area */
typedef struct _ohci_hcca

{
  u32 HccaInterruptTable[32];
  u16 HccaFrameNumber;
  u16 HccaPad1;
  u32 HccaDoneHead;
  u8  reserved[120]; 
  /* Strange that according to docs this isn't 256bytes in size ? */
} ohci_hcca_t __attribute__((packed));

/* General Transfer descriptor (hardware form) */
typedef struct _gt_desc

{
  u32 control;
  void *cbp;
  struct _gt_desc *NextTD;
  void *be;
} gt_desc __attribute__((packed));

/* Isochronous Transfer descriptor (hardware form) */
typedef struct _it_desc

{
  u32 control;
  void *bp0;
  struct _it_desc *NextTD;
  void *be;
  u16 offset[8];
} it_desc __attribute__((packed));

/* Endpoint descriptor (hardware form) */
typedef struct _ep_desc

{
  u32 control;
  void *TailP;
  void *HeadP;
  struct _ep_desc *NextED;
} ep_desc __attribute__((packed));

#define EP_CONTROL_SPD_LO (1 << 13)
#define EP_CONTROL_SPR_HI 0
#define EP_CONTROL_SKIP  (1 << 14)
#define EP_CONTROL_D_IN  (1 << 12)
#define EP_CONTROL_D_OUT (1 << 11)
#define EP_CONTROL_D_TD  (3 << 11)
#define EP_CONTROL_F_ISO (1 << 15)

#define EP_HALT 1
#define EP_CARRY 2

/* ioreq = data transfer */
typedef struct _ioreq

{
   struct _ioreq *next, *prev;
   u32 type;
   struct _ep *ep;
   void *td[4]; /* Should be at most 4 td's per request */
   UsbTransferDoneCallBack cb;
   u32 gp;
   void *priv_data;
} ioreq_t;

/* All endpoints are stored in this type of struct */
/* MUST be aligned to 16bytes */
typedef struct _ep

{
   ep_desc ep;          /* The hw endpoint */
   struct _ep *next;	
   struct _ep *prev;
   u32 pipe_id;         /* When allocated this is the pipe's id value */
   u32 state;
   u32 type; /* Type of endpoint, BULK, CONTROL etc */
   void *td_head; /* Head of transfer descriptors */
} ep_t;

/* All gtds are stored in this structure. MUST be 16 byte aligned */
typedef struct _gtd

{
   gt_desc gt;
   struct _gtd *next;
   struct _gtd *prev;
   u32 state; /* i.e. USED, LINKED etc */
   /* The ioreq this td is associated with */
   ioreq_t *ioreq;
} gtd_t;

typedef struct _itd

{
   it_desc it;
   struct _itd *next;
   struct _itd *prev;
   u32 state;
   ioreq_t *ioreq;
} itd_t;

#define EP_STATE_FREE   0
#define EP_STATE_NEW    1
#define EP_STATE_DEL    2
#define EP_STATE_UNLINK 3
#define EP_STATE_OPER   4
#define EP_STATE_LINKED 5

#define IOREQ_TYPE_ISO  1
#define IOREQ_TYPE_INT  2
#define IOREQ_TYPE_CTL  3
#define IOREQ_TYPE_BLK  4

#define ED_NO_INTRS	32
#define ED_NO_TOTAL	((ED_NO_INTRS * 2) - 1)

#define ED_INT_1MS      0
#define ED_INT_2MS      1
#define ED_INT_4MS      3
#define ED_INT_8MS      7
#define ED_INT_16MS     15
#define ED_INT_32MS     31

/* 0 */
#define OHCI_REV 0xFF

/* 4 */
#define OHCI_CBSR (3 << 0)
#define OHCI_PLE  (1 << 2)
#define OHCI_IE   (1 << 3)
#define OHCI_CLE  (1 << 4)
#define OHCI_BLE  (1 << 5)
#define OHCI_HCFS (3 << 6)
#define OHCI_IR   (1 << 8)
#define OHCI_RWC  (1 << 9)
#define OHCI_RWE  (1 << 10)

#define OHCI_CONTROL_RESET (0 << 6)
#define OHCI_CONTROL_RESUME (1 << 6)
#define OHCI_CONTROL_OPER (2 << 6)
#define OHCI_CONTROL_SUSPEND (3 << 6)

/* 8 */
#define OHCI_HCR  (1 << 0)
#define OHCI_CLF  (1 << 1)
#define OHCI_BLF  (1 << 2)
#define OHCI_OCR  (1 << 3)
#define OHCI_SOC  (3 << 16)

/* C, 10, 14 */
#define OHCI_INT_SO   (1 << 0)
#define OHCI_INT_WDH  (1 << 1)
#define OHCI_INT_SF   (1 << 2)
#define OHCI_INT_RD   (1 << 3)
#define OHCI_INT_UE   (1 << 4)
#define OHCI_INT_FNO  (1 << 5)
#define OHCI_INT_RHSC (1 << 6)
#define OHCI_INT_OC   (1 << 30)
#define OHCI_INT_MIE  (1 << 31)

/* 48 */
#define OHCI_RHA_NDP (255 << 0)
#define OHCI_RHA_PSM (1 << 8)
#define OHCI_RHA_NPS (1 << 9)
#define OHCI_RHA_DT  (1 << 10)
#define OHCI_RHA_OCPM (1 << 11)
#define OHCI_RHA_NOCP (1 << 12)
#define OHCI_RHA_POTPGT (255 << 24)

/* 4c */
#define OHCI_RHS_LPS (1 << 0)
#define OHCI_RHS_OCI (1 << 1)
#define OHCI_RHS_DRWE (1 << 15)
#define OHCI_RHS_LPSC (1 << 16)
#define OHCI_RHS_OCIC (1 << 17)
#define OHCI_RHS_CRWE (1 << 31)

/* 50/54 */
#define OHCI_RHP_CCS (1 << 0)
#define OHCI_RHP_PES (1 << 1)
#define OHCI_RHP_PSS (1 << 2)
#define OHCI_RHP_POCI (1 << 3)
#define OHCI_RHP_RPS (1 << 4)
#define OHCI_RHP_PPS (1 << 8)
#define OHCI_RHP_LSDA (1 << 9)
#define OHCI_RHP_CSC (1 << 16)
#define OHCI_RHP_PESC (1 << 17)
#define OHCI_RHP_PSSC (1 << 18)
#define OHCI_RHP_OCIC (1 << 19)
#define OHCI_RHP_PRSC (1 << 20)

typedef struct _hub_data
{
   /* What devices are on the ports */
   struct _dev_data *ports;
   /* Number of ports */
   int num_ports;
   /* What hub is this attached to. NULL for root */
   struct _hub_data *parent;
   /* What port on the hub is this attached to */
   int hub_port;
} hub_data_t;

typedef struct _pipe
{
   /* Pipe number */
   u32 pipeno;
   /* List of ioreqs for this pipe */
   ioreq_t *ioreqs;
   struct _pipe *next, *prev;
   /* The endpoint descriptor */
   UsbEndpointDescriptor *ed;
} pipe_t;

typedef struct _dev_data
{
   /* USB Device Number */
   u32 devno;
   /* Pointer to the owning LDD. If NULL then nobody wants this device */
   UsbDriver *dev_ldd;
   /* Linked list of devices */
   struct _dev_data *next_dev;
   struct _dev_data *prev_dev;
   u8 *static_desc;
   /* Which hub is this device attached to ? */
   hub_data_t *hub; 
   /* If this is a hub it is set to 0, else set to the 1..n port index */
   int hub_port;    
   /* Speed of device */
   int lowspd;
   /* List of pipes associated with this device */
   pipe_t *pipes;
   /* Private device data */
   void *priv_data;
} dev_data_t;

#endif
