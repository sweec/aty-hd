/*
 *  ATY_HW.h
 *  ATY_HD
 *
 *  Created by Dong Luo on 3/11/09.
 *  Copyright 2009 Boston University. All rights reserved.
 *
 */

#ifndef _ATY_HW_H
#define _ATY_HW_H

#include "ATY_Struct.h"

void Apple_GetHWSenseCode(DriverGlobal *aDriverRecPtr, UInt8 connectedFlags, UInt8* connectType, UInt8* connectData);
void Apple_GetHWSenseCode2(DriverGlobal *aDriverRecPtr, UInt8 connectedFlags, UInt8* connectType, UInt8* connectData);
bool HW_DAC1Sense(DriverGlobal *aDriverRecPtr);
bool RF_DAC2Sense(DriverGlobal *aDriverRecPtr);
bool HW_DAC1On(DriverGlobal *aDriverRecPtr);
bool HW_DAC2On(DriverGlobal *aDriverRecPtr);
bool HWIsCRTCDisplayOn(DriverGlobal *aDriverRecPtr, UInt8 dispNum);
bool HWProgramCRTCDisplayOn(DriverGlobal *aDriverRecPtr, UInt8 dispNum);
void HWProgramCRTCOff(DriverGlobal *aDriverRecPtr, UInt8 dispNum);
void HWProgramCRTCBlank(DriverGlobal *aDriverRecPtr, UInt8 value, UInt8 dispNum);
void HW_DAC1OnOff(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8 dispNum);
void HW_DAC1Blank(DriverGlobal *aDriverRecPtr, bool onOff);
void HW_SaveDAC2OnOff(DriverGlobal *aDriverRecPtr);
void HW_DAC2OnOff(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8 dispNum);
void HW_DAC2Blank(DriverGlobal *aDriverRecPtr, bool onOff);
void HW_RestoreDAC2OnOff(DriverGlobal *aDriverRecPtr);
UInt32 HW_GetSClk(DriverGlobal *aDriverRecPtr);
UInt32 HW_GetMClk(DriverGlobal *aDriverRecPtr);
void SetPerformanceWeights(DriverGlobal *aDriverRecPtr);
void InitHWSetting(DriverGlobal *aDriverRecPtr);
void HW_InitMEMConfiguration(DriverGlobal *aDriverRecPtr);
void RF_MiscSetup(DriverGlobal *aDriverRecPtr);
void HW_InitDisplay(DriverGlobal *aDriverRecPtr);
void HW_EnableDynamicMode(DriverGlobal *aDriverRecPtr);
OSStatus InitInterrupts(DriverGlobal *aDriverRecPtr);

bool HALValidCRTCParameters(DriverGlobal *aDriverRecPtr, CrtcValues *mode, UInt8 connection);
void HALInitEDIDModes(DriverGlobal *aDriverRecPtr, UInt8 connection);
void HALSetupMonitorTable(DriverGlobal *aDriverRecPtr, UInt32 connection);
void HALInitSimulscanModes(DriverGlobal *aDriverRecPtr);

void kdelay(SInt32 time);
void regw8(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt8 value);
void regw16(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt16 value);
void regw32(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt32 value);
UInt8 regr8(DriverGlobal *aDriverRecPtr, UInt16 addr);
UInt16 regr16(DriverGlobal *aDriverRecPtr, UInt16 addr);
UInt32 regr32(DriverGlobal *aDriverRecPtr, UInt16 addr);
void rmsw32(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt32 mask, UInt8 offset, UInt32 value);
UInt32 rmsr32(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt32 mask, UInt8 offset);
void pllw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value);
UInt32 pllr32(DriverGlobal *aDriverRecPtr, UInt8 index);
void pllmsw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 mask, UInt8 offset, UInt32 value);
UInt32 pllmsr32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 mask, UInt8 offset);
void memws32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value, UInt32 mask);
void memw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value);
UInt32 memrs32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 mask);
UInt32 memr32(DriverGlobal *aDriverRecPtr, UInt16 index);
UInt32 idx_regr32(DriverGlobal *aDriverRecPtr, UInt32 index);
void idx_regw32(DriverGlobal *aDriverRecPtr, UInt32 index, UInt32 value);
void pciXw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value);
UInt32 pciXr32(DriverGlobal *aDriverRecPtr, UInt16 index);

#endif