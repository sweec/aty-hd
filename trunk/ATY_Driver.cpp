#include "ATY_Driver.h"

void MY_DEBUG(const char *fmt, ...) {
 va_list ap;
 va_start(ap, fmt);
 IOLog("ATY_HD: ");
 IOLog(fmt, ap);
}

OSStatus OPENDRVR(DriverGlobal *aDriverRecPtr, void* contents) {
	return noErr;
	/*
	OSStatus ret = noErr;
	MVAD mvad;
	DetailRecord* dRecord;
	CrtcValues *mode;
	UInt32 index;
	
	aDriverRecPtr->currentModeID = GetDefaultMode(aDriverRecPtr);
	//aDriverRecPtr->11622 = aDriverRecPtr->11621;
	//aDriverRecPtr->11623 = aDriverRecPtr->11622;
	aDriverRecPtr->currentDepth = kDepthMode3;
	bool done = false;
	if ((aDriverRecPtr->driverFlags & (1 << 1)) && !done) {
		done = true;
		if (aDriverRecPtr->dispNum == 0) {
			bcopy(aDriverRecPtr->mvad, &mvad, sizeof(MVAD));
			dRecord = aDriverRecPtr->dr;
		} else {
			bcopy(&aDriverRecPtr->mvad[1], &mvad, sizeof(MVAD));
			dRecord = &aDriverRecPtr->dr[1];
		}
		if (HALRestoreDetailTiming(aDriverRecPtr, mvad.modeID, dRecord) != noErr) done = 0;
	}
	if (done) {
		if (mvad.4 != aDriverRecPtr->monPara->0) done = false;
		if (aDriverRecPtr->edidP_lowByte != mvad.edidP_lowByte) done = false;
		if (aDriverRecPtr->driverFlags & (1 << 11)) done = false;	//CRT1?
	}
	if (done) {
		mode = HALFindCrtcValues(aDriverRecPtr, mvad.modeID);
		if (mode != NULL) {
			aDriverRecPtr->currentDepth = mvad.depth;
			aDriverRecPtr->currentModeID = mvad.modeID;
			if (IsDetailMode(aDriverRecPtr, mvad.modeID)) aDriverRecPtr->currentModeID = -5;
		}
	}
	mode = HALFindCrtcValues(aDriverRecPtr, aDriverRecPtr->currentModeID);
	if (mode == NULL) ret = controlErr;
	if (ret == noErr) {
		if (HALGetCrtcValues(aDriverRecPtr, aDriverRecPtr->currentModeID, aDriverRecPtr->currentDepth) == NULL)
			aDriverRecPtr->currentDepth = kDepthMode3;
		aDriverRecPtr->colorDepth = HALPixelSize(aDriverRecPtr, aDriverRecPtr->currentDepth, mode);
		aDriverRecPtr->colorBits = HALColorBits(aDriverRecPtr, aDriverRecPtr->currentDepth, mode);
		aDriverRecPtr->colorFormat = HALColorFormat(aDriverRecPtr, aDriverRecPtr->currentDepth, mode);
		aDriverRecPtr->gammaRecord = NULL;
		aDriverRecPtr->gammaRecord = PoolAllocateResident(sizeof(VDGammaRecord), 1);		//0x60C
		if ((aDriverRecPtr->gammaRecord == NULL) || (aDriverRecPtr->colorDepth == 0)) ret = controlErr;
	}
	aDriverRecPtr->aShare->driverFlags |= (1 << 4);
	some_data var_3C;
	if ((ret == noErr) && (aDriverRecPtr->driverFlags & (1 << 5))) {
		if (aDriverRecPtr->dispNum == 0) {
			ATIDeviceEnabler(var_3C, 5, aDriverRecPtr);
			ATIDeviceEnabler(var_3C, 0xA, aDriverRecPtr);
			index = 1;
		} else index = 8;
		ATIDeviceEnabler(var_3C, index, aDriverRecPtr);
		ATIDeviceEnabler(var_3C, 0x13, aDriverRecPtr);
		kdelay(1);
	}
	if (ret == noErr) {
		CreateLinearGamma(aDriverRecPtr);
		ret = HALSetupMyDisplay(aDriverRecPtr);
		HWProgramCRTCBlank(aDriverRecPtr, 0, aDriverRecPtr->dispNum);
	}
	aDriverRecPtr->driverFlags &= ~(1 << 4);
	if (reg == noErr) aDriverRecPtr->driverFlags |= 0x200000040;	//interrupt, newConnection
	return ret;
	 */
}

OSStatus CLOSEDRVR(DriverGlobal *aDriverRecPtr, void* contents) {
	return unimpErr;
	/*
	if (aDriverRecPtr->gammaRecord != NULL) {
		PoolDeallocate(aDriverRecPtr->gammaRecord);
		aDriverRecPtr->gammaRecord = NULL;
	}
	SetCrsrState(aDriverRecPtr, 0, 0, 0, aDriverRecPtr->dispNum);
	return noErr;
	 */
}

OSStatus CONTROLDRVR(DriverGlobal *aDriverRecPtr, void* contents) {
#ifdef ATY_HD_DEBUG
	const char cmdName[][35] = {
		"cscReset                       ",		//0
		"cscKillIO                      ",
		"cscSetMode                     ",
		"cscSetEntries                  ",
		"cscSetGamma                    ",
		"cscGrayPage                    ",
		"cscGrayScreen                  ",
		"cscSetGray                     ",
		"cscSetInterrupt                ",
		"cscDirectSetEntries            ",
		"cscSetDefaultMode              ",
		"cscSwitchMode                  ",
		"cscSetSync                     ",		//11
		"12                             ",
		"13                             ",
		"14                             ",
		"15                             ",
		"cscSavePreferredConfiguration  ",		//16
		"17                             ",
		"18                             ",
		"19                             ",
		"20                             ",
		"21                             ",
		"cscSetHardwareCursor           ",		//22
		"cscDrawHardwareCursor          ",
		"cscSetConvolution              ",
		"cscSetPowerState               ",
		"cscPrivateControlCall          ",		//26
		"27                             ",
		"cscSetMultiConnect             ",		//28
		"cscSetClutBehavior             ",		//29
		"30                             ",
		"cscSetDetailedTiming           ",		//31
		"32                             ",
		"cscDoCommunication             ",		//33
		"cscProbeConnection             ",		//34
		"35                             ",
		"cscSetScaler                   ",		//36
		"cscSetMirror                   ",
		"cscSetFeatureConfiguration     ",		//38
		"cscUnusedCall                  ",		//127
		"cscSetModeCallback             ",		//128
		"cscSetARMCallback              ",		//132
		"cscSetUnderscan                ",		//226
		"cscSetDispParams               "		//227
	};
#endif
	
	IONDRVControlParameters *pb = (IONDRVControlParameters *) contents;
	
	LOG1("%s	", cmdName[(pb->code < 39)?pb->code:((pb->code == 128)?40:((pb->code == 132)?41:((pb->code == 226)?42:((pb->code == 227)?43:39))))], false);
    switch (pb->code)
    {
        case cscSetEntries:
        case cscSetGamma:
            return noErr;
            break;
			
        default:
            return unimpErr;
            break;
    }
	
	/*
	if (!(aDriverRecPtr->driverFlags & 1)) return noErr;		//display not usable
	if (!(aDriverRecPtr->driverFlags & (1 << 12)) && (pb->code != cscSetPowerState)) return noErr;	//display in sleep
	if (pb->code > 227) return statusErr;						//227: cscSetDisplayParams
	switch (pb->code) {
		case cscReset:
			return DoVideoReset(pb->params, aDriverRecPtr);
		case cscSetMode:
			return DoSetVideoMode(pb->params, aDriverRecPtr);
		case cscSetEntries:
			return DoSetEntries(pb->params, aDriverRecPtr);
		case cscSetGamma:
			return DoSetGamma(pb->params, aDriverRecPtr);
		case cscGrayPage:
			return DoGrayPage(pb->params, aDriverRecPtr);
		case cscSetInterrupt:
			return DoSetInterrupt(pb->params, aDriverRecPtr);
		case cscDirectSetEntries:
			return DoDirectSetEntries(pb->params, aDriverRecPtr);
		case cscSetDefaultMode:
			return DoSetDefaultMode(pb->params, aDriverRecPtr);
		case cscSwitchMode:
			return DoSwitchMode(pb->params, aDriverRecPtr);
		case cscSetSync:
			return DoSetSync(pb->params, aDriverRecPtr);
		case cscSavePreferredConfiguration:
			return DoSaveConfiguration(pb->params, aDriverRecPtr);
		case cscSetHardwareCursor:
			return DoSetHardwareCursor(pb->params, aDriverRecPtr);
		case cscDrawHardwareCursor:
			return DoDrawHardwareCursor(pb->params, aDriverRecPtr);
		case cscSetPowerState:
			return DoSetPowerState(pb->params, aDriverRecPtr);
		case cscSetClutBehavior:
			return DoSetClutBehavior(pb->params, aDriverRecPtr);
		case cscSetDetailedTiming:
			return DoSetDetailTiming(pb->params, aDriverRecPtr);
		case cscDoCommunication:
			return DoATICommunication(pb->params, aDriverRecPtr);
		case cscProbeConnection:
			return DoSetVideoStatus(pb->params, aDriverRecPtr);
		case cscSetScaler:
			return DoSetScaler(pb->params, aDriverRecPtr);
		case cscSetMirror:
			return DoSetMirror(pb->params, aDriverRecPtr);
		case cscSetFeatureConfiguration:
			return DoSetFeatureConfiguratoin(pb->params, aDriverRecPtr);	//original source has the typo error
		case 128:		//this and below are not defined in IOMacOSVideo.h
			return DoATISetModeCallback(pb->params, aDriverRecPtr);
		case 132:
			return DoATISetARMCallback(pb->params, aDriverRecPtr);
		case 226:
			return DoSetUnderscan(pb->params, aDriverRecPtr);
		case 227:
			return DoATISetDispParams(pb->params, aDriverRecPtr);
		default:
			return controlErr;
	}
	 */
}

enum { kIOBootNDRVDisplayMode = 100 };

