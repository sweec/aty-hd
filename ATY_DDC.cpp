/*
 *  ATY_DDC.cpp
 *  ATY_HD
 *
 *  Created by Dong Luo on 3/11/09.
 *  Copyright 2009 Boston University. All rights reserved.
 *
 */
#include "ATY_Driver.h"
#include "ATY_HW.h"
#include "ATY_DDC.h"

UInt8 COMGetSense(DriverGlobal *aDriverRecPtr, UInt8 ddc) {
	//will dissemble later, this is hardware level
	return 0;
}

void COMSetSense(DriverGlobal *aDriverRecPtr, UInt8 ddc, UInt8 sense) {
	//will dissemble later, this is hardware level
}

UInt8 DDC1GetSense(DriverGlobal *aDriverRecPtr) {
	return COMGetSense(aDriverRecPtr, 1);
}

void DDC1SetSense(DriverGlobal *aDriverRecPtr, UInt8 sense) {
	COMSetSense(aDriverRecPtr, 1, sense);
}

UInt8 DDC3GetSense(DriverGlobal *aDriverRecPtr) {
	return COMGetSense(aDriverRecPtr, 3);
}

void DDC3SetSense(DriverGlobal *aDriverRecPtr, UInt8 sense) {
	COMSetSense(aDriverRecPtr, 3, sense);
}

bool SelectDDCConnectionSenseLines(DriverGlobal *aDriverRecPtr, UInt8 connection) {
#ifdef ATY_Caretta
	if (connection == (1 << 3)) {				//DFP2
		aDriverRecPtr->DDCGetSense = DDC1GetSense;
		aDriverRecPtr->DDCSetSense = DDC1SetSense;
	} else if (connection == (1 << 4)) {		//CRT1
		aDriverRecPtr->DDCGetSense = DDC1GetSense;
		aDriverRecPtr->DDCSetSense = DDC1SetSense;
	} else return false;
#endif
#ifdef ATY_Wormy
	if (connection == (1 << 5)) {				//CRT2
		aDriverRecPtr->DDCGetSense = DDC1GetSense;
		aDriverRecPtr->DDCSetSense = DDC1SetSense;
	} else if (connection == (1 << 6)) {		//LVDS
		aDriverRecPtr->DDCGetSense = DDC3GetSense;
		aDriverRecPtr->DDCSetSense = DDC3SetSense;
	} else if (connection == (1 << 2)) {		//DFP1
		aDriverRecPtr->DDCGetSense = DDC1GetSense;
		aDriverRecPtr->DDCSetSense = DDC1SetSense;
	} else return false;
#endif
	return true;
}

bool EdidDigital(DriverGlobal *aDriverRecPtr, UInt8 *edid) {
	return (edid[0x14] & (1 << 7)); //bit7: Analog = 0, Digital = 1
}

bool EdidTV(DriverGlobal *aDriverRecPtr, UInt8 *edid) {
	if ((edid[0xA] != 8) && (edid[0xA] != 12)) return false;	//0x0A,0B: Vendor assigned code, relate to TV?
	if (edid[0xB] != 0x9D) return false;
	if (edid[8] != 6) return false;	//0x8,9: Manufacturer Name, relate to TV?
	if (edid[9] != 16) return false;
	return true;
}

void ByteToCStr(UInt32 number, RegPropertyName* name) {
	const RegPropertyName h[17] = "0123456789ABCDEF";
	name[0] = h[(number >> 4) & 0xF];
	name[1] = h[(number & 0xF)];
	name[2] = 0;
}

void GetEDIDName(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 ddcBlockNumber, RegPropertyName* name) {
	const RegPropertyName connectorEDID[8][10] = {
		"NONE,EDID", "TV,EDID", "DFP1,EDID", "DFP2,EDID",
		"CRT1,EDID", "CRT2,EDID", "LVDS,EDID", "COMP,EDID"
	};
	CStrCopy(name, connectorEDID[GetConnectionNumber(aDriverRecPtr, connection)]);
	if (ddcBlockNumber <= 1) return;
	ByteToCStr(ddcBlockNumber, name + CStrLen(name)); // add ddcBlockNumber hex value to end of name
}

void SaveEDIDBlock(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 blockNum, UInt8 *edid) {
	RegPropertyName name[256];
	GetEDIDName(aDriverRecPtr, connection, blockNum, name);
	RegPrint(&aDriverRecPtr->regIDDevice, name, edid, 128);
}

void DeleteEDIDBlock(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 blockNum) {
	RegPropertyName name[256];
	
	GetEDIDName(aDriverRecPtr, connection, blockNum, name);
	RegistryPropertyDelete(&aDriverRecPtr->regIDDevice, name);
}

OSStatus CheckLoadEDIDBlock(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 ddcBlockNumber, UInt8* edid) {
	RegPropertyName name[256];
	RegPropertyValueSize size;
	
	GetEDIDName(aDriverRecPtr, connection, ddcBlockNumber, name);
	OSStatus ret = RegistryPropertyGetSize(&aDriverRecPtr->regIDDevice, name, &size);
	if (ret != noErr || edid == NULL || size < 128) return ret;
	return RegGet(&aDriverRecPtr->regIDDevice, name, edid, 128);
}

void DeleteEDIDProperty(DriverGlobal *aDriverRecPtr, UInt8 connection) {
	UInt16 i;
	for (i = 1;i < 256;i++) {
		if (CheckLoadEDIDBlock(aDriverRecPtr, connection, i, NULL) != noErr) break;
		DeleteEDIDBlock(aDriverRecPtr, connection, i);
	}
}

