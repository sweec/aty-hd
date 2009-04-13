/*
 *  ATY_Driver.h
 *  ATY_HD
 *
 *  Created by Dong Luo on 3/6/09.
 *  Copyright 2009 Boston University. All rights reserved.
 *
 */
#ifndef _ATY_DRIVER_H
#define _ATY_DRIVER_H

#include <IOKit/IOLib.h>
#include <IOKit/platform/ApplePlatformExpert.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOMessage.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <IOKit/graphics/IOGraphicsInterfaceTypes.h>
#include <IOKit/i2c/IOI2CInterface.h>
#include <IOKit/pci/IOAGPDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/assert.h>

#include <libkern/c++/OSContainers.h>
#include <string.h>
#include "ATY_Struct.h"
#include "ATY_HW.h"

extern bool ATIPCIRangeIndexAlloc(ATY_HD *aDriver, UInt8 index, UInt32 bar);
extern IOPhysicalAddress ATIPCIRangeMapGetPhysicalRange(ATY_HD *aDriver, UInt8 index, UInt32 *bar, IOMemoryMap **map);
extern IOVirtualAddress ATIPCIRangeMapAlloc(ATY_HD *aDriver, UInt8 index, IOPhysicalAddress *addr);
extern IOVirtualAddress ATIPCIRangeMapAllocSubRange(ATY_HD *aDriver, UInt8 index, IOPhysicalAddress offset, UInt32 length);
extern IOVirtualAddress ATIMapRegisters(ATY_HD *aDriver);
extern IOVirtualAddress ATIMapRom(ATY_HD *aDriver);
extern IOVirtualAddress ATIMapFrameBuffer(ATY_HD *aDriver, IOPhysicalAddress offset, UInt32 length);
extern IOVirtualAddress ATIMapFrameBufferReserved(ATY_HD *aDriver, IOPhysicalAddress offset, UInt32 length);
extern void ATIRequireMaxBusStall(ATY_HD *aDriver, UInt32 ns);

OSStatus RegPrint(RegEntryID* reg, const RegPropertyName* key, const void* value, RegPropertyValueSize size);
OSStatus RegGet(RegEntryID* reg, RegPropertyName* key, void* value, RegPropertyValueSize size);
OSStatus RegFind(DriverGlobal *aDriverRecPtr, RegPropertyName *name);
UInt32 GetConnectionNumber(DriverGlobal *aDriverRecPtr, UInt8 connection);
MonitorParameter* GetMonitorInfo(DriverGlobal *aDriverRecPtr, UInt8 connection);
OSStatus InitPCIDevice(DriverGlobal *aDriverRecPtr);
UInt32 FindDisplayNumber(DriverGlobal *aDriverRecPtr, RegEntryID *regIDNub, RegEntryID *regIDDevice);
OSStatus FindParentID(RegEntryID* reg, RegEntryID* parent);
UInt32 FindDriverNodes(DriverGlobal *aDriverRecPtr, RegEntryID* reg, UInt32 level);
OSStatus DupProperty(DriverGlobal *aDriverRecPtr, const RegPropertyName *name);
UInt32 FindCardAddress(DriverGlobal *aDriverRecPtr, RegEntryID *reg);
UInt32 FindRegAddress(DriverGlobal *aDriverRecPtr, RegEntryID *reg);
OSStatus GetPCICardBaseAddress(RegEntryID *reg, UInt32 *cardAddr, UInt32 regNum, UInt32 *regAddr);
OSStatus InitTimingTable(DriverGlobal *aDriverRecPtr);
OSStatus InitTVParameters(DriverGlobal *aDriverRecPtr);
void InitPreferences(DriverGlobal *aDriverRecPtr);
OSStatus InitAddresses(DriverGlobal *aDriverRecPtr, UInt8 dispNum);
OSStatus InitConfiguration(DriverGlobal *aDriverRecPtr);
void GetDisplayProperties(DriverGlobal *aDriverRecPtr);
void SetUpBacklightProperties(DriverGlobal *aDriverRecPtr);

extern ConnectorInfo * FindConnectorInfo(DriverGlobal *aDriverRecPtr, UInt8 connection);
extern ConnectorInfo * FindConnectorInfoByType(DriverGlobal *aDriverRecPtr, UInt32 connectorType);
extern OSStatus CopyProperty(DriverGlobal *aDriverRecPtr, RegPropertyName *fromName, RegPropertyName *toName);
extern void InitPlatformInfo(DriverGlobal *aDriverRecPtr);
extern OSStatus InitHardware(DriverGlobal *aDriverRecPtr);
extern OSStatus DoNVPrefs(DriverGlobal *aDriverRecPtr, MVAD **mvad, bool yesNo, UInt32* data);
extern OSStatus SaveNVConfigure(DriverGlobal *aDriverRecPtr);
extern void SaveNVRAMDisplayParameters(DriverGlobal *aDriverRecPtr, NVRAMDisplayParameters *nvramParas);
extern bool FactorySetting(DriverGlobal *aDriverRecPtr);
extern void InitScalerInfo(DriverGlobal *aDriverRecPtr);
extern bool InitSurfaceInfo(DriverGlobal *aDriverRecPtr);
extern OSStatus CheckLoadEDIDBlock(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 ddcBlockNumber, UInt8* edid);
extern void CheckLVDSConnection(DriverGlobal *aDriverRecPtr, UInt32 *lvds, UInt8 value);
extern OSStatus DoDDCForceRead(EdidInfo *edidInfo, DriverGlobal *aDriverRecPtr, UInt8 value, UInt8 connection, UInt8 some_connection, UInt8 connectedFlags);
extern void InitConnections(DriverGlobal *aDriverRecPtr);
extern OSStatus GetEDIDProperties(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt32 *edidP);
extern bool EdidDigital(DriverGlobal *aDriverRecPtr, UInt8 *edid);
extern bool EDID_StoreDetailTiming(DriverGlobal *aDriverRecPtr, CrtcValues *mode, UInt8* edidDT, bool isDigital, UInt8 connection);

//#ifdef ATY_Prionace
void InitBootOptions(DriverGlobal *aDriverRecPtr);
//#endif
#endif