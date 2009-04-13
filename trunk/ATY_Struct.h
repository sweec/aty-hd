#ifndef _ATY_STRUCT_H
#define _ATY_STRUCT_H

#include <IOKit/ndrvsupport/IONDRVFramebuffer.h>
#include "IONDRV.h"
#include "IONDRVLibraries.h"
#include "ATY_String.h"

#define ATY_Caretta
//#define ATY_Wormy
//#define ATY_Prionace

#define ATY_HD_DEBUG

/* __func__ is really nice, but not universal */
#if !defined(__GNUC__) && !defined(C99)
#define __func__ "unknown"
#endif

#ifdef ATY_HD_DEBUG
#define RHDFUNC(ptr) IOLog("ATY_HD: %s\n", __func__)
#define RHDFUNCI(scrnIndex) IOLog("ATY_HD: %s\n", __func__)
#define LOG(fmt, h) do {if ((h)) IOLog("ATY_HD: "); IOLog((fmt));} while (0)
#define LOG1(fmt, a1, h) do {if ((h)) IOLog("ATY_HD: "); IOLog((fmt), (a1));} while (0)
#define LOG2(fmt, a1, a2, h) do {if ((h)) IOLog("ATY_HD: "); IOLog((fmt), (a1), (a2));} while (0)
#define LOG3(fmt, a1, a2, a3, h) do {if ((h)) IOLog("ATY_HD: "); IOLog((fmt), (a1), (a2), (a3));} while (0)
void MY_DEBUG(const char *fmt, ...);	//this seems not working
#else
#define RHDFUNC(ptr)
#define RHDFUNCI(scrnIndex)
#define LOG(fmt)
#define LOG1(fmt, a1)
#define LOG2(fmt, a1, a2)
#define LOG3(fmt, a1, a2, a3)
inline void MY_DEBUG(const char *fmt, ...) {}
#endif

#define REG_COPY(to, from) do {int i; for(i = 0;i < 4;i++) (to)->opaque[i] = (from)->opaque[i];} while (0)
#ifdef __IONDRV__
typedef struct RegEntryID RegEntryID;
typedef struct DriverInitInfo DriverInitInfo;
#endif

#define RADEON_HOST_PATH_CNTL               0x0130
#       define RADEON_HDP_SOFT_RESET        (1 << 26)
#       define RADEON_HDP_APER_CNTL         (1 << 23)

enum {
	noBridgeErr					= -93,
	memFullErr					= -108,
};

typedef struct {
	UInt32				modeID;
	UInt32				mFlag;				//bit2 == 1 means default mode
} ModeFlag;

typedef struct {
	UInt32				minID;
	UInt32				maxID;
	UInt8				connection;
} TimingFlags;

typedef struct {
	UInt32				atyMemSize;
	UInt32				vramMemSize;
} ATYVRAM;

typedef struct {
	IOPhysicalAddress	offset;
	UInt32				length;
} ATIMemory;

typedef struct {
	UInt32				size;				//0
											//4
											//8
	UInt32				totalVram;			//C
	IOVirtualAddress	IOMapBase;			//1C
	IOVirtualAddress	screenOffset;		//24
	UInt32				chipID;				//48
} ATIInfo;

typedef struct  {							//size 0x20
	InterruptMemberNumber member;			//0
	UInt32				intFlags;			//4
	UInt32				dispNum;			//8
	UInt32				mask1;				//C
	UInt32				addr1;				//10
	UInt32				value2;				//14
	UInt32				addr2;				//18
	UInt32				unknown7;			//1C
} ATIInterrupt;

typedef struct {
	UInt32				aliasModeID;		//0
	UInt32				modeID;				//4
} AliasID;