OSStatus ReadOverrideEDIDProperty(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 blockNum, RegPropertyName *name, UInt8 *edid) {
	RegPropertyName newName[256];
	OSStatus ret = paramErr;
	UInt32 size;
	RegEntryID* regID;
	ConnectorInfo* conInfo;
	RegPropertyName *theName;
	int connectorType = -1;
	
	conInfo = FindConnectorInfo(aDriverRecPtr, connection);
	theName = name;
	UInt8 i, j;
	for (i = 0;aDriverRecPtr->connectorNum >= i;i++) {
		regID = &aDriverRecPtr->atiRegs[i];
		ret = RegGet(regID, "connector-type", &connectorType, 4);
		if ((ret != noErr) || (conInfo == NULL) || (conInfo->connectorType != connectorType)) {
#ifdef ATY_Caretta
			if (!(connection & (1 << 4)) || ((ret == noErr) && (conInfo != NULL))) continue;		//not CRT1
#endif
#ifdef ATY_Wormy
			if (!(connection & (1 << 5))) || ((ret == noErr) && (conInfo != NULL))) continue;		//not CRT2
#endif
		}
		ret = RegistryPropertyGetSize(regID, theName, &size);
		if (ret != noErr) {
			regID = &aDriverRecPtr->regIDDevice;
			theName = newName;
			snprintf(newName, 8, "AAPL%02X,", i);
			strncat(newName, name, strlen(name) + 7);
			ret = RegistryPropertyGetSize(regID, theName, &size);
		}
		if (ret != noErr) return paramErr;
		if ((blockNum > 1) && ((size / 128) < blockNum)) return paramErr;
		UInt8 *data = (UInt8 *) PoolAllocateResident(size, 1);
		if (data == NULL) return paramErr;
		ret = RegGet(regID, theName, (void *) data, size);
		if (ret == noErr) {
			for (j = 0;j < 128;j++) edid[j] = data[(blockNum - 1) * 128 + j];
			aDriverRecPtr->edidFlags |= connection;
		}
		PoolDeallocate(data);
		return ret;
	}
	return ret;
}

UInt8 ddc_io;

void DDCSetClock(DriverGlobal *aDriverRecPtr, UInt8 value) {
	UInt8 sense = ddc_io;
	sense |= (1 << 4);
	if (value) sense |= (1 << 1);
	else sense &= ~(1 << 1);
	aDriverRecPtr->DDCSetSense(aDriverRecPtr, sense);
	kdelay(-5);
	ddc_io = sense;
}

UInt8 DDCGetClock(DriverGlobal *aDriverRecPtr) {
	return (aDriverRecPtr->DDCGetSense(aDriverRecPtr) >> 1 & 1);
}

void DDCFreeClock(DriverGlobal *aDriverRecPtr) {
	UInt8 sense = ddc_io;
	sense &= ~(1 << 4);
	aDriverRecPtr->DDCSetSense(aDriverRecPtr, sense);
	kdelay(-5);
	ddc_io = sense;
}

UInt8 DDCWaitClockHigh(DriverGlobal *aDriverRecPtr) {
	DDCSetClock(aDriverRecPtr, 1);
	UInt16 i;
	for (i = 0;i < 500;i++) {
		if (DDCGetClock(aDriverRecPtr)) break;
		kdelay(-10);
	}
	return DDCGetClock(aDriverRecPtr);
}

void DDCSetData(DriverGlobal *aDriverRecPtr, UInt8 value) {
	UInt8 sense = ddc_io;
	sense |= (1 << 5);
	if (value) sense |= (1 << 2);
	else sense &= ~(1 << 5);
	aDriverRecPtr->DDCSetSense(aDriverRecPtr, sense);
	kdelay(-5);
	ddc_io = sense;
}

UInt8 DDCGetData(DriverGlobal *aDriverRecPtr) {
	return (aDriverRecPtr->DDCGetSense(aDriverRecPtr) >> 2 & 1);
}

void DDCFreeData(DriverGlobal *aDriverRecPtr) {
	UInt8 sense = ddc_io;
	sense &= ~(1 << 5);
	aDriverRecPtr->DDCSetSense(aDriverRecPtr, sense);
	kdelay(-5);
	ddc_io = sense;
}

UInt8 DDCReceiveBit(DriverGlobal *aDriverRecPtr) {
	DDCSetClock(aDriverRecPtr, 0);
	kdelay(-5);
	DDCSetData(aDriverRecPtr, 1);
	kdelay(-5);
	DDCWaitClockHigh(aDriverRecPtr);
	kdelay(-10);
	kdelay(-15);
	UInt8 getBit = DDCGetData(aDriverRecPtr);
	DDCSetClock(aDriverRecPtr, 0);
	kdelay(-5);
	return getBit;
}

OSStatus DDCSendBit(DriverGlobal *aDriverRecPtr, UInt8 value) {
	DDCSetClock(aDriverRecPtr, 0);
	kdelay(-5);
	DDCSetData(aDriverRecPtr, value);
	kdelay(-5);
	if (!DDCWaitClockHigh(aDriverRecPtr)) return 0xD54C;
	kdelay(-10);
	DDCSetClock(aDriverRecPtr, 0);
	kdelay(-5);
	return noErr;
}

void DDCInit(DriverGlobal *aDriverRecPtr) {
	DDCSetClock(aDriverRecPtr, 1);
	DDCSetData(aDriverRecPtr, 1);
	DDCFreeClock(aDriverRecPtr);
	DDCFreeData(aDriverRecPtr);
	ddc_io= 0;
}

OSStatus DDCSetStart(DriverGlobal *aDriverRecPtr) {
	DDCSetData(aDriverRecPtr, 1);
	kdelay(-5);
	if (!DDCWaitClockHigh(aDriverRecPtr)) return 0xD54C;
	kdelay(-5);
	DDCSetData(aDriverRecPtr, 0);
	kdelay(-15);
	DDCSetClock(aDriverRecPtr, 0);
	kdelay(-5);
	return noErr;
}

OSStatus DDCSetStop(DriverGlobal *aDriverRecPtr) {
	DDCSetClock(aDriverRecPtr, 0);
	kdelay(-5);
	DDCSetData(aDriverRecPtr, 0);
	kdelay(-5);
	if (!DDCWaitClockHigh(aDriverRecPtr)) return 0xD54C;
	kdelay(-5);
	DDCSetData(aDriverRecPtr, 1);
	kdelay(-15);
	return noErr;
}

OSStatus DDCSendByte(DriverGlobal *aDriverRecPtr, UInt8 value) {
	OSStatus ret = noErr;
	UInt8 i;
	for (i = 0;i < 8;i++) {
		ret = DDCSendBit(aDriverRecPtr, value & (1 << (7 - i)));
		if (ret == noErr) continue;
		return ret;
	}
	if (DDCReceiveBit(aDriverRecPtr)) ret = 0xD54C;
	return ret;
}

