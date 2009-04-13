#include "ATY_HD.h"

extern "C" IOReturn _IONDRVLibrariesMappingInitialize( IOService * provider );
extern "C" IOReturn _IONDRVLibrariesInitialize( IOService * provider );
extern "C" IOReturn _IONDRVLibrariesFinalize( IOService * provider );
extern OSStatus DoDriverIO(DriverGlobal *aDriverRecPtr, int index, UInt32 cmdID, void *contents, UInt32 cmdCode, UInt32 cmdKind);

#define kFrameBuffer			0
#define kIORegister				1
#define kVideoRom				2
#define kFrameBufferReserved	3	//in fact _BAR[3] = _BAR[0]

#undef super
#define super IONDRVFramebuffer

OSDefineMetaClassAndStructors(ATY_HD, IONDRVFramebuffer)

bool ATY_HD::PCIRangeIndexAlloc(UInt8 index, UInt32 bar) {
	_BAR[index] = bar;
	return true;
}

void ATY_HD::PCIRangeMapFree(UInt8 index) {
	if (_map[index] != NULL) {
		_map[index]->release();
		_map[index] = NULL;
	}
	if (_mem[index] != NULL) {
		_mem[index]->release();
		_mem[index] = NULL;
	}
}

void ATY_HD::PCIRangeIndexFree(UInt8 index) {
	if (_map[index] != NULL) PCIRangeMapFree(index);
	_BAR[index] = 0;
}

IOVirtualAddress ATY_HD::PCIRangeMapGet(UInt8 index, UInt32 *bar, IOPhysicalAddress *addr) {
	if (bar != NULL) *bar = _BAR[index];
	if (_map[index] != NULL) {
		if (addr != NULL) *addr = _map[index]->getPhysicalAddress();
		return _map[index]->getVirtualAddress();
	}
	if (addr != NULL) addr = NULL;
	return 0;
}

IOPhysicalAddress ATY_HD::PCIRangeMapGetPhysicalRange(UInt8 index, UInt32 *bar, IOMemoryMap** map) {
	if (_BAR[index] == 0) return 0;
	IODeviceMemory * mem = aPCIDevice->getDeviceMemoryWithRegister(_BAR[index]);
	if (bar != NULL) *bar = _BAR[index];
	if (mem != NULL) {
		if (map != NULL) *map = mem->map();
		return mem->getPhysicalAddress();
	}
	if (map != NULL) map = NULL;
	return 0;
}


IOVirtualAddress ATY_HD::PCIRangeMapAllocSubRange(UInt8 index, IOPhysicalAddress offset, UInt32 length) {
	if (_BAR[index] = 0) return 0;
	PCIRangeMapFree(index);
	if (length == 0) return 0;
	IODeviceMemory* mem = aPCIDevice->getDeviceMemoryWithRegister(_BAR[index]);
	if (mem == NULL) return 0;
	_mem[index] = IODeviceMemory::withSubRange(mem, offset, length);
	if (_mem[index] == NULL) return 0;
	_map[index] = _mem[index]->map(0x101);
	if (_map[index] != NULL) return _map[index]->getVirtualAddress();
	PCIRangeMapFree(index);
	return 0;
}


IOVirtualAddress ATY_HD::PCIRangeMapAlloc(UInt8 index, IOPhysicalAddress *addr) {
	if (_BAR[index] == 0) return 0;
	PCIRangeMapFree(index);
	_map[index] = aPCIDevice->mapDeviceMemoryWithRegister(_BAR[index], 0x100);
	return PCIRangeMapGet(index, NULL, addr);
}

void ATY_HD::PCIRangeMapTableInit(void) {
	int i;
	for (i = 0;i < 16;i++) {
		_map[i] = NULL;
		_BAR[i] = 0;
		_mem[i] = NULL;
	}
}

void ATY_HD::PCIRangeMapTableFree(void) {
	int i;
	for (i = 0;i < 16;i++) PCIRangeIndexFree(i);
}