typedef struct {
	UInt32				modeID;				//0
	UInt32				timingMode;			//4
	ushort				mFlag;				//8 0:Horizontal Polarity, 1:Vertical Polarity, 3:interlaced, 4:DFP2
												//6:stretch, 4:onOff, 8&9:rotate90, 9&10:rotate180, 8&10:rotate270
												//2:hSyncACntl, 12:clock > 16500
	ushort				clock;				//A 10000Hz as unit
	ushort				width;				//C
	ushort				HTotal;				//E
	ushort				dHSyncS;			//10
	ushort				dHSyncE;			//12
	ushort				height;				//14
	ushort				VTotal;				//16 525, 625, 750, 1125
	ushort				dVSyncS;			//18
	ushort				dVSyncE;			//1A
} CrtcValues;

/*
typedef struct {
	UInt32				modeID;				//0
	UInt32				timingMode;			//4
	UInt32				mFlag;				//8
	UInt32				clock;				//C 10000Hz as unit
	UInt32				width;				//10
	UInt32				HTotal;				//14
	UInt32				dHSyncS;			//18
	UInt32				dHSyncE;			//1C
	UInt32				height;				//20
	UInt32				VTotal;				//24
	UInt32				dVSyncS;			//28
	UInt32				dVSyncE;			//2C
} CrtcValuesV2;
*/

typedef struct {
	ushort				width;				//0
	ushort				height;				//2
	UInt32				unknown1;			//4
	UInt32				unknown2;			//8
} ScaledParameters;

typedef struct {
	UInt32				modeID;				//0
	UInt32				modeSeed;			//4
	UInt32				modeState;			//8
	UInt32				modeAlias;			//C
	UInt32				clock;				//10
	UInt32				width;				//14
	UInt32				height;				//18
	UInt32				horiInset;			//1C
	UInt32				verInset;			//20
} DetailTimingState;

typedef struct {							//size 0x18
	ushort				clock;				//0
	ushort				width;				//2
	ushort				HTotal;				//4
	ushort				dHSyncS;			//6
	ushort				dHSyncE;			//8
	ushort				height;				//A
	ushort				VTotal;				//C
	ushort				dVSyncS;			//E
	ushort				dVSyncE;			//10
	ushort				width2;				//12
	ushort				height2;			//14
	UInt8				flagHB;				//16
	UInt8				flagLB;				//17
} DetailRecord;

typedef struct {
	ushort				sense;
	UInt8				unknown2;
	UInt32				length;
	ModeFlag			*modeFlag;
} MonitorParameter;

typedef struct {
	UInt32				clock;				//0
	UInt32				maxHFreq;			//4
	UInt32				minHFreq;			//8
	UInt32				maxRefRate;			//C
	UInt32				minRefRate;			//10
	UInt32				maxWidth;			//14
	UInt32				maxHeight;			//18
} monitorLimit;

typedef struct {							//size 0x14 for Caretta, 0x20 for Wormy
	union {
		UInt32			connectorType;		//0
		struct {
			UInt8			dispNum;		//0
			UInt8			connectedFlags;	//1
			UInt8			connectTaggedType1;		//2
			UInt8			connectTaggedType2;		//3
		};
	};
	UInt8				connectTaggedData1;	//4
	UInt8				connectTaggedData2;	//5
	UInt8				unknown3;			//6
	UInt8				unknown4;			//7
	UInt32				edidP1;				//8
	UInt32				edidP2;				//C 16-31:ID Product Code, 0-7:checksum
												//8:D/A 9:GTF 10:APV 11:has mode in 5760-7680 or (1920 and interlaced)
												//12:has mode in 3840-5760 13:has mode 1920 and interlaced 
//#ifdef ATY_Wormy
	UInt32				edidP3;				//10
	UInt32				tvDDC;				//14
	UInt32				unknown8;			//18
//#endif
	UInt8				unknown5;			//10/1C
	UInt8				edidFlags;			//11/1D
	bool				shellClosed;		//12/1E
	bool				shellStateChanged;	//13/1F
} ConnectionInfo;

typedef struct {
	UInt32				connectorType;			//0
	UInt32				connectorFlags;			//4
} ConnectorInfo;

typedef struct {
	UInt8				**features;
	UInt32				size;
} FeatureList;