OSStatus DDCReceiveByte(DriverGlobal *aDriverRecPtr, UInt8 *value, UInt32 bitValue) {
	UInt8 getByte = 0;
	UInt8 i;
	for (i = 0;i < 8;i++) {
		if (DDCReceiveBit(aDriverRecPtr) == noErr) continue;
		getByte |= (1 << (7 - i));
	}
	*value = getByte;
	return DDCSendBit(aDriverRecPtr, bitValue);
}

bool DDCReadEDID(DriverGlobal *aDriverRecPtr, UInt32 blockNum, UInt8* edid) {
	UInt8 i;
	UInt8 receiveByte;
	UInt32 extraBytes = 100;
	
	OSStatus ret = DDCSetStart(aDriverRecPtr);
	if (ret != noErr) {
		DDCSetStop(aDriverRecPtr);
		return false;
	}
	if (DDCSendByte(aDriverRecPtr, 0xA0) != noErr) {
		DDCSetStop(aDriverRecPtr);
		return false;
	}
	DDCSendByte(aDriverRecPtr, 0);
	DDCSetStop(aDriverRecPtr);
	kdelay(-500);
	if (DDCSetStart(aDriverRecPtr) != noErr) {
		DDCSetStop(aDriverRecPtr);
		return false;
	}
	if (DDCSendByte(aDriverRecPtr, 0xA1) != noErr) {
		DDCSetStop(aDriverRecPtr);
		return false;
	}
	if (blockNum > 1) {
		for (i = 0;i < 127;i++) DDCReceiveByte(aDriverRecPtr, &receiveByte, 0);
		if (blockNum > (receiveByte + 1)) {
			DDCReceiveByte(aDriverRecPtr, &receiveByte, 1);
			DDCSetStop(aDriverRecPtr);
			return false;
		}
		DDCReceiveByte(aDriverRecPtr, &receiveByte, 0);
		extraBytes = blockNum * 128 - 256;
		for (i = 0;i < extraBytes;i++) DDCReceiveByte(aDriverRecPtr, &receiveByte, 0);
	}
	for (i = 0;i < 128;i++) DDCReceiveByte(aDriverRecPtr, &edid[i], (i == 127));
	DDCSetStop(aDriverRecPtr);
	return true;
}

bool DDCGetEDID(DriverGlobal *aDriverRecPtr, UInt32 blockNum, UInt8* edid, UInt8 connection) {
	bool gotEDID = false;
	bool displayOn = true;
	bool dacOn = true;
	bool var_9 = 0;
	UInt8 i;
	for (i = 0;i < 128;i++) edid[i] = 0xFF;
	
	if (!(aDriverRecPtr->driverFlags & (1 << 27))) {
#ifdef ATY_Caretta
		if (connection == (1 << 4)) {		//CRT1
			dacOn = HW_DAC1On(aDriverRecPtr);
			displayOn = HWIsCRTCDisplayOn(aDriverRecPtr, 0);
			if (aDriverRecPtr->driverFlags & (1 << 17)) var_9 = 1;
		}
#endif
#ifdef ATY_Wormy
		if (connection == (1 << 5)) {		//CRT2
			dacOn = HW_DAC2On(aDriverRecPtr);
			displayOn = HWIsCRTCDisplayOn(aDriverRecPtr, 0);
			if (aDriverRecPtr->driverFlags & (1 << 17)) var_9 = 1;
		}
#endif
		
		for (i = 0;i < 8;i++) {	//so here it read for 8 times
			if (!DDCReadEDID(aDriverRecPtr, blockNum, edid)) continue;
			return true;
		}
	}
	
	if (aDriverRecPtr->driverFlags & (1 << 27)) return false;
	if (!displayOn || !dacOn || !var_9) {
		if (!displayOn) {
			HWProgramCRTCDisplayOn(aDriverRecPtr, 0);
			HWProgramCRTCBlank(aDriverRecPtr, 1, 0);
		}
#ifdef	ATY_Caretta
		if (!dacOn && (connection == 1 << 4)) {	//CRT1
			HW_DAC1OnOff(aDriverRecPtr, 1 << 4, 0);
			if (!dacOn) HW_DAC1Blank(aDriverRecPtr, 1);
		}
#endif
#ifdef	ATY_Wormy
		if (!dacOn && (connection == 1 << 5)) {	//CRT2
			HW_SaveDAC2OnOff(aDriverRecPtr);
			HW_DAC2OnOff(aDriverRecPtr, 1 << 5, 0);
			if (!dacOn) HW_DAC2Blank(aDriverRecPtr, 1);
		}
#endif
		for (i = 0;i < 16;i++) {
			kdelay(100);
			if (!DDCReadEDID(aDriverRecPtr, blockNum, edid)) continue;
			gotEDID = true;
			break;
		}
		
		if (!displayOn) {
			HWProgramCRTCOff(aDriverRecPtr, 0);
			HWProgramCRTCBlank(aDriverRecPtr, 0, 0);
		}
#ifdef	ATY_Caretta
		if (!dacOn && (connection == (1 << 4))) {	//CRT1
			HW_DAC1OnOff(aDriverRecPtr, 0, 0);
			HW_DAC1Blank(aDriverRecPtr, 0);
		}
#endif
#ifdef	ATY_Wormy
		if (!dacOn && (connection == (1 << 5))) {	//CRT2
			HW_DAC2OnOff(aDriverRecPtr, 0, 0);
			HW_DAC2Blank(aDriverRecPtr, 0);
			HW_RestoreDAC2OnOff(aDriverRecPtr);
		}
#endif
	}
	return gotEDID;
}

bool DDCCheckEDID(DriverGlobal *aDriverRecPtr, UInt8 *edid, UInt32 index, UInt32 size) {
	bool notAllZero = false;
	UInt8 sum = 0;
	UInt8 i;
	
	for (i = 0;i < size;i++) {
		if (edid[i] != 0) notAllZero = true;
		sum += edid[i];
	}
	if (notAllZero && (sum == 0)) return true;
	return false;
}