IOService * ATY_HD::probe(IOService * provider, SInt32 * score) {
	if (OSDynamicCast(IOPCIDevice, provider) == NULL) return this;	//provider is IONDRVDevice, we matched and will driver it

	//if provider is IOPCIDevice, we won't match but let's inject the entries
	if (!provider->getProperty(kPropertyAAPLAddress)) _IONDRVLibrariesMappingInitialize(provider);	//build "AAPL,address" property
	if (provider->getProperty("@0,name")) return NULL;				//already injected, no need to do it again
	
	LOG("Start inject entries:\n", true);							//Thanks natit for codes below, all copied from him
	
	OSDictionary *entriesToAdd = NULL;
	OSString *tmpString = 0;
	OSNumber *tmpNumber = 0;
	OSData   *tmpData = 0;
	OSData   *tmpObj = 0;
	
	UInt32 tmpUI32;
	
	OSIterator *iter = 0;
	const OSSymbol *dictKey = 0;
	
	entriesToAdd = OSDynamicCast(OSDictionary, getProperty("entriesToAdd"));
	if (!(entriesToAdd)) {
		LOG("Required dictionaries not found in plist\n", true);
		return NULL;
	}
	
	iter = OSCollectionIterator::withCollection(entriesToAdd);
	if (iter) {
		while ((dictKey = (const OSSymbol *)iter->getNextObject())) {
			tmpObj = 0;
			
			tmpString = OSDynamicCast(OSString, entriesToAdd->getObject(dictKey));
			if (tmpString) {
				LOG2("Setting %s=%s\n", dictKey->getCStringNoCopy(), tmpString->getCStringNoCopy(), true);
				tmpObj = OSData::withBytes(tmpString->getCStringNoCopy(), tmpString->getLength()+1);
			}
			
			tmpNumber = OSDynamicCast(OSNumber, entriesToAdd->getObject(dictKey));
			if (tmpNumber) {
				LOG2("Setting %s=%#010x\n", dictKey->getCStringNoCopy(), tmpNumber->unsigned32BitValue(), true);
				tmpUI32 = tmpNumber->unsigned32BitValue();
				tmpObj = OSData::withBytes(&tmpUI32, sizeof(UInt32));
			}
			
			tmpData = OSDynamicCast(OSData, entriesToAdd->getObject(dictKey));
			if (tmpData) {
				LOG1("Setting %s=<data not shown>\n", dictKey->getCStringNoCopy(), true);
				tmpObj = tmpData;
			}
			
			if (tmpObj) {
				provider->setProperty(dictKey, tmpObj);
			}
		}
	}
	return NULL;	//job already complete, no need to go further
}

bool ATY_HD::start(IOService * provider) {
	aDriverRecPtr = (DriverGlobal *) IOMalloc(sizeof(DriverGlobal));
	if (aDriverRecPtr == NULL) {
		IOLog(" ATICLASS::Start Cannot allocate globals");
		return false;
	}
	bzero(aDriverRecPtr, sizeof(DriverGlobal));
	aDriverRecPtr->aProvider = provider;
	aDriverRecPtr->aDriver = this;
	aPCIDevice = OSDynamicCast(IOPCIDevice, provider);
	if (aPCIDevice == NULL) aPCIDevice = OSDynamicCast(IOPCIDevice, provider->getProvider());
	if (aPCIDevice == NULL) {
		IOLog("ATICLASS::Start IOPCIDevice not found");
		return false;
	}
	aPCIDevice->setMemoryEnable(true);
	PCIRangeMapTableInit();
	return super::start(provider);
}

bool ATIPCIRangeIndexAlloc(ATY_HD *aDriver, UInt8 index, UInt32 bar) {
	return aDriver->PCIRangeIndexAlloc(index, bar);
}


IOPhysicalAddress ATIPCIRangeMapGetPhysicalRange(ATY_HD *aDriver, UInt8 index, UInt32 *bar, IOMemoryMap **map) {
	return aDriver->PCIRangeMapGetPhysicalRange(index, bar, map);
}

IOVirtualAddress ATIPCIRangeMapAlloc(ATY_HD *aDriver, UInt8 index, IOPhysicalAddress *addr) {
	return aDriver->PCIRangeMapAlloc(index, addr);
}


IOVirtualAddress ATIPCIRangeMapAllocSubRange(ATY_HD *aDriver, UInt8 index, IOPhysicalAddress offset, UInt32 length) {
	return aDriver->PCIRangeMapAllocSubRange(index, offset, length);
}


IOVirtualAddress ATIMapRegisters(ATY_HD *aDriver) {
	return ATIPCIRangeMapAlloc(aDriver, kIORegister, NULL);
}

IOVirtualAddress ATIMapRom(ATY_HD *aDriver) {
	return ATIPCIRangeMapAlloc(aDriver, kVideoRom, NULL);
}

IOVirtualAddress ATIMapFrameBuffer(ATY_HD *aDriver, IOPhysicalAddress offset, UInt32 length) {
	return ATIPCIRangeMapAllocSubRange(aDriver, kFrameBuffer, offset, length);
}

IOVirtualAddress ATIMapFrameBufferReserved(ATY_HD *aDriver, IOPhysicalAddress offset, UInt32 length) {
	return ATIPCIRangeMapAllocSubRange(aDriver, kFrameBufferReserved, offset, length);		//kIOPCIConfigBaseAddress0, 3 is the same as 0
}

void ATIRequireMaxBusStall(ATY_HD *aDriver, UInt32 ns) {
	aDriver->requireMaxBusStall(ns);
}

void ATY_HD::RedirectVRAM(UInt8 redirect) {
	aMemory = getVRAMRange();
	if (aMemory == NULL) return;
	aMemory->redirect(kernel_task, (redirect != 0));
	aMemory->release();
}