typedef struct {
	UInt32				array[2];			//0
	UInt32				sFlags[2];			//28
	UInt32				rDegree[2];			//30
} ATIFEDSInfo;

typedef struct {							//size 0x60
	UInt32				unknown1[8];		//0
	UInt8				connectTypeFlags;	//21
	UInt8				unknown2;			//22
	UInt8				unknown3;			//23
	UInt32				efiDirection;		//24
	UInt32				unknown4;			//28
	UInt32				unknown5;			//2C
	UInt32				unknown6;			//30
	FourCharCode		name;				//34
	UInt16				unknown7[4];		//38
	UInt32				unknown8;			//40
	UInt8				unknown9[3];		//44
	UInt8				unknown10;			//47
	UInt8				unknown11;			//48
	UInt8				unknown12;			//49
	UInt8				unknown13;			//4A
	UInt8				unknown14;			//4B
	UInt8				unknown15;			//4C
	UInt8				unknown16;			//4D
	UInt8				unknown17;			//4E
	UInt8				unknown18;			//4F
} NVRAMDisplayParameters;

typedef struct {							//size 0x260
	UInt32				size;				//0
	FourCharCode		name;				//4 = 0x44503031/'DP01'
	UInt32				unknown2;			//8
	UInt8				connectedFlags;		//C
	UInt8				controlFlags;		//10
	UInt8				activeFlags;		//14
	UInt8				timingFlags;		//18
	ushort				scaledFlags;		//1C
	UInt8				connectorType;		//20
	UInt32				connectorFlags;		//24
	UInt16				unknown12[10];		//28
	UInt8				unknown4[3];		//3C
	UInt8				unknown19;			//3F
	UInt8				unknown20;			//40
	UInt8				unknown13;			//41
	UInt8				unknown15;			//44
	UInt8				unknown5;			//47
	SInt8				unknown14;			//4A
	UInt8				unknown6;			//51
	UInt16				unknown21;			//54
	UInt8				unknown7;			//56
	UInt8				unknown8;			//57
	UInt8				unknown16;			//5A
	UInt8				unknown9;			//5D
	UInt8				unknown10;			//5E
	UInt8				powerStateAF;		//5F
	char				model[256];			//60
	char				slotName[256];		//160
} DisplayParameters;

typedef struct {							//size 0x4000
	
} CursorImage;

typedef struct {
	short				gVersion;
	short				gType;
	short				gFormulaSize;		//Formula data size
	short				gChanCnt;			//1 or 3
	short				gDataCnt;			//data count of each table
	short				gDataWidth;			//data bits
	UInt32				gFormulaData[1];	//point to entry of Formula data & Correction data (gamma table)
} GammaTb1;

typedef struct {								//size 0x34
	UInt32				daca_cntl;				//0
	UInt32				unknown1;				//4
	UInt32				unknown2;				//8
	UInt32				unknown3;				//C
	UInt32				dacb_cntl;				//10
	UInt32				unknown4;				//14
	UInt32				unknown5;				//18
	UInt32				unknown6;				//1C
	UInt32				unknown7;				//20
	UInt32				unknown8;				//24
	UInt32				unknown9;				//28
	UInt32				unknown10;				//2C
	UInt32				unknown11;				//30
} ATYSettings;