OSStatus DoDDCForceRead(EdidInfo *edidInfo, DriverGlobal *aDriverRecPtr, UInt8 value, UInt8 connection, UInt8 some_connection, UInt8 connectedFlags) {
	const RegPropertyName names[4][30] = {
		"override-no-connect",
		"override-no-edid",
		"override-has-edid-digital",
		"override-has-edid"
	};
	RegPropertyName *name;
	OSStatus ret;
	
	if (!(edidInfo->unknown2 & 1) || edidInfo->unknown1) return paramErr;
	
	if (connection & some_connection) {
		name = (RegPropertyName *)names[0];	//"override-no-connect"
		ret = ReadOverrideEDIDProperty(aDriverRecPtr, connection, edidInfo->blockNum, name, edidInfo->edid);
		if (ret != noErr) return readErr;
	} else {
		DDCInit(aDriverRecPtr);
		if (DDCGetEDID(aDriverRecPtr, edidInfo->blockNum, edidInfo->edid, connection) != 1) {
			ret = readErr;
			if (connection & connectedFlags) {
				name = (RegPropertyName *)names[1];	//"override-no-edid"
				ret = ReadOverrideEDIDProperty(aDriverRecPtr, connection, edidInfo->blockNum, name, edidInfo->edid);
			}
			if (ret != noErr) return readErr;
		} else {
			if (EdidDigital(aDriverRecPtr, edidInfo->edid)) name = (RegPropertyName *)names[2];	//"override-has-edid-digital"
			else name = (RegPropertyName *)names[3];	//"override-has-edid"
			ReadOverrideEDIDProperty(aDriverRecPtr, connection, edidInfo->blockNum, name, edidInfo->edid);
		}
	}
	if (!DDCCheckEDID(aDriverRecPtr, edidInfo->edid, edidInfo->blockNum - 1, 128)) return readErr;
	return noErr;
}

bool EDID_StoreDetailTiming(DriverGlobal *aDriverRecPtr, CrtcValues *mode, UInt8* edidDT, bool isDigital, UInt8 connection) {
	UInt16 blank[2]; //used to store horizontal and vertical blanking
	
	if (!isDigital && ((edidDT[17] >> 3 & 3) != 3)) return false; //analog composite not supported
	if (mode == NULL) return false;
	mode->mFlag = 0;
	mode->clock = edidDT[0] + edidDT[1] << 8;
	if ((mode->clock == 0) || (mode->clock == 257)) return false; //pixel clock <= 2570000 not supported
	mode->width = (edidDT[4] & 0xF0) << 4 + edidDT[2]; //get Horizontal Active, please refer to EDIDv1.3
	if (mode->width == 0) return false;
	blank[0] = (edidDT[4] & 0x0F) << 8 + edidDT[3]; ; //get Horizontal Blanking
	if (blank[0] == 0) return false;
	mode->HTotal = mode->width + blank[0];
	mode->dHSyncS = (edidDT[11] & 0xC0) << 2 + edidDT[8]; //get Horizontal Sync Offset
	if (mode->dHSyncS == 0) return false;
	mode->dHSyncE = (edidDT[11] & 0x30) << 4 + edidDT[9]; //get Horizontal Sync Pulse Width
	if (mode->dHSyncE == 0) return false;
	mode->width &= ~1; //make it dividable by 2
	if (mode->width == 0) return false;
	mode->height = (edidDT[7] & 0xF0) << 4 + edidDT[5]; //get Vertical Active
	if (mode->height == 0) return false;
	blank[1] = (edidDT[7] & 0x0F) << 8 + edidDT[6]; //get Vertical Blanking
	if (blank[1] == 0) return false;
	mode->VTotal = mode->height + blank[1];
	if (mode->VTotal == 0) return false;
	mode->dVSyncS = (edidDT[11] & 0xC) << 2 + (edidDT[10] & 0xF0); //get Vertical Sync Offset
	if (mode->dVSyncS == 0) return false;
	mode->dVSyncE = (edidDT[11] & 3) << 4 + (edidDT[10] & 0x0F); //get Vertical Sync Pulse Width
	if (mode->dVSyncE == 0) return false;
	if ((edidDT[17] >> 3 & 3) == 3) {
		if (edidDT[17] & (1 << 1)) mode->mFlag |= 1;		//flag bit0 Horizontal Polarity
		if (edidDT[17] & (1 << 2)) mode->mFlag |= (1 << 1); //flag bit1 Vertical Polarity
	}
	if (edidDT[17] & (1 << 7)) {		//interlaced
		mode->mFlag |= (1 << 3);		//flag bit3 interlaced
		mode->VTotal = mode->VTotal * 2 + 1;
		mode->dVSyncS = mode->dVSyncS * 2 + 1;
		mode->dVSyncE = mode->dVSyncE * 2;
		mode->height = mode->height * 2;
		if (isDigital && (mode->clock == 2700) && (mode->width == 1440) && ((mode->height == 480) || (mode->height == 576))) return false; //this mode does not support interlace
	}
#ifdef	ATY_Caretta
	if ((connection & (1 << 3)) && (mode->clock > 16500)) mode->mFlag |= (1 << 12);					//DFP2
#endif
#ifdef	ATY_Wormy
	if ((connection & ((1 << 6) + (1 << 2))) && (mode->clock > 16500)) mode->mFlag |= (1 << 12);	//DFP1 or LVDS
#endif
	return true;
}

