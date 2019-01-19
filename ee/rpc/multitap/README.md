# libmtap - Playstation 2 multi-tap access library

Copyright (c) 2004 Nicholas Van Veen <nickvv@xtra.co.nz>


## Introduction

libmtap is a simple library which allows you to poll controllers and memorycards connected
through a PS2 multi-tap. libmtap works alongside the standard pad library,
libpad. Steps for polling a controller or memorycard connected to a multi-tap are as
follows:

 1. Load required IRX files 
 2. Init the multi-tap library
 3. Init the pad library
 4. Open the multi-tap port
 5. Check if a multi-tap is connected to the opened port - if not, you may
    close the port (note that a port must be opened in order to determine
    if a multi-tap is connected).
 6. "Open" each controller with padPortOpen(port, slot); (if using multiple controllers)
 7. Use standard libpad functions such as padGetState, padRead to get the state
    of each controller connected to the multi-tap.

```
If You Are Working With The Memory Cards Connected to a Multitap make sure you include libmc in your project.
You Will also need to call mcInit(MC_TYPE_XMC);

You can Optionally Include The iomanX/fileXio IRX Modules If you need them
```

### Ports & Slots


The pad library requires two parameters when using a controller - port, and slot.
Port relates to either of the two ports on the PS2. Slot relates to one of the
4 slots present on a multi-tap. The diagram below should give you a better idea
of this system.

This Diagram is Also The Same For Memorycards. 
However You Will Need to Load The freesio2,MCMAN,MCSERV Modules in addition to the multitap module.



```C
/*
				      _________[ Port 1, Slot 2 ]
                                     /    _____[ Port 1, Slot 3 ]
  |------------|                     |   /
  |            |                   |-------|
  |            |                   | C   D |
  |     PS2    |          |--------|       | <- Multi-tap
  |            |          |        | A   B |
  |            |          |        |-------|
  |            |          |          |   \_____ [ Port 1, Slot 1 ]
  |ConsolePort2|]---------|          \__________[ Port 1, Slot 0 ]
  |            |
  |ConsolePort1|]---[ Port 0, Slot 0 ]
  |            |
  |------------|
/*
```
#### Port Slot Info

```C
/*
 1A: PORT = 0,SLOT = 0
 1B: PORT = 0,SLOT = 1
 1C: PORT = 0,SLOT = 2
 1D: PORT = 0,SLOT = 3
 
 2A: PORT = 1,SLOT = 0
 2B: PORT = 1,SLOT = 1
 2C: PORT = 1,SLOT = 2
 2D: PORT = 1,SLOT = 3
*/
```


#### MTAP Port Info

MTAP Ports Should NOT Be Confused With Controller & MC ports!
Meaning that Mtap Port 2 is Still Memory Card Port 0 (Console Port 1)

```C
// MTAP Controller Ports
mtapPortOpen(0); // MTAP PORT0 = Memory Card Port 0 (Console Port 1)
mtapPortOpen(1); // MTAP PORT1 = Memory Card Port 1 (Console Port 2)
// MTAP Memory Card Ports
mtapPortOpen(2); // MTAP PORT2 = Memory Card Port 0 (Console Port 1)
mtapPortOpen(3); // MTAP PORT3 = Memory Card Port 1 (Console Port 2)
```



### Example Code

Multitap Introduction Page Edited by Based_Skid on 1/19/2019

#### Some More Info Before you Proceed.

ACCORING TO THE Original MULTITAP SAMPLE it Declares that YOU MUST USE XMODULES IN ORDER TO USE THE MULTITAP.

Please Be Advised That You Can Use The Software IRX modules with the Multitap and the Board Secific Xmodules are not Required.

The Software Modules Are Meant to Be Drop in Replacments for The XModules!

See The LoadModule Function in This File and Check the Makefile to See How Software Modules Can be Used in Your Project.


##### A Quick Note about XMODULES 
You Should NEVER Use XMODULES in your Applications
X Modules are Board Specific and Compatibility Across all PS2 Models is Not Guarenteed. 
This Application uses the Software module (freemtap.irx) for the mutltitap module.
All of The Modules Loaded in this Application Are The Software Replacement Modules provided by the ps2sdk


#### 1. Loading the required IRX files
```C
SifExecModuleBuffer(&iomanX, size_iomanX, 0, NULL, NULL); // Optional iomanX

SifExecModuleBuffer(&fileXio, size_fileXio, 0, NULL, NULL); // Optional fileXio
// Init fileXio After Loading it
fileXioInit();

SifExecModuleBuffer(&freesio2, size_freesio2, 0, NULL, NULL); //XSIO2MAN Software Module Replacement 
SifExecModuleBuffer(&mtapman, size_mtapman, 0, NULL, NULL); //XMTAPMAN Software Module Replacement
SifExecModuleBuffer(&freepad, size_freepad, 0, NULL, NULL); //FreePad Software Module
SifExecModuleBuffer(&mcman, size_mcman, 0, NULL, NULL); //XMCMAN Software Module Replacement
SifExecModuleBuffer(&mcserv, size_mcserv, 0, NULL, NULL); //XMCSERV Software Module Replacement.
```