OSStatus STATUSDRVR(DriverGlobal *aDriverRecPtr, void* contents) {
#ifdef ATY_HD_DEBUG
	const char cmdName[][35] = {
		"0                              ",		//0
		"1                              ",
		"cscGetMode                     ",
		"cscGetEntries                  ",
		"cscGetPageCnt/cscGetPages      ",
		"cscGetPageBase/cscGetBaseAddr  ",
		"cscGetGray                     ",
		"cscGetInterrupt                ",
		"cscGetGamma                    ",
		"cscGetDefaultMode              ",
		"cscGetCurMode                  ",
		"cscGetSync                     ",
		"cscGetConnection               ",
		"cscGetModeTiming               ",
		"cscGetModeBaseAddress          ",
		"cscGetScanProc                 ",
		"cscGetPreferredConfiguration   ",
		"cscGetNextResolution           ",
		"cscGetVideoParameters          ",
		"19                             ",
		"cscGetGammaInfoList            ",
		"cscRetrieveGammaTable          ",
		"cscSupportsHardwareCursor      ",
		"cscGetHardwareCursorDrawState  ",
		"cscGetConvolution              ",
		"cscGetPowerState               ",
		"cscPrivateStatusCall           ",
		"cscGetDDCBlock                 ",
		"cscGetMultiConnect             ",
		"cscGetClutBehavior             ",
		"cscGetTimingRanges             ",
		"cscGetDetailedTiming           ",
		"cscGetCommunicationInfo        ",
		"33                             ",
		"cscGetVideoStatus              ",
		"cscGetScalerInfo               ",
		"cscGetScaler                   ",
		"cscGetMirror                   ",
		"cscGetFeatureConfiguration     ",		//38
		"cscGetFeatureList              ",		//39
		"cscUnusedCall                  ",		//127
		"cscATIGetInfo                  ",		//128
		"cscGetUnderscan                ",		//226
		"cscGetDispParams               ",		//227
	};
#endif

	IONDRVControlParameters* pb = (IONDRVControlParameters*) contents;
	OSStatus ret;

	LOG1("%s	", cmdName[(pb->code < 40)?pb->code:((pb->code == 128)?41:((pb->code == 226)?42:((pb->code == 227)?43:40)))], false);
    switch (pb->code)
    {
        case cscGetCurMode:
		{
			VDSwitchInfoRec * switchInfo = (VDSwitchInfoRec *) (pb->params);
			
			switchInfo->csData     = kIOBootNDRVDisplayMode;
			switchInfo->csMode     = kDepthMode1;
			switchInfo->csPage     = 1;
			switchInfo->csBaseAddr = (Ptr) (1 | (uintptr_t) (aDriverRecPtr->baseAddr));
			ret = noErr;
		}
            break;
			
        case cscGetNextResolution:
		{
			VDResolutionInfoRec * resInfo = (VDResolutionInfoRec *) (pb->params);
			
			if ((kDisplayModeIDFindFirstResolution == (SInt32) resInfo->csPreviousDisplayModeID)
				|| (kDisplayModeIDCurrent == (SInt32) resInfo->csPreviousDisplayModeID))
			{
				resInfo->csDisplayModeID 	= kIOBootNDRVDisplayMode;
				resInfo->csMaxDepthMode		= kDepthMode1;
				resInfo->csHorizontalPixels	= aDriverRecPtr->width;
				resInfo->csVerticalLines	= aDriverRecPtr->height;
				resInfo->csRefreshRate		= 0 << 16;
				ret = noErr;
			}
			else if (kIOBootNDRVDisplayMode == resInfo->csPreviousDisplayModeID)
			{
				resInfo->csDisplayModeID = kDisplayModeIDNoMoreResolutions;
				ret = noErr;
			}
			else
			{
				resInfo->csDisplayModeID = kDisplayModeIDInvalid;
				ret = paramErr;
			}
		}
            break;
			
        case cscGetVideoParameters:
		{
			VDVideoParametersInfoRec * pixelParams = (VDVideoParametersInfoRec *) (pb->params);
			
			if ((kIOBootNDRVDisplayMode != pixelParams->csDisplayModeID)
				|| (kDepthMode1 != pixelParams->csDepthMode))
			{
				ret = paramErr;
				break;
			}
			VPBlock *	pixelInfo = pixelParams->csVPBlockPtr;
			
			pixelInfo->vpBounds.left	= 0;
			pixelInfo->vpBounds.top	= 0;
			pixelInfo->vpBounds.right	= aDriverRecPtr->width;
			pixelInfo->vpBounds.bottom	= aDriverRecPtr->height;
			pixelInfo->vpRowBytes	= aDriverRecPtr->pitch;
			pixelInfo->vpPlaneBytes	= 0;
			pixelInfo->vpPixelSize	= aDriverRecPtr->colorDepth;
			ret = noErr;
		}
            break;
			
        case cscGetModeTiming:
		{
			VDTimingInfoRec * timingInfo = (VDTimingInfoRec *) (pb->params);
			
			if (kIOBootNDRVDisplayMode != timingInfo->csTimingMode)
			{
				ret = paramErr;
				break;
			}
			timingInfo->csTimingFormat = kDeclROMtables;
			timingInfo->csTimingFlags  = kDisplayModeValidFlag | kDisplayModeSafeFlag;
			ret = noErr;
		}
            break;
			
        default:
            ret = unimpErr;
            break;
    }
	
	return ret;
	
	/*
	if (!(aDriverRecPtr->driverFlags & 1)) return noErr;		//display not usable
	if (!(aDriverRecPtr->driverFlags & (1 << 12)) && (pb->code != cscGetPowerState))
		return noErr;											//display in sleep
	if (pb->code > 227) return statusErr;						//227: cscGetDisplayParams
	switch (pb->code) {
		case cscGetMode:
			return DoGetMode(pb->params, aDriverRecPtr);
		case cscGetEntries:
			return DoGetEntries(pb->params, aDriverRecPtr);
		case cscGetPages:
			return DoGetPages(pb->params, aDriverRecPtr);
		case cscGetBaseAddr:
			return DoGetBaseAddr(pb->params, aDriverRecPtr);
		case cscGetInterrupt:
			return DoGetInterrupt(pb->params, aDriverRecPtr);
		case cscGetGamma:
			return DoGetGamma(pb->params, aDriverRecPtr);
		case cscGetDefaultMode:
			return DoGetDefaultMode(pb->params, aDriverRecPtr);
		case cscGetCurMode:
			return DoGetCurrentMode(pb->params, aDriverRecPtr);
		case cscGetSync:
			return DoGetSync(pb->params, aDriverRecPtr);
		case cscGetConnection:
			return DoGetConnection(pb->params, aDriverRecPtr);
		case cscGetModeTiming:
			return DoGetModeTiming(pb->params, aDriverRecPtr);
		case cscGetPreferredConfiguration:
			return DoGetConfiguration(pb->params, aDriverRecPtr);
		case cscGetNextResolution:
			return DoGetNextResolution(pb->params, aDriverRecPtr);
		case cscGetVideoParameters:
			return DoGetVideoParameters(pb->params, aDriverRecPtr);
		case cscSupportsHardwareCursor:
			return DoSupportsHardwareCursor(pb->params, aDriverRecPtr);
		case cscGetHardwareCursorDrawState:
			return DoGetHardwareCursorState(pb->params, aDriverRecPtr);
		case cscGetPowerState:
			return DoGetPowerState(pb->params, aDriverRecPtr);
		case cscGetDDCBlock:
			return DoFetchDDCBlock(pb->params, aDriverRecPtr);
		case cscGetClutBehavior:
			return DoGetClutBehavior(pb->params, aDriverRecPtr);
		case cscGetTimingRanges:
			return DoGetTimingRanges(pb->params, aDriverRecPtr);
		case cscGetDetailedTiming:
			return DoGetDetailTiming(pb->params, aDriverRecPtr);
		case cscGetCommunicationInfo:
			return DoATIGetCommunicationInfo(pb->params, aDriverRecPtr);
		case 34:		//not defined in IOMacOSVideo.h
			return DoGetVideoStatus(pb->params, aDriverRecPtr);
		case cscGetScalerInfo:
			return DoGetScalerInfo(pb->params, aDriverRecPtr);
		case cscGetScaler:
			return DoGetScaler(pb->params, aDriverRecPtr);
		case cscGetMirror:
			return DoGetMirror(pb->params, aDriverRecPtr);
		case cscGetFeatureConfiguration:
			return DoGetFeatureConfiguratoin(pb->params, aDriverRecPtr);	//original source has the typo error
		case 39:		//this and below are not defined in IOMacOSVideo.h
			return DoGetFeatureList(pb->params, aDriverRecPtr);
		case 128:
			return DoATIGetInfo(pb->params, aDriverRecPtr);
		case 226:
			return DoGetUnderscan(pb->params, aDriverRecPtr);
		case 227:
			return DoATIGetDispParams(pb->params, aDriverRecPtr);
		default:
			return statusErr;
	}
	*/
}

OSStatus DoOpenCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	aDriverRecPtr->instCount++;
	if (aDriverRecPtr->instCount == 1) return OPENDRVR(aDriverRecPtr, contents);
	return noErr;
}

OSStatus DoCloseCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	aDriverRecPtr->instCount--;
	if (aDriverRecPtr->instCount == 0) return CLOSEDRVR(aDriverRecPtr, contents);
	return noErr;
}

OSStatus DoKillIOCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	return noErr;
}

OSStatus InternalStart(DriverGlobal *aDriverRecPtr, UInt16 refNum, RegEntryID regID, bool isReplace) {
	PE_Video		bootDisplay;
	UInt32			bpp;
	IODeviceMemory	* mem;
	UInt32          numMaps, i;
	bool            matched = false;
	IOService		*service;

	if (isReplace) return unimpErr;
	if (aDriverRecPtr == NULL) return noBridgeErr;
	if (aDriverRecPtr->aProvider == NULL) return controlErr;
	service = aDriverRecPtr->aProvider;
	if (service == NULL) return controlErr;
	
	IOService::getPlatform()->getConsoleInfo( &bootDisplay);
	numMaps = service->getDeviceMemoryCount();
	for (i = 0; (!matched) && (i < numMaps); i++)
	{
	    mem = service->getDeviceMemoryWithIndex(i);
	    if (!mem) continue;
	    matched = (bootDisplay.v_baseAddr >= mem->getPhysicalAddress())
		&& ((bootDisplay.v_baseAddr < (mem->getPhysicalAddress() + mem->getLength())));
	}
		
	if (matched)
	{
	    aDriverRecPtr->baseAddr	    = (void *) bootDisplay.v_baseAddr;
	    aDriverRecPtr->pitch	    = bootDisplay.v_rowBytes;
	    aDriverRecPtr->width	    = bootDisplay.v_width;
	    aDriverRecPtr->height	    = bootDisplay.v_height;
	    bpp = bootDisplay.v_depth;
	    if (bpp == 15)
			bpp = 16;
	    else if (bpp == 24)
			bpp = 32;
	    aDriverRecPtr->colorDepth = bpp;
		return noErr;
	}
	return controlErr;

	OSStatus ret = noErr;
	if (aDriverRecPtr == NULL) return noBridgeErr;
	REG_COPY(&aDriverRecPtr->regIDNub, &regID);
	aDriverRecPtr->refNum = refNum;
	RegPrint(&aDriverRecPtr->regIDNub, aNdrv_version, a1_5f37, strlen(a1_5f37) + 1);
	RegPrint(&aDriverRecPtr->regIDNub, aMach_o, a1_5f37, strlen(a1_5f37) + 1);
	RegPrint(&aDriverRecPtr->regIDNub, aNdrv_date, aDec142007, strlen(aDec142007) + 1);
	ret = InitPCIDevice(aDriverRecPtr);							//initialize baseIO, baseFB and connectorNum
	if (ret != noErr) return ret;
	
#ifndef ATY_Prionace
	if (RegGet(&aDriverRecPtr->regIDDevice, "ATY,SG", &aDriverRecPtr->aShare, 4) != noErr) {
		aDriverRecPtr->aShare = (ATYSG *)PoolAllocateResident(sizeof(ATYSG), true);
		if (aDriverRecPtr->aShare == NULL) return controlErr;
		RegPrint(&aDriverRecPtr->regIDDevice, "ATY,SG", &aDriverRecPtr->aShare, 4);
	} else if (aDriverRecPtr->aShare->dispNum == 0) return controlErr;
	if (aDriverRecPtr->aShare == NULL) return controlErr;
	aDriverRecPtr->dispNum = aDriverRecPtr->aShare->dispNum;
	aDriverRecPtr->memConfig = &aDriverRecPtr->aShare->memConfig;
	aDriverRecPtr->aShare->nubIDs[aDriverRecPtr->dispNum] = &aDriverRecPtr->regIDNub;
	aDriverRecPtr->aShare->aDriverRecPtrs[aDriverRecPtr->dispNum] = aDriverRecPtr;
	if (aDriverRecPtr->connectorNum < aDriverRecPtr->dispNum) return controlErr;
#else
	aDriverRecPtr->dispNum = 0;
	if (!RegistryEntryIDCompare(&aDriverRecPtr->regIDDevice, &aDriverRecPtr->regIDNub))
		aDriverRecPtr->dispNum = FindDisplayNumber(aDriverRecPtr, &aDriverRecPtr->regIDNub, &aDriverRecPtr->regIDDevice);
	
	if (aDriverRecPtr->connectorNum < aDriverRecPtr->dispNum) return controlErr;
	
	if ((aDriverRecPtr->dispNum == 0)
		|| (RegGet(&aDriverRecPtr->regIDDevice, "my_shared_globals", &aDriverRecPtr->aShare, 4) != noErr)) {
		aDriverRecPtr->aShare = PoolAllocateResident(sizeof(ATYSG), 1);
		if (!aDriverRecPtr->aShare) return controlErr;
		RegPrint(&aDriverRecPtr->regIDDevice, "my_shared_globals", &aDriverRecPtr->aShare, 4);
	}	
	if (aDriverRecPtr->dispNum == 0)
		RegPrint(&aDriverRecPtr->regIDDevice, "my_Id1", &aDriverRecPtr->regIDNub, 16);
	else
		RegPrint(&aDriverRecPtr->regIDDevice, "my_Id2", &aDriverRecPtr->regIDNub, 16);
#endif	
	if (ret == noErr) ret = InitTimingTable(aDriverRecPtr);		//allocate memory for mode related stuff
	if (ret == noErr) ret = InitTVParameters(aDriverRecPtr);	//initialize MVcode
	if (ret == noErr) InitPreferences(aDriverRecPtr);
	if (ret == noErr) ret = InitAddresses(aDriverRecPtr, aDriverRecPtr->dispNum);
	if (ret == noErr) ret = InitHardware(aDriverRecPtr);
	if (ret == noErr) ret = InitConfiguration(aDriverRecPtr);
	if (ret == noErr) ret = InitInterrupts(aDriverRecPtr);
#ifndef ATY_Prionace
	ATIMemory theMemory;
	theMemory.length = aDriverRecPtr->aShare->totalVram;
	theMemory.offset = aDriverRecPtr->aaplVramBase;
	RegPrint(&aDriverRecPtr->regIDNub, "AAPL,vram-memory", &theMemory, 8);
#endif
	if (ret == noErr) {
		aDriverRecPtr->driverFlags |= 1;
#ifndef ATY_Prionace
		aDriverRecPtr->aShare->dispNum++;
#else
		UInt32 data = aDriverRecPtr->dispNum + 1;
		RegPrint(&aDriverRecPtr->regIDDevice, "my_count", &data, 4);
		if (aDriverRecPtr->dispNum != 0) {
			data = 0x73656364;		//"secd"
			RegPrint(&aDriverRecPtr->regIDNub, "my_display", &data, 4);
		} else {
			data = 0x7072696D;		//"prim"
			RegPrint(&aDriverRecPtr->regIDNub, "my_display", &data, 4);
		}
		InitBootOptions(aDriverRecPtr);
#endif	
	}
	return ret;
}