OSStatus GetEDIDProperties(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 *edidP) {
	UInt8 *edid = aDriverRecPtr->edid;	//var_18
	OSStatus ret = noErr;	//var_1A
	CrtcValues mode;		//var_38
	UInt8 i, offset;
	
	if (edidP == NULL) return noErr;
	*edidP = 0;
	ret = CheckLoadEDIDBlock(aDriverRecPtr, connection, 1, edid);
	if (ret != noErr) return ret;
	*edidP = (edid[0xB] | (edid[0xA] << 8)) << 16 | edid[0x7F];		//edidP higher 2 bytes: ID Product Code, low byte:checksum
	if (EdidDigital(aDriverRecPtr, edid) == 1) *edidP |= (1 << 8);	//edidP bit8 Digital/Analog
	if (edid[0x18] & 1) *edidP |= (1 << 9);							//edidP bit9 GTF support or not
	if ((edid[8] == 6) && (edid[9] == 16)) *edidP |= (1 << 10);		//edidP bit10 means "APV" is the Manufacturer
	UInt32 tVram = aDriverRecPtr->totalVram;		//var_C
	UInt32 dlb = aDriverRecPtr->displayLineBuffer;	//var_10
	aDriverRecPtr->totalVram = aDriverRecPtr->aShare->totalVram / 2;
	if (aDriverRecPtr->dispNum == 0) aDriverRecPtr->displayLineBuffer = 2;
	else aDriverRecPtr->displayLineBuffer = 4;
	for (i = 0, offset = 0x36;i <= 3;i++, offset += 0x12) {		//get all detail timing
		if (!EDID_StoreDetailTiming(aDriverRecPtr, &mode, &edid[offset], EdidDigital(aDriverRecPtr, edid), connection)) continue;
		if (EdidDigital(aDriverRecPtr, edid) && (mode.width == 1920) && (mode.mFlag & (1 << 3)))	//interlaced
			*edidP |= (1 << 13);													//edidP bit13
		mode.modeID = kDisplayModeIDInvalid;
		mode.timingMode = timingInvalid;
		if (!HALValidCRTCParameters(aDriverRecPtr, &mode, connection)) continue;
		if ((mode.width <= 5760) && (mode.width> 3840)) *edidP |= (1 << 12);		//edidP bit12
		if ((mode.width > 5760) && (mode.width<= 7680)) *edidP |= (1 << 11);		//edidP bit11
		if ((mode.width == 1920) && (mode.mFlag & (1 << 3))) *edidP |= (1 << 11);	//edidP bit11
	}
	aDriverRecPtr->displayLineBuffer = dlb;
	aDriverRecPtr->totalVram = tVram;
	if ((CheckLoadEDIDBlock(aDriverRecPtr, connection, 2, edid) == noErr) && (edid[0] == 2)) {
		if (EdidDigital(aDriverRecPtr, edid)) *edidP |= (1 << 13);	//edidP bit13
		*edidP |= (1 << 11);										//edidP bit11
	}
	return ret;
}

UInt8 CheckDDCConnection(DriverGlobal *aDriverRecPtr, UInt8 connection, bool noEDID, UInt32* edidP, UInt8 some_connection, UInt8 connectedFlags) {
	bool isDigital = false;	//var_12
	bool isTV = false;		//var_11
	bool beDigital = false;	//var_10
	UInt8 DDC = 0;			//var_F
	EdidInfo edidInfo;		//var_A4
	
	if (SelectDDCConnectionSenseLines(aDriverRecPtr, connection) == 0) return readErr;
#ifdef ATY_Caretta
	if (connection == (1 << 3)) beDigital = true;		//DFP2
#endif
#ifdef ATY_Wormy
	if (connection == (1 << 2)) beDigital = true;		//DFP1
	if (connection == (1 << 6)) beDigital = true;		//LVDS
#endif
	
	UInt8 i, j, k;
	OSStatus ret = readErr;
	if (!noEDID) {
		ret = CheckLoadEDIDBlock(aDriverRecPtr, connection, 1, edidInfo.edid);
		if (ret != noErr) noEDID = true;
		else {
			isDigital = EdidDigital(aDriverRecPtr, edidInfo.edid);
			if (!isDigital) isTV = EdidTV(aDriverRecPtr, edidInfo.edid);
		}
	}
	if (noEDID) {
		ret = noErr;
		DeleteEDIDProperty(aDriverRecPtr, connection);
		for (i = 0, j = 1;i < j;i++) {
			edidInfo.unknown2 = 1;
			edidInfo.blockNum = i + 1;
			edidInfo.unknown1 = 0;
			ret = DoDDCForceRead(&edidInfo, aDriverRecPtr, 1, connection, some_connection, connectedFlags);
			for (k = 0;k < 128;k++);	//nothing is done here, maybe waiting for ddc reading
			if (ret != noErr) break;
			if (i == 0) {
				if ((UInt16) *(edidInfo.edid+ 0x7E) < 255) j++;	//give another DDC reading chance
				isDigital = EdidDigital(aDriverRecPtr, edidInfo.edid);
				if (isDigital != beDigital) ret = readErr;
				if (!isDigital) isTV = EdidTV(aDriverRecPtr, edidInfo.edid);
			}
			if (ret != noErr) break;
			SaveEDIDBlock(aDriverRecPtr, connection, edidInfo.blockNum, edidInfo.edid);
		}
	}
	if ((ret == noErr) || (i != 0)) {
		if (isDigital) DDC = 2;
		else if (isTV) DDC = 3;
		else DDC = 1;
		if (edidP != NULL) GetEDIDProperties(aDriverRecPtr, connection, edidP);
	}
	return DDC;
}

OSStatus WriteLM63(DriverGlobal *aDriverRecPtr, UInt8 val1, UInt8 val2) {
	aDriverRecPtr->DDCGetSense = DDC3GetSense;
	aDriverRecPtr->DDCSetSense = DDC3SetSense;
	DDCInit(aDriverRecPtr);
	DDCSetStart(aDriverRecPtr);
	DDCSendByte(aDriverRecPtr, 0x98);
	DDCSendByte(aDriverRecPtr, val1);
	DDCSendByte(aDriverRecPtr, val2);
	DDCSetStop(aDriverRecPtr);
	return noErr;
}

UInt8 ReadLM63(DriverGlobal *aDriverRecPtr, UInt8 val1) {
	UInt8 value;
	
	aDriverRecPtr->DDCGetSense = DDC3GetSense;
	aDriverRecPtr->DDCSetSense = DDC3SetSense;
	DDCInit(aDriverRecPtr);
	DDCSetStart(aDriverRecPtr);
	DDCSendByte(aDriverRecPtr, 0x98);
	DDCSendByte(aDriverRecPtr, val1);
	DDCSetStart(aDriverRecPtr);
	DDCSendByte(aDriverRecPtr, 0x99);
	DDCReceiveByte(aDriverRecPtr, &value, 1);
	DDCSetStop(aDriverRecPtr);
	return value;
}

