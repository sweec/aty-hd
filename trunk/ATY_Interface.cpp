#include "ATY_Interface.h"

/***			AMDNDRVService			***/
OSDefineMetaClassAndStructors(AMDNDRVService, IOService)

/***			ATY_HD_Services			***/
#undef super
#define super IOService

OSDefineMetaClassAndStructors(ATY_HD_Services, AMDNDRVService)

IOService * ATY_HD_Services::probe(IOService * provider, SInt32 * score) {
	return super::probe(provider, score);
}

bool ATY_HD_Services::start(IOService * provider) {
	aDriver = NULL;
	if (!super::start(provider)) return false;
	aDriver = OSDynamicCast(ATY_HD, provider);
	if (aDriver == NULL) return false;
	setName("AMDNDRVService", 0);
	registerService(0);
	return true;
}

bool ATY_HD_Services::init(OSDictionary * dictionary) {
	return super::init(dictionary);
}

IOReturn ATY_HD_Services::newUserClient(task * owningTask, void *securityID, UInt32 type, IOUserClient ** handler) {
	ATY_HD_User* WormyUser = IONew(ATY_HD_User, 1);
	if (WormyUser == NULL) return kIOReturnInternalError;
	WormyUser->owningTask = owningTask;
	bool initFailed;
	if (!WormyUser->init() || !WormyUser->attach(this) || !WormyUser->start(this)) initFailed = true;
	else initFailed = false;
	if (initFailed) {
		WormyUser->detach(this);
		WormyUser->release();
		WormyUser = NULL;
		return kIOReturnInternalError;
	}
	*handler = WormyUser;
	return kIOReturnSuccess;
}

IOReturn ATY_HD_Services::display_change_handler(void *, IOFramebuffer *, SInt32 some_value, void *) {
	if ((some_value <= 4) && ((1 << some_value) & ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4)))) return kIOReturnSuccess;
	return kIOReturnError;
}

void ATY_HD_Services::stop(IOService * provider) {
	super::stop(provider);
}

void ATY_HD_Services::free(void) {
	super::free();
}

typedef struct {
	UInt32			_reserved;				//0
	UInt32			code;					//4
	void			*params;				//8
} NDRV_PARAMETERS;

/***			ATY_HD_User			***/
#undef super
#define super IOUserClient

OSDefineMetaClassAndStructors(ATY_HD_User, IOUserClient)

bool ATY_HD_User::start(IOService * provider) {
	enum { kMethodObjectThis = 0, kMethodObjectOwner };
	
    static const IOExternalMethod methodDescs[2] =
    {
	/* 0 */  { (IOService*) kMethodObjectThis, (IOMethod) &ATY_HD_User::do_ndrv_status, kIOUCStructIStructO, kIOUCVariableStructureSize, kIOUCVariableStructureSize },
	/* 1 */  { (IOService*) kMethodObjectThis, (IOMethod) &ATY_HD_User::do_ndrv_control, kIOUCStructIStructO, kIOUCVariableStructureSize, kIOUCVariableStructureSize },
	};
	    
	aService = NULL;
	aDriver = NULL;
	if (!super::start(provider)) return false;
	aService = OSDynamicCast(ATY_HD_Services, provider);
	if (aService == NULL) return false;
	aDriver = aService->aDriver;
	if (aDriver == NULL) return false;
	setName("AMDNDRVUser", 0);
	methodTemplate = methodDescs;
	return true;
}

void ATY_HD_User::stop(IOService * provider) {
	super::stop(provider);
}

IOReturn ATY_HD_User::clientClose(void) {
	if (aService != NULL) {
		stop(aService);
		detach(aService);
	}
	return kIOReturnSuccess;
}

IOExternalMethod * ATY_HD_User::getTargetAndMethodForIndex(IOService **target, UInt32 index) {
	*target = this;
	if (index <= 1) return (IOExternalMethod *) &methodTemplate[index];		//each size 24bytes
	return NULL;
}

IOReturn ATY_HD_User::do_ndrv_control(void *contents, void *params, UInt32 inCount, UInt32 *outCount) {
	IOReturn ret = kIOReturnNoDevice;
	NDRV_PARAMETERS *content = (NDRV_PARAMETERS *) contents;
	int i = 0;
	UInt8 *inParams = (UInt8 *) (content->params);
	UInt8 *outParams = (UInt8 *) params;
	if (aDriver != NULL) {
		for (i = 0;i < *outCount;i++) outParams[i] = inParams[i];
		ret = aDriver->doControl(content->code, params);
	}
	return ret;
}

IOReturn ATY_HD_User::do_ndrv_status(void *contents, void *params, UInt32 inCount, UInt32 *outCount) {
	IOReturn ret = kIOReturnNoDevice;
	NDRV_PARAMETERS *content = (NDRV_PARAMETERS *) contents;
	int i = 0;
	UInt8 *inParams = (UInt8 *) (content->params);
	UInt8 *outParams = (UInt8 *)params;
	if (aDriver != NULL) {
		for (i = 0;i < *outCount;i++) outParams[i] = inParams[i];
		ret = aDriver->doStatus(content->code, params);
	}
	return ret;
}