OSStatus InternalEnd(DriverGlobal *aDriverRecPtr, UInt16 refNum, RegEntryID regID, bool isReplace) {
	return unimpErr;
	/*
	if (aDriverRecPtr == NULL) return noErr;
	
	UInt32 data;
	if (aDriverRecPtr->driverFlags & 1) {
		RegGet(&aDriverRecPtr->regIDDevice, "my_count", &data, 4);
		if (data) data--;
		RegPrint(&aDriverRecPtr->regIDDevice, "my_count", &data, 4);
	}
	
	if 	(isReplace) {
		Save_DR drSave;
		drSave.depth = aDriverRecPtr->currentDepth;
		drSave.modeID = aDriverRecPtr->currentModeID;
		drSave.4 = aDriverRecPtr->monPara->0;
		drSave.edidP_lowByte = aDriverRecPtr->edidP_lowByte;
		drSave.3 = 0;
		HALSaveDetailRecord(aDriverRecPtr, &drSave.dr, &drSave, aDriverRecPtr->currentModeID, 0);
		drSave.28 = aDriverRecPtr->4A8;
		drSave.serviceID = aDriverRecPtr->serviceID;
		drSave.20 = aDriverRecPtr->8;
		DoReplacementInfo(aDriverRecPtr, &drSave, 1);
	} else {
		aDriverRecPtr->driverFlags &= ~1;		//disable the driver
		DeviceOutputsOff(aDriverRecPtr);
		HWProgramCRTCOff(aDriverRecPtr, aDriverRecPtr->dispNum);
	}
	
	FreeInterrupts(aDriverRecPtr);
	return noErr;
	 */
}

OSStatus DoInitializeCmd(DriverGlobal *aDriverRecPtr, void *contents) {
	DriverInitInfo *info = (DriverInitInfo *)contents;
	return InternalStart(aDriverRecPtr, info->refNum, info->deviceEntry, false);
}

OSStatus DoReplaceCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	DriverInitInfo *info = (DriverInitInfo *)contents;
	return InternalStart(aDriverRecPtr, info->refNum, info->deviceEntry, true);
}

OSStatus DoFinalizeCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	DriverInitInfo *info = (DriverInitInfo *)contents;
	return InternalEnd(aDriverRecPtr, info->refNum, info->deviceEntry, false); 
}

OSStatus DoSupersededCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	DriverInitInfo *info = (DriverInitInfo *)contents;
	return InternalEnd(aDriverRecPtr, info->refNum, info->deviceEntry, true); 
}

OSStatus DoReadCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	return noErr;
}

OSStatus DoWriteCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	return noErr;
}

OSStatus DoControlCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	return CONTROLDRVR(aDriverRecPtr, contents);
}

OSStatus DoStatusCmd(DriverGlobal *aDriverRecPtr, void* contents) {
	return STATUSDRVR(aDriverRecPtr, contents);
}

OSStatus DoDriverIO(DriverGlobal *aDriverRecPtr, int index, UInt32 cmdID,
			 		void *contents, UInt32 cmdCode, UInt32 cmdKind) {
	OSStatus ret = noErr;
	if (aDriverRecPtr != NULL) aDriverRecPtr->index = index;
	if ((cmdCode - kIONDRVOpenCommand) < 11) {
	  switch (cmdCode) {
		case kIONDRVInitializeCommand:
			return DoInitializeCmd(aDriverRecPtr, contents);
		case kIONDRVFinalizeCommand:
			return DoFinalizeCmd(aDriverRecPtr, contents);
		case kIONDRVReplaceCommand:
			return DoReplaceCmd(aDriverRecPtr, contents);
		case kIONDRVSupersededCommand:
			return DoSupersededCmd(aDriverRecPtr, contents);
		case kIONDRVOpenCommand:
			return DoOpenCmd(aDriverRecPtr, contents);
		case kIONDRVCloseCommand:
			return DoCloseCmd(aDriverRecPtr, contents);
		case kIONDRVKillIOCommand:
			return DoKillIOCmd(aDriverRecPtr, contents);
		case kIONDRVReadCommand:
			ret = DoReadCmd(aDriverRecPtr, contents);
			break;
		case kIONDRVWriteCommand:
			ret = DoWriteCmd(aDriverRecPtr, contents);
			break;
		case kIONDRVControlCommand:
			ret = DoControlCmd(aDriverRecPtr, contents);
			break;
		case kIONDRVStatusCommand:
			ret = DoStatusCmd(aDriverRecPtr, contents);
			break;
		default:
			break;
	  }
	}
	if (!(cmdKind & kIONDRVImmediateIOCommandKind)) ret = IOCommandIsComplete((IOCommandID) cmdID, ret);
	return ret;
}

OSStatus InitPCIDevice(DriverGlobal *aDriverRecPtr) {
	UInt32 vendorID;
	aDriverRecPtr->connectorNum = 0;
	FindParentID(&aDriverRecPtr->regIDNub, &aDriverRecPtr->regIDDevice);
	RegGet(&aDriverRecPtr->regIDDevice, "vendor-id", &vendorID, 4);
	REG_COPY(&aDriverRecPtr->atiRegs[0], &aDriverRecPtr->regIDNub);
	
	if (vendorID == 0x1002) {
		aDriverRecPtr->connectorNum = 1;
		aDriverRecPtr->connectorNum = FindDriverNodes(aDriverRecPtr, &aDriverRecPtr->regIDDevice, 1);
#ifdef ATY_Prionace
		DupProperty(aDriverRecPtr, "vendor-id");
		DupProperty(aDriverRecPtr, "device-id");
		DupProperty(aDriverRecPtr, "revision-id");
		DupProperty(aDriverRecPtr, "interrupts");
		DupProperty(aDriverRecPtr, "driver-ist");
		DupProperty(aDriverRecPtr, "AAPL,address");
		DupProperty(aDriverRecPtr, "AAPL,slot-name");
		DupProperty(aDriverRecPtr, "AAPL,interrupts");
		DupProperty(aDriverRecPtr, "assigned-addresses");
#endif
	} else REG_COPY(&aDriverRecPtr->regIDDevice, &aDriverRecPtr->regIDNub);

#ifndef ATY_Prionace
	ATIPCIRangeIndexAlloc(aDriverRecPtr->aDriver, 0, kIOPCIConfigBaseAddress0);		//aDriver->BAR[0] = 16;FBMap
	ATIPCIRangeIndexAlloc(aDriverRecPtr->aDriver, 3, kIOPCIConfigBaseAddress0);		//aDriver->BAR[3] = 16;
	ATIPCIRangeIndexAlloc(aDriverRecPtr->aDriver, 1, kIOPCIConfigBaseAddress2);		//aDriver->BAR[1] = 24;IOMap
	ATIPCIRangeIndexAlloc(aDriverRecPtr->aDriver, 2, kIOPCIConfigExpansionROMBase);	//aDriver->BAR[2] = 48;
	
	aDriverRecPtr->aaplIOBase = ATIPCIRangeMapGetPhysicalRange(aDriverRecPtr->aDriver, 1, NULL, &aDriverRecPtr->aaplIOMap);
	if ((aDriverRecPtr->aaplIOBase == 0) || (aDriverRecPtr->aaplIOMap == NULL)) return controlErr;
	aDriverRecPtr->aaplVramBase = ATIPCIRangeMapGetPhysicalRange(aDriverRecPtr->aDriver, 0, NULL, &aDriverRecPtr->aaplVramMap);
	if ((aDriverRecPtr->aaplVramBase == 0) || (aDriverRecPtr->aaplVramMap == NULL)) return controlErr;
	aDriverRecPtr->aaplExpRomBase = ATIPCIRangeMapGetPhysicalRange(aDriverRecPtr->aDriver, 2, NULL, &aDriverRecPtr->aaplExpRomMap);
	aDriverRecPtr->baseIOMap = ATIPCIRangeMapAlloc(aDriverRecPtr->aDriver, 1, &aDriverRecPtr->IOPhysBase);		//kIOPCIConfigBaseAddress2
	if ((aDriverRecPtr->baseIOMap == NULL) || (aDriverRecPtr->IOPhysBase == NULL)) return controlErr;
	aDriverRecPtr->ExpRomMapBase = ATIPCIRangeMapAlloc(aDriverRecPtr->aDriver, 2, &aDriverRecPtr->ExpRomPhysBase);//kIOPCIConfigExpansionROMBase
	aDriverRecPtr->subFBLength = 0x20000;	//128kb
	if (aDriverRecPtr->baseFBMap != 0) aDriverRecPtr->currentFBMapBase = aDriverRecPtr->baseFBMap;	//here baseFBMap never initialized, better use Prionace way
	else aDriverRecPtr->currentFBMapBase = ATIMapFrameBufferReserved(aDriverRecPtr->aDriver, 0, aDriverRecPtr->subFBLength);
#else
	aDriverRecPtr->baseFBMap = FindCardAddress(aDriverRecPtr, &aDriverRecPtr->regIDDevice);
	aDriverRecPtr->baseIOMap = FindRegAddress(aDriverRecPtr, &aDriverRecPtr->regIDDevice);
	if ((aDriverRecPtr->baseIOMap == 0) || (aDriverRecPtr->baseFBMap == 0)) return controlErr;
#endif
	aDriverRecPtr->connectorUsableNum = aDriverRecPtr->connectorNum;
	return noErr;
}