void ATIRedirectVRAM(ATY_HD *aDriver, UInt8 redirect) {
	aDriver->RedirectVRAM(redirect);
}

IOReturn ATY_HD::doDriverIO( UInt32 commandID, void * contents,
							UInt32 commandCode, UInt32 commandKind ) {
#ifdef	ATY_HD_DEBUG
	const char cmdName[11][35] = {
		"kIONDRVOpenCommand         ",
		"kIONDRVCloseCommand        ",
		"kIONDRVReadCommand         ",
		"kIONDRVWriteCommand        ",
		"kIONDRVControlCommand      ",
		"kIONDRVStatusCommand       ",
		"kIONDRVKillIOCommand       ",
		"kIONDRVInitializeCommand   ",
		"kIONDRVFinalizeCommand     ",
		"kIONDRVReplaceCommand      ",
		"kIONDRVSupersededCommand   "
	};
#endif
	
	IOReturn ret = kIOReturnSuccess;
	OSArray *properties;
	OSDictionary *dict;
	OSObject *obj;
	OSString *key, *str;
	OSData *data;
	OSNumber *num;
	DriverInitInfo info;
	void *_contents;
	int i;
	
	if ((commandCode - kIONDRVOpenCommand) < 11) {
		LOG1("%s", cmdName[commandCode - kIONDRVOpenCommand], true);
		switch (commandCode) {
			case kIONDRVInitializeCommand:
			case kIONDRVReplaceCommand:
				properties = OSDynamicCast(OSArray, getProperty("aty_properties"));
				if (properties != NULL) {
					i = 0;
					while (properties->getCapacity() > i) {
						dict = OSDynamicCast(OSDictionary, properties->getObject(i));
						if (dict != NULL) {
							obj = OSDynamicCast(OSObject, dict->getObject("value"));
							key = OSDynamicCast(OSString, dict->getObject("key"));
							data = OSDynamicCast(OSData, obj);
							if (data == NULL) {
								num = OSDynamicCast(OSNumber, obj);
								if ( num != NULL) {
									unsigned long long data64 = num->unsigned64BitValue();
									data = OSData::withBytes(&data64, 8);
								} else {
									str = OSDynamicCast(OSString, obj);
									if (str != NULL)
										data = OSData::withBytes(str->getCStringNoCopy(), str->getLength());
								}
							}
							if (key != NULL && data != NULL)
								getProvider()->setProperty(key, data);
						}
						i++;
					}
				}
				ret = _IONDRVLibrariesInitialize(getProvider());
				if (ret != kIOReturnSuccess) break;
				
			case kIONDRVFinalizeCommand:
			case kIONDRVSupersededCommand:
				data = OSDynamicCast(OSData, getProvider()->getProperty(kAAPLRegEntryIDKey));
				if (data == NULL) {
					ret = kIOReturnBadArgument;
					break;
				}
				info.refNum = (UInt32) this & 0xFFFF;	//not clear what this mean
				RegEntryID *regID = (RegEntryID *) data->getBytesNoCopy();
				REG_COPY(&info.deviceEntry, regID);
				contents = &info;
				break;
				
			default:
				_contents = contents;
				break;
		}
    }
	if (ret == kIOReturnSuccess) {
		if ((commandCode == kIONDRVInitializeCommand) || (commandCode == kIONDRVReplaceCommand)) {
			ret = DoDriverIO(aDriverRecPtr, 0, commandID, contents, commandCode, commandKind);
			setName("ATY_HD");
		} else {
			OSIncrementAtomic( &ndrvEnter );
			ret = DoDriverIO(aDriverRecPtr, 0, commandID, contents, commandCode, commandKind);
			OSDecrementAtomic( &ndrvEnter );
		}
	}
	if (commandCode == kIONDRVFinalizeCommand) {
		if (aDriverRecPtr->aShare != NULL) aDriverRecPtr->aShare->aDriverRecPtrs[aDriverRecPtr->dispNum] = NULL;
		PCIRangeMapTableFree();
		IOFree(aDriverRecPtr, sizeof(DriverGlobal));
		aDriverRecPtr = NULL;
	}
	LOG1("returned %d\n", ret, false);
	return ret;
}

void ATIEventPowerPlayChange(ATY_HD *aDriver, bool willChange) {
	IOIndex event = kIOFBNotifyDidChangeSpeed;
	if (willChange) event = kIOFBNotifyWillChangeSpeed;
	aDriver->handleEvent(event, NULL);
}

void ATIEventDisplayChange(ATY_HD *aDriver, bool willChange) {
	IOIndex event = kIOFBNotifyDisplayModeDidChange;
	if (willChange) event = kIOFBNotifyDisplayModeWillChange;
	aDriver->handleEvent(event, NULL);
}

