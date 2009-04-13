#ifndef _ATY_HD_H
#define _ATY_HD_H

#include "ATY_Struct.h"
#include <IOKit/pci/IOPCIDevice.h>

class ATY_HD_Services;
class ATY_HD_User;

class ATY_HD:public IONDRVFramebuffer {
	
    friend class ATY_HD_Services;
    friend class ATY_HD_User;
	friend void ATIEventPowerPlayChange(ATY_HD *aDriver, bool willChange);
	friend void ATIEventDisplayChange(ATY_HD *aDriver, bool willChange);
	
	OSDeclareAbstractStructors(ATY_HD)
	
protected:
	IODeviceMemory		*aMemory;				//200
	DriverGlobal		*aDriverRecPtr;			//204
	IOPCIDevice			*aPCIDevice;			//214
	IODeviceMemory		*_mem[16];				//218
	IOMemoryMap			*_map[16];				//258
	UInt32				_BAR[16];				//298
	
public:
	bool PCIRangeIndexAlloc(UInt8 index, UInt32 bar);
	void PCIRangeMapFree(UInt8 index);
	void PCIRangeIndexFree(UInt8 index);
	IOVirtualAddress PCIRangeMapGet(UInt8 index, UInt32 *bar, IOPhysicalAddress *addr);
	IOPhysicalAddress PCIRangeMapGetPhysicalRange(UInt8 index, UInt32 *bar, IOMemoryMap** map);
	IOVirtualAddress PCIRangeMapAllocSubRange(UInt8 index, IOPhysicalAddress offset, UInt32 length);
	IOVirtualAddress PCIRangeMapAlloc(UInt8 index, IOPhysicalAddress *addr);
	void PCIRangeMapTableInit(void);
	void PCIRangeMapTableFree(void);
	virtual IOService * probe(IOService * provider, SInt32 * score);
	virtual bool start(IOService * provider);
	void RedirectVRAM(UInt8 redirect);
    virtual IOReturn doDriverIO( UInt32 commandID, void * contents, UInt32 commandCode, UInt32 commandKind );
};

#endif	//ATY_HD