typedef struct {
	UInt32				unknown3;				// /0
	UInt32				unknown4;				// /4
	UInt32				unknown5;				// /8
	UInt32				unknown6;				// /C
	UInt32				unknown7;				// /10
	UInt32				unknown8;				// /14
	UInt32				unknown9;				// /18
	UInt32				unknown10;				// /1C
	UInt32				unknown11;				// /20
	UInt32				unknown12;				// /24
	UInt32				unknown13;				// /28
	UInt32				unknown14;				// /2C
	UInt32				unknown15;				// /30
	UInt32				unknown16;				// /34
	UInt32				unknown17;				// /38
	
	UInt32				configMemSize;			//0/3C = regr32(0xF8)
	UInt32				agpLocation;			//4
	UInt32				cntl0;					// /40
	UInt32				cntl1;					// /44
	UInt32				rfsh_cntl;				// /48
	UInt32				arb_min;				// /4C
	UInt32				arb_timers;				// /50
	UInt32				unknown33;				// /54
	UInt32				arb_dram_penalties1;	// /58
	UInt32				arb_dram_penalties2;	// /5C
	UInt32				arb_dram_penalties3;	// /60
	UInt32				arb_rdwr_switch;		// /64
	
	UInt32				cmdPadCntl1;			//8
	UInt32				cmdPadCntl2;			//C
	UInt32				unknown20;				//10
	UInt32				unknown21;				//14
	UInt32				unknown22;				//18
	UInt32				unknown23;				//1C
	UInt32				unknown18;				//20
	UInt32				unknown19;				//24
	UInt32				unknown1;				//28
	UInt32				writeAge1;				//2C
	UInt32				writeAge2;				//30/A8
	UInt32				dataPadCntl1;			//34
	UInt32				dataPadCntl2;			//38
	UInt32				agpBase1;				//3C
	UInt32				unknown24;				//40
	UInt32				unknown25;				//44
	UInt32				unknown26;				//48
	UInt32				unknown27;				//4C
	UInt32				unknown28;				//50
	UInt32				unknown29;				//54
	UInt32				unknown30;				//58
	UInt32				unknown31;				//5C
	UInt32				unknown32;				//60
	UInt32				unknown34;				//64
	UInt32				unknown35;				//68
	UInt32				unknown36;				//6C
	UInt32				unknown37;				//70
	UInt32				unknown38;				//74
	UInt32				unknown39;				//78
	UInt32				unknown40;				//7C
	UInt32				unknown41;				//80
	UInt32				seq_rd_ctl2;			//84
	UInt32				seq_wr_ctl1;			//88
	UInt32				seq_wr_ctl2;			//8C
	UInt32				seq_io_ctl1;			//90
	UInt32				unknown42;				//94
	UInt32				unknown43;				//98
	UInt32				unknown44;				//9C
	UInt32				unknown45;				//A0
	UInt32				seq_io_ctl2;			//A4
	UInt32				seq_npl_ctl1;			//A8
	UInt32				seq_npl_ctl2;			//AC
	UInt32				seq_ck_pad_cntl1;		//B0
	
	UInt32				seq_dq_pad_cntl2;		//C4
	UInt32				seq_qs_pad_cntl1;		//C8
	UInt32				seq_qs_pad_cntl2;		//CC
	UInt32				seq_a_pad_cntl2;		//D0
	
	UInt32				unknown46;				//E4
	UInt32				unknown47;				//E8
	UInt32				unknown48;				//EC
	UInt32				unknown49;				//F0
	
	UInt32				i0_pad_cntl_l2;			//104
	UInt32				i0_pad_cntl;			//108
	
	UInt32				i0_rd_cntl_l2;			//114
	UInt32				i0_rd_qs_cntl_l1;		//118
	UInt32				i0_wr_cntl_l2;			//11C
	UInt32				i0_ck_pad_cntl_l1;		//120
	
	UInt32				io_qs_pad_cntl_l2;		//12C
	UInt32				unknown50;				//130
	UInt32				unknown51;				//134
	UInt32				unknown52;				//138
	UInt32				memResetSeqNum;			//13C/198
	UInt32				memResetSequence[128];	//140/19C 0x200
	ATYSettings			atySettings;			//340/39C
	UInt32				l_r600_tc;				// 3CC
} MemoryConfiguration;

typedef struct {								//size 0x40
	short				modeID;					//0
	UInt8				depth;					//2
	UInt8				unknown1;				//3
	UInt8				unknown2;				//4
	UInt8				unknown3;				//5
	UInt8				unknown4;				//6
	UInt8				edidP_lowByte;			//7
} MVAD;