UInt32 FindDisplayNumber(DriverGlobal *aDriverRecPtr, RegEntryID *regIDNub, RegEntryID *regIDDevice) {
	UInt32 data;
	OSStatus ret = RegGet(&aDriverRecPtr->regIDNub, "my_display", (void *) &data, 4);
	if (ret == noErr) {
		if (data == 0x73656364) return 1;	//"secd"
		else return 0;
	}
	ret = RegGet(&aDriverRecPtr->regIDDevice, "my_count", (void *) &data, 4);
	if (ret != noErr) return 0;
	if (data > 1) return 0;
	return data;
}

OSStatus FindParentID(RegEntryID* reg, RegEntryID* parent) {
	RegCStrEntryName name;
	Boolean done;
	return RegistryCStrEntryToName(reg, parent, &name, &done);
}

OSStatus RegPrint(RegEntryID* reg, const RegPropertyName* key, const void* value, RegPropertyValueSize size) {
	if (reg == NULL || key == NULL || value == NULL) return paramErr;
	OSStatus ret = RegistryPropertySet(reg, key, value, size);
	if (ret != noErr) RegistryPropertyCreate(reg, key, value, size);
	return ret;
}

OSStatus RegGet(RegEntryID* reg, RegPropertyName* key, void* value, RegPropertyValueSize size) {
	if (reg == NULL || key == NULL || value == NULL) return paramErr;
	return RegistryPropertyGet(reg, key, value, &size);
}

UInt32 FindDriverNodes(DriverGlobal *aDriverRecPtr, RegEntryID* reg, UInt32 level) {
	UInt8 i = 0;					//var_11
	OSStatus ret = noErr;			//var_10
	RegPropertyValueSize size = 9;	//var_34
	RegEntryID atiReg;				//var_28, var_24, var_20, var_1C
	RegEntryIter iter;				//var_2C
	RegEntryIterationOp relationship;
	Boolean done;					//var_2D
	char devType[8];				//var_3D
	
	RegistryEntryIDInit(&atiReg);
	ret = RegistryEntryIterateCreate(&iter);
	if (ret != noErr) return 0;
	ret = RegistryEntryIterateSet(&iter, reg);
	if (ret == noErr) {
		relationship = kRegIterChildren;
		for (i = 0;i <= level;i++) {
			ret = RegistryEntryIterate(&iter, relationship, &atiReg, &done);
			relationship = kRegIterContinue;
			if (done || ret != noErr) { //done means all entries have been searched
				i = (i == 0)?0:(i - 1);
				break;
			}
			if (RegistryPropertyGetSize(&atiReg, "device_type", &size) != noErr) continue;
			if (size > 8) continue;
			devType[0] = 0;
			if (RegGet(&atiReg, "device_type", devType, size) != noErr) continue;
			if (strncmp(devType, "display", strlen("display")) != 0) continue;
			REG_COPY(&aDriverRecPtr->atiRegs[i], &atiReg);
			
			if (i == level) break;
		}
	}
	ret = RegistryEntryIterateDispose(&iter);
	return i;	
}

OSStatus DupProperty(DriverGlobal *aDriverRecPtr, RegPropertyName *name) {
	UInt32 size = 256;
	UInt8 data[256];
	OSStatus ret = RegistryPropertyGet(&aDriverRecPtr->regIDDevice, name, data, &size);
	if (ret != noErr) return qErr;
	ret = RegPrint(&aDriverRecPtr->regIDNub, name, data, size);
	return ret;
}

UInt32 FindCardAddress(DriverGlobal *aDriverRecPtr, RegEntryID *reg) {
	OSStatus ret = readErr;
	UInt32 cardAddr, regAddr;
	ret = GetPCICardBaseAddress(reg, &cardAddr, kIOPCIConfigBaseAddress0, &regAddr);
	if (ret == noErr) return cardAddr;
	return 0;
}

UInt32 FindRegAddress(DriverGlobal *aDriverRecPtr, RegEntryID *reg) {
	OSStatus ret = readErr;
	UInt32 cardAddr, regAddr;
	ret = GetPCICardBaseAddress(reg, &cardAddr, kIOPCIConfigBaseAddress2, &regAddr);
	if (ret == noErr) return regAddr;
	return 0;
}

OSStatus GetPCICardBaseAddress(RegEntryID *reg, UInt32 *cardAddr, UInt32 regNum, UInt32 *regAddr) {
	*cardAddr = 0;
	UInt32 size = 16 * sizeof(IOPCIPhysicalAddress);
	struct IOPCIPhysicalAddress assignedAddresses[16];	//each size 0x14, a pci device has a maximum of 16 addresses
	OSStatus ret = RegistryPropertyGet(reg, "assigned-addresses", (void *)assignedAddresses, &size);
	if ((ret != noErr) || (size == 0)) return readErr;
	UInt16 count = size / sizeof(struct IOPCIPhysicalAddress);
	size = 0x40;
	UInt32 aaplAddress[16];
	ret = RegistryPropertyGet(reg, "AAPL,address", (void *)aaplAddress, &size);
	if ((ret != noErr) || (size == 0)) return readErr;
	UInt32 i;
	for (i = 0;i < count;i++) {
		if (assignedAddresses[i].physHi.s.registerNum != regNum) continue;
		*regAddr = assignedAddresses[i].physLo;
		*cardAddr = aaplAddress[i];
		return noErr;
	}
	return readErr;
}

OSStatus InitTimingTable(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->modesNum = 0;
	aDriverRecPtr->aliasIDsNum = 0;
	aDriverRecPtr->modesIndex = 0;
	aDriverRecPtr->maxModesNum = 0x800;
	aDriverRecPtr->modes = (CrtcValues *)PoolAllocateResident(aDriverRecPtr->maxModesNum * sizeof(CrtcValues), 0);	//0x1C
	if (aDriverRecPtr->modes == NULL) return memFullErr;
	aDriverRecPtr->aliasIDs = (AliasID *)PoolAllocateResident(aDriverRecPtr->maxModesNum * sizeof(AliasID), 0);
	if (aDriverRecPtr->aliasIDs == NULL) return memFullErr;
	aDriverRecPtr->modeFlags = (ModeFlag *)PoolAllocateResident(aDriverRecPtr->maxModesNum * sizeof(ModeFlag), 1);	//1 means zeroed
	if (aDriverRecPtr->modeFlags == NULL) return memFullErr;
	return noErr;
}

OSStatus InitTVParameters(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->MVcode = 0;
	return noErr;
}

void ConnectorsOnOff(DriverGlobal *aDriverRecPtr, UInt32 connectType, bool turnOff) {
	UInt32 i;
	ConnectorInfo *cInfo;
	for(i = 0;i <= 30;i++) {	//connector with 31 types?
		cInfo = FindConnectorInfoByType(aDriverRecPtr, connectType & (1 << i));
		if (cInfo == NULL) continue;
		if (turnOff) {
			aDriverRecPtr->aShare->rgiValue &= ~(cInfo->connectorType & 0xFF);
			aDriverRecPtr->aShare->connectorFlags &= ~(cInfo->connectorFlags & 0xFF);
		} else {
			aDriverRecPtr->aShare->rgiValue |= (cInfo->connectorType & 0xFF);
			aDriverRecPtr->aShare->connectorFlags |= (cInfo->connectorFlags & 0xFF);
		}
	}
}