void ProgramLM63FPWMValue(DriverGlobal *aDriverRecPtr, UInt8 value) {
	WriteLM63(aDriverRecPtr, 0x11, 0x10);
	WriteLM63(aDriverRecPtr, 0x4A, ReadLM63(aDriverRecPtr, 0x4A) & 0xC4 | 0x38);
	WriteLM63(aDriverRecPtr, 0x4B, ReadLM63(aDriverRecPtr, 0x4B) & 0xC0 | 0x1A);
	WriteLM63(aDriverRecPtr, 0x4D, ReadLM63(aDriverRecPtr, 0x4D) & 0xE0 | 0x17);
	WriteLM63(aDriverRecPtr, 0x4C, value);
}

OSStatus SetupLM63FanCntrlLookUP(DriverGlobal *aDriverRecPtr) {
	const UInt8 PWM_LookUp_Table[16] = {
		0, 5, 0x37, 0xA, 0x5B, 0xD, 0x5F, 0xF,
		0x62, 0x16, 0x62, 0x16, 0x62, 0x16, 0x62, 0x16
	};
	
	UInt8 i;
	ProgramLM63FPWMValue(aDriverRecPtr, 0x2E);
	for (i = 0;i < 16;i++) WriteLM63(aDriverRecPtr, 80 + i, PWM_LookUp_Table[i]);
	WriteLM63(aDriverRecPtr, 0x4A, ReadLM63(aDriverRecPtr, 0x4A) & 0xDF);
	return noErr;
}

void CheckTMDS1Connection(DriverGlobal *aDriverRecPtr, ConnectionInfo* conInfo, bool unknown2) {
	UInt32 edidP;
	if (aDriverRecPtr->aShare->connectorFlags & (1 << 2)) return;
	if (CheckDDCConnection(aDriverRecPtr, 1 << 2, unknown2, &edidP, 0, 0) != 2) return;
	conInfo->connectedFlags |= (1 << 2);
	conInfo->edidP2 = edidP;
}

void CheckTMDS2Connection(DriverGlobal *aDriverRecPtr, ConnectionInfo* conInfo, bool noEDID) {
	UInt32 edidP;
	if (aDriverRecPtr->aShare->connectorFlags & (1 << 3)) return;
	if (CheckDDCConnection(aDriverRecPtr, 1 << 3, noEDID, &edidP, 0, 0) != 2) return;
	conInfo->connectedFlags |= (1 << 3);
	conInfo->edidP2 = edidP;
}

#ifdef ATY_Caretta
const int DriversConnectorsLen = 1;
const ConnectorInfo DriversConnectors[1] = {{1 << 2, (1 << 3) | (1 << 4)}};
#endif
#ifdef ATY_Wormy
const int DriversConnectorsLen = 2;
const ConnectorInfo DriversConnectors[2] = {
{1 << 1, 1 << 6},
{1 << 2, (1 << 1) | (1 << 2) | (1 << 5) | (1 << 7)}
};
#endif
#ifdef ATY_Prionace
const int DriversConnectorsLen = 2;
const ConnectorInfo DriversConnectors[2] = {
{1 << 9, (1 << 3) | (1 << 4)},
{1 << 2, (1 << 2) | (1 << 5)}
};
#endif

ConnectorInfo * FindConnectorInfo(DriverGlobal *aDriverRecPtr, UInt8 connection) {
	UInt8 i;
	for(i = 0;i < DriversConnectorsLen;i++) {
		if (DriversConnectors[i].connectorFlags & connection) return (ConnectorInfo *)&DriversConnectors[i];
	}
	return NULL;
}

ConnectorInfo * FindConnectorInfoByType(DriverGlobal *aDriverRecPtr, UInt32 connectorType) {
	connectorType &= ~(1 << 31);
	UInt8 i;
	for(i = 0;i < DriversConnectorsLen;i++) {
		if (DriversConnectors[i].connectorType == connectorType) return (ConnectorInfo *)&DriversConnectors[i];
	}
	return NULL;
}

void GetOverrideNoEDID(DriverGlobal *aDriverRecPtr, UInt8 connection, ConnectionInfo* conInfo, UInt32* edidP) {
	UInt8 connected = 0;
	UInt8 connectFlags = 0;
	if (aDriverRecPtr->aShare->connectorFlags & connection) return;
	if (!(conInfo->connectedFlags & connection)) return;		//not connected
	if (CheckDDCConnection(aDriverRecPtr, connection, 1, edidP, 0, conInfo->connectedFlags) != 2) return;
	ConnectorInfo *connectorInfo = FindConnectorInfo(aDriverRecPtr, connection);
	if (connectorInfo != NULL) connectFlags = connectorInfo->connectorFlags;
#ifdef ATY_Caretta
	if ((connectorInfo == NULL) && (connection & (1 << 4))) connectFlags = (1 << 4);
	if ((connectFlags & (1 << 3)) && !(conInfo->connectedFlags & (1 << 3))
		&& (CheckDDCConnection(aDriverRecPtr, 1 << 3, 1, &conInfo->edidP2, 0, 1 << 3) == 2))	//DFP2 use DDC2
		connected = (1 << 3);
#endif
#ifdef ATY_Wormy
	if ((connectorInfo == NULL) && (connection & (1 << 5))) connectFlags = (1 << 5);
	if ((connectFlags & (1 << 2)) && !(conInfo->connectedFlags & (1 << 2))
		&& (CheckDDCConnection(aDriverRecPtr, 1 << 2, 1, conInfo->edidP2, 0, 1 << 2) == 2))	//DFP2 use DDC2
		connected = (1 << 2);
	if ((connectFlags & (1 << 6)) && !(conInfo->connectedFlags & (1 << 2))
		&& (CheckDDCConnection(aDriverRecPtr, 1 << 6, 1, conInfo->edidP3, 0, 1 << 6) == 2))	//DFP2 use DDC2
		connected = (1 << 6);
#endif
	if (connected == 0) return;
	conInfo->connectedFlags |= connected;
	conInfo->connectedFlags &= ~connection;
}