```C
/*
	This Xmodule Code is Shown For Historical Reference. 

	SifLoadModule("rom0:XSIO2MAN", 0, NULL);
	SifLoadModule("rom0:XMTAPMAN", 0, NULL);
	SifLoadModule("rom0:XPADMAN", 0, NULL);
	SifLoadModule("rom0:XMCMAN", 0, NULL);
	SifLoadModule("rom0:XMCSERV", 0, NULL);
*/
```

#### 2. Init

```C
 
    
   //Init LibMC
    mcInit(MC_TYPE_XMC);
    
    // Init libmtap
    mtapInit();

    // Init libpad (or FreePad)
    padInit(0);
```
#### 3. Check connections 


```C

// MTPT,mtP = Multitap Port
// mtRV = Multitap Retured Value
// Global Variable Used to Determine if the mtGO Function has already been used.
int MtapOPEN;

void mtOpenV()
{
    MtapOPEN = 1;                
}
// Closes Multitap Ports Automatically if mtGO() is called when multitap ports are already open in case you need to call mtGO() again.
void mtCloseP()
{
    int MTPT;
    for  (MTPT =0; MTPT<4; MTPT++)
    {
        mtapPortClose(MTPT);
    }
}

// A Simple For Loop To Handle Opening and Closing MultiTap ports. 
void mtGO()
{
    int MTPT;
    
    mtOpenV();
    
    // Close All MTAP Ports Automatically if This Function has Already Been Called
    if(MtapOPEN == 1)
    {
        mtCloseP();
    }
    
    for  (MTPT =0; MTPT<4; MTPT++)
    {
        mtDetect(MTPT);
    }
}


/*
    
    The mtDetect() Function Will Handle Detection of The Multitap and will also handle closing the port if it is not connected,
    
    Usage:     mtDetect(<MTAP Port number>) [0,1,2,3]
    
    Example:mtDetect(1);
        
    WARNING: calling this function without specifying the MTAP Port Number as ang ARG will Crash the PS2!
             Using a Negative Number (-1) or a Number Greater then 4 Will Also Result In a Crash!
    */
void mtDetect(int mtP)
{
    int mtRV;
        
    if (mtP ==0 || mtP ==1 || mtP ==2 || mtP ==3)
    {
        mtapPortOpen(mtP);
        mtRV = mtapGetConnection(mtP);
        if (mtRV == 1)
        {
            printf("Multitap Detected Port %d Opened.\n", mtP);
        }
        else
        {
            printf("Mutitap Not Connected!\n");
            //Close MTAP Port if No Multitap is Connected
            mtapPortClose(mtP);
        }
    }
}
```



#### 4. Open the controllers 

```C
	padPortOpen(0, 0, padBuf1A);
	padPortOpen(0, 1, padBuf1B);
	padPortOpen(0, 2, padBuf1C);
	padPortOpen(0, 3, padBuf1D);
```

### Some More Information

You DONT have to Open Mulitap Ports 1 & 2 if you are just looking to access the Memory Cards. 

Libmc Works with Cards that are connected to the multitap!
 
There are Examples Below to Demonstrate "Port,Slot" Use

See The MC sample for more information about working with memory cards: 
https://github.com/ps2dev/ps2sdk/blob/master/ee/rpc/memorycard/samples/mc_example.c#L78 

You Can Also Refer To This Project For Some More Information:
https://github.com/Based-Skid/Mass-Format-Utility-PS2

```C
//The Calls Below are modified slightly from the example and are to show how the port and slot are passed to calls that expect them.
/*
mcGetInfo(0, 0, &mc_Type, &mc_Free, &mc_Format); << This would Target the 1A Port on the Multitap (port 0, slot 0)
mcGetInfo(0, 1, &mc_Type, &mc_Free, &mc_Format); << 1B Port
mcGetInfo(0, 2, &mc_Type, &mc_Free, &mc_Format); << 1C Port
mcGetInfo(0, 3, &mc_Type, &mc_Free, &mc_Format); << 1D Port
mcGetInfo(1, 0, &mc_Type, &mc_Free, &mc_Format); << 2A Port
mcGetInfo(1, 1, &mc_Type, &mc_Free, &mc_Format); << 2B Port
mcGetInfo(1, 2, &mc_Type, &mc_Free, &mc_Format); << 2C Port
mcGetInfo(1, 3, &mc_Type, &mc_Free, &mc_Format); << 2D Port
*/