void InitPreferences(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->atyFlags = 0;
#ifdef ATY_Caretta
	aDriverRecPtr->unknown53 = true;
#endif
#ifdef ATY_Wormy
	aDriverRecPtr->clamClose = true;
#endif
	
	UInt32 data;
	if (RegGet(&aDriverRecPtr->regIDDevice, "no-hotplug-support", &data, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 18);
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,no-hotplug-support", &data, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 18);
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL01,no-hotplug-support", &data, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 18);
#ifdef ATY_Prionace
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,no-hotplug-interrupt", &data, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 18);
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL01,no-hotplug-interrupt", &data, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 18);
#else
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,no-hotplug-interrupt", &data, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 23);
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL01,no-hotplug-interrupt", &data, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 23);
#endif
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,IgnoreConnection", &data, 4) == noErr)
		ConnectorsOnOff(aDriverRecPtr, data, false);
	if (RegGet(&aDriverRecPtr->regIDDevice, "AAPL01,IgnoreConnection", &data, 4) == noErr)
		ConnectorsOnOff(aDriverRecPtr, data, false);
		
#ifndef ATY_Prionace
	 if (aDriverRecPtr->dispNum == 0) CopyProperty(aDriverRecPtr, "AAPL00,display-alias", "AAPL,display-alias");
	 else CopyProperty(aDriverRecPtr, "AAPL01,display-alias", "AAPL,display-alias");
#endif
	
#ifdef ATY_Prionace
	aDriverRecPtr->aShare->unknown11 = 20;
	aDriverRecPtr->aShare->unknown12 = 0;
#endif
#ifdef ATY_Wormy
	aDriverRecPtr->aShare->unknown8 = 5;
	aDriverRecPtr->aShare->unknown9 = 200;
	aDriverRecPtr->aShare->unknown10 = 500;
#endif
	aDriverRecPtr->driverFlags |= (1 << 4);
	if (RegGet(&aDriverRecPtr->regIDDevice, "ATY,Flags", &aDriverRecPtr->atyFlags, 4) == noErr)
		aDriverRecPtr->driverFlags |= (1 << 2);
#ifndef ATY_Caretta
	if (RegGet(&aDriverRecPtr->regIDDevice, "ATY,PlatformInfo", &aDriverRecPtr->atyPlatformInf, sizeof(PlatformInfo)) == noErr)
		InitPlatformInfo(aDriverRecPtr);
#endif
	
	aDriverRecPtr->driverFlags &= ~(1 << 1);
	if (DoNVPrefs(aDriverRecPtr, &aDriverRecPtr->mvad1, false, &data) == noErr) aDriverRecPtr->driverFlags |= (1 << 1);
	if (FactorySetting(aDriverRecPtr)) aDriverRecPtr->driverFlags |= (1 << 11);
	
	InitScalerInfo(aDriverRecPtr);
#ifdef ATY_Prionace
	InitBootOptions(aDriverRecPtr);
#else
	InitSurfaceInfo(aDriverRecPtr);
#endif
	GetDisplayProperties(aDriverRecPtr);
#ifndef ATY_Prionace
	SetUpBacklightProperties(aDriverRecPtr);
	aDriverRecPtr->driverFlags |= (1 << 8);
#endif
}

OSStatus CopyProperty(DriverGlobal *aDriverRecPtr, RegPropertyName *fromName, RegPropertyName *toName) {
	UInt32 size = 256;
	UInt8 data[256];
	OSStatus ret = RegistryPropertyGet(&aDriverRecPtr->regIDDevice, fromName, data, &size);
	if (ret != noErr) return qErr;
	ret = RegPrint(&aDriverRecPtr->regIDNub, toName, data, size);
	return ret;
}

OSStatus DoNVPrefs(DriverGlobal *aDriverRecPtr, MVAD **mvad, bool yesNo, UInt32* data) {
	RegPropertyIter* iter;
	RegPropertyName* name;
	RegPropertyModifiers *modifiers;
	Boolean done; //reach root RegEntryID?
	OSStatus ret = paramErr;
	
	if (data != NULL) *data = 0;
	if(yesNo && ((ret = RegPrint(&aDriverRecPtr->regIDDevice, "MVAD", mvad, sizeof(MVAD))) == noErr)
	   && (RegistryPropertyGetMod(&aDriverRecPtr->regIDDevice, "MVAD", modifiers) == noErr)) {
		RegistryPropertySetMod(&aDriverRecPtr->regIDDevice, "MVAD", *modifiers);
		ret = SaveNVConfigure(aDriverRecPtr);	
	}
	if (yesNo) return ret;
	UInt32 size = sizeof(MVAD);
	ret = RegistryPropertyGet(&aDriverRecPtr->regIDDevice, "MVAD", mvad, &size);
	if (ret == noErr) {
		if (data != NULL) *data = size;
		return ret;
	}
	ret = RegistryPropertyIterateCreate(&aDriverRecPtr->regIDDevice, iter);
	if (ret != noErr) return ret;
  	do {
		ret = RegistryPropertyIterate(iter, name, &done);
		if (ret != noErr) break;
		ret = RegistryPropertyGetMod(&aDriverRecPtr->regIDDevice, name, modifiers);
		if (ret != noErr || !(*modifiers & (1 << 5))) continue;
		RegistryPropertyDelete(&aDriverRecPtr->regIDDevice, name);		
  	} while (!done);
	RegistryPropertyIterateDispose(iter);
	return ret;
}

OSStatus SaveNVConfigure(DriverGlobal *aDriverRecPtr) {
	UInt32 size = sizeof(NVRAMConfiguration);
	NVRAMDisplayParameters nvramParas;
	BlockZero(&nvramParas, sizeof(NVRAMDisplayParameters));
	NVRAMConfiguration nvramConfig;		//the structure need define
	OSStatus ret = RegistryPropertyGet(&aDriverRecPtr->regIDDevice, "saved-config", &nvramConfig, &size);
	if (ret != noErr) BlockZero(&nvramConfig, sizeof(NVRAMConfiguration));
	size = sizeof(MVAD);
	ret = RegistryPropertyGet(&aDriverRecPtr->regIDDevice, "MVAD", &nvramConfig.mvad, &size);
	UInt8 i;
	for (i = 0;i < 8;i++) nvramParas.unknown1[i] = aDriverRecPtr->dps.data[i];
	nvramParas.connectTypeFlags = aDriverRecPtr->connectorFlags;
	nvramParas.unknown2 = aDriverRecPtr->dlcb & 0xFF;
	nvramParas.unknown3 = aDriverRecPtr->dpcb & 0xFF;
	nvramParas.efiDirection = 0;
	UInt32 data;
	ret = RegGet(&aDriverRecPtr->regIDDevice, "ATY,EFIOrientation", &data, 4);
	if (ret == noErr) nvramParas.efiDirection |= data << 24;
	if (aDriverRecPtr->hasDCF == 0) nvramParas.efiDirection |= 1 << 4;
	nvramParas.unknown4 = aDriverRecPtr->unknown39;
	nvramParas.unknown5 = aDriverRecPtr->unknown40;
	nvramParas.unknown6 = aDriverRecPtr->unknown41;
	SaveNVRAMDisplayParameters(aDriverRecPtr, &nvramParas);
	if (aDriverRecPtr->dispNum == 0) memcpy(&nvramConfig.dispPara[0], &nvramParas, sizeof(NVRAMDisplayParameters));
	else memcpy(&nvramConfig.dispPara[1], &nvramParas, sizeof(NVRAMDisplayParameters));
	ret = RegPrint(&aDriverRecPtr->regIDDevice, "saved-config", &nvramConfig, sizeof(NVRAMConfiguration));
	if ((ret == noErr) && (RegistryPropertyGetMod(&aDriverRecPtr->regIDDevice, "saved-config", &data) == noErr))
		RegistryPropertySetMod(&aDriverRecPtr->regIDDevice, "saved-config", data | (1 << 5));
	return ret;
}

void SaveNVRAMDisplayParameters(DriverGlobal *aDriverRecPtr, NVRAMDisplayParameters *nvramParas) {
	UInt8 i;
	nvramParas->name = 'DP01';
	for (i = 0;i < 4;i++) nvramParas->unknown7[i] = aDriverRecPtr->currentDispPara.unknown12[i];
	for (i = 0;i < 3;i++) nvramParas->unknown9[i] = aDriverRecPtr->currentDispPara.unknown4[i];
	nvramParas->unknown10 = aDriverRecPtr->currentDispPara.unknown13;
	nvramParas->unknown8 = aDriverRecPtr->currentDispPara.unknown14;
	nvramParas->unknown11 = aDriverRecPtr->currentDispPara.unknown15;
	nvramParas->unknown12 = aDriverRecPtr->currentDispPara.unknown5;
	nvramParas->unknown13 = aDriverRecPtr->currentDispPara.unknown9;
	nvramParas->unknown14 = aDriverRecPtr->currentDispPara.unknown10;
	nvramParas->unknown15 = aDriverRecPtr->currentDispPara.powerStateAF;
	nvramParas->unknown16 = aDriverRecPtr->currentDispPara.unknown7;
	nvramParas->unknown17 = aDriverRecPtr->currentDispPara.unknown8;
	nvramParas->unknown18 = aDriverRecPtr->currentDispPara.unknown16;
}

bool FactorySetting(DriverGlobal *aDriverRecPtr) {
	RegEntryID *entry;
	FourCharCode value = '0000';
	RegPropertyValueSize size = 4;
	
	if (RegistryCStrEntryLookup(0, "Devices:device-tree:options", entry) == noErr)
		RegistryPropertyGet(entry, "factory-rnin", (void *)&value, &size);
	return (value == 'RNIN');
}

bool DetectHWScaler(DriverGlobal *aDriverRecPtr, UInt8 dispNum) {
	return true;
}

bool DetectSWScaler(DriverGlobal *aDriverRecPtr, UInt8 dispNum) {
	if (RegGet(&aDriverRecPtr->regIDDevice, "ATIFEDSInfo", aDriverRecPtr->swScalerInfo, 4) == noErr) return true;
	aDriverRecPtr->swScalerInfo = NULL;
	return false;
}

bool DetectRotationScaler(DriverGlobal *aDriverRecPtr, UInt8 dispNum) {
	if (!(aDriverRecPtr->driverFlags & (1 << 25))) return false;	//rotation is supported by SWScaler
	return (aDriverRecPtr->swScalerInfo->sFlags[dispNum] & (1 << 1));
}

void InitScalerInfo(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->driverFlags &= ~((1 << 24) | (1 << 25) | (1 << 26)); // disable HWScaler,SWScaler,RotationScaler
	if (DetectHWScaler(aDriverRecPtr, aDriverRecPtr->dispNum))
		aDriverRecPtr->driverFlags |= (1 << 24); //enable HWScaler
	if (DetectSWScaler(aDriverRecPtr, aDriverRecPtr->dispNum))
		aDriverRecPtr->driverFlags |= (1 << 25); //enable SWScaler
	if (DetectRotationScaler(aDriverRecPtr, aDriverRecPtr->dispNum))
		aDriverRecPtr->driverFlags |= (1 << 26); //enable rotationScaler
}

bool InitSurfaceInfo(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->aShare->surfaceInfo.info1 = 0;
	aDriverRecPtr->aShare->surfaceInfo.info2 = 0;
	ATYSurfaceInfo *surfaceInfo;
	if (RegGet(&aDriverRecPtr->regIDDevice, "ATY,SurfInfo", (void *)&surfaceInfo, 4) != noErr) return false;
	aDriverRecPtr->aShare->surfaceInfo.info1 = surfaceInfo->info1;
	aDriverRecPtr->aShare->surfaceInfo.info2 = surfaceInfo->info2;
	return true;
}

void GetDisplayProperties(DriverGlobal *aDriverRecPtr) {
#ifdef ATY_Wormy
	UInt8 *edid = aDriverRecPtr->edid;	//var_10
	bool done = false;
	UInt32 data;
	aDriverRecPtr->hasDCF = false;
	aDriverRecPtr->dpcb = 8;
	aDriverRecPtr->dlcb = 8;
	aDriverRecPtr->cBitIs6 = 0;
	aDriverRecPtr->hasDither = 0;
	aDriverRecPtr->linkType = 0;
	aDriverRecPtr->dualLink = 0;
	aDriverRecPtr->dps.data[0] = 0;
	aDriverRecPtr->dps.data[1] = 1;
	aDriverRecPtr->dps.data[2] = 200;
	aDriverRecPtr->dps.data[3] = 200;
	aDriverRecPtr->dps.data[4] = 1;
	aDriverRecPtr->dps.data[5] = 0;
	aDriverRecPtr->dps.data[6] = 400;
	OSStatus ret = RegGet(&aDriverRecPtr->regIDNub, "display-power-sequence", (void *)&aDriverRecPtr->dps, sizeof(PowerSequence));	//0x30
	if (ret != noErr) {
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,T1", &data, 4);
		if (ret == noErr) aDriverRecPtr->dps.data[0] = data;
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,T2", &data, 4);
		if (ret == noErr) aDriverRecPtr->dps.data[1] = data;
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,T3", &data, 4);
		if (ret == noErr) aDriverRecPtr->dps.data[2] = data;
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,T4", &data, 4);
		if (ret == noErr) aDriverRecPtr->dps.data[3] = data;
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,T5", &data, 4);
		if (ret == noErr) aDriverRecPtr->dps.data[4] = data;
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,T6", &data, 4);
		if (ret == noErr) aDriverRecPtr->dps.data[5] = data;
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,T7", &data, 4);
		if (ret == noErr) aDriverRecPtr->dps.data[6] = data;
	}
	if ((RegGet(&aDriverRecPtr->regIDNub, "display-connect-flags", &data, 4) == noErr) && (data & (1 << 18)))
		aDriverRecPtr->hasDCF = true;
	UInt32 dlcb;
	ret = RegGet(&aDriverRecPtr->regIDNub, "display-link-component-bits", &dlcb, 4);
	if (ret != noErr) {
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,LinkFormat", &data, 4);
		if (ret == noErr) {
			if (data) dlcb = 8;
			else dlcb = 6;
		}
	}
	if (ret == noErr) {
		done = true;
		aDriverRecPtr->dlcb = dlcb;
	}
	UInt32 dpcb;
	ret = RegGet(&aDriverRecPtr->regIDNub, "display-pixel-component-bits", &dpcb, 4);
	if (ret != noErr) {
		ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,PixelFormat", &data, 4);
		if (ret == noErr) {
			if (data) dpcb = 8;
			else dpcb = 6;
		}
	}
	if (ret == noErr) {
		done = true;
		aDriverRecPtr->dpcb = dpcb;
	}
	UInt32 bits;
	if (dlcb && dpcb) bits = (dlcb > dpcb)?dpcb:dlcb;	//pick the smaller one
	else bits = (dlcb > dpcb)?dlcb:dpcb;				//pick the bigger one as the other is 0
	if (bits == 0) bits = 8;
	if (bits == 6) aDriverRecPtr->cBitIs6 = 1;
	UInt32 dds;
	if (aDriverRecPtr->cBitIs6) {
		ret = RegGet(&aDriverRecPtr->regIDNub, "display-dither-support", &dds, 4);
		if (ret != noErr) ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,Dither", &dds, 4);
		if ((ret == noErr) && dds) aDriverRecPtr->hasDither = 1;
	}
	ret = RegGet(&aDriverRecPtr->regIDNub, "display-link-type", &data, 4);
	if (ret != noErr) ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,LinkType", &data, 4);
	if (ret == noErr) {
		aDriverRecPtr->linkType = data & 0xFF;
		done = true;
	}
	data = 0;
	if (RegFind(aDriverRecPtr, "display-dual-link")) {
		ret = noErr;
		data = 1;
	} else ret = paramErr;
	if (ret != noErr) ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,DualLink", &data, 4);
	if (ret == noErr) {
		done = true;
		aDriverRecPtr->dualLink = data & 0xFF;
	}
	if (done) return;
	ConnectorInfo conInfo;
	if (CheckLoadEDIDBlock(aDriverRecPtr, aDriverRecPtr->connection, 1, edid)) CheckLVDSConnection(aDriverRecPtr, &conInfo, 1);
	if (CheckLoadEDIDBlock(aDriverRecPtr, 1 << 6, 1, edid)) return;
	if (edid[0x48]) return;
	if (edid[0x49]) return;
	if (edid[0x4A]) return;
	if (edid[0x4B] != 1) return;
	if (edid[0x4C]) return;
	if (edid[0x4D] != 6) return;
	if (edid[0x4E] != 16) return;
	aDriverRecPtr->linkType = edid[4F] & 0xF;
	aDriverRecPtr->dualLink = 0;
	if (edid[0x4F] & (1 << 4)) aDriverRecPtr->dualLink = 1;
	if (edid[0x50] & 0xF0) return;
	aDriverRecPtr->cBitIs6 = 1;
#endif
}

OSStatus RegFind(DriverGlobal *aDriverRecPtr, RegPropertyName *name) {
	UInt32 size;
	if (name != NULL) return (RegistryPropertyGetSize(&aDriverRecPtr->regIDNub, name, &size) == noErr);
	return paramErr;
}

UInt32 GetConnectionNumber(DriverGlobal *aDriverRecPtr, UInt8 connection) {
	UInt32 i;
	for (i = 0;i < 8;i++) {
		if (connection & 1) return i;
		connection /= 2;
	}
	return 0;
}

void SetUpBacklightProperties(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->invertDefault = 255;
	aDriverRecPtr->unknown49 = 7;
	aDriverRecPtr->invertFreq = 21000;
	aDriverRecPtr->unknown52 = 256;
	aDriverRecPtr->invertCurrent = 0;
	aDriverRecPtr->hasInverter = false;
	UInt32 data;
	OSStatus ret = RegGet(&aDriverRecPtr->regIDNub, "display-inverter", (void *) &data, 4);
	if (ret != noErr) ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,Inverter", (void *) &data, 4);
	if (ret == noErr) {
		if (data == 3) aDriverRecPtr->hasInverter = true;
		if (data == 2) aDriverRecPtr->hasInverter = false;
	}
	ret = RegGet(&aDriverRecPtr->regIDNub, "display-inverter-default-cycle", (void *) &data, 4);
	if (ret != noErr) ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,InverterDefault", (void *) &data, 4);
	if (ret == noErr) aDriverRecPtr->invertDefault = data & 0xFF;
	ret = RegGet(&aDriverRecPtr->regIDNub, "backlight-PWM-freq", (void *) &data, 4);
	if (ret != noErr) ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,InverterFrequency", (void *) &data, 4);
	if (ret == noErr) aDriverRecPtr->invertFreq = data;
	ret = RegGet(&aDriverRecPtr->regIDNub, "inverter-current", (void *) &data, 4);
	if (ret != noErr) ret = RegGet(&aDriverRecPtr->regIDDevice, "AAPL00,InverterCurrent", (void *) &data, 4);
	if (ret == noErr) aDriverRecPtr->invertCurrent = data & 0xFF;
}

//#ifdef ATY_Prionace
void InitBootOptionsCaps(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->bootOptionCaps = 0;
	if (aDriverRecPtr->connectorNum) aDriverRecPtr->bootOptionCaps |= 1;
	if (aDriverRecPtr->driverFlags & (1 << 11)) return;
	aDriverRecPtr->bootOptionCaps |= (1 << 1);
	if (aDriverRecPtr->serviceID == NULL) aDriverRecPtr->bootOptionCaps &= ~(1 << 1);
	aDriverRecPtr->bootOptionCaps |= 0x8101FC;
}

void InitBootOptions(DriverGlobal *aDriverRecPtr) {
	InitBootOptionsCaps(aDriverRecPtr);
	aDriverRecPtr->bootOptions = 0;
	if (aDriverRecPtr->driverFlags & (1 << 1))
		aDriverRecPtr->bootOptions = (aDriverRecPtr->mvad1->unknown4 << 24) | (aDriverRecPtr->mvad2->unknown4 << 16)
		| (aDriverRecPtr->mvad1->unknown1 << 8) | (aDriverRecPtr->mvad2->unknown1);
	aDriverRecPtr->bootOptions &= aDriverRecPtr->bootOptionCaps;
	if (!(aDriverRecPtr->bootOptionCaps & 1)) aDriverRecPtr->bootOptions |= 1;
	if (!(aDriverRecPtr->bootOptionCaps & (1 << 1))) aDriverRecPtr->bootOptions |= (1 << 1);
	if (!(aDriverRecPtr->bootOptionCaps & (1 << 2))) aDriverRecPtr->bootOptions |= (1 << 2);
	if (!(aDriverRecPtr->bootOptionCaps & (1 << 3))) aDriverRecPtr->bootOptions |= (1 << 3);
	if (!(aDriverRecPtr->bootOptionCaps & (1 << 9))) aDriverRecPtr->bootOptions |= (1 << 9);
	aDriverRecPtr->connectorUsableNum = aDriverRecPtr->connectorNum;
	if (aDriverRecPtr->bootOptions & 1) aDriverRecPtr->connectorUsableNum = 0;
	if ((aDriverRecPtr->bootOptions & 0xC0) == 0xC0) aDriverRecPtr->bootOptions &= ~((1 << 6) + (1 << 7));
}
//#endif

UInt32 GetChipID(DriverGlobal *aDriverRecPtr) {
	UInt32 revID = pllr32(aDriverRecPtr, 0x34);
	RegPrint(&aDriverRecPtr->regIDDevice, "ATY,RevID", &revID, 4);
	return revID;
}

OSStatus InitAddresses(DriverGlobal *aDriverRecPtr, UInt8 dispNum) {
	UInt32 data32;	//var_28
	UInt16 data16;	//var_2A
	UInt8 data8;	//var_2B
	UInt32 cas;		//var_24
	UInt32 cras;	//var_20
	UInt32 hpc;		//var_1C
	UInt32 memSize;	//var_44
	UInt32 fbInternalAddr;	//var_14
	UInt32 d1psa;	//var_10
	UInt32 d2psa;	//var_C
	
	ExpMgrConfigReadLong(&aDriverRecPtr->regIDDevice, 0, &data32);
	aDriverRecPtr->chipID = data32;
	ExpMgrConfigReadByte(&aDriverRecPtr->regIDDevice, (void *)8, &(aDriverRecPtr->revID));
	ExpMgrConfigReadWord(&aDriverRecPtr->regIDDevice, (void *)4, &data16);
	ExpMgrConfigWriteWord(&aDriverRecPtr->regIDDevice, (void *)4, data16 | 2);
	ExpMgrConfigReadByte(&aDriverRecPtr->regIDDevice, (void *)0x3D, &data8);
	aDriverRecPtr->chipID = (aDriverRecPtr->chipID >> 16) | (aDriverRecPtr->revID << 24);	//combine devID, revID
	aDriverRecPtr->chipID = GetChipID(aDriverRecPtr);
	cas = regr32(aDriverRecPtr, 0x108);				//CONFIG_APER_SIZE
	cras = regr32(aDriverRecPtr, 0x110) & 0x7FFFF;	//CONFIG_REG_APER_SIZE
	if (regr32(aDriverRecPtr, 0x104) < aDriverRecPtr->aaplVramBase)	aDriverRecPtr->noHDPAperCntl = true;	//CONFIG_APER_1_BASE
	hpc = (1 << 23);
	if (aDriverRecPtr->noHDPAperCntl) {
		memSize = cas;
		hpc &= ~(1 << 23);
	} else {
		memSize = cas * 2;
		hpc |= (1 << 23);
	}
	aDriverRecPtr->aShare->configMemSize = regr32(aDriverRecPtr, 0xF8);	//CONFIG_MEM_SIZE
	aDriverRecPtr->aShare->totalVram = (aDriverRecPtr->aShare->configMemSize > memSize)?memSize:aDriverRecPtr->aShare->configMemSize;
	aDriverRecPtr->totalVram = aDriverRecPtr->aShare->totalVram;
	if (dispNum == 0) {
		regw32(aDriverRecPtr, 0x130, hpc);							//HOST_PATH_CNTL
		regw32(aDriverRecPtr, 0x70, 0x7F0000);						//MC_INDEX
#ifdef ATY_Caretta
		fbInternalAddr = memr32(aDriverRecPtr, 1) & 0xFFFF << 16;	//MC_FB_LOCATION startAddr
#endif
#ifdef ATY_Wormy
		fbInternalAddr = memr32(aDriverRecPtr, 4) & 0xFFFF << 16;	//MC_FB_LOCATION startAddr
#endif
		d1psa = regr32(aDriverRecPtr, 0x6110) - fbInternalAddr;		//D1GRPH_PRIMARY_SURFACE_ADDRESS
		d2psa = regr32(aDriverRecPtr, 0x6910) - fbInternalAddr;		//D2GRPH_PRIMARY_SURFACE_ADDRESS
		memw32(aDriverRecPtr, 1, 0xFFFF0000);
		regw32(aDriverRecPtr, 0x134, 0);							//HDP_FB_LOCATION
		regw32(aDriverRecPtr, 0x6110, d1psa);						//D1GRPH_PRIMARY_SURFACE_ADDRESS
		regw32(aDriverRecPtr, 0x6910, d2psa);						//D2GRPH_PRIMARY_SURFACE_ADDRESS
#ifdef ATY_Caretta
		memw32(aDriverRecPtr, 1, (aDriverRecPtr->aShare->configMemSize - 1) & 0xFFFF0000);	//MC_FB_LOCATION topAddr
#endif
#ifdef ATY_Wormy
		memw32(aDriverRecPtr, 4, (aDriverRecPtr->aShare->configMemSize - 1) & 0xFFFF0000);	//MC_FB_LOCATION topAddr
#endif
	}
	return noErr;
}

void InitOutputs(DriverGlobal *aDriverRecPtr) {
	const UInt8 ConnectionsPriorityLen = 8;
	const UInt8 ConnectionsPriority[8] = {
		1 << 6, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 7, 1 << 1, 1 << 0		//LVDS, DFP1, DFP2, CRT1, CRT2, COMPTV, TV, NONE
	};
	const UInt8 SharedConnectionsLen = 1;
	const UInt8 SharedConnections[1] = {0};
	
	aDriverRecPtr->controlFlags = 0;
	if (aDriverRecPtr->connectorUsableNum != 0) {
		UInt8 i;
		for (i = 0;i < ConnectionsPriorityLen;i++) {
			if (!(ConnectionsPriority[i] & aDriverRecPtr->connectedFlags)) continue;
			aDriverRecPtr->controlFlags = ConnectionsPriority[i];
			break;
		}
		if (aDriverRecPtr->controlFlags == 0)
			for (i = 0;i < 8;i++) {
				if (!(aDriverRecPtr->connectedFlags & (1 << i))) continue;
				aDriverRecPtr->controlFlags = 1 << i;
				break;
			}
#ifdef ATY_Caretta
		if ((aDriverRecPtr->controlFlags & ((1 << 3) | (1 << 4))) && (aDriverRecPtr->connectedFlags & ((1 << 3) | (1 << 4)) == ((1 << 3) | (1 << 4))))
			aDriverRecPtr->controlFlags |= ((1 << 3) | (1 << 4));
#endif
#ifdef ATY_Wormy
		if ((aDriverRecPtr->controlFlags & ((1 << 2) | (1 << 5))) && (aDriverRecPtr->connectedFlags & ((1 << 2) | (1 << 5)) == ((1 << 2) | (1 << 5))))
			aDriverRecPtr->controlFlags |= ((1 << 2) | (1 << 5));
#endif
		for (i = 0;i < SharedConnectionsLen;i++)
			if ((SharedConnections[i] & aDriverRecPtr->controlFlags)) aDriverRecPtr->controlFlags |= SharedConnections[i];
	} else aDriverRecPtr->controlFlags = ~ aDriverRecPtr->controlFlags;
#ifdef ATY_Wormy
	aDriverRecPtr->controlFlags |= (1 << 6);
#endif
	if (aDriverRecPtr->dispNum != 0) aDriverRecPtr->controlFlags = ~ aDriverRecPtr->controlFlags;
	aDriverRecPtr->activeFlags = aDriverRecPtr->connectedFlags & aDriverRecPtr->controlFlags;
#ifdef ATY_Wormy
	if ((aDriverRecPtr->activeFlags & (1 << 6)) && (aDriverRecPtr->activeFlags & (1 << 2))) {	//seems do not support LVDS and DFP1 at the same time
		aDriverRecPtr->activeFlags &= ~(1 << 2);
		aDriverRecPtr->connectedFlags &= ~(1 << 2);
	}
#endif
	if (aDriverRecPtr->activeFlags == 0) aDriverRecPtr->activeFlags = 1 << 0;	//means NONE
	RegPrint(&aDriverRecPtr->regIDNub, "ATY,ActiveFlags", &aDriverRecPtr->activeFlags, 1);
	RegPrint(&aDriverRecPtr->regIDNub, "ATY,ControlFlags", &aDriverRecPtr->controlFlags, 1);
}

void Panel_GetSenseCode(DriverGlobal *aDriverRecPtr, UInt8* connectType, UInt8* connectData) {
	*connectType = 0x90;
	*connectData = 0xFF;
}

void TV_GetSenseCode(DriverGlobal *aDriverRecPtr, UInt8* connectType, UInt8* connectData) {
	*connectType = 0x88;
	*connectData = 0xFF;
}

void GetConnectionSense(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8* connectType, UInt8* connectData) {
	switch (connection) {
#ifdef ATY_Caretta
		case (1 << 3):				//DFP2
			Panel_GetSenseCode(aDriverRecPtr, connectType, connectData);
			break;
		case (1 << 4):				//CRT1
			Apple_GetHWSenseCode(aDriverRecPtr, aDriverRecPtr->connectedFlags, connectType, connectData);
			break;
#endif
#ifdef ATY_Wormy
		case (1 << 1):				//TV
			TV_GetSenseCode(aDriverRecPtr, connectType, connectData);
			break;
		case (1 << 2):				//DFP1
			Panel_GetSenseCode(aDriverRecPtr, connectType, connectData);
			break;
		case (1 << 5):				//CRT2
			Apple_GetHWSenseCode2(aDriverRecPtr, aDriverRecPtr->connectedFlags, connectType, connectData);
			break;
		case (1 << 6):				//LVDS
			Panel_GetSenseCode(aDriverRecPtr, connectType, connectData);
			break;
#endif
		default:
			*connectType = 7;
			*connectData = 0x3F;
			break;
	}
}

MonitorParameter* GetMonitorInfo(DriverGlobal *aDriverRecPtr, UInt8 connection) {
#ifdef ATY_Caretta
	ModeFlag kMonitorVGA[48] = {
		{0x407, 7}, {0x408, 1}, {0x409, 1}, {0x40A, 1}, {0x40B, 1}, {0x410, 1}, {0x411, 1}, {0x412, 1},
		{0x413, 1}, {0x414, 1}, {0x418, 1}, {0x41A, 1}, {0x41B, 1}, {0x41C, 1}, {0x41E, 1}, {0x422, 1},
		{0x439, 1}, {0x423, 1}, {0x43A, 1}, {0x424, 1}, {0x425, 1}, {0x40C, 1}, {0x40D, 1}, {0x40E, 1},
		{0x415, 1}, {0x416, 1}, {0x417, 1}, {0x41F, 1}, {0x420, 1}, {0x421, 1}, {0x426, 1}, {0x427, 1},
		{0x428, 1}, {0x429, 1}, {0x42A, 1}, {0x42B, 1}, {0x42C, 1}, {0x42D, 1}, {0x42E, 1}, {0x430, 1},
		{0x431, 1}, {0x433, 1}, {0x435, 1}, {0x432, 1}, {0x434, 1}, {0x436, 1}, {0x437, 1}, {0x438, 1}
	};
	ModeFlag kNoMonitor[1] = {
		{0x3000, 7}
	};
	const UInt32 MonitorParameterTableLen = 3;
	const MonitorParameter MonitorParameterTable[3] = {
		{0x717, 0xA, 48, kMonitorVGA},
		{0x90FF, 0x2, 0, NULL},
		{0x73F, 0x16, 1, kNoMonitor}
	};
#endif
#ifdef ATY_Wormy
	ModeFlag kMonitorVGA[48] = {
		{0x407, 1}, {0x408, 1}, {0x409, 1}, {0x40A, 1}, {0x40B, 1}, {0x410, 1}, {0x411, 1}, {0x412, 1},
		{0x413, 1}, {0x414, 1}, {0x418, 1}, {0x41A, 1}, {0x41B, 1}, {0x41C, 1}, {0x41E, 1}, {0x422, 1},
		{0x439, 1}, {0x423, 1}, {0x43A, 1}, {0x424, 1}, {0x425, 1}, {0x40C, 1}, {0x40D, 1}, {0x40E, 1},
		{0x415, 1}, {0x416, 1}, {0x417, 1}, {0x41F, 1}, {0x420, 1}, {0x421, 1}, {0x426, 1}, {0x427, 1},
		{0x428, 1}, {0x429, 1}, {0x42A, 1}, {0x42B, 1}, {0x42C, 1}, {0x42D, 1}, {0x42E, 1}, {0x430, 1},
		{0x431, 1}, {0x433, 1}, {0x435, 1}, {0x432, 1}, {0x434, 1}, {0x436, 1}, {0x437, 1}, {0x438, 1}
	};
	ModeFlag kMonitorATITV[15] = {
		{0xC01, 0x45}, {0xC02, 0x41}, {0xC05, 0x41}, {0xC03, 0x41}, {0xC04, 0x41}, {0x1001, 0x41}, {0x1006, 0x41}, {0xC06, 0x41},
		{0xC07, 0x41}, {0x1002, 0x41}, {0x1003, 0x41}, {0x1007, 0x41}, {0x1004, 0x41}, {0x1005, 0x41}, {0x1008, 0x41}
	};
	ModeFlag kNoMonitor[1] = {
		{0x3000, 7}
	};
	const UInt32 MonitorParameterTableLen = 4;
	const MonitorParameter MonitorParameterTable[4] = {
		{0x717, 0xA, 48, kMonitorVGA},
		{0x90FF, 0x2, 0, NULL},
		{0x88FF, 0xB, 15, kMonitorATITV},
		{0x73F, 0x16, 1, kNoMonitor}
	};
#endif
	UInt8 connectType, connectData;
	UInt16 sense;
	GetConnectionSense(aDriverRecPtr, connection, &connectType, &connectData);
	sense = (connectType << 8) & connectData;
	UInt8 i;
	for (i = 0;i < MonitorParameterTableLen;i++)
		if (sense == MonitorParameterTable[i].sense) return (MonitorParameter *)&MonitorParameterTable[i];
	return NULL;
}

void InitDefault(DriverGlobal *aDriverRecPtr) {
	const UInt8 DefaultConnectTableLen = 8;
	const UInt8 DefaultConnectTable[8] = {
		1 << 4, 1 << 5, 1 << 6, 1 << 2, 1 << 3, 1 << 7, 1 << 1, 1 << 0	//CRT1, CRT2, LVDS, DFP1, DFP2, COMPTV, TV, NONE
	};
	const char connectors[8][7] = {
		"NONE", "TV", "LCD", "LCD",
		"CRT", "CRT", "LCD", "COMPTV"
	};
	
	aDriverRecPtr->connection = 1;
	UInt8 i;
	for (i= 0;i < DefaultConnectTableLen;i++) {
		if (!(DefaultConnectTable[i] & aDriverRecPtr->activeFlags)) continue;
		aDriverRecPtr->connection = DefaultConnectTable[i];
		break;
	}
	i = GetConnectionNumber(aDriverRecPtr, aDriverRecPtr->connection);
	RegPrint(&aDriverRecPtr->regIDNub, "display-type", &connectors[i], CStrLen(connectors[i]) + 1);
	aDriverRecPtr->monPara = GetMonitorInfo(aDriverRecPtr, aDriverRecPtr->connection);
	UInt32 edidP;
	GetEDIDProperties(aDriverRecPtr, aDriverRecPtr->connection, &edidP);
	aDriverRecPtr->edidP_lowByte = edidP & 0xFF;
	GetDisplayProperties(aDriverRecPtr);
#ifdef ATY_Wormy
	SetUpBacklightProperties(aDriverRecPtr);
#endif
}

OSStatus HALSetUpDetailTable(DriverGlobal *aDriverRecPtr) {
	UInt32 firstDTModeID = 0x7FB;										//CRT, TV
#ifdef ATY_Caretta
	if (aDriverRecPtr->connection == (1 << 3)) firstDTModeID = 0x5FFB;	//DFP
#endif
#ifdef ATY_Wormy
	if ((aDriverRecPtr->connection == (1 << 2)) || (aDriverRecPtr->connection == (1 << 6)))
		firstDTModeID = 0x5FFB;											//DFP
#endif
	UInt8 i;
	for (i = 0;i < 4;i++) {
		aDriverRecPtr->detailModes[i].modeID = firstDTModeID + i;
		aDriverRecPtr->DTState[i].modeID = firstDTModeID + i;
		aDriverRecPtr->DTState[i].modeSeed = 0;
		aDriverRecPtr->DTState[i].modeState = 2;
		aDriverRecPtr->DTState[i].modeAlias = 0x7FFFFFFF;
		aDriverRecPtr->DTState[i].clock = 0;
		aDriverRecPtr->DTState[i].width = 0;
		aDriverRecPtr->DTState[i].height = 0;
		aDriverRecPtr->DTState[i].horiInset = 0;
		aDriverRecPtr->DTState[i].verInset = 0;
	}
	return noErr;
}

void InitDisplayLineBuffer(DriverGlobal *aDriverRecPtr) {
	UInt32 largeConnect = 0;	//can display width in 5680 - 7680
	UInt32 bigConnect = 0;		//can display width in 3840 - 5680
	
	aDriverRecPtr->displayLineBuffer = 2;
	UInt32 data = aDriverRecPtr->activeFlags & aDriverRecPtr->connectedFlags;
	if ((data == 0) || (data == aDriverRecPtr->connectedFlags)) return;
	aDriverRecPtr->displayLineBuffer = 0;
#ifdef ATY_Caretta
	if (aDriverRecPtr->conInfo.edidP2 & (1 << 11)) largeConnect |= (1 << 3);
	if (aDriverRecPtr->conInfo.edidP2 & (1 << 12)) bigConnect |= (1 << 3);
	if (aDriverRecPtr->conInfo.edidP1 & (1 << 11)) largeConnect |= (1 << 4);
	if (aDriverRecPtr->conInfo.edidP1 & (1 << 12)) bigConnect |= (1 << 4);
#endif
#ifdef ATY_Wormy
	if (aDriverRecPtr->conInfo.edidP2 & (1 << 11)) largeConnect |= (1 << 2);
	if (aDriverRecPtr->conInfo.edidP2 & (1 << 12)) bigConnect |= (1 << 2);
	if (aDriverRecPtr->conInfo.edidP1 & (1 << 11)) largeConnect |= (1 << 5);
	if (aDriverRecPtr->conInfo.edidP1 & (1 << 12)) bigConnect |= (1 << 5);
	if (aDriverRecPtr->conInfo.edidP3 & (1 << 11)) largeConnect |= (1 << 6);
	if (aDriverRecPtr->conInfo.edidP3 & (1 << 12)) bigConnect |= (1 << 6);
#endif
	largeConnect &= aDriverRecPtr->connectedFlags;
	bigConnect &= aDriverRecPtr->connectedFlags;
	if (((bigConnect | largeConnect) & aDriverRecPtr->activeFlags)
		&& (((bigConnect | largeConnect) & aDriverRecPtr->activeFlags) != (bigConnect | largeConnect)))
		largeConnect = 0;
	if (largeConnect == 0) return;
	data = 3840;
	if (((aDriverRecPtr->dispNum == 0) && (aDriverRecPtr->activeFlags & largeConnect))
		|| ((aDriverRecPtr->dispNum != 0) && !(aDriverRecPtr->activeFlags & largeConnect)))
		data = 7680;
	aDriverRecPtr->displayLineBuffer = ((data / 8 * 16) & 0xFFFF7FF0) | 4;
}

void InitMemPartition(DriverGlobal *aDriverRecPtr) {
	ATYVRAM memSize[2];
	
	memSize[1].atyMemSize = 0;
	memSize[1].vramMemSize = 0;
	aDriverRecPtr->totalVram = aDriverRecPtr->aShare->totalVram;
	if (!(aDriverRecPtr->activeFlags & ~1)) aDriverRecPtr->totalVram = 0;
	if (aDriverRecPtr->connectedFlags != aDriverRecPtr->activeFlags) aDriverRecPtr->totalVram /= 2;
	memSize[0].atyMemSize = aDriverRecPtr->totalVram;
	memSize[0].vramMemSize = aDriverRecPtr->aShare->configMemSize;
	RegPrint(&aDriverRecPtr->regIDNub, "ATY,memsize", memSize, 8);
	RegPrint(&aDriverRecPtr->regIDNub, "VRAM,memsize", memSize, 8);
	InitDisplayLineBuffer(aDriverRecPtr);
}

void InitMonitorTables(DriverGlobal *aDriverRecPtr) {
	aDriverRecPtr->modesNum = 0;
	aDriverRecPtr->aliasIDsNum = 0;
	aDriverRecPtr->modesIndex = 0;
#ifdef ATY_Caretta
	HALSetupMonitorTable(aDriverRecPtr, 1 << 3);	//DFP2
	HALSetupMonitorTable(aDriverRecPtr, 1 << 4);	//CRT1
#endif
#ifdef ATY_Wormy
	HALSetupMonitorTable(aDriverRecPtr, 1 << 6);	//LVDS
	HALSetupMonitorTable(aDriverRecPtr, 1 << 2);	//DFP1
	HALSetupMonitorTable(aDriverRecPtr, 1 << 5);	//CRT2
	HALSetupMonitorTable(aDriverRecPtr, 1 << 1);	//TV
#endif
	HALSetupMonitorTable(aDriverRecPtr, 1 << 0);	//NONE
	HALInitSimulscanModes(aDriverRecPtr);
}

const ATYColor aBlackBodyRGBTable[86] = {
	{255, 118, 0, 1600}, {255, 124, 0, 1700}, {255, 130, 0, 1800}, {255, 135, 0, 1900}, {255, 141, 11, 2000},
	{255, 146, 29, 2100}, {255, 152, 41, 2200}, {255, 157, 51, 2300}, {255, 162, 60, 2400}, {255, 166, 69, 2500},
	{255, 170, 77, 2600}, {255, 174, 84, 2700}, {255, 178, 91, 2800}, {255, 182, 98, 2900}, {255, 185, 105, 3000},
	{255, 189, 111, 3100}, {255, 192, 118, 3200}, {255, 195, 124, 3300}, {255, 198, 130, 3400}, {255, 201, 135, 3500},
	{255, 203, 141, 3600}, {255, 206, 146, 3700}, {255, 208, 151, 3800}, {255, 211, 156, 3900}, {255, 213, 161, 4000},
	{255, 215, 166, 4100}, {255, 217, 171, 4200}, {255, 219, 175, 4300}, {255, 221, 180, 4400}, {255, 223, 184, 4500},
	{255, 225, 188, 4600}, {255, 226, 192, 4700}, {255, 228, 196, 4800}, {255, 229, 200, 4900}, {255, 231, 204, 5000},
	{255, 232, 208, 5100}, {255, 234, 211, 5200}, {255, 235, 215, 5300}, {255, 237, 218, 5400}, {255, 238, 222, 5500},
	{255, 239, 225, 5600}, {255, 240, 228, 5700}, {255, 241, 231, 5800}, {255, 243, 234, 5900}, {255, 244, 237, 6000},
	{255, 245, 240, 6100}, {255, 246, 243, 6200}, {255, 247, 245, 6300}, {255, 248, 248, 6400}, {255, 249, 251, 6500},
	{255, 249, 253, 6600}, {254, 250, 255, 6700}, {252, 248, 255, 6800}, {250, 247, 255, 6900}, {247, 245, 255, 7000},
	{245, 244, 255, 7100}, {243, 243, 255, 7200}, {241, 241, 255, 7300}, {239, 240, 255, 7400}, {238, 239, 255, 7500},
	{236, 238, 255, 7600}, {234, 237, 255, 7700}, {233, 236, 255, 7800}, {231, 234, 255, 7900}, {229, 233, 255, 8000},
	{228, 233, 255, 8100}, {227, 232, 255, 8200}, {225, 231, 255, 8300}, {224, 230, 255, 8400}, {223, 229, 255, 8500},
	{221, 228, 255, 8600}, {220, 227, 255, 8700}, {219, 226, 255, 8800}, {218, 226, 255, 8900}, {217, 225, 255, 9000},
	{216, 224, 255, 9100}, {215, 223, 255, 9200}, {214, 223, 255, 9300}, {213, 222, 255, 9400}, {212, 221, 255, 9500},
	{211, 221, 255, 9600}, {210, 220, 255, 9700}, {209, 220, 255, 9800}, {208, 219, 255, 9900}, {207, 218, 255, 10000}, {0, 0, 0, 0}
};

bool FindBlackBodyTemp(UInt32 indexValue, UInt16 *Temp) {
	UInt16 index = indexValue * 84 / 100;
	if (index > 84) return false;
	*Temp = aBlackBodyRGBTable[index].Temp;
	return true;
}

void VerifyDispParams(DriverGlobal *aDriverRecPtr, DisplayParameters* dispPara) {
	ConnectorInfo* conInfo = FindConnectorInfo(aDriverRecPtr, aDriverRecPtr->connectorFlags);
	dispPara->size = sizeof(DisplayParameters);
	dispPara->name = 'DP01';
	dispPara->unknown2 = 0;
	dispPara->connectedFlags = aDriverRecPtr->connectedFlags;
	dispPara->activeFlags = aDriverRecPtr->activeFlags;
	dispPara->controlFlags = aDriverRecPtr->controlFlags;
	dispPara->timingFlags = aDriverRecPtr->connectorFlags;
	dispPara->scaledFlags = aDriverRecPtr->scaledFlags;
	
	if (dispPara->unknown13 > 100) dispPara->unknown13 = 100;
	if (dispPara->unknown4[2] > 100) dispPara->unknown4[2] = 100;
	if (dispPara->unknown15 > 100) dispPara->unknown15 = 100;
	if (dispPara->unknown14 > 30) dispPara->unknown13 = 30;
	if (dispPara->unknown14 < -30) dispPara->unknown13 = -30;
	if (dispPara->unknown6 > 100) dispPara->unknown6 = 100;
	if (dispPara->unknown4[1] > 1) dispPara->unknown4[1] = 1;
	if (dispPara->unknown5 > 6) dispPara->unknown5 = 6;
	if (dispPara->unknown8 > 4) dispPara->unknown8 = 4;
	if (dispPara->unknown16 > 10) dispPara->unknown16 = 10;
	
	if (FindBlackBodyTemp(dispPara->unknown6, &dispPara->unknown21) == false)
		dispPara->unknown6 = aDriverRecPtr->currentDispPara.unknown6;
	FindBlackBodyTemp(dispPara->unknown6, &dispPara->unknown21);
	dispPara->connectorType = 1;
	dispPara->connectorFlags = 0;
	
	if (conInfo != NULL) {
		dispPara->connectorType = conInfo->connectorType;
		dispPara->connectorFlags = conInfo->connectorFlags;
	}
	
	dispPara->model[0] = 0;
	dispPara->slotName[0] = 0;
	RegGet(&aDriverRecPtr->regIDDevice, "model", dispPara->model, 256);
	RegGet(&aDriverRecPtr->regIDDevice, "AAPL,slot-name", dispPara->slotName, 256);
}

void InitDisplayParameters(DriverGlobal *aDriverRecPtr, DisplayParameters* dispPara) {
	const DisplayParameters defaultDP = {
		0x260, 'DP01', 0, 0, 0, 0, 0, 0, 1, 0,
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 50},
		100, 0, 50, 100, 0, 50, 100, 256, 6, 0, 0, 0, 30, 0, 226, 255, 0, 59, 100, 0, 0, 0, 0, 0, 4, 0, 0, 10, 0, 0, 0, 0,
		0,
		0,
	};
	
	DisplayParameters tempDP;
	memcpy(&tempDP, &defaultDP, sizeof(DisplayParameters));
	memcpy(dispPara, &tempDP, sizeof(DisplayParameters));
	VerifyDispParams(aDriverRecPtr, dispPara);
}

OSStatus InitConfiguration(DriverGlobal *aDriverRecPtr) {
	InitConnections(aDriverRecPtr);
	InitOutputs(aDriverRecPtr);
	InitDefault(aDriverRecPtr);
	HALSetUpDetailTable(aDriverRecPtr);
	InitMemPartition(aDriverRecPtr);
	InitMonitorTables(aDriverRecPtr);
	
	if (aDriverRecPtr->modesNum == 0) {
		aDriverRecPtr->activeFlags = 1;
		InitDefault(aDriverRecPtr);
		InitMemPartition(aDriverRecPtr);
		InitMonitorTables(aDriverRecPtr);
	}
	
	if (aDriverRecPtr->modesNum == 0) return controlErr;
	InitDisplayParameters(aDriverRecPtr, &aDriverRecPtr->currentDispPara);
	memcpy(&aDriverRecPtr->previousDispPara, &aDriverRecPtr->currentDispPara, sizeof(DisplayParameters));
	aDriverRecPtr->powerState = 3;
	aDriverRecPtr->powerFlags = 1;
	if (aDriverRecPtr->activeFlags == 0) return qErr;
	return noErr;
}

