/*
	MtapHtlper
	Created By Based_Skid
	
*/

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
	
	Usage: 	mtDetect(<MTAP Port number>) [0,1,2,3]
	
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