void CheckDAC1Connection(DriverGlobal *aDriverRecPtr, ConnectionInfo* conInfo, bool unknown1, bool noEDID) {
	UInt8 DDC = 0;
	UInt32 edidP;
	
	if (aDriverRecPtr->aShare->connectorFlags & (1 << 4)) return;
	if (aDriverRecPtr->driverFlags & (1 << 11)) {
		conInfo->connectedFlags |= (1 << 4);			//CRT1
		return;
	}
	aDriverRecPtr->driverFlags |= (1 << 27);
	DDC = CheckDDCConnection(aDriverRecPtr, 1 << 4, noEDID, &edidP, 0, conInfo->connectedFlags);
	aDriverRecPtr->driverFlags &= ~(1 << 27);
	if (DDC == 3) return;
	if (DDC == 1) {
		conInfo->connectedFlags |= (1 << 4);			//CRT1 use DDC1
		conInfo->edidP1 = edidP;
	}
	if (conInfo->connectedFlags & (1 << 4)) return;		//CRT1 connected
	if (aDriverRecPtr->edidFlags & (1 << 4)) return;	//already has CRT1 EDID
	if (!unknown1 && HW_DAC1Sense(aDriverRecPtr)) conInfo->connectedFlags |= (1 << 4);
	if ((conInfo->connectedFlags & (1 << 4))
		&& (CheckDDCConnection(aDriverRecPtr, 1 << 4, noEDID, &edidP, 0, conInfo->connectedFlags) == 1))
		conInfo->edidP1 = edidP;
	if (!(conInfo->connectedFlags & (1 << 4))) return;
	if (conInfo->edidP1) return;
	GetOverrideNoEDID(aDriverRecPtr, 1 << 4, conInfo, &conInfo->edidP1);
}

void CheckDAC2Connection(DriverGlobal *aDriverRecPtr, ConnectionInfo* conInfo, bool unknown1, bool noEDID) {
	UInt8 DDC = 0;
	UInt32 edidP;
	
	if (aDriverRecPtr->aShare->connectorFlags & (1 << 5)) return;
	if (aDriverRecPtr->driverFlags & (1 << 11)) {
		conInfo->connectedFlags |= (1 << 5);			//CRT2
		return;
	}
	aDriverRecPtr->driverFlags |= (1 << 27);
	DDC = CheckDDCConnection(aDriverRecPtr, 1 << 5, noEDID, &edidP, 0, conInfo->connectedFlags);
	aDriverRecPtr->driverFlags &= ~(1 << 27);
	if (DDC == 4) return;
	if ((DDC == 1) || (DDC == 3)) {
		conInfo->connectedFlags |= (1 << 5);			//CRT2 use DDC1 or DDC3
		conInfo->edidP1 = edidP;
	}
	if (!(conInfo->connectedFlags & (1 << 5)) && !(aDriverRecPtr->edidFlags & (1 << 5))) {
		if (RF_DAC2Sense(aDriverRecPtr)) conInfo->connectedFlags |= (1 << 5);
		if (conInfo->connectedFlags & (1 << 5)) {
			DDC = CheckDDCConnection(aDriverRecPtr, 1 << 4, noEDID, &edidP, 0, conInfo->connectedFlags);
			if ((DDC == 1) || (DDC = 3)) conInfo->edidP1 = edidP;
		}
		if ((conInfo->connectedFlags & (1 << 5)) && !(conInfo->edidP1))
			GetOverrideNoEDID(aDriverRecPtr, 1 << 5, conInfo, &conInfo->edidP1);
	}
	if (conInfo->connectedFlags & (1 << 5)) {
		conInfo->connectedFlags |= (1 << 1);		//TV
		if (DDC == 3) conInfo->tvDDC = 3;
	}
}

void CheckLVDSConnection(DriverGlobal *aDriverRecPtr, ConnectionInfo *conInfo, bool noEDID) {
	UInt32 edidP;
	if (aDriverRecPtr->aShare->connectorFlags & (1 << 6)) return;	//1 << 6 means LVDS
	if (CheckDDCConnection(aDriverRecPtr, 1 << 6, noEDID, &edidP, 0, 0) != 2) return;
	conInfo->connectedFlags |= (1 << 6);
	conInfo->edidP3 = edidP;
}

void GetOverrideNoConnect(DriverGlobal *aDriverRecPtr, UInt8 connection, ConnectionInfo* conInfo, UInt32* edidP, UInt8 DDC) {
	if (aDriverRecPtr->aShare->connectorFlags & connection) return;
	if (conInfo->connectedFlags & connection) return;
	if (CheckDDCConnection(aDriverRecPtr, connection, 1, edidP, connection, 0) != DDC) return;
	conInfo->unknown5 |= connection;
}

void CheckOverrideNoConnection(DriverGlobal *aDriverRecPtr, ConnectionInfo* conInfo) {
#ifdef ATY_Caretta
	GetOverrideNoConnect(aDriverRecPtr, 1 << 3, conInfo, &conInfo->edidP2, 2);		//DFP2 use DDC line 2?
	GetOverrideNoConnect(aDriverRecPtr, 1 << 4, conInfo, &conInfo->edidP1, 1);		//CRT1 use DDC line 1?
#endif
#ifdef ATY_Caretta
	GetOverrideNoConnect(aDriverRecPtr, 1 << 2, conInfo, &conInfo->edidP2, 2);		//DFP1 use DDC line 2?
	GetOverrideNoConnect(aDriverRecPtr, 1 << 6, conInfo, &conInfo->edidP3, 2);		//LVDS use DDC line 2?
	GetOverrideNoConnect(aDriverRecPtr, 1 << 5, conInfo, &conInfo->edidP1, 1);		//CRT2 use DDC line 1?
#endif
	conInfo->connectedFlags |= conInfo->unknown5;
	conInfo->edidFlags = aDriverRecPtr->edidFlags;
}

void GetConnectionInfo(DriverGlobal *aDriverRecPtr, ConnectionInfo *conInfo, bool unknown1, bool noEDID, bool unknown3) {
	const UInt32 IllegalConnectionsLen = 1;
	const UInt8 IllegalConnections[1] = {0};
	
	if ((aDriverRecPtr->connectorNum != 0) && (unknown3) && (aDriverRecPtr->aShare->conInfo != NULL)
		&& (aDriverRecPtr->aShare->conInfo != &aDriverRecPtr->conInfo)) {
		memcpy(conInfo, aDriverRecPtr->aShare->conInfo, sizeof(ConnectionInfo));
		aDriverRecPtr->aShare->conInfo = NULL;
		if (conInfo->dispNum != aDriverRecPtr->dispNum) return;
	}
	aDriverRecPtr->aShare->conInfo = NULL;
	conInfo->dispNum = aDriverRecPtr->dispNum;
	conInfo->connectedFlags = 0;
	conInfo->connectTaggedType1 = 7;
	conInfo->connectTaggedType2 = 7;
	conInfo->connectTaggedData1 = 0x3F;
	conInfo->connectTaggedData2 = 0x3F;
	conInfo->edidP1 = 0;
	conInfo->edidP2 = 0;
#ifdef ATY_Wormy
	conInfo->unknown6 = 0;
	conInfo->unknown7 = 0;
	conInfo->unknown8 = aDriverRecPtr->unknown54;
	conInfo->unknown9 = 0;
	conInfo->unknown10 = 0;
#endif
	conInfo->unknown5 = 0;
	conInfo->edidFlags = 0;
	aDriverRecPtr->edidFlags = 0;
#ifdef ATY_Caretta
	DeleteEDIDProperty(aDriverRecPtr, (1 << 3));				//DFP2
	if (!(aDriverRecPtr->aShare->connectorFlags & (1 << 3))) CheckTMDS2Connection(aDriverRecPtr, conInfo, noEDID);
	DeleteEDIDProperty(aDriverRecPtr, (1 << 4));				//CRT1
	if (!(aDriverRecPtr->aShare->connectorFlags & (1 << 4))) CheckDAC1Connection(aDriverRecPtr, conInfo, unknown1, noEDID);
#endif
#ifdef ATY_Wormy
	DeleteEDIDProperty(aDriverRecPtr, (1 << 2));				//DFP1
	if (!(aDriverRecPtr->aShare->connectorFlags & (1 << 2))) CheckTMDS1Connection(aDriverRecPtr, conInfo, noEDID);
	DeleteEDIDProperty(aDriverRecPtr, (1 << 6));				//LVDS
	if (!(aDriverRecPtr->aShare->connectorFlags & (1 << 6))) CheckLVDSConnection(aDriverRecPtr, conInfo, noEDID);
	DeleteEDIDProperty(aDriverRecPtr, (1 << 5));				//CRT2
	if (!(aDriverRecPtr->aShare->connectorFlags & (1 << 5))) CheckDAC2Connection(aDriverRecPtr, conInfo, unknown1, noEDID);
#endif
	if (conInfo->connectedFlags == 0) CheckOverrideNoConnection(aDriverRecPtr, conInfo);
#ifdef ATY_Wormy
	conInfo->shellClosed = ClamshellClosed(aDriverRecPtr);
	if ((conInfo->connectedFlags != (1 << 6)) && conInfo->shellClosed) conInfo->connectedFlags &= ~(1 << 6);
	if (conInfo->shellClosed != aDriverRecPtr->aShare->shellClosed) conInfo->shellStateChanged = true;
#endif
	UInt8 i;
	for (i = 0;i < IllegalConnectionsLen;i++) {
		if ((conInfo->connectedFlags & IllegalConnections[i]) == IllegalConnections[i])
			conInfo->connectedFlags &= ~IllegalConnections[i];	//exclude illegal connections
	}
#ifdef ATY_Caretta
	if (conInfo->connectedFlags & (1 << 4))						//CRT1
		Apple_GetHWSenseCode(aDriverRecPtr, conInfo->connectedFlags, &conInfo->connectTaggedType1, &conInfo->connectTaggedData1);
#endif
#ifdef ATY_Wormy
	if (conInfo->connectedFlags & (1 << 5))						//CRT2
		Apple_GetHWSenseCode2(aDriverRecPtr, conInfo->connectedFlags, &conInfo->connectTaggedType2, &conInfo->connectTaggedData2);
#endif
	RegPrint(&aDriverRecPtr->regIDDevice, "ATY,ConnectedFlags", &conInfo->connectedFlags, 1);
	RegPrint(&aDriverRecPtr->regIDDevice, "ATY,Connected", conInfo, sizeof(ConnectionInfo));
	if (aDriverRecPtr->connectorNum != 0) aDriverRecPtr->aShare->conInfo = &aDriverRecPtr->conInfo;
}

void Print_connection_info(ConnectionInfo* conInfo) {
}

void CopyConectionInfo2Globals(DriverGlobal *aDriverRecPtr, ConnectionInfo* conInfo) {
	aDriverRecPtr->connectedFlags = conInfo->connectedFlags;
	aDriverRecPtr->edidFlags = conInfo->edidFlags;
#ifdef ATY_Wormy
	aDriverRecPtr->unknown54 = conInfo->unknown8;
	aDriverRecPtr->aShare->shellClosed = conInfo->shellClosed;
#endif
	if (&aDriverRecPtr->conInfo != conInfo) memcpy(&aDriverRecPtr->conInfo, conInfo, sizeof(ConnectionInfo));
}

void InitConnections(DriverGlobal *aDriverRecPtr) {
	bool unknown1 = false;
	bool noEDID = false;
	if (aDriverRecPtr->driverFlags & (1 << 2)) unknown1 = true;
	if (aDriverRecPtr->driverFlags & (1 << 11)) {
		unknown1 = false;
		noEDID = true;
	}
	GetConnectionInfo(aDriverRecPtr, &aDriverRecPtr->conInfo, unknown1, noEDID, true);
	Print_connection_info(&aDriverRecPtr->conInfo);
	CopyConectionInfo2Globals(aDriverRecPtr, &aDriverRecPtr->conInfo);
}

