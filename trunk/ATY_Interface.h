#ifndef _ATY_HD_INTERFACE_H
#define _ATY_HD_INTERFACE_H

#include <IOKit/IOUserClient.h>
#include "ATY_HD.h"

class ATY_HD_User:public IOUserClient {
	
	friend class ATY_HD_Services;
	
	OSDeclareAbstractStructors(ATY_HD_User)

protected:
	task *owningTask;						//78
	const IOExternalMethod *methodTemplate;	//7C
	ATY_HD_Services *aService;				//80
	ATY_HD *aDriver;						//84
	
public:
	virtual bool start(IOService * provider);
	virtual void stop(IOService * provider);
	virtual IOReturn clientClose(void);
	virtual IOExternalMethod* getTargetAndMethodForIndex(IOService **target, UInt32 index);
	IOReturn do_ndrv_status(void *contents, void *params, UInt32 inCount, UInt32 *outCount);
	IOReturn do_ndrv_control(void *contents, void *params, UInt32 inCount, UInt32 *outCount);
};

class AMDNDRVService:public IOService {		//this is an abstract class used only for matching purpose
	OSDeclareAbstractStructors(AMDNDRVService)
};
	
class ATY_HD_Services:public AMDNDRVService {

	friend class ATY_HD_User;
	
	OSDeclareAbstractStructors(ATY_HD_Services)

protected:
	ATY_HD *aDriver;						//50

public:
	virtual IOService * ATY_HD_Services::probe(IOService * provider, SInt32 * score);
	virtual bool ATY_HD_Services::start(IOService * provider);
	virtual bool ATY_HD_Services::init(OSDictionary * dictionary);
	virtual IOReturn ATY_HD_Services::newUserClient(task * owningTask, void *securityID, UInt32 type, IOUserClient ** handler);
	IOReturn ATY_HD_Services::display_change_handler(void *, IOFramebuffer *, long val1, void *);
	virtual void ATY_HD_Services::stop(IOService * provider);
	virtual void ATY_HD_Services::free(void);
};

#endif