typedef struct {								//size 0x100
	MVAD				mvad;					//0 size 0x40
	NVRAMDisplayParameters dispPara[2];			//40 size 0xC0
} NVRAMConfiguration;

typedef struct {
	ushort				depth;					//0
	short				modeID;					//2
												//8
} ATIConfigurationRec;

typedef struct {
	UInt32				info1;					//0
	UInt32				info2;					//4
} ATYSurfaceInfo;

typedef struct {								//size 0x30
	UInt32				unknown1;				//0
	UInt32				data[7];				//4
	
} PowerSequence;

/*
typedef struct {								//size 0x40
	UInt32				unknown1;				//0
	UInt32				unknown2;				//4
	UInt32				unknown3;				//8
	UInt32				unknown4;				//C
	UInt32				unknown5;				//10
	UInt32				unknown6;				//14
	UInt32				unknown7;				//18
	UInt32				unknown8;				//1C
	UInt32				unknown9;				//20
	UInt32				unknown10;				//24
	UInt32				unknown11;				//28
	UInt32				unknown12;				//2C
	UInt32				unknown13;				//30
	UInt32				unknown14;				//34
	UInt32				unknown15;				//38
	UInt32				unknown16;				//3C
} ReducePower;
*/

typedef struct {
	UInt32				red;					//0
	UInt32				green;					//4
	UInt32				blue;					//8
	UInt32				Temp;					//C
} ATYColor;

typedef struct {								//size 0x80
	UInt8				unknown1;				//0
} PlatformInfo;

typedef struct {
	UInt32				blockNum;				//0
	UInt32				unknown1;				//4
	UInt32				unknown2;				//8
	UInt32				unknown3;				//C
	UInt8				edid[128];
} EdidInfo;

typedef struct {								//size 0x654/0x6B8
	UInt32				dispNum;				//0
	UInt32				unknown13;				//4
	UInt32				configMemSize;			//8 = regr32(0xF8)
	UInt32				totalVram;				//C
	UInt32				configBAR0;				//10
	RegEntryID			*nubIDs[2];				//14
	ConnectionInfo		*conInfo;				//1C
	UInt32				reducePower[9];			//20
	UInt32				powerState[7];			//44
	UInt8				array[2];				//63 = crtcID
	UInt32				array2[2];				//6C, 70
	UInt32				bottomCount;			//80
	UInt32				topCount;				//84
	UInt32				vertCount;				//8C
	UInt32				topCount2;				//90
	UInt32				bottomCount2;			//94
	struct _VSLService	*serviceID[2];			//98
	UInt32				unknown11;				// / /C4
	UInt32				unknown12;				// / /C8
	MemoryConfiguration	memConfig;				//A0
	UInt32				memClock;				//414/470
	UInt32				sysClock;				//418/474
	UInt32				clock1;					//41C/478
	UInt32				clock2;					//420/47C
	UInt32				driverFlags;			//42C/488
	UInt8				connectorFlags;			//430/48C
	UInt8				rgiValue;				//431/48D
	UInt8				crtcID;					//433
	UInt32				unknown1;				//434
	UInt32				unknown2;				//438
	UInt32				unknown3;				//43C
	UInt32				unknown4;				//448
	UInt32				unknown5;				//44C
	UInt32				unknown6;				//450
	AbsoluteTime		someTime2;				// /4B8
	AbsoluteTime		someTime;				// /4C0 size 8
	AbsoluteTime		startTime;				//474/4C8, size 8
	UInt32				unknown15;				// /4D8
	UInt32				unknown8;				// /4DC
	UInt32				unknown9;				// /4E0
	UInt32				unknown10;				// /4E4
	UInt8				shellClosed;			// /4E8
	UInt32				iconColor1[2];			//490
	UInt32				iconColor2[2];			//498
	UInt32				array3[2];				//4A0
	UInt32				array4[2];				//4A8, size 0x44
	UInt32				iconStart[2];			//530
	UInt32				array5[2];				//538
	UInt32				array6[2];				//540
	UInt32				iconSizeX[2];			//548
	UInt32				iconSizeY[2];			//550
	UInt32				array7[2];				//558
	UInt32				array8[2];				//560
	UInt32				array9[2];				//568
	UInt32				array10[2];				//570
	UInt32				array11[2];				//578
	UInt32				array12[2];				//580,584
	UInt8				array13[2];				//588
	UInt8				array14[2];				//58A
	UInt32				cursorPosition;			//628
	UInt8				unknown7;				//62C
	ATYSurfaceInfo		surfaceInfo;			// 64C
	struct DriverGlobal	*aDriverRecPtrs[2];		//63C/6A0
} ATYSG;	//shared global in IORegistry

class ATY_HD;
typedef struct DriverGlobal {					//size 0x11794
	UInt32				instCount;				//0
	UInt32				driverFlags;			//4:	24 HWScaler, 25 SWScaler, 26 RotationScaler, 1 ndrvPrevs initialized
														//0 on/off, 18 noHotPlug, 23 noHPInterrupt, 2 atyFlags available
														//11 CRT1/DDCBlock, 13 HWMirror, 8 ClutBehavior,
														//29 interrupt, 12 wake, 28 something changed or not,
														//16:HWScaler, 15:rotate/swscale, 6:newConnection, 5:interrupt
//#ifdef ATY_Prionace
	UInt32				bootOptions;			//prionace 8
	UInt32				bootOptionCaps;			//prionace C
//#endif
	UInt32				index;					//8 always 0
	RegEntryID			regIDNub;				//C
	RegEntryID			regIDDevice;			//1C
	RegEntryID			atiRegs[2];				//2C, each size 0x10
	UInt16				refNum;					//4C/4C/84
	MVAD				*mvad1;					//4E size 0x40
	UInt8				unknown1;				//50
	UInt32				unknown2;				//52
	MVAD				*mvad2;					//56 size 0x40
	UInt8				unknown3;				//58
	DetailRecord		*dr1;					//5E size 24
	UInt32				unknown4;				//5A
	UInt32				unknown5;				//6A
	DetailRecord		*dr2;					//76 size 24
	UInt32				unknown7;				//90
	UInt8				connectorUsableNum;		//94
	UInt8				connectorNum;			//95
	UInt8				revID;					//96
	UInt32				chipID;					//98 including DevID and VenID
	UInt32				totalVram;				//9C
	bool				noHDPAperCntl;			//A0
	IOPhysicalAddress	aaplIOBase;				//A4 kIOPCIConfigBaseAddress2
	IOPhysicalAddress	aaplExpRomBase;			//A8 kIOPCIConfigExpansionROMBase
	IOPhysicalAddress	aaplVramBase;			//AC kIOPCIConfigBaseAddress0
	IOMemoryMap			*aaplIOMap;				//B0
	IOMemoryMap			*aaplExpRomMap;			//B4
	IOMemoryMap			*aaplVramMap;			//B8
	IOVirtualAddress	baseFBMap;				//BC
	IOVirtualAddress	baseIOMap;				//C4
	IOVirtualAddress	ExpRomMapBase;			//C8
	IOVirtualAddress	screenMapBase;			//CC
	UInt32				screenOffset;			//D0 = 115B0->C - 9C
	IOPhysicalAddress	IOPhysBase;				//D4
	IOPhysicalAddress	ExpRomPhysBase;			//D8
	UInt32				screenSize;				//DC
	void				*baseAddr;				//E0
	IOVirtualAddress	currentFBMapBase;		//E4
	UInt32				subFBLength;			//E8 initialized as 0x20000 and never changed
	MemoryConfiguration	*memConfig;				//EC = 115B0->A0
	UInt8				unknown8;				//F0
	UInt32				unknown9;				//F4
	InterruptServiceIDType serviceID;			//F8
	InterruptMemberNumber member;				//FC
	UInt32				powerState;				//110 these two relate to powerState
	UInt32				powerFlags;				//114
	UInt32				currentModeID;			//118
	UInt32				currentDepth;			//11C default as kDepthMode3 {8, 32}
	UInt32				colorDepth;				//120 bitsPerPixel
	UInt32				colorFormat;			//124 8,16,32:0 64:0,1,2,3
	UInt32				colorBits;				//128 bitsPerColor 8:8 16:5 32:10(ARGB2101010),8(others) 64:16
	UInt32				pitch;					//12C
	UInt32				width;					//130
	UInt32				height;					//134
	UInt32				deltaHOverScan;			//138
	UInt32				deltaVOverScan;			//13C
	CrtcValues			*currentMode;			//140
	UInt32				changeFlags;			//144 0:mode, 1:pitch, 14:screenOffset/Size, 3:driverFlags bit15, 7:->1152C/width
														//6:->1152E/height, 1&4:->4bit15, 4:->11534/11536/->144bit8==1,
														//2:timingFlags, 9:depth, 5:rotate, 8:width/height, 7:->1152C
					 
	MonitorParameter	*monPara;				//148
	UInt32				modesNum;				//14C
	UInt32				modesIndex;				//150
	ModeFlag			*modeFlags;				//154 each size 8
	CrtcValues			*modes;					//158 each size 0x1C
	UInt32				maxModesNum;			//15C
	UInt32				aliasIDsNum;			//160
	AliasID				*aliasIDs;				//164 each size 8
	UInt8				underScan;				//168
	UInt8				DTileMode;				//169
	void				(*DDCSetSense)(struct DriverGlobal *aDriverRecPtr, UInt8 sense);	//16C, ddcSetData/clock callback
	UInt8				(*DDCGetSense)(struct DriverGlobal *aDriverRecPtr);					//170, ddcGetData/clock callback
	VDGammaRecord		*gammaRecord;			//174
	UInt32				colorRange[256];		//178
	UInt8				unknown10;				//430
	UInt8				unknown11;				//431
	UInt32				unknown12;				//578
	UInt32				unknown13;				//57C
	UInt32				ARM[5];					//580-590
	UInt32				unknown55;				//594
	void				(*unknownFunc)(UInt32 unknown1, UInt32 unknown2, UInt32 unknown3);	//598
												//5A4
	CursorImage			crsrImage1;				//5A8  size 0x4000, 64*64 at biginning, crsrImage data
	CursorImage			crsrImage2;				//45A8 size 0x4000, 64*64, crsrImage data
	CursorImage			crsrImage3;				//85A8 size 0x4000, 64*64, crsrImage data
	CursorImage			crsrImage4;				//C5A8 size 0x4000, 64*64, crsrImage data
	UInt32				cursorAddress;			//105A8
	UInt32				cursorX;				//105AC
	UInt32				cursorY;				//105B0
	UInt32				unknown14;				//105B4
	UInt32				unknown15;				//105B8
	Boolean				cursorVisible;			//105BC
	UInt8				cursorMode;				//105C0
	UInt8				unknown16;				//105C1
	Boolean				cursorSet;				//105C2
	UInt32				iconSurfaceAddr;		//105C4
	UInt32				unknown17;				//105D0
	UInt8				unknown18;				//105D4
												//105D8
	UInt8				sync;					//105D5 0:hsync, 1:vSync, 2:both, 7:sync
	RGBColor			rgbData[2];				//105E0 each size 8
	ColorSpec			colorTable[256];		//105F0 each size 8
	DisplayParameters	previousDispPara;		//10DF0 size 0x260
	DisplayParameters	currentDispPara;		//11050/11050 size 0x260
	UInt32				MVcode;					// /112B0/D270
	UInt8				unknown54;				// /112B4
	UInt32				atyFlags;				//112B0/112B8
	PlatformInfo		atyPlatformInf;			// /112BC size 0x80
	bool				unknown53;				//112B4/1133C
	bool				unknown20;				//112B5/1133D
	UInt8				pciXState;				//112B6/1133E
	UInt8				connection;				//112B7/1133F NONE, TV, DFP1, DFP2,	CRT1, CRT2, LVDS, COMP (0-7)
	UInt8				dispNum;				//112B8/11340
	ConnectionInfo		conInfo;				//112BC/11344, size 0x14/0x20
	UInt8				edidP_lowByte;			//112D0/11364
	CrtcValues			*scaledMode;			//112D4
												//112D8
	ushort				scaledFlags;			//112DC/11370 12:dual-link, 
	ushort				currentClock;			//112DE/11372
	ushort				unknown24;				//112E0
												//112E4
	ushort				unknown25;				//112E8
												//112EC
	void				*refCon;				//112F0/11384
	InterruptHandler	handlerFunction;		//112F4/11388
	InterruptEnabler	enableFunction;			//112F8/1138C
	InterruptDisabler	disablerFunction;		//112FC/11390
	InterruptSetMember	setMember;				//11300/11394
	InterruptSetID		setID;					//11308//1139C
	UInt8				activeFlags/*timingFlags*/;//1130C/113A0, 4:CRT>5:CRT>6:LVDS>2:DFP1>3:DFP2>7:COMPTV>1:TV>0:NONE
	UInt8				connectedFlags;			//1130D/113A1
	UInt8				controlFlags;			//1130E/113A2
	UInt8				connectorFlags;			//1130F/113A3
	UInt8				edidFlags;				//11310/113A4 0-7 means corresponding display has edid or not
	DetailTimingState	DTState[4];				//11314/113A8, each size 0x24
	CrtcValues			detailModes[4];			//113A4/11438, each size 1C
	ATIInfo				*atiInfo;				//11414, size 0x4C
	UInt8				edid[128];				//114A8/1153C
	UInt32				mirrorFlags;			//11528
	ScaledParameters	HWscaledPara;			//1152C,1152E
	ushort				nativeWidth;			//11530
	ushort				nativeHeight;			//11532
	ScaledParameters	SWscaledPara;			//11534,11536
	ATIFEDSInfo			*swScalerInfo;			//11538
	UInt32				rotate;					//1153C, 1:90 2:180 3:270
	UInt8				dualLink;				// /115D4
	UInt8				linkType;				// /115D5
	bool				cBitsIs6;				// /115D6
	bool				hasDither;				// /115D7
	bool				clamClose;				// /115D8
	UInt32				displayLineBuffer;		//11540/115DC
	UInt32				unknown44;				// /115E0
	UInt32				dlcb;					//11548/115E4
	UInt32				dpcb;					//1154C/115E8
	UInt8				htidValue;				//11550/115EC
	UInt8				hasDCF;					//11551/115ED
	//UInt8				;						// /115EF
	PowerSequence		dps;					//11554/115F0 size 0x30
	bool				hasInverter;			// /11620
	UInt8				invertDefault;			// /11621
	UInt8				pwrseqState;			// /11623
	UInt8				unknown49;				// /11624
	UInt32				invertFreq;				// /11628
	UInt32				unknown52;				// /1162C
	UInt8				invertCurrent;			// /11630
												//11594
												//1159C
												//115A4
	ATYSG				*aShare;				//115B0/11660 size 0x654
	UInt8				reducePowerIndex;		// /11664
	IOService			*aProvider;				//115B4/11668
	ATY_HD				*aDriver;				//115B8/1166C
	UInt32				videoClock;				//115BC/11670
	UInt32				unknown39;				//115C0/11674
	UInt32				unknown40;				//115C4/11678
	UInt32				unknown41;				//115C8/1167C
	UInt8				unknown36;				//11621
	UInt8				unknown37;				//11622
	UInt8				unknown38;				//11623
	ATIInterrupt		*currentInt;			//11648/116FC
	UInt8				unknown42[13];			//1164C
	UInt32				intFlags[2];			//116CC/11780
	UInt8				unknown43;				//11A06 wommy
} DriverGlobal;

#endif