/*
 *  ATY_HW.cpp
 *  ATY_HD
 *
 *  Created by Dong Luo on 3/11/09.
 *  Copyright 2009 Boston University. All rights reserved.
 *
 */
#include "ATY_Driver.h"
#include "ATY_HW.h"

#define ABS(a) (((a) > 0)?(a):(-(a)))
#define upDIV(a, b) ABS((a) / (b))
#define signedShift(src, bit, des) do {(*(des)) = (src) + ((src) >> 31);if ((*(des)) & (1 << 31)) (*(des)) = ((*(des)) >> (bit)) + (1 << 31); else (*(des)) >>= (bit);} while (0)

extern OSStatus SetupLM63FanCntrlLookUP(DriverGlobal *aDriverRecPtr);

bool HWIsCRTCDisplayOn(DriverGlobal *aDriverRecPtr, UInt8 dispNum) {
	if (dispNum == 0) {
		if (regr32(aDriverRecPtr, 0x6080) & 1) return true;	//D1CRTC_CONTROL
		else return false;
	}
	if (regr32(aDriverRecPtr, 0x6880) & 1) return true;		//D2CRTC_CONTROL
	else return false;
}

UInt32 PLL_FB_DIV_Calc(DriverGlobal *aDriverRecPtr, UInt32 clock, UInt32 clkRatio, UInt32 unknown1, UInt32 unknown2, UInt32 aClock) {
	if ((aClock != 0) || (unknown1 != 0)) return (clock * clkRatio * aClock * 2 / unknown2 / unknown1 + 1) / 2;
	return 0;
}

UInt32 PLL_Freq(DriverGlobal *aDriverRecPtr, UInt32 aRatio, UInt32 pfd, UInt32 unknown2, UInt32 unknown4, UInt32 aClock)  {
	if ((aClock != 0) || (aRatio != 0)) return (unknown4 * pfd * unknown2 / aClock / aRatio);
	return 0;
}

UInt32 RF_VCLKmaxPNM(DriverGlobal *aDriverRecPtr, UInt32 clock, UInt32* newRatio, UInt32 *newPfd, UInt32* newClock, UInt8 dispNum, UInt8 connection) {
	int step = 1;					//var_4C
	UInt32 inClk = clock * 100;		//var_2C
	UInt32 minClk = 10;				//var_24
	UInt32 minClkTarget = 90000;	//var_20
	UInt32 maxClkTarget = 110000;	//var_1C
	bool goOut = false;				//var_16
	bool done = false;				//var_15
	
	if (clock == 0) return 0;
	minClk = upDIV(clock, 500);
#ifdef ATY_Wormy
	if (connection & (1 << 1)) minClk = 0;	//TV?
	if (connection & (1 << 6)) {			//LVDS?
		minClkTarget = 90000;
		maxClkTarget = 110000;
		minClkTarget = 80000;
		maxClkTarget = 100000;
		step = -1;
	}
#endif
	UInt32 minClkRatio = minClkTarget / clock;		//var_3C
	if (minClkTarget % clock != 0) minClkRatio++;
	UInt32 maxClkRatio = maxClkTarget / clock;		//var_38
	if (minClkRatio < 2) minClkRatio = 2;			
	if (maxClkRatio < 2) maxClkRatio = 2;
	if (minClkRatio > 127) minClkRatio = 127;
	if (maxClkRatio > 127) maxClkRatio = 127;
	if (minClkRatio > maxClkRatio) minClkRatio = maxClkRatio;
	UInt32 high = 27;					//var_40
	UInt32 low = 1;						//var_44
	if (high < 1) high = 1;
	if (low < 1) low = 1;
	if (high > 255) high = 255;
	if (low > 255) low = 255;
	if (low > high) low = high;
	UInt32 aClock, aRatio;					//var_44/var_50, var_48/var_54
	for (aClock = (step < 0)?high:low;((aClock <= high) && (aClock >= low));aClock += step) {
		for  (aRatio = minClkRatio;((aRatio <= maxClkRatio) && (aRatio >= minClkRatio));aRatio++) {
			UInt32 pfd = PLL_FB_DIV_Calc(aDriverRecPtr, clock * 10, aRatio, 1, 2700, aClock);	//var_48
			pfd = upDIV((pfd + 5), 10);
			pfd *= 10;
			if (pfd <= 19 || pfd > 327670) continue;
			int temp;
			temp = pfd / 10;
			if (temp < 0) temp++;
			if (temp < (20 - 2 * (pfd - temp * 10))) continue;
			UInt32 clock1 = pfd * 270 / aClock;		//var_34
			goOut = false;
#ifdef ATY_Wormy
			if (connection & (1 << 6)) {
				UInt32 min1 = (UInt32) ((float) pfd * 75 / 100000 / 1.9);
				UInt32 max1 = (UInt32) ((float) pfd * 75 / 100000 / 0.1);
				if (min1 < 4) min1 = 4;
				if (max1 > 63) max1 = 63;
				if (min1 > 63) goOut = true;
				if (max1 < 4) goOut = true;
				UInt32 minMax = (13500 / aClock / min1 / 30 + 1) / 2;
				if (minMax <= 1) goOut = true;
				if (minMax > 63) goOut = true;
				minMax = (13500 / aClock / max1 / 50 + 1) / 2;
				if (minMax <= 1) goOut = true;
				if (minMax > 63) goOut = true;
			}
#endif
			if (!done || !goOut) {
				done = true;
				UInt32 clock2 = ABS(clock1 / aRatio - clock);	//var_28
				if (clock2 >= inClk) continue;
				if (!goOut) inClk = clock2;
				*newPfd = pfd;
				*newRatio = aRatio;
				*newClock = aClock;
				if (!goOut && (inClk < minClk)) break;
			}
		}
		if (!goOut && (inClk <= minClk)) break;
	}
	UInt32 pFreq = PLL_Freq(aDriverRecPtr, *newRatio, *newPfd, 1, 2700, *newClock);	//var_6C
	return upDIV(pFreq, 10);
}

UInt32 CalculateVClk(DriverGlobal *aDriverRecPtr, UInt32 clock, UInt8 dispNum, UInt8 connection) {
	UInt32 aRatio, pfd, aClock;
	return RF_VCLKmaxPNM(aDriverRecPtr, clock, &aRatio, &pfd, &aClock, dispNum, connection);
}

void WaitForVBL(DriverGlobal *aDriverRecPtr, UInt8 dispNum, bool entered) {
	UInt32 addr;
	AbsoluteTime oldTime, newTime;
	Nanoseconds nanoTime;
	
	if (dispNum > 2) return;
	if (!HWIsCRTCDisplayOn(aDriverRecPtr, dispNum)) return;
	if (dispNum == 0) addr = 0x609C;				//D1CRTC_STATUS
	else addr = 0x689C;								//D2CRTC_STATUS
	if (!entered) {	//enter first
		oldTime = UpTime();
		while(regr32(aDriverRecPtr, addr) & 1) {
			newTime = UpTime();
			nanoTime = AbsoluteToNanoseconds(SubAbsoluteFromAbsolute(newTime, oldTime));
			if ((nanoTime.hi > 0) || (nanoTime.lo > 80000000)) break;	//80ms
		}
	}				//then leave
	oldTime = UpTime();
	while(!(regr32(aDriverRecPtr, addr) & 1)) {
		newTime = UpTime();
		nanoTime = AbsoluteToNanoseconds(SubAbsoluteFromAbsolute(newTime, oldTime));
		if ((nanoTime.hi > 0) || (nanoTime.lo > 80000000)) break;	//80ms
	}
}

UInt32 HW_ProgramVclk(DriverGlobal *aDriverRecPtr, UInt32 val1, UInt32 clock) {
	UInt32 pfd = 0;
	UInt32 aRatio, aClock;
	UInt32 p1pll_cntl = regr32(aDriverRecPtr, 0x450);	//P1PLL_CNTL var_28
	UInt32 e1ppd = regr32(aDriverRecPtr, 0x43C);		//EXT1_PPLL_POST_DIV var_14
	UInt32 e1pfd = regr32(aDriverRecPtr, 0x430);		//EXT1_PPLL_FB_DIV var_24
	UInt32 newPfd = e1pfd;	//var_20
	UInt32 e1prd = regr32(aDriverRecPtr, 0x404);		//EXT1_PPLL_REF_DIV var_1C
	UInt32 newPrd = e1prd;	//var_18
	UInt32 ret = RF_VCLKmaxPNM(aDriverRecPtr, clock, &aRatio, &pfd, &aClock, 1, aDriverRecPtr->connectorFlags);
	aDriverRecPtr->unknown39 = pfd;
	aDriverRecPtr->unknown40 = aClock;
	aDriverRecPtr->unknown41 = aRatio;
	regw32(aDriverRecPtr, 0x400, 1);			//EXT1_PPLL_REF_DIV_SRC
	newPrd = newPrd & 0xFFFFFC00 | aClock;
	if ((pfd % 10) != 0)
		newPfd = ((pfd / 10 + 1) << 16) | (newPfd & 0xF800FFFF) & 0xFFFFFFF0 | (10 - pfd % 10);
	else
		newPfd = ((pfd / 10) << 16) | (newPfd & 0xF800FFFF) & 0xFFFFFFF0;
	e1ppd &= ~0x7F;
	e1ppd |= (aRatio & 0x7F);
	if ((newPfd != e1pfd) || (newPrd != e1prd) || (p1pll_cntl & 3)) {
		if (!(p1pll_cntl & 1)) {
			WaitForVBL(aDriverRecPtr, aDriverRecPtr->dispNum, 0);
			regw32(aDriverRecPtr, 0x438, 0);
			kdelay(-1);
			regw32(aDriverRecPtr, 0x450, p1pll_cntl | 1);
			kdelay(-5);
		}
		regw32(aDriverRecPtr, 0x404, newPrd);
		regw32(aDriverRecPtr, 0x430, newPfd);
		UInt16 i;
		if (p1pll_cntl & (1 << 1)) {
			p1pll_cntl &= ~(1 << 1);
			regw32(aDriverRecPtr, 0x450, p1pll_cntl);
			kdelay(50);
			p1pll_cntl &= ~1;
			regw32(aDriverRecPtr, 0x450, p1pll_cntl);
			kdelay(10);
			for(i = 0;i < 10000;i++) {
				kdelay(-10);
				if (regr32(aDriverRecPtr, 0x454) & (1 << 21)) break;
			}
			p1pll_cntl |= 1;
			regw32(aDriverRecPtr, 0x450, p1pll_cntl);
			kdelay(1);
		}
		for (i = 0;i < 10000;i++) {
			kdelay(-10);
			if (regr32(aDriverRecPtr, 0x454) & (1 << 21)) break;
		}
	}
	regw32(aDriverRecPtr, 0x43C, e1ppd);		//AVIVO_EXT1_PPLL_POST_DIV
	kdelay(-1);
	regw32(aDriverRecPtr, 0x438, 1);			//AVIVO_EXT1_PPLL_POST_DIV_SRC
	return ret;
}

UInt32 HW_ProgramV2clk(DriverGlobal *aDriverRecPtr, UInt32 val1, UInt32 clock) {
	UInt32 pfd = 0;
	UInt32 aRatio, aClock;
	UInt32 p2pll_cntl = regr32(aDriverRecPtr, 0x454);	//P2PLL_CNTL var_20
	UInt32 e2ppd = regr32(aDriverRecPtr, 0x444);		//EXT2_PPLL_POST_DIV var_C
	UInt32 e2pfd = regr32(aDriverRecPtr, 0x434);		//EXT2_PPLL_FB_DIV var_1C
	UInt32 newPfd = e2pfd;	//var_18
	UInt32 e2prd = regr32(aDriverRecPtr, 0x414);		//EXT2_PPLL_REF_DIV var_14
	UInt32 newPrd = e2prd;	//var_10
	UInt32 ret = RF_VCLKmaxPNM(aDriverRecPtr, clock, &aRatio, &pfd, &aClock, 1, aDriverRecPtr->connectorFlags);
	aDriverRecPtr->unknown39 = pfd;
	aDriverRecPtr->unknown40 = aClock;
	aDriverRecPtr->unknown41 = aRatio;
	regw32(aDriverRecPtr, 0x410, 1);			//EXT2_PPLL_REF_DIV_SRC
	newPrd = newPrd & 0xFFFFFC00 | aClock;
	if ((pfd % 10) != 0)
		newPfd = ((pfd / 10 + 1) << 16) | (newPfd & 0xF800FFFF) & 0xFFFFFFF0 | (10 - pfd % 10);
	else
		newPfd = ((pfd / 10) << 16) | (newPfd & 0xF800FFFF) & 0xFFFFFFF0;
	e2ppd &= ~0x7F;
	e2ppd |= (aRatio & 0x7F);
	if ((newPfd != e2pfd) || (newPrd != e2prd) || (p2pll_cntl & 3)) {
		if (!(p2pll_cntl & 1)) {
			WaitForVBL(aDriverRecPtr, aDriverRecPtr->dispNum, 0);
			regw32(aDriverRecPtr, 0x440, 0);
			kdelay(-1);
			regw32(aDriverRecPtr, 0x454, p2pll_cntl | 1);
			kdelay(-5);
		}
		regw32(aDriverRecPtr, 0x414, newPrd);
		regw32(aDriverRecPtr, 0x434, newPfd);
		UInt16 i;
		if (p2pll_cntl & (1 << 1)) {
			p2pll_cntl &= ~(1 << 1);
			regw32(aDriverRecPtr, 0x454, p2pll_cntl);
			kdelay(50);
			p2pll_cntl &= ~1;
			regw32(aDriverRecPtr, 0x450, p2pll_cntl);
			kdelay(10);
			for(i = 0;i < 10000;i++) {
				kdelay(-10);
				if (regr32(aDriverRecPtr, 0x454) & (1 << 21)) break;
			}
			p2pll_cntl |= 1;
			regw32(aDriverRecPtr, 0x454, p2pll_cntl);
		}
		for (i = 0;i < 10000;i++) {
			kdelay(-10);
			if (regr32(aDriverRecPtr, 0x454) & (1 << 21)) break;
		}
	}
	regw32(aDriverRecPtr, 0x444, e2ppd);		//AVIVO_EXT2_PPLL_POST_DIV
	kdelay(-1);
	regw32(aDriverRecPtr, 0x440, 1);			//AVIVO_EXT2_PPLL_POST_DIV_SRC
	return ret;
}

bool HWProgramCRTCDisplayOn(DriverGlobal *aDriverRecPtr, UInt8 dispNum) {
	UInt32 offset = 0;
	if (dispNum) offset = 0x800;
	regw32(aDriverRecPtr, 0x6008 + offset, 0x800000);
	regw32(aDriverRecPtr, 0x600C + offset, 0);
	regw32(aDriverRecPtr, 0x6004 + offset, 0xD803F8);
	regw32(aDriverRecPtr, 0x6000 + offset, 0x41F);
	regw32(aDriverRecPtr, 0x6028 + offset, 0x40000);
	regw32(aDriverRecPtr, 0x602C + offset, 0);
	regw32(aDriverRecPtr, 0x6024 + offset, 0x1B0273);
	regw32(aDriverRecPtr, 0x6020 + offset, 0x273);
	regw32(aDriverRecPtr, 0x6584 + offset, 0x3200258);
	regw32(aDriverRecPtr, 0x6134 + offset, 0x320);
	regw32(aDriverRecPtr, 0x6138 + offset, 600);
	regw32(aDriverRecPtr, 0x6120 + offset, 0x340);
	regw32(aDriverRecPtr, 0x6100 + offset, 1);
	regw32(aDriverRecPtr, 0x6104 + offset, 0x200002);
	if (dispNum == 0) {
		HW_ProgramVclk(aDriverRecPtr, 0, 4000);
		regw32(aDriverRecPtr, 0x480, 0);
	} else {
		HW_ProgramV2clk(aDriverRecPtr, 0, 0xFA0);
		regw32(aDriverRecPtr, 0x484, 0x10000);
	}
	regw32(aDriverRecPtr, 0x6080 + offset, 0x10001);
	IOSleep(20);
	return true;
}

void HWProgramCRTCBlank(DriverGlobal *aDriverRecPtr, UInt8 value, UInt8 dispNum) {
	WaitForVBL(aDriverRecPtr, dispNum, 0);
	//set bit8 according to value
	if (dispNum == 0) rmsw32(aDriverRecPtr, 0x6084, (1 << 8), 8, (value != 0));	//D1CRTC_BLANK_CONTROL
	else rmsw32(aDriverRecPtr, 0x6884, (1 << 8), 8, (value != 0));				//D2CRTC_BLANK_CONTROL
}

void Apple_GetHWSenseCode(DriverGlobal *aDriverRecPtr, UInt8 connectedFlags, UInt8* connectType, UInt8* connectData) {
	*connectType = 7;
	if (connectedFlags & (1 << 4)) *connectData = 0x17;
	else *connectData = 0x3F;
}

void Apple_GetHWSenseCode2(DriverGlobal *aDriverRecPtr, UInt8 connectedFlags, UInt8* connectType, UInt8* connectData) {
	*connectType = 7;
	if (connectedFlags & (1 << 5)) *connectData = 0x17;
	else *connectData = 0x3F;
}

bool HW_DAC1Sense(DriverGlobal *aDriverRecPtr) {
	UInt8 copySense = 0;
	UInt8 sense = 0;
	
	UInt32 dass = regr32(aDriverRecPtr, 0x7804);		//DACA_SOURCE_SELECT
	UInt32 dap = regr32(aDriverRecPtr, 0x7850);			//DACA_POWERDOWN
	UInt32 dac = regr32(aDriverRecPtr, 0x7854);			//DACA_CONTROL1
	UInt32 daac = regr32(aDriverRecPtr, 0x7828);		//DACA_AUTODETECT_CONTROL
	UInt8 dae = regr8(aDriverRecPtr, 0x7800);			//DACA_ENABLE
	UInt32 unknown1 = regr32(aDriverRecPtr, 0x4DC);
	UInt32 p1c = regr32(aDriverRecPtr, 0x450);			//P1PLL_CNTL
	UInt32 p2c = regr32(aDriverRecPtr, 0x454);			//P2PLL_CNTL
	if (p1c & 3) {
		HW_ProgramVclk(aDriverRecPtr, 0, 4000);
		regw32(aDriverRecPtr, 0x480, 0);
	}
	if (p2c & 3) {
		HW_ProgramV2clk(aDriverRecPtr, 0, 0xFA0);
		regw32(aDriverRecPtr, 0x484, 0x10000);
	}
	UInt32 daac2 = 0;
	if (unknown1 != 0) daac2 = 0x252 / unknown1;
	if (daac2 > 255) daac2 = 255;
	regw32(aDriverRecPtr, 0x782C, daac2);			//DACA_AUTODETECT_CONTROL2
	regw32(aDriverRecPtr, 0x7854, aDriverRecPtr->memConfig->atySettings.daca_cntl);
	regw32(aDriverRecPtr, 0x7850, 0);
	kdelay(10);
	regw8(aDriverRecPtr, 0x7800, 1);
	kdelay(50);
	regw8(aDriverRecPtr, 0x7838, 1);				//DACA_AUTODETECT_INT_CONTROL
	kdelay(1);
	regw8(aDriverRecPtr, 0x7828, 1);
	kdelay(10);
	UInt8 i;
	for (i = 0;i < 100;i++) {
		sense = regr32(aDriverRecPtr, 0x7834) & 0xFF;
		if ((sense & 0x2020200) || (sense != copySense)) {
			copySense = sense;
			i = 0;
		}
		kdelay(1);
	}
	regw32(aDriverRecPtr, 0x7828, daac);
	regw8(aDriverRecPtr, 0x7838, 1);
	kdelay(1);
	regw8(aDriverRecPtr, 0x7800, dae);
	regw32(aDriverRecPtr, 0x7850, dap);
	kdelay(50);
	regw8(aDriverRecPtr, 0x7854, dac);
	regw32(aDriverRecPtr, 0x7804, dass);
	regw32(aDriverRecPtr, 0x450, p1c);
	regw32(aDriverRecPtr, 0x454, p2c);
	return ((sense & 0x1010110) != 0);
}

UInt32 DAC2SenseResult(DriverGlobal *aDriverRecPtr, bool unknown1) {
	UInt32 copySense = 0;
	UInt32 sense = 0;
	
	UInt32 dbss = regr32(aDriverRecPtr, 0x7A04);		//DACB_SOURCE_SELECT
	UInt32 dbp = regr32(aDriverRecPtr, 0x7A50);			//DACB_POWERDOWN
	UInt32 dbc = regr32(aDriverRecPtr, 0x7A54);			//DACB_CONTROL1
	UInt32 dbac = regr32(aDriverRecPtr, 0x7A28);		//DACB_AUTODETECT_CONTROL
	UInt8 dbe = regr8(aDriverRecPtr, 0x7A00);			//DACB_ENABLE
	UInt32 data = regr32(aDriverRecPtr, 0x4DC);
	UInt32 p1c = regr32(aDriverRecPtr, 0x450);			//P1PLL_CNTL
	UInt32 p2c = regr32(aDriverRecPtr, 0x454);			//P2PLL_CNTL
	if (p1c & 3) {
		HW_ProgramVclk(aDriverRecPtr, 0, 4000);
		regw32(aDriverRecPtr, 0x480, 0);
	}
	if (p2c & 3) {
		HW_ProgramV2clk(aDriverRecPtr, 0, 0xFA0);
		regw32(aDriverRecPtr, 0x484, 0x10000);
	}
	UInt32 dbac2 = 0;
	if (data != 0) dbac2 = 0x252 / data;
	if (dbac2 > 255) dbac2 = 255;
	regw32(aDriverRecPtr, 0x7A2C, dbac2);			//DACB_AUTODETECT_CONTROL2
	if (unknown1) regw32(aDriverRecPtr, 0x7A58, 0);
	else regw32(aDriverRecPtr, 0x7A58, 0x100);
	regw32(aDriverRecPtr, 0x7A54, aDriverRecPtr->memConfig->atySettings.dacb_cntl);
	regw32(aDriverRecPtr, 0x7A50, 0);
	kdelay(10);
	regw8(aDriverRecPtr, 0x7A00, 1);
	kdelay(50);
	regw8(aDriverRecPtr, 0x7A38, 1);				//DACB_AUTODETECT_INT_CONTROL
	kdelay(1);
	regw8(aDriverRecPtr, 0x7A28, 1);
	kdelay(10);
	UInt8 i;
	for (i = 0;i < 100;i++) {
		sense = regr32(aDriverRecPtr, 0x7A34);
		if ((sense & 0x2020200) || (sense != copySense)) {
			copySense = sense;
			i = 0;
		}
		kdelay(1);
	}
	regw32(aDriverRecPtr, 0x7A28, dbac);
	regw8(aDriverRecPtr, 0x7A38, 1);
	kdelay(1);
	regw8(aDriverRecPtr, 0x7A00, dbe);
	regw32(aDriverRecPtr, 0x7A50, dbp);
	kdelay(50);
	regw8(aDriverRecPtr, 0x7A54, dbc);
	regw32(aDriverRecPtr, 0x7A04, dbss);
	regw32(aDriverRecPtr, 0x450, p1c);
	regw32(aDriverRecPtr, 0x454, p2c);
	return sense;
}

bool RF_DAC2Sense(DriverGlobal *aDriverRecPtr) {
	UInt32 sense = DAC2SenseResult(aDriverRecPtr, 1);
	return ((sense & 0x1010110) == 0x1010110);
}

UInt8 HW_DAC1State(DriverGlobal *aDriverRecPtr) {
	if (regr32(aDriverRecPtr, 0x7800) & 1) return (regr32(aDriverRecPtr, 0x7804) & 1 + 1);	//DACA_SOURCE_SELECT
	return 0;				//DACA_ENABLE
}

UInt8 HW_DAC2State(DriverGlobal *aDriverRecPtr) {
	UInt32 dss;
	if (regr32(aDriverRecPtr, 0x7A00) & 1) {	//DACB_ENABLE
		dss = regr32(aDriverRecPtr, 0x7A04);	//DACB_SOURCE_SELECT
		if ((dss & 3) == 2) dss = regr32(aDriverRecPtr, 0x60FC);
		return (dss & 1 + 1);
	}
	return 0;
}

bool HW_DAC1On(DriverGlobal *aDriverRecPtr) {
	if (!regr32(aDriverRecPtr, 0x7800)) return false;
	if (regr32(aDriverRecPtr, 0x7850) & 0x1010101) return false;
	return true;
}

bool HW_DAC2On(DriverGlobal *aDriverRecPtr) {
	if (!regr32(aDriverRecPtr, 0x7A00)) return false;
	if (regr32(aDriverRecPtr, 0x7A50) & 0x1010101) return false;
	return true;
}

void HWSetDAC1On(DriverGlobal *aDriverRecPtr, bool onOff) {
	regw32(aDriverRecPtr, 0x198, regr32(aDriverRecPtr, 0x198) & 0xFF00 | 2);		//GPIOPAD_MASK
	UInt32 data = 0;
	if (onOff) data = 0x200;
	regw32(aDriverRecPtr, 0x19C, regr32(aDriverRecPtr, 0x19C) & 0x00FD | data);		//GPIOPAD_A
	regw32(aDriverRecPtr, 0x1A0, regr32(aDriverRecPtr, 0x1A0) & 0xFF00 | 2);		//GPIOPAD_EN;
}

void HW_DAC1Blank(DriverGlobal *aDriverRecPtr, bool onOff) {
	regw32(aDriverRecPtr, 0x7840, 0);						//DACA_FORCE_DATA
	if (onOff) regw32(aDriverRecPtr, 0x783C, 0x701);		//DACA_FORCE_OUTPUT, 0x701: Enable synchronnous, all three channels
	else regw32(aDriverRecPtr, 0x783C, 0);
}

void HW_DAC1OnOff(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8 dispNum) {
	UInt32 powerDown = 0;	//0 disable;1 enable
	UInt32 source = 0;		//0 CRTC1;1 CRTC2;2 TV;3 RESERVED
	UInt32 onOff = 0;
	
	if (!(connection & (1 << 4))) {	//not CRT1 turn it off
		onOff &= ~1;					//bit0 = 0 means disable DACA
		powerDown |= 0x1010101;			//enable all power down
	} else {							//turn CRT1 on
		if (dispNum) source |= 1;
		onOff |= 1;						//bit0 = 1 means enable DACA
		HWSetDAC1On(aDriverRecPtr, 1);
	}
	regw32(aDriverRecPtr, 0x783C, 0);			//DACA_FORCE_OUTPUT
	regw32(aDriverRecPtr, 0x7850, powerDown);	//DACA_POWERDOWN
	regw32(aDriverRecPtr, 0x7804, source);		//DACA_SOURCE_SELECT
	regw32(aDriverRecPtr, 0x7854, aDriverRecPtr->memConfig->atySettings.daca_cntl);//DACA_CONTROL1
	regw32(aDriverRecPtr, 0x7800, onOff);		//DACA_ENABLE
}

void HW_SaveDAC2OnOff(DriverGlobal *aDriverRecPtr) {
	//not implemented in Wormy
}

void Program_DTO(DriverGlobal *aDriverRecPtr, UInt32 time) {
	//dissemle later, used by tv encoder
}

void TVEncoder_PAL(DriverGlobal *aDriverRecPtr) {
	regw32(aDriverRecPtr, 0x5F9C, 2);
	regw32(aDriverRecPtr, 0x5F98, 0x16B000A);
	regw32(aDriverRecPtr, 0x5E00, 1);
	regw32(aDriverRecPtr, 0x5E04, 0xD4C);
	regw32(aDriverRecPtr, 0x5E08, 0x70270);
	regw32(aDriverRecPtr, 0x5E14, 0);
	regw32(aDriverRecPtr, 0x5E18, 0x7026E);
	regw32(aDriverRecPtr, 0x5E1C, 0x613);
	regw32(aDriverRecPtr, 0x5E20, 0xFA0000);
	regw32(aDriverRecPtr, 0x5E24, 0x7D0000);
	regw32(aDriverRecPtr, 0x5E28, 0x72306A6);
	regw32(aDriverRecPtr, 0x5E2C, 0x5AC0000);
	regw32(aDriverRecPtr, 0x5E30, 0xC5206A6);
	regw32(aDriverRecPtr, 0x5E34, 0x784DD);
	regw32(aDriverRecPtr, 0x5E38, 0x7026C);
	regw32(aDriverRecPtr, 0x5E3C, 0x20000);
	regw32(aDriverRecPtr, 0x5E40, 0x28271);
	regw32(aDriverRecPtr, 0x5E44, 0x78012A);
	regw32(aDriverRecPtr, 0x5E48, 0x31340006);
	regw32(aDriverRecPtr, 0x5E4C, 0x26C013E);
	regw32(aDriverRecPtr, 0x5E50, 0xCFF022F);
	regw32(aDriverRecPtr, 0x5E54, 0x65608D5);
	regw32(aDriverRecPtr, 0x5E58, 0x135002D);
	regw32(aDriverRecPtr, 0x5E5C, 0x26E829E);
	regw32(aDriverRecPtr, 0x5E60, 0x6A4022D);
	regw32(aDriverRecPtr, 0x5E64, 0x135002C);
	regw32(aDriverRecPtr, 0x5E68, 0x26E029E);
	regw32(aDriverRecPtr, 0x5E8C, 0xD16022F);
	regw32(aDriverRecPtr, 0x5E90, 0x66D08D5);
	regw32(aDriverRecPtr, 0x5E94, 0x3EE01AD);
	regw32(aDriverRecPtr, 0x5E98, 0x2FF);
	regw32(aDriverRecPtr, 0x5E9C, 0x1D70150);
	regw32(aDriverRecPtr, 0x5EA0, 0x100012B);
	regw32(aDriverRecPtr, 0x5EA4, 0x1760108);
	regw32(aDriverRecPtr, 0x5EA8, 0xEB00EB);
	regw32(aDriverRecPtr, 0x5EAC, 0);
	regw32(aDriverRecPtr, 0x5EB0, 0x1110011);
	regw32(aDriverRecPtr, 0x5EB4, 0x1000401);
	regw32(aDriverRecPtr, 0x5EB8, 2);
	regw32(aDriverRecPtr, 0x5EBC, 0);
	regw32(aDriverRecPtr, 0x5EC0, 0);
	regw32(aDriverRecPtr, 0x5EC4, 0);
	regw32(aDriverRecPtr, 0x5EC8, 0x2107);
	regw32(aDriverRecPtr, 0x5ECC, 0);
	regw32(aDriverRecPtr, 0x5EF0, 0x844F01B2);
	regw32(aDriverRecPtr, 0x5EF4, 0x2078FC);
	regw32(aDriverRecPtr, 0x5EF8, 0x15555D38);
	regw32(aDriverRecPtr, 0x5EFC, 0x15555620);
	regw32(aDriverRecPtr, 0x5F00, 0xD4C);
	regw32(aDriverRecPtr, 0x5F04, 0x15555D38);
	regw32(aDriverRecPtr, 0x5F08, 0x155556EC);
	regw32(aDriverRecPtr, 0x5F0C, 0x802C0203);
	regw32(aDriverRecPtr, 0x5F10, 0x8C99A1E5);
	regw32(aDriverRecPtr, 0x5F14, 0x9620639);
	regw32(aDriverRecPtr, 0x5F18, 0xB713CE4);
	regw32(aDriverRecPtr, 0x5F1C, 0);
	regw32(aDriverRecPtr, 0x5F28, 0xFFFF);
	regw32(aDriverRecPtr, 0x5F90, 0x200);
	regw32(aDriverRecPtr, 0x5F8C, 0x20023E);
	regw32(aDriverRecPtr, 0x5F94, 0);
	regw32(aDriverRecPtr, 0x5DFC, 0xF01000C2);
	Program_DTO(aDriverRecPtr, 50);
}

void TVEncoder_NTSC(DriverGlobal *aDriverRecPtr) {
	//something similar to TVEncoder_PAL
}

void ProgramMVMode(DriverGlobal *aDriverRecPtr) {
	//something related to tv
}

void HW_DAC2OnOff(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8 dispNum) {
	UInt32 powerDown = 0;	//0 disable;1 enable
	UInt32 source = 0;		//0 CRTC1;1 CRTC2;2 TV;3 RESERVED
	UInt32 onOff = 0;
	UInt32 control1 = aDriverRecPtr->memConfig->atySettings.dacb_cntl;

	if (!(connection & (1 << 1))) {	//not TV turn it off
		regw32(aDriverRecPtr, 0x5DFC, regr32(aDriverRecPtr, 0x5DFC) | 1);
		regw32(aDriverRecPtr, 0x5E00, regr32(aDriverRecPtr, 0x5E00) & ~1);
		regw32(aDriverRecPtr, 0x4F4, regr32(aDriverRecPtr, 0x4F4) & ~(1 << 31));
	}
	if (!(connection & ((1 << 1) | (1 << 5)))) {	//turn it off
		onOff &= ~1;					//bit0 = 0 means disable DACA
		powerDown |= 0x1010101;			//enable all power down
	} else {	//turn TV/CRTC2 on
		if (dispNum) source = 1;
		if (connection & (1 << 1)) {	//tv handling
			source = 2;
			if (dispNum) regw32(aDriverRecPtr, 0x60FC, 1);
			else regw32(aDriverRecPtr, 0x60FC, 0);
			if (aDriverRecPtr->currentMode->mFlag & (1 << 5)) {
				control1 = aDriverRecPtr->memConfig->atySettings.unknown3;
				TVEncoder_PAL(aDriverRecPtr);
			} else {
				control1 = aDriverRecPtr->memConfig->atySettings.unknown2;
				TVEncoder_NTSC(aDriverRecPtr);
			}
			ProgramMVMode(aDriverRecPtr);
		}
		onOff |= 1;
	}
	regw32(aDriverRecPtr, 0x7A3C, 0);			//DACB_FORCE_OUTPUT
	regw32(aDriverRecPtr, 0x7A54, control1);	//DACB_CONTROL1
	regw32(aDriverRecPtr, 0x7A58, 0);			//DACB_CONTROL2
	regw32(aDriverRecPtr, 0x7A04, source);		//DACB_SOURCE_SELECT
	regw32(aDriverRecPtr, 0x7A50, powerDown);	//DACA_POWERDOWN
	kdelay(5);
	regw32(aDriverRecPtr, 0x7A00, onOff);		//DACA_ENABLE
}

void HW_DAC2Blank(DriverGlobal *aDriverRecPtr, bool onOff) {
	regw32(aDriverRecPtr, 0x7A40, 0);						//DACB_FORCE_DATA
	if (onOff) regw32(aDriverRecPtr, 0x7A3C, 0x701);		//DACB_FORCE_OUTPUT, 0x701: Enable synchronnous, all three channels
	else regw32(aDriverRecPtr, 0x7A3C, 0);
}

void HW_RestoreDAC2OnOff(DriverGlobal *aDriverRecPtr) {
	//not implemented in ATY_Wormy
}

UInt8 HW_TMDS1State(DriverGlobal *aDriverRecPtr) {
	if (regr32(aDriverRecPtr, 0x7880) & 1) return (regr32(aDriverRecPtr, 0x7884) & 1 + 1);	//LVTMB_SOURCE_SELECT
	return 0;			//LVTMB_CNTL
}

UInt8 HW_TMDS2State(DriverGlobal *aDriverRecPtr) {
	if (regr32(aDriverRecPtr, 0x7A80) & 1) return (regr32(aDriverRecPtr, 0x7A84) & 1 + 1);	//LVTMA_SOURCE_SELECT
	return 0;			//LVTMA_CNTL
}

void HW_TMDS1Dither(DriverGlobal *aDriverRecPtr) {
	if (aDriverRecPtr->htidValue) regw32(aDriverRecPtr, 0x7894, 0);	//disable all
	else regw32(aDriverRecPtr, 0x7894, 0x110000);	//LVTMA_BIT_DEPTH_CONTROL, set TEMPORAL_DITHER
}

void HW_TMDS2Dither(DriverGlobal *aDriverRecPtr) {
	if (aDriverRecPtr->htidValue) regw32(aDriverRecPtr, 0x7A94, 0);	//disable all
	else regw32(aDriverRecPtr, 0x7A94, 0x110000);	//LVTMA_BIT_DEPTH_CONTROL, set TEMPORAL_DITHER
}

void HW_TMDS1OnOff(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8 dispNum) {
	UInt8 cntlEn = regr32(aDriverRecPtr, 0x7880) & 1;		//LVTMB_CNTL, bit1 EN
	Duration miniTime = 500;							//miniseconds
	UInt32 connectFlags = 0;						//var_28
	
	UInt32 cntlAll = regr32(aDriverRecPtr, 0x7880);
	UInt32 transEn = regr32(aDriverRecPtr, 0x7904);			//LVTMA_TRANSMITTER_ENABLE, R5xx only
	UInt32 transControl = regr32(aDriverRecPtr, 0x7910);	//LVTMA_TRANSMITTER_CONTROL
	UInt8 pllBypassRef = !(regr32(aDriverRecPtr, 0x7910) >> 28) & 1;
	UInt32 macroControl;
	UInt32 pllBypass;
	
	if (connection & (1 << 2)) {			//turn DFP1 on
		if (!cntlEn && (aDriverRecPtr->aShare->startTime.lo)) {
			Duration deltaTime = AbsoluteDeltaToDuration(UpTime(), aDriverRecPtr->aShare->startTime);
			if (deltaTime < 0)				//if negative microseconds else milliseconds
				deltaTime = upDIV((-deltaTime), 1000);	//convert to milliseconds
			if (deltaTime <= miniTime) kdelay(miniTime - deltaTime);
		}
		regw8(aDriverRecPtr, 0x7884, dispNum);		//LVTMB_SOURCE_SELECT
		regw32(aDriverRecPtr, 0x78D8, 1);			//LVTMB_DATA_SYNCHRONIZATION
		cntlAll |= 1;								//LVTMB_CNTL set EN bit
		if ((cntlEn == 1) && (aDriverRecPtr->scaledFlags & (1 << 12))) {
			cntlAll |= (1 << 24);					//LVTMB_CNTL set DUAL_LINK bit
			transEn = 0x1D1F;
			macroControl = aDriverRecPtr->memConfig->atySettings.unknown5;
		} else {
			cntlAll &= ~(1 << 24);					//LVTMB_CNTL clear DUAL_LINK bit
			transEn = 0x1F;
			macroControl = aDriverRecPtr->memConfig->atySettings.unknown4;
		}
		regw32(aDriverRecPtr, 0x7880, cntlAll);
		regw32(aDriverRecPtr, 0x790C, macroControl);		//LVTMA_MACRO_CONTROL
		if (aDriverRecPtr->conInfo.edidP2 & (1 << 10)) {
			if (aDriverRecPtr->currentClock >= 11500) pllBypass = 1;
			else pllBypass = 0;
			if (aDriverRecPtr->scaledFlags & (1 << 12)) pllBypass = 0;
			if ((aDriverRecPtr->conInfo.edidP2 & (1 << 10))
				&& ((aDriverRecPtr->currentClock == 11227)
					|| ((aDriverRecPtr->conInfo.edidP2 & 0xFFFF0000) == 0x1D920000))) pllBypass = 0;
		}
		if ((RegGet(&aDriverRecPtr->regIDNub, "display-connect-flags", (void *)&connectFlags, 4) == noErr)
			&& (connectFlags & (1 << 18))) pllBypass = 0;
		if (aDriverRecPtr->currentDispPara.powerStateAF) pllBypass = !pllBypass;
		if (pllBypassRef != pllBypass) cntlEn = 0;
		if (pllBypass != 0) transControl &= ~(1 << 28);		//clear LVTMA_BYPASS_PLL bit
		else transControl |= (1 << 28);				//set LVTMA_BYPASS_PLL bit
		if (!(transControl & 1)) {
			transControl |= 1;						//set LVTMA_PLL_ENABLE, LVTMA_USE_CLK_DATA
			regw32(aDriverRecPtr, 0x7910, transControl);
			kdelay(1);
		}
		if (transControl & (1 << 1)) {				//LVTMA_PLL_RESET
			transControl &= ~(1 << 1);				//clear LVTMA_PLL_RESET
			regw32(aDriverRecPtr, 0x7910, transControl);
			kdelay(20);
		}
		HW_TMDS1Dither(aDriverRecPtr);
		regw32(aDriverRecPtr, 0x7904, transEn);			//LVTMA_TRANSMITTER_ENABLE
		if ((cntlEn == 0) && (aDriverRecPtr->scaledFlags & (1 << 12))) {
			cntlAll |= 0x1000001;				//set LVTMA_CNTL EN, DUAL-LINK bits
			transEn = 0x1D1F;					//enable link0 and link1
			macroControl = aDriverRecPtr->memConfig->atySettings.unknown5;
			regw32(aDriverRecPtr, 0x7880, cntlAll);
			regw32(aDriverRecPtr, 0x780C, macroControl);
			kdelay(40);
			regw32(aDriverRecPtr, 0x7804, transEn);
		}
		if (cntlEn == 0) {
			regw32(aDriverRecPtr, 0x7910, transControl | (1 << 31));//LVTMA_INPUT_TEST_CLK_SET
			kdelay(-10);
			regw32(aDriverRecPtr, 0x7910, transControl & ~(1 << 31));
		}
		UInt8 i;
		for (i = 0;i < 8;i++) {
			kdelay(-10);
			regw32(aDriverRecPtr, 0x78D8, 0x101);		//set LVTMA_DATA_SYNCHRONIZATION
		}
		aDriverRecPtr->hasDCF = (pllBypass == 0);
	} else {	//turn it off
		regw8(aDriverRecPtr, 0x7884, 0);
		regw32(aDriverRecPtr, 0x7894, 0);
		regw32(aDriverRecPtr, 0x7880, cntlAll & ~1);		//clear LVTMA_CNTL EN bit
		regw32(aDriverRecPtr, 0x7904, 0);
		regw32(aDriverRecPtr, 0x7910, transControl | (1 << 28) & ~1 | 2);
		if (cntlEn) aDriverRecPtr->aShare->startTime = UpTime();
	}
}

void HW_TMDS2OnOff(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8 dispNum) {
	UInt8 cntlEn = regr32(aDriverRecPtr, 0x7A80) & 1;		//LVTMA_CNTL, bit1 EN
	Duration miniTime = 500;							//miniseconds
	UInt32 connectFlags = 0;
	
	UInt32 cntlAll = regr32(aDriverRecPtr, 0x7A80);
	UInt32 transEn = regr32(aDriverRecPtr, 0x7B04);			//LVTMA_TRANSMITTER_ENABLE, R5xx only
	UInt32 transControl = regr32(aDriverRecPtr, 0x7B10);	//LVTMA_TRANSMITTER_CONTROL
	UInt8 pllBypassRef = !(regr32(aDriverRecPtr, 0x7B10) >> 28) & 1;
	UInt8 pllBypass;
	UInt32 macroControl;
	
	if (connection & (1 << 3)) {			//turn DFP2 on
		if (!cntlEn && (aDriverRecPtr->aShare->startTime.lo)) {
			Duration deltaTime = AbsoluteDeltaToDuration(UpTime(), aDriverRecPtr->aShare->startTime);
			if (deltaTime < 0)				//if negative microseconds else milliseconds
				deltaTime = upDIV((-deltaTime), 1000);	//convert to milliseconds
			if (deltaTime <= miniTime) kdelay(miniTime - deltaTime);
		}
		regw32(aDriverRecPtr, 0x7B00, 1);			//LVTMA_MODE, 0:LVDS, 1:TMDS
		regw8(aDriverRecPtr, 0x7A84, dispNum);		//LVTMA_SOURCE_SELECT
		regw32(aDriverRecPtr, 0x7AD8, 1);			//LVTMA_DATA_SYNCHRONIZATION
		cntlAll |= 1;								//LVTMA_CNTL set EN bit
		if ((cntlEn == 1) && (aDriverRecPtr->scaledFlags & (1 << 12))) {
			cntlAll |= (1 << 24);					//LVTMA_CNTL set DUAL_LINK bit
			transEn = 0x3E3E;
			macroControl = aDriverRecPtr->memConfig->atySettings.unknown8;
		} else {
			cntlAll &= 0xFEFFFFFF;					//LVTMA_CNTL clear DUAL_LINK bit
			transEn = 0x3E;
			if (aDriverRecPtr->currentClock > 11900) macroControl = aDriverRecPtr->memConfig->atySettings.unknown7;
			else macroControl = aDriverRecPtr->memConfig->atySettings.unknown6;
		}
		regw32(aDriverRecPtr, 0x7A80, cntlAll);
		regw32(aDriverRecPtr, 0x7B0C, macroControl);		//LVTMA_MACRO_CONTROL
		regw32(aDriverRecPtr, 0x7B14, aDriverRecPtr->memConfig->atySettings.unknown11);	//LVTMA_REG_TEST_OUTPUT
		if (aDriverRecPtr->conInfo.edidP2 & (1 << 10)) {
			if (aDriverRecPtr->currentClock >= 11500) pllBypass = 1;
			else pllBypass = 0;
			if (aDriverRecPtr->scaledFlags & (1 << 12)) pllBypass = 0;
			if ((aDriverRecPtr->conInfo.edidP2 & (1 << 10))
				&& ((aDriverRecPtr->currentClock == 11227)
					|| ((aDriverRecPtr->conInfo.edidP2 & 0xFFFF0000) == 0x1D920000))) pllBypass = 0;
		}
		if ((RegGet(&aDriverRecPtr->regIDNub, "display-connect-flags", (void *)&connectFlags, 4) == noErr)
			&& (connectFlags & (1 << 18))) pllBypass = 0;
		if (aDriverRecPtr->currentDispPara.powerStateAF) pllBypass = !pllBypass;
		if (pllBypassRef != pllBypass) cntlEn = 0;
		if (pllBypass != 0) transControl &= ~(1 << 28);		//clear LVTMA_BYPASS_PLL bit
		else transControl |= (1 << 28);				//set LVTMA_BYPASS_PLL bit
		if ((cntlEn == 0) || ((transControl | 0x20000001) != 0x20000001)) {
			cntlEn = 0;
			transControl |= 0x20000001;				//set LVTMA_PLL_ENABLE, LVTMA_USE_CLK_DATA
			regw32(aDriverRecPtr, 0x7B10, transControl);
			kdelay(1);
		}
		if ((cntlEn == 0) || (transControl & (1 << 1))) {//LVTMA_PLL_RESET
			cntlEn = 0;
			transControl &= ~(1 << 1);				//clear LVTMA_PLL_RESET
			regw32(aDriverRecPtr, 0x7B10, transControl);
			kdelay(20);
		}
		HW_TMDS2Dither(aDriverRecPtr);
		regw32(aDriverRecPtr, 0x7B04, transEn);			//LVTMA_TRANSMITTER_ENABLE
		if ((cntlEn == 0) && (aDriverRecPtr->scaledFlags & (1 << 12))) {
			cntlAll |= 0x1000001;				//set LVTMA_CNTL EN, DUAL-LINK bits
			transEn = 0x3E3E;					//enable link0 and link1
			macroControl = aDriverRecPtr->memConfig->atySettings.unknown8;
			regw32(aDriverRecPtr, 0x7A80, cntlAll);
			regw32(aDriverRecPtr, 0x7B0C, macroControl);
			kdelay(40);
			regw32(aDriverRecPtr, 0x7B04, transEn);
		}
		if (cntlEn == 0) {
			regw32(aDriverRecPtr, 0x7B10, transControl | (1 << 31));//LVTMA_INPUT_TEST_CLK_SET
			kdelay(-10);
			regw32(aDriverRecPtr, 0x7B10, transControl & ~(1 << 31));
		}
		UInt8 i;
		for (i = 0;i < 8;i++) {
			kdelay(-10);
			regw32(aDriverRecPtr, 0x7AD8, 0x101);		//set LVTMA_DATA_SYNCHRONIZATION
		}
		aDriverRecPtr->hasDCF = (pllBypass == 0);
	} else {	//turn it off
		regw8(aDriverRecPtr, 0x7A84, 0);
		regw32(aDriverRecPtr, 0x7A80, cntlAll & ~1);		//clear LVTMA_CNTL EN bit
		regw32(aDriverRecPtr, 0x7B04, 0);
		regw32(aDriverRecPtr, 0x7A94, 0);
		regw32(aDriverRecPtr, 0x7B10, transControl & ~1 | 2);
		if (cntlEn) aDriverRecPtr->aShare->startTime = UpTime();
	}
}

bool ClamshellClosed(DriverGlobal *aDriverRecPtr) {
	UInt32 sense;
	if (aDriverRecPtr->clamClose) {
		OSStatus ret = VSLGestalt('clam', &sense);
		if (ret == noErr) return (sense & 1);
	}
	return false;
}

void HW_GetPWRDivParams(DriverGlobal *aDriverRecPtr, UInt32 *tbd, UInt8 *pwr) {
	UInt32 data = (54000000 / aDriverRecPtr->invertFreq / 256 + 1) / 2;
	*pwr = (data + 54000000 % aDriverRecPtr->invertFreq) / 32;
	*tbd = (data * 2 / (*pwr + 1) + 1) / 2;
}

UInt32 HW_GetTimeBaseDiv(DriverGlobal *aDriverRecPtr) {
	UInt32 tbd;
	UInt8 pwr;
	HW_GetPWRDivParams(aDriverRecPtr, &tbd, &pwr);
	return tbd;
}

UInt32 HW_GetTimeBaseFreq(DriverGlobal *aDriverRecPtr) {
	return (27000000 / HW_GetTimeBaseDiv(aDriverRecPtr));
}

void HW_SetGPIOPAD_bit(DriverGlobal *aDriverRecPtr, UInt32 mask, bool enable) {
	UInt32 ga = regr32(aDriverRecPtr, 0x19C);			//var_14
	UInt32 ge = regr32(aDriverRecPtr, 0x1A0) | mask;	//var_10
	UInt32 gm = regr32(aDriverRecPtr, 0x198) | mask;	//var_C
	if (enable) ga |= mask;
	else ga &= ~mask;
	regw32(aDriverRecPtr, 0x19C, ga);
	regw32(aDriverRecPtr, 0x1A0, ge);
	regw32(aDriverRecPtr, 0x198, gm);
}

void HW_InternalSS(DriverGlobal *aDriverRecPtr, float data, UInt8 enable, UInt8 dispNum) {
	UInt32 pisc;	//var_10
	UInt32 epfd;	//var_14
	UInt32 eprd;	//var_28
	float f;		//var_24
	SInt32 data1;	//var_1C
	SInt32 data2;	//var_20
	SInt32 data3;	//var_2C
	UInt8 i;		//var_18
	UInt8 en = enable;	//var_3C
	if (dispNum == 0) pisc = regr32(aDriverRecPtr, 0x458);
	else pisc = regr32(aDriverRecPtr, 0x45C);
	UInt8 cntlEn = pisc & 1;	//var_9
	if (cntlEn == en) return;
	if (dispNum == 0) {
		epfd = regr32(aDriverRecPtr, 0x430);
		eprd = regr32(aDriverRecPtr, 0x404) & 0x3FF;
	} else {
		epfd = regr32(aDriverRecPtr, 0x434);
		eprd = regr32(aDriverRecPtr, 0x414) & 0x3FF;
	}
	data3 = (epfd & 0x7FF0000 >> 16) * 10 + (epfd & 0xF);
	for (f = 0.1;f <= 1.9;f += 0.1) {
		data1 = (SInt32) ((float) data3 * data / 1000.0 / f);
		if (data1 > 63) continue;
		if (data1 == 0) break;
		signedShift((upDIV((13500 / eprd / data1), 40) + 1), 1, &data2);
		if (data2) data2--;
		break;
	}
	if (data2 <= 0) data2 = 1;
	if (data2 > 63) data2 = 63;
	if ((data1 == 0) || (data2 == 0)) en = 0;
	if (en) {
		pisc = (data1 << 16) | ((SInt32) f << 4) | (((SInt32) ((f - (float)(SInt32)f) * 10)) << 8) | (data2 << 24);
		if (cntlEn) pisc |= 1;
		if (dispNum == 0) regw32(aDriverRecPtr, 0x458, pisc);
		else regw32(aDriverRecPtr, 0x45C, pisc);
		pisc |= 1;
	} else pisc &= ~1;
	if (dispNum == 0) regw32(aDriverRecPtr, 0x458, pisc);
	else regw32(aDriverRecPtr, 0x45C, pisc);
	kdelay(1);
	if (en) return;
	pisc = (pisc & 0x3F0000) >> 16 + 4;
	if (dispNum == 0) {
		regw32(aDriverRecPtr, 0x458, 0);
		for (i = 0;i < pisc;i++) {
			regw32(aDriverRecPtr, 0x430, epfd + 1);
			kdelay(-3);
			regw32(aDriverRecPtr, 0x430, epfd);
			kdelay(-3);
		}
	} else {
		regw32(aDriverRecPtr, 0x45C, 0);
		for (i = 0;i < pisc;i++) {
			regw32(aDriverRecPtr, 0x434, epfd + 1);
			kdelay(-3);
			regw32(aDriverRecPtr, 0x434, epfd);
			kdelay(-3);
		}
	}
}

void HW_SSControl(DriverGlobal *aDriverRecPtr, int data, UInt8 dispNum) {
	HW_InternalSS(aDriverRecPtr, (float) data / (float) 100, (data != 0), dispNum);
}

void HW_SetBackLight(DriverGlobal *aDriverRecPtr) {
	UInt8 l_r600_ps1 = 0;	//var_E
	if (aDriverRecPtr->unknown49) l_r600_ps1 = aDriverRecPtr->pwrseqState;
	if (aDriverRecPtr->hasInverter) l_r600_ps1 = 255 - l_r600_ps1;
	UInt8 l_r600_ps2 = regr32(aDriverRecPtr, 0x7AF8) & 0xFF00 >> 8;	//var_D
	if ((l_r600_ps2 < 32) && (l_r600_ps1 >= 32) && (aDriverRecPtr->aShare->someTime2.lo > 0)) {
		Duration deltaTime = AbsoluteDeltaToDuration(UpTime(), aDriverRecPtr->aShare->someTime2);
		if (deltaTime < 0) deltaTime = upDIV((-deltaTime), 1000);
		if (deltaTime < aDriverRecPtr->dps.data[2]) kdelay(aDriverRecPtr->dps.data[2] - deltaTime);
	}
	UInt32 tbd;	//var_18
	UInt8 pwr;	//var_14
	HW_GetPWRDivParams(aDriverRecPtr, &tbd, &pwr);
	regw32(aDriverRecPtr, 0x4DC, tbd);
	regw32(aDriverRecPtr, 0x7AE4, regr32(aDriverRecPtr, 0x7AE4) & 0xF000FFFF | (pwr << 16));
	regw32(aDriverRecPtr, 0x7AF8, l_r600_ps1 << 8 | 1);
	if ((l_r600_ps2 >= 32) && (l_r600_ps1 < 32)) aDriverRecPtr->aShare->someTime2 = UpTime();
}

void HW_LVDSDither(DriverGlobal *aDriverRecPtr) {
	UInt32 lbdc;
	if (aDriverRecPtr->cBitsIs6) {
		if (!aDriverRecPtr->hasDither) lbdc =  0x1010000 | (aDriverRecPtr->unknown44 << 28);
	} else lbdc = 0x1110000 | (aDriverRecPtr->unknown44 << 28);
	if (aDriverRecPtr->htidValue) lbdc = 0;
	regw32(aDriverRecPtr, 0x7A94, lbdc);
}

void HW_LVDSOnOff(DriverGlobal *aDriverRecPtr, UInt8 connection, UInt8 dispNum) {
	UInt8 i;
	UInt8 connect = connection;
	PowerSequence ps;
	ps.data[0] = 0;	//var_5C
	ps.data[1] = 1;
	ps.data[2] = 200;
	ps.data[3] = 200;
	ps.data[4] = 1;
	ps.data[5] = 0;
	ps.data[6] = 400;
	UInt32 minTime = 400;	//var_40
	UInt32 l_r600_bmc = 0;	//var_C
	UInt32 l_r600_7B0C;		//var_28
	UInt32 l_cntl = regr32(aDriverRecPtr, 0x7A80);		//var_2C
	UInt32 l_r600_m = regr32(aDriverRecPtr, 0x7B04);	//var_20
	UInt32 l_r600_mc = regr32(aDriverRecPtr, 0x7B10);	//var_24
	UInt32 l_r600_pd2 = regr32(aDriverRecPtr, 0x7AF0) | 1;	//var_1C
	if (connect & (1 << 6)) {	//turn LVDS on
		aDriverRecPtr->aShare->shellClosed = ClamshellClosed(aDriverRecPtr);
		if (aDriverRecPtr->aShare->shellClosed) connect = 0;	//turn it off
	}
	for (i = 0;i < 7;i++) ps.data[i] = aDriverRecPtr->dps.data[i];
	minTime = ps.data[6];
	regw32(aDriverRecPtr, 0x4DC, HW_GetTimeBaseDiv(aDriverRecPtr));
	UInt32 tbf = HW_GetTimeBaseFreq(aDriverRecPtr) / 0x1000;
	ps.data[0] = ps.data[0] * tbf / 1000 + 1;
	if (ps.data[0] > 255) ps.data[0] = 255;
	ps.data[1] = ps.data[1] * tbf / 1000;
	if (ps.data[1] > 255) ps.data[1] = 255;
	if ((ps.data[1] + ps.data[0]) > 255) ps.data[0] = 255 - ps.data[1];
	ps.data[2] = ps.data[2] * tbf / 1000 + 1;
	if (ps.data[2] > 255) ps.data[2] = 255;
	ps.data[3] = ps.data[3] * tbf / 1000 + 1;
	if (ps.data[3] > 255) ps.data[3] = 255;
	ps.data[4] = ps.data[4] * tbf / 1000 + 1;
	if (ps.data[4] > 255) ps.data[4] = 255;
	ps.data[5] = ps.data[5] * tbf / 1000;
	if (ps.data[5] > 255) ps.data[5] = 255;
	if ((ps.data[5] + ps.data[4]) > 255) ps.data[5] = 255 - ps.data[4];
	ps.data[6] = ps.data[6] * tbf / 1000 + 1;
	if (ps.data[6] > 255) ps.data[6] = 255;
	regw32(aDriverRecPtr, 0x7AE4, 0xFFF);
	regw32(aDriverRecPtr, 0x7AE8, (ps.data[0] + ps.data[1]) | (ps.data[2] << 8) | (ps.data[3] << 16) | ((ps.data[4] + ps.data[5]) << 24));
	regw32(aDriverRecPtr, 0x7AEC, ps.data[6]);
	HW_SetGPIOPAD_bit(aDriverRecPtr, 1, (aDriverRecPtr->invertCurrent != 0));
	regw32(aDriverRecPtr, 0x198, regr32(aDriverRecPtr, 0x198) & ~(1 << 7));
	regw32(aDriverRecPtr, 0x1A0, regr32(aDriverRecPtr, 0x1A0) | (1 << 7));
	l_r600_pd2 = 1;
	if (aDriverRecPtr->hasInverter) l_r600_pd2 |= (1 << 26);
	UInt8 ps_state = regr32(aDriverRecPtr, 0x7AF4) & 1;	//var_2D
	if (connect & (1 << 6))	{	//turn it on
		if (aDriverRecPtr->cBitsIs6) l_r600_bmc = 0;
		else l_r600_bmc = 17;
		regw32(aDriverRecPtr, 0x7B00, aDriverRecPtr->linkType);
		regw8(aDriverRecPtr, 0x7A84, dispNum);
		regw32(aDriverRecPtr, 0x7AD8, 1);
		if ((aDriverRecPtr->linkType != 0) || (!(aDriverRecPtr->scaledFlags & (1 << 12)) && !aDriverRecPtr->dualLink)) {
			l_cntl &= ~(1 << 24);
			l_r600_m = 0x3E;
			if (aDriverRecPtr->linkType != 0) {
				if (aDriverRecPtr->currentClock > 11900) l_r600_7B0C = aDriverRecPtr->memConfig->atySettings.unknown7;
				else l_r600_7B0C = aDriverRecPtr->memConfig->atySettings.unknown6;
			} else l_r600_7B0C = aDriverRecPtr->memConfig->atySettings.unknown5;
		} else {
			l_cntl |= (1 << 24);
			l_r600_m = 0x3E3E;
			if (aDriverRecPtr->linkType != 0) l_r600_7B0C = aDriverRecPtr->memConfig->atySettings.unknown8;
			else l_r600_7B0C = aDriverRecPtr->memConfig->atySettings.unknown10;
		}
		regw32(aDriverRecPtr, 0x7A80, l_cntl);
		regw32(aDriverRecPtr, 0x7B0C, l_r600_7B0C);
		regw32(aDriverRecPtr, 0x7B14, aDriverRecPtr->memConfig->l_r600_tc);
		regw32(aDriverRecPtr, 0x7AFC, l_r600_bmc);
		if (aDriverRecPtr->linkType != 0) l_r600_mc |= (1 << 29);
		else l_r600_mc &= ~(1 << 29);
		regw32(aDriverRecPtr, 0x7B10, l_r600_mc | 1);
		kdelay(1);
		regw32(aDriverRecPtr, 0x7B10, l_r600_mc & ~(1 << 1));
		if (ps_state == 0) kdelay(20);
		regw32(aDriverRecPtr, 0x7B04, l_r600_m);
		if (ps_state == 0) {
			regw32(aDriverRecPtr, 0x7B10, l_r600_mc | (1 << 31));
			kdelay(-10);
			regw32(aDriverRecPtr, 0x7B10, l_r600_mc & ~(1 << 31));
		}
		if (aDriverRecPtr->linkType == 0) HW_SSControl(aDriverRecPtr, 75, dispNum);
		else HW_SSControl(aDriverRecPtr, 0, dispNum);
		kdelay(-10);
		regw32(aDriverRecPtr, 0x7AD8, 0x101);
		HW_SetBackLight(aDriverRecPtr);
		if ((ps_state == 0) && (aDriverRecPtr->aShare->someTime.lo > 0)) {
			Duration deltaTime = AbsoluteDeltaToDuration(UpTime(), aDriverRecPtr->aShare->someTime);
			if (deltaTime < 0) deltaTime = upDIV((-deltaTime), 1000);
			if (deltaTime < minTime) kdelay(minTime - deltaTime);
		}
		regw32(aDriverRecPtr, 0x7AF0, l_r600_pd2 | (1 << 4));
		if (ps_state == 0) while (!(regr32(aDriverRecPtr, 0x7AF4) & (1 << 4))) kdelay(1);
		for (i = 0;i < 4;i++) {
			kdelay(-10);
			regw32(aDriverRecPtr, 0x7AD8, 0x101);
		}
		HW_LVDSDither(aDriverRecPtr);
	} else {	//turn it off
		regw32(aDriverRecPtr, 0x7AF0, l_r600_pd2 & (1 << 4));
		if (ps_state != 0) while (!(regr32(aDriverRecPtr, 0x7AF4) & (1 << 4))) kdelay(1);
		if (ps_state != 0) aDriverRecPtr->aShare->someTime = UpTime();
		regw32(aDriverRecPtr, 0x7B04, 0);
		regw32(aDriverRecPtr, 0x7A80, l_cntl & ~1);
		regw32(aDriverRecPtr, 0x7B10, l_r600_mc & ~1 | (1 << 1));
		HW_SSControl(aDriverRecPtr, 0, dispNum);
	}
}

void HWProgramCRTCOff(DriverGlobal *aDriverRecPtr, UInt8 dispNum) {
	UInt32 offset = 0;
	/* bool displayOn = */HWIsCRTCDisplayOn(aDriverRecPtr, dispNum);
	regw32(aDriverRecPtr, 0x65D8, 0);
	regw32(aDriverRecPtr, 0x6DD8, 0);
	if (dispNum) offset = 0x800;
	UInt32 sclEn = regr32(aDriverRecPtr, 0x6590 + offset);
#ifdef ATY_Caretta
	if (HW_TMDS2State(aDriverRecPtr) == (dispNum + 1)) HW_TMDS2OnOff(aDriverRecPtr, 0, dispNum);
	if (HW_DAC1State(aDriverRecPtr) == (dispNum + 1)) HW_DAC1OnOff(aDriverRecPtr, 0, dispNum);
#endif
#ifdef ATY_Wormy
	if (HW_TMDS1State(aDriverRecPtr) == (dispNum + 1)) HW_TMDS1OnOff(aDriverRecPtr, 0, dispNum);
	if (HW_TMDS2State(aDriverRecPtr) == (dispNum + 1)) HW_LVDSOnOff(aDriverRecPtr, 0, dispNum);
	if (HW_DAC2State(aDriverRecPtr) == (dispNum + 1)) HW_DAC2OnOff(aDriverRecPtr, 0, dispNum);
#endif
	regw32(aDriverRecPtr, 0x6414 + offset, 0x1FFF1FFF);	//D1/D2 CUR_POSITION
	regw32(aDriverRecPtr, 0x6454 + offset, 0x1FFF1FFF);	//D1/D2 ICON_START_POSITION
	regw32(aDriverRecPtr, 0x65C0 + offset, regr32(aDriverRecPtr, 0x65C0 + offset) & ~(1 << 24));
	regw32(aDriverRecPtr, 0x659C + offset, 0);
	regw32(aDriverRecPtr, 0x6594 + offset, 0x101);
	if (regr32(aDriverRecPtr, 0x6590 + offset) != 0) kdelay(20);
	regw32(aDriverRecPtr, 0x6594 + offset, 0);
	regw32(aDriverRecPtr, 0x6590 + offset, 0);
	if (sclEn != 0) kdelay(20);
	if (regr32(aDriverRecPtr, 0x65D0 + offset) != 0) {
		regw32(aDriverRecPtr, 0x6590 + offset, 1);
		regw32(aDriverRecPtr, 0x65D0 + offset, 0x100);
		kdelay(20);
		regw32(aDriverRecPtr, 0x6590 + offset, 0);
	}
	regw32(aDriverRecPtr, 0x6080 + offset, 0x1000000);	//D1/D2 CRTC_CONTROL
	regw32(aDriverRecPtr, 0x6520, aDriverRecPtr->displayLineBuffer);
	UInt8 i;
	for (i = 0;i < 30;i++) {		//D1/D2 CRTC_DOUBLE_BUFFER_CONTROL
		if (((regr32(aDriverRecPtr, 0x60EC + offset) & 1) == 0) && (regr32(aDriverRecPtr, 0x65CC + offset) == 0)) break;
		kdelay(1);
	}
	if (regr32(aDriverRecPtr, 0x60EC + offset) & 1) {
		regw32(aDriverRecPtr, 0x60EC + offset, 0x100);
		kdelay(1);
		regw32(aDriverRecPtr, 0x60EC + offset, 0);
	}
	kdelay(20);
	aDriverRecPtr->videoClock = 0;
	if (dispNum == 0) {
		regw32(aDriverRecPtr, 0x450, regr32(aDriverRecPtr, 0x450) | 1);	//RADEON_OV0_VID_BUF4_BASE_ADRS
		kdelay(1);
		regw32(aDriverRecPtr, 0x450, regr32(aDriverRecPtr, 0x450) | 3);
		kdelay(5);
		regw32(aDriverRecPtr, 0xB74, 0);	//RADEON_SURFACE7_LOWER_BOUND
		regw32(aDriverRecPtr, 0xB78, 0);	//RADEON_SURFACE7_UPPER_BOUND
		regw32(aDriverRecPtr, 0xB7C, 0);
	} else {
		regw8(aDriverRecPtr, 0x454, regr8(aDriverRecPtr, 0x454) | 1);
		kdelay(1);
		regw8(aDriverRecPtr, 0x454, regr8(aDriverRecPtr, 0x454) | 3);
		kdelay(5);
		regw32(aDriverRecPtr, 0xB64, 0);
		regw32(aDriverRecPtr, 0xB68, 0);
		regw32(aDriverRecPtr, 0xB6C, 0);
	}
}

UInt32 freq(UInt32 a1, UInt32 a2, UInt32 a3) {
	float v1 = (float) ((a1 + 1) * a2);
	float v2 = (float) (a3 + 1);
	if (v2 == 0.0) return 0;
	return (UInt32) (v1 / v2);
}

UInt32 RF_R520GetSClk(DriverGlobal *aDriverRecPtr, UInt32 value) {
	UInt32 data = pllr32(aDriverRecPtr, 0);	//CLOCK_CNTL_DATA
	UInt32 part1 = (data & 0x1C) >> 2;
	UInt32 part2 = (data & 0x1FE0) >> 5;
	UInt32 part3 = ((data & 0xF0000) >> 16) + ((data & 0xF00000) >> 20) + 2;
	return (freq(part2 | 1, value, part1) / part3);
}

UInt32 HW_GetSClk(DriverGlobal *aDriverRecPtr) {
	return RF_R520GetSClk(aDriverRecPtr, 2700);
}

UInt32 RF_R520GetMClk(DriverGlobal *aDriverRecPtr, UInt32 value) {
	UInt32 part1 = pllr32(aDriverRecPtr, 4) & 0x1C / 4;
	UInt32 part2 = pllr32(aDriverRecPtr, 4) & 0x1FE0 / 32;
	return (((freq(part2 | 1, value, part1) >> 31) + freq(part2 | 1, value, part1)) / 2);
}

UInt32 HW_GetMClk(DriverGlobal *aDriverRecPtr) {
	return RF_R520GetMClk(aDriverRecPtr, 2700);
}

void SetPerformanceWeights(DriverGlobal *aDriverRecPtr) {
	UInt32 value = memrs32(aDriverRecPtr, 0xF0, 0x7F0000);
	memws32(aDriverRecPtr, 0x30, 0xBFFF, 0x7F0000);
	memws32(aDriverRecPtr, 0x18, 3, 0x7F0000);
	if ((value & 0xF0000000) != 0x30000000) return;
	if ((value & 0xFF000000) == 0x30000000) {
		memws32(aDriverRecPtr, 0x11, 0x108CD9C3, 0x7F0000);
		memws32(aDriverRecPtr, 0x10, 0x7F53C726, 0x7F0000);
		memws32(aDriverRecPtr, 0x12, 0x4B0E06, 0x7F0000);
		memws32(aDriverRecPtr, 0x17, 0xAD3302, 0x7F0000);
		memws32(aDriverRecPtr, 0x20, 0x1FAD1938, 0x7F0000);
		memws32(aDriverRecPtr, 0x21, 0x1FAD1938, 0x7F0000);
		memws32(aDriverRecPtr, 0x22, 0x1FAD1938, 0x7F0000);
		memws32(aDriverRecPtr, 0x23, 0x1FAD1938, 0x7F0000);
		memws32(aDriverRecPtr, 0x24, 0x1577D268, 0x7F0000);
		memws32(aDriverRecPtr, 0x25, 0x1577D268, 0x7F0000);
		memws32(aDriverRecPtr, 0x26, 0x1577D268, 0x7F0000);
		memws32(aDriverRecPtr, 0x27, 0x1577D268, 0x7F0000);
		memws32(aDriverRecPtr, 0x28, 0xD3FB, 0x7F0000);
		memws32(aDriverRecPtr, 0x29, 0xD3FB, 0x7F0000);
		memws32(aDriverRecPtr, 0x2A, 0xD3FB, 0x7F0000);
		memws32(aDriverRecPtr, 0x2B, 0xD3FB, 0x7F0000);
		memws32(aDriverRecPtr, 0x2C, 0x3584B053, 0x7F0000);
		memws32(aDriverRecPtr, 0x2D, 0x31A75B6B, 0x7F0000);
		memws32(aDriverRecPtr, 0x37, 0x88881519, 0x7F0000);
		memws32(aDriverRecPtr, 0x38, 0x8A8888, 0x7F0000);
	}
	if ((value & 0xFF000000) == 0x31000000) {
		memws32(aDriverRecPtr, 0x11, 0x1EB83A92, 0x7F0000);
		memws32(aDriverRecPtr, 0x10, 0xB29C680D, 0x7F0000);
		memws32(aDriverRecPtr, 0x12, 0x5A0D03, 0x7F0000);
		memws32(aDriverRecPtr, 0x17, 0xA27303, 0x7F0000);
		memws32(aDriverRecPtr, 0x20, 0x2249E87C, 0x7F0000);
		memws32(aDriverRecPtr, 0x21, 0x2249E87C, 0x7F0000);
		memws32(aDriverRecPtr, 0x22, 0x2249E87C, 0x7F0000);
		memws32(aDriverRecPtr, 0x23, 0x2249E87C, 0x7F0000);
		memws32(aDriverRecPtr, 0x24, 0x259DE198, 0x7F0000);
		memws32(aDriverRecPtr, 0x25, 0x259DE198, 0x7F0000);
		memws32(aDriverRecPtr, 0x26, 0x259DE198, 0x7F0000);
		memws32(aDriverRecPtr, 0x27, 0x259DE198, 0x7F0000);
		memws32(aDriverRecPtr, 0x28, 0x1ECB, 0x7F0000);
		memws32(aDriverRecPtr, 0x29, 0x1ECB, 0x7F0000);
		memws32(aDriverRecPtr, 0x2A, 0x1ECB, 0x7F0000);
		memws32(aDriverRecPtr, 0x2B, 0x1ECB, 0x7F0000);
		memws32(aDriverRecPtr, 0x2C, 0x1935A876, 0x7F0000);
		memws32(aDriverRecPtr, 0x2D, 0x21956AB9, 0x7F0000);
		memws32(aDriverRecPtr, 0x37, 0x88880F90, 0x7F0000);
		memws32(aDriverRecPtr, 0x38, 0xC8888, 0x7F0000);
	}
}

bool ReadEFIHWSettings(DriverGlobal *aDriverRecPtr) {
	UInt32 size;
	
	OSStatus ret = RegistryPropertyGetSize(&aDriverRecPtr->regIDDevice, "ATY,Settings", &size);
	if ((ret != noErr) || (size == 0) || (size > (13 * 4))) return false;
	ret = RegistryPropertyGet(&aDriverRecPtr->regIDDevice, "ATY,Settings", &aDriverRecPtr->memConfig->atySettings, &size);
	if (ret == noErr) return true;
	return false;
}

void InitHWSetting(DriverGlobal *aDriverRecPtr) {
#ifdef ATY_Caretta
	if (ReadEFIHWSettings(aDriverRecPtr)) return;	//initialized by registry
#endif
	aDriverRecPtr->memConfig->atySettings.daca_cntl = 0x50802;
	aDriverRecPtr->memConfig->atySettings.unknown1 = 0x51203;
	aDriverRecPtr->memConfig->atySettings.unknown2 = 0x51201;
	aDriverRecPtr->memConfig->atySettings.unknown3 = 0x51100;
	aDriverRecPtr->memConfig->atySettings.dacb_cntl = 0x51002;
	aDriverRecPtr->memConfig->atySettings.unknown4 = 0xB00416;
	aDriverRecPtr->memConfig->atySettings.unknown5 = 0xB00416;
	aDriverRecPtr->memConfig->atySettings.unknown6 = 0xF2061D;
	aDriverRecPtr->memConfig->atySettings.unknown7 = 0xF2061D;
	aDriverRecPtr->memConfig->atySettings.unknown8 = 0xF2061D;
	aDriverRecPtr->memConfig->atySettings.unknown9 = 0xC720406;
	aDriverRecPtr->memConfig->atySettings.unknown10 = 0xC720406;
	aDriverRecPtr->memConfig->atySettings.unknown11 = 0x200000;	
}

OSStatus ReadFcodeMemroyResetSequence(DriverGlobal *aDriverRecPtr) {
	UInt32 size;
	
	aDriverRecPtr->memConfig->memResetSeqNum = 0;
	OSStatus ret = RegistryPropertyGetSize(&aDriverRecPtr->regIDDevice, "ATY,MRT", &size);
	if ((ret != noErr) || (size == 0) || (size > 0x200)) return ret;
	ret = RegistryPropertyGet(&aDriverRecPtr->regIDDevice, "ATY,MRT", aDriverRecPtr->memConfig->memResetSequence, &size);
	if (ret == noErr) aDriverRecPtr->memConfig->memResetSeqNum = size / 4;
	return ret;
}

void HW_InitMEMConfiguration(DriverGlobal *aDriverRecPtr) {
	if (aDriverRecPtr->memConfig->configMemSize != 0) return;
#ifdef ATY_Wormy
	aDriverRecPtr->memConfig->unknown3 = pciXr32(aDriverRecPtr, 1);
	aDriverRecPtr->memConfig->unknown4 = pciXr32(aDriverRecPtr, 3);
	aDriverRecPtr->memConfig->unknown5 = pciXr32(aDriverRecPtr, 0x70);
	aDriverRecPtr->memConfig->unknown6 = pciXr32(aDriverRecPtr, 0x85);
	aDriverRecPtr->memConfig->unknown7 = pciXr32(aDriverRecPtr, 0x91);
	aDriverRecPtr->memConfig->unknown8 = pciXr32(aDriverRecPtr, 0x92);
	aDriverRecPtr->memConfig->unknown9 = pciXr32(aDriverRecPtr, 0x93);
	aDriverRecPtr->memConfig->unknown10 = pciXr32(aDriverRecPtr, 0xA0);
	aDriverRecPtr->memConfig->unknown11 = pciXr32(aDriverRecPtr, 0xB0);
	aDriverRecPtr->memConfig->unknown12 = pciXr32(aDriverRecPtr, 0xE0);
	aDriverRecPtr->memConfig->unknown13 = pciXr32(aDriverRecPtr, 0x10);
	aDriverRecPtr->memConfig->unknown14 = pciXr32(aDriverRecPtr, 0x90);
	aDriverRecPtr->memConfig->unknown15 = pciXr32(aDriverRecPtr, 0x400);
	aDriverRecPtr->memConfig->unknown16 = pciXr32(aDriverRecPtr, 0xE1);
	aDriverRecPtr->memConfig->unknown17 = 3;
#endif
	ReadFcodeMemroyResetSequence(aDriverRecPtr);
#ifdef ATY_Caretta
	aDriverRecPtr->memConfig->unknown1 = 2;
	aDriverRecPtr->memConfig->configMemSize = regr32(aDriverRecPtr, 0xF8);
	aDriverRecPtr->memConfig->agpLocation = memr32(aDriverRecPtr, 5);
	aDriverRecPtr->memConfig->cmdPadCntl2 = memr32(aDriverRecPtr, 0x8D);
	aDriverRecPtr->memConfig->unknown18 = memr32(aDriverRecPtr, 0x2D);
	aDriverRecPtr->memConfig->cmdPadCntl1 = (memr32(aDriverRecPtr, 0x2E) | (1 << 8)) & ~(1 << 9);
	aDriverRecPtr->memConfig->unknown19 = memr32(aDriverRecPtr, 0x31) & ~(1 << 28);
	aDriverRecPtr->memConfig->unknown22 = memr32(aDriverRecPtr, 0x35);
	aDriverRecPtr->memConfig->unknown20 = memr32(aDriverRecPtr, 0x3B);
	aDriverRecPtr->memConfig->unknown21 = memr32(aDriverRecPtr, 0x3C);
	aDriverRecPtr->memConfig->writeAge1 = memr32(aDriverRecPtr, 0x37);
	aDriverRecPtr->memConfig->writeAge2 = memr32(aDriverRecPtr, 0x38);
	aDriverRecPtr->memConfig->dataPadCntl1 = memr32(aDriverRecPtr, 0x8E);
	aDriverRecPtr->memConfig->dataPadCntl2 = memr32(aDriverRecPtr, 0x8F);
	aDriverRecPtr->memConfig->agpBase1 = memr32(aDriverRecPtr, 6);
	aDriverRecPtr->memConfig->unknown24 = memr32(aDriverRecPtr, 0xF);
	aDriverRecPtr->memConfig->unknown25 = memr32(aDriverRecPtr, 0x3D);
	aDriverRecPtr->memConfig->unknown26 = memr32(aDriverRecPtr, 0x3F);
	aDriverRecPtr->memConfig->unknown27 = memr32(aDriverRecPtr, 0x41);
	aDriverRecPtr->memConfig->unknown28 = memr32(aDriverRecPtr, 0x43);
	aDriverRecPtr->memConfig->unknown29 = memr32(aDriverRecPtr, 0x3E);
	aDriverRecPtr->memConfig->unknown30 = memr32(aDriverRecPtr, 0x40);
	aDriverRecPtr->memConfig->unknown31 = memr32(aDriverRecPtr, 0x42);
	aDriverRecPtr->memConfig->unknown32 = memr32(aDriverRecPtr, 0x44);
	aDriverRecPtr->memConfig->unknown34 = memr32(aDriverRecPtr, 0x4D);
	aDriverRecPtr->memConfig->unknown35 = memr32(aDriverRecPtr, 0x4E);
	aDriverRecPtr->memConfig->unknown36 = memr32(aDriverRecPtr, 0x4F);
	aDriverRecPtr->memConfig->unknown37 = memr32(aDriverRecPtr, 0x50);
	aDriverRecPtr->memConfig->unknown38 = memr32(aDriverRecPtr, 0x55);
	aDriverRecPtr->memConfig->unknown39 = memr32(aDriverRecPtr, 0x56);
	aDriverRecPtr->memConfig->unknown40 = memr32(aDriverRecPtr, 0x57);
	aDriverRecPtr->memConfig->unknown41 = memr32(aDriverRecPtr, 0x58);
	aDriverRecPtr->memConfig->seq_rd_ctl2 = memr32(aDriverRecPtr, 0x65);
	aDriverRecPtr->memConfig->seq_wr_ctl1 = memr32(aDriverRecPtr, 0x66);
	aDriverRecPtr->memConfig->seq_wr_ctl2 = memr32(aDriverRecPtr, 0x67);
	aDriverRecPtr->memConfig->seq_io_ctl1 = memr32(aDriverRecPtr, 0x68);
	aDriverRecPtr->memConfig->unknown42 = memr32(aDriverRecPtr, 0x5D);
	aDriverRecPtr->memConfig->unknown43 = memr32(aDriverRecPtr, 0x5E);
	aDriverRecPtr->memConfig->unknown44 = memr32(aDriverRecPtr, 0x5F);
	aDriverRecPtr->memConfig->unknown45 = memr32(aDriverRecPtr, 0x60);
	aDriverRecPtr->memConfig->seq_io_ctl2 = memr32(aDriverRecPtr, 0x69);
	aDriverRecPtr->memConfig->seq_npl_ctl1 = memr32(aDriverRecPtr, 0x6A);
	aDriverRecPtr->memConfig->seq_npl_ctl2 = memr32(aDriverRecPtr, 0x6B);
	aDriverRecPtr->memConfig->seq_ck_pad_cntl1 = memr32(aDriverRecPtr, 0x6C);
	aDriverRecPtr->memConfig->seq_dq_pad_cntl2 = memr32(aDriverRecPtr, 0x71);
	aDriverRecPtr->memConfig->seq_qs_pad_cntl1 = memr32(aDriverRecPtr, 0x72);
	aDriverRecPtr->memConfig->seq_qs_pad_cntl2 = memr32(aDriverRecPtr, 0x73);
	aDriverRecPtr->memConfig->seq_a_pad_cntl2 = memr32(aDriverRecPtr, 0x74);
	aDriverRecPtr->memConfig->unknown46 = memr32(aDriverRecPtr, 0x79);
	aDriverRecPtr->memConfig->unknown47 = memr32(aDriverRecPtr, 0x7A);
	aDriverRecPtr->memConfig->unknown48 = memr32(aDriverRecPtr, 0x7B);
	aDriverRecPtr->memConfig->unknown49 = memr32(aDriverRecPtr, 0x7C);
	aDriverRecPtr->memConfig->i0_pad_cntl_l2 = memr32(aDriverRecPtr, 0x81);
	aDriverRecPtr->memConfig->i0_pad_cntl = memr32(aDriverRecPtr, 0x82);
	aDriverRecPtr->memConfig->i0_rd_cntl_l2 = memr32(aDriverRecPtr, 0x85);
	aDriverRecPtr->memConfig->i0_rd_qs_cntl_l1 = memr32(aDriverRecPtr, 0x86);
	aDriverRecPtr->memConfig->i0_wr_cntl_l2 = memr32(aDriverRecPtr, 0x89);
	aDriverRecPtr->memConfig->i0_ck_pad_cntl_l1 = memr32(aDriverRecPtr, 0x8A);
	aDriverRecPtr->memConfig->io_qs_pad_cntl_l2 = memr32(aDriverRecPtr, 0x91);
	aDriverRecPtr->memConfig->unknown50 = memr32(aDriverRecPtr, 0x30);
	aDriverRecPtr->memConfig->unknown51 = memr32(aDriverRecPtr, 0x2F);
	aDriverRecPtr->memConfig->unknown52 = pllr32(aDriverRecPtr, 0xA);
#endif
#ifdef ATY_Wormy
	regw32(aDriverRecPtr, 0x70, 0x7F0000);
	aDriverRecPtr->memConfig->configMemSize = regr32(aDriverRecPtr, 0xF8);
	aDriverRecPtr->memConfig->cntl0 = memr32(aDriverRecPtr, 8);
	aDriverRecPtr->memConfig->cntl1 = memr32(aDriverRecPtr, 9);
	aDriverRecPtr->memConfig->rfsh_cntl = memr32(aDriverRecPtr, 0xA);
	aDriverRecPtr->memConfig->arb_dram_penalties1 = memr32(aDriverRecPtr, 0x13);
	aDriverRecPtr->memConfig->arb_dram_penalties2 = memr32(aDriverRecPtr, 0x14);
	aDriverRecPtr->memConfig->arb_dram_penalties3 = memr32(aDriverRecPtr, 0x15);
	aDriverRecPtr->memConfig->arb_min = memr32(aDriverRecPtr, 0x10);
	aDriverRecPtr->memConfig->arb_timers = memr32(aDriverRecPtr, 0x12);
	aDriverRecPtr->memConfig->unknown33 = memr32(aDriverRecPtr, 0x11);
	aDriverRecPtr->memConfig->arb_rdwr_switch = memr32(aDriverRecPtr, 0x17);
	//......
#endif
}

void InitPCIExpress(DriverGlobal *aDriverRecPtr) {
	pciXw32(aDriverRecPtr, 1, 0x1E040000);
	pciXw32(aDriverRecPtr, 3, 0x900003);
	pciXw32(aDriverRecPtr, 0x70, 0x400A1807);
	pciXw32(aDriverRecPtr, 0x85, 0x140000);
	pciXw32(aDriverRecPtr, 0x91, 0);
	pciXw32(aDriverRecPtr, 0x92, 0);
	pciXw32(aDriverRecPtr, 0x93, 0);
#ifdef ATY_Caretta
	pciXw32(aDriverRecPtr, 0xA0, 0x40000000);
#endif
#ifdef ATY_Wormy
	if (aDriverRecPtr->pciXState) pciXw32(aDriverRecPtr, 0xA0, 0x40001000);
	else pciXw32(aDriverRecPtr, 0xA0, 0x40001300);
#endif
	pciXw32(aDriverRecPtr, 0xB0, 0x81);
	pciXw32(aDriverRecPtr, 0xE0, 1);
	pciXw32(aDriverRecPtr, 0x10, 0x40);
	pciXw32(aDriverRecPtr, 0x90, 0x10);
	pciXw32(aDriverRecPtr, 0x400, 0x13F);
	pciXw32(aDriverRecPtr, 0xE1, 0x2200);
	if (aDriverRecPtr->unknown7) pciXw32(aDriverRecPtr, 0xA1, aDriverRecPtr->unknown7);
}

const UInt16 int_status_regs[2] = {0x7EDC, 0x44};
static UInt32 int_status_vals[2];
static ATIInterrupt *pAtiIntSet;
const UInt32 maxIntSet = 13;
ATIInterrupt atiIntSet[13] = {
{2, 0x4, 0, 0x10, 0x6540, 0x10, 0x653C, 0},
{1, 0x10, 0, 0x1, 0x6540, 0x10, 0x6534, 0},
{8, 0x20, 0, 0x100, 0x6540, 0x10, 0x6D34, 0},
{5, 0x40000, 0, 0x10000, 0x7D08, 0x1, 0x7D08, 0},
{10, 0x80000, 0, 0x10000, 0x7D18, 0x1, 0x7D18, 0},
{3, 0x80, 0, 0x10000, 0x60DC, 0x100, 0x60BC, 0},
{7, 0x1000, 0, 0x10000, 0x68DC, 0x100, 0x68BC, 0},
{4, 0x40, 0, 0x1, 0x60DC, 0x2, 0x60C8, 0},
{9, 0x800, 0, 0x1, 0x68DC, 0x2, 0x68C8, 0},
{19, 0x2000000, 1, 0x2000000, 0x40, 0x2000000, 0x44, 0},
{17, 0x80000, 1, 0x80000, 0x40, 0x80000, 0x44, 0},
{23, 0x40000000, 1, 0x40000000, 0x40, 0x40000000, 0x44, 0},
{24, 0x80000000, 1, 0x80000000, 0x40, 0x40000000, 0x44, 0}
};

void InitInterruptMask(DriverGlobal *aDriverRecPtr) {
	UInt8 i;
	for (i = 0; i < maxIntSet; i++) aDriverRecPtr->intFlags[atiIntSet[i].dispNum] |= atiIntSet[i].intFlags;
}

void DisableHWInterrupts(DriverGlobal *aDriverRecPtr) {
	UInt8 i;
	for (i = 0;i < maxIntSet;i++) {
		regw32(aDriverRecPtr, atiIntSet[i].addr2, atiIntSet[i].value2);
		regw32(aDriverRecPtr, atiIntSet[i].addr1, regr32(aDriverRecPtr, atiIntSet[i].addr1) & ~ atiIntSet[i].mask1);
		regw32(aDriverRecPtr, atiIntSet[i].addr2, atiIntSet[i].value2);
	}
	kdelay(17);
	aDriverRecPtr->aShare->driverFlags &= ~(1 << 6);
}

void RearmMSIInterrupt(DriverGlobal *aDriverRecPtr) {
	regw32(aDriverRecPtr, 0x160, 1);	//MC_PT0_CONTEXT4_MULTI_LEVEL_BASE_ADDRESS
}

void ATIDeviceEnabler(InterruptSetMember ISTmember, void *refCon) {
	DriverGlobal *aDriverRecPtr = (DriverGlobal *)refCon;
	UInt8 i;
	for (i = 0;i < maxIntSet;i++) {
		if (ISTmember.member != atiIntSet[i].member) continue;
		UInt32 mask = regr32(aDriverRecPtr, atiIntSet[i].addr1);
		if (atiIntSet[i].mask1 & mask) break;
		regw32(aDriverRecPtr, atiIntSet[i].addr2, atiIntSet[i].value2);
		regw32(aDriverRecPtr, atiIntSet[i].addr1, atiIntSet[i].mask1 | mask);
		break;
	}
}

InterruptSourceState ATIDeviceDisabler(InterruptSetMember ISTmember, void *refCon) {
	DriverGlobal *aDriverRecPtr = (DriverGlobal *)refCon;
	UInt8 i;
	for (i = 0;i < maxIntSet;i++) {
		if (ISTmember.member != atiIntSet[i].member) continue;
		UInt32 mask = regr32(aDriverRecPtr, atiIntSet[i].addr1);
		if (!(atiIntSet[i].mask1 & mask)) break;
		regw32(aDriverRecPtr, atiIntSet[i].addr1, atiIntSet[i].mask1 & mask);
		break;
	}
	return kSourceWasDisabled;
}

void fast_clk_program_for_vbl(DriverGlobal *aDriverRecPtr, UInt32 unknown1) {
	//a lot of register RW here
}

void SetUpFP1InterruptPolarity(DriverGlobal *aDriverRecPtr, bool onOff) {
	UInt8 fp1InterruptPolariy = 0;
	if (onOff) {
		regw8(aDriverRecPtr, 0x7D08, 1);	//DC_HOT_PLUG_DETECT1_INT_ACK
		kdelay(5);
		regr32(aDriverRecPtr, 0x7D04);		//DC_HOT_PLUG_DETECT1_INT_STATUS
	}
	if (regr32(aDriverRecPtr, 0x7D04) & (1 << 1)) fp1InterruptPolariy =0;	//0/1: nothing/panel connected
	else fp1InterruptPolariy = 1;
	regw8(aDriverRecPtr, 0x7D09, fp1InterruptPolariy);	//DC_HOT_PLUG_DETECT1_INT_POLARITY, 0/1: generate interrupt on disconnect/connect
}

void SetUpFP2InterruptPolarity(DriverGlobal *aDriverRecPtr, bool onOff) {
	UInt8 fp2InterruptPolariy = 0;
	if (onOff) {
		regw8(aDriverRecPtr, 0x7D18, 1);	//DC_HOT_PLUG_DETECT2_INT_ACK
		kdelay(5);
		regr32(aDriverRecPtr, 0x7D14);		//DC_HOT_PLUG_DETECT2_INT_STATUS
	}
	if (regr32(aDriverRecPtr, 0x7D14) & (1 << 1)) fp2InterruptPolariy =0;	//0/1: nothing/panel connected
	else fp2InterruptPolariy = 1;
	regw8(aDriverRecPtr, 0x7D19, fp2InterruptPolariy);	//DC_HOT_PLUG_DETECT2_INT_POLARITY, 0/1: generate interrupt on disconnect/connect
}

void RearmMSInterrupt(DriverGlobal *aDriverRecPtr) {
	regw32(aDriverRecPtr, 0x160, 1);
}

InterruptMemberNumber ATIIsrDispatcher(InterruptSetMember ISTmember, void *refCon, UInt32 theIntCount) {
	DriverGlobal *aDriverRecPtr = (DriverGlobal *)refCon;
	if (!(aDriverRecPtr->aShare->driverFlags & (1 << 6))) return kIsrIsComplete;
	if (aDriverRecPtr->member != kIsrIsComplete) ISTmember.member = kIsrIsComplete;
	else {
		UInt8 i;
		for (i = 0;i < 2;i++) int_status_vals[i] = regr32(aDriverRecPtr, int_status_regs[i]) & aDriverRecPtr->intFlags[i];
		pAtiIntSet = atiIntSet;
		for (i = 0;i < 2;i++) if (int_status_vals[i] & aDriverRecPtr->intFlags[i]) break;
		ISTmember.member = kIsrIsComplete;
		if (i < 2)
			while (pAtiIntSet != &atiIntSet[maxIntSet]) {
				if (int_status_vals[pAtiIntSet->dispNum] & pAtiIntSet->intFlags) {
					int_status_vals[pAtiIntSet->dispNum] &= ~pAtiIntSet->intFlags;
					if (pAtiIntSet->mask1 & regr32(aDriverRecPtr, pAtiIntSet->addr1)) {
						ISTmember.member = pAtiIntSet->member;
						aDriverRecPtr->currentInt = pAtiIntSet;
						break;
					}
				}
				pAtiIntSet = &pAtiIntSet[1];		//go to next atiIntSet
			}
	}
	if ((ISTmember.member == kIsrIsComplete) && (aDriverRecPtr->member != kIsrIsComplete)) {
		ISTmember.member = aDriverRecPtr->member;
		aDriverRecPtr->member = kIsrIsComplete;
	}
	if (((ISTmember.member == 1) || (ISTmember.member == 2)) && (aDriverRecPtr->aShare->array2[0] & (1 << 2))) {
		bool goOut = false;
		UInt32 vertCount = regr32(aDriverRecPtr, 0x60A0) & 0x1FFF;	//D1CRTC_STATUS_POSITION
		if (aDriverRecPtr->aShare->bottomCount > aDriverRecPtr->aShare->topCount) {
			if (aDriverRecPtr->aShare->bottomCount < vertCount) goOut = true;
			if (aDriverRecPtr->aShare->topCount > vertCount) goOut = true;
		} else if ((aDriverRecPtr->aShare->topCount > vertCount) && (aDriverRecPtr->aShare->bottomCount < vertCount)) goOut = true;
		if (!goOut) {
			do {} while (!(regr32(aDriverRecPtr, 0x609C) & 1));		//D1CRTC_STATUS
			if (!goOut) {
				fast_clk_program_for_vbl(aDriverRecPtr, aDriverRecPtr->aShare->array2[0]);
				aDriverRecPtr->aShare->array2[0] &= 0xFFFFFFF8;
			}
		}
	}
	if ((ISTmember.member == 8) && (aDriverRecPtr->aShare->array2[1] & (1 << 2))) {
		bool goOut = false;
		UInt32 vertCount = regr32(aDriverRecPtr, 0x68A0) & 0x1FFF;	//D2CRTC_STATUS_POSITION
		if (aDriverRecPtr->aShare->bottomCount > aDriverRecPtr->aShare->topCount) {
			if (aDriverRecPtr->aShare->bottomCount < vertCount) goOut = true;
			if (aDriverRecPtr->aShare->topCount > vertCount) goOut = true;
		} else if ((aDriverRecPtr->aShare->topCount > vertCount) && (aDriverRecPtr->aShare->bottomCount < vertCount))
			goOut = true;
		if (!goOut) {
			do {} while (!(regr32(aDriverRecPtr, 0x689C) & 1));		//D2CRTC_STATUS
			if (!goOut) {
				vertCount = regr32(aDriverRecPtr, 0x60A0) & 0x1FFF;
				if (aDriverRecPtr->aShare->vertCount > vertCount) goOut = true;
				if (aDriverRecPtr->aShare->bottomCount2 > aDriverRecPtr->aShare->topCount2) {
					if (aDriverRecPtr->aShare->bottomCount2 < vertCount) goOut = true;
					if (aDriverRecPtr->aShare->topCount2 > vertCount) goOut = true;
				} else if ((aDriverRecPtr->aShare->topCount2 > vertCount) && (aDriverRecPtr->aShare->bottomCount2 < vertCount))
					goOut = true;
			}
			if (!goOut) {
				fast_clk_program_for_vbl(aDriverRecPtr, aDriverRecPtr->aShare->array2[1]);
				aDriverRecPtr->aShare->array2[1] &= 0xFFFFFFF8;
			}
		}
	}
	if (ISTmember.member == 19) {
		UInt32 data = regr32(aDriverRecPtr, 0x1C);
		regw32(aDriverRecPtr, 0x1C, 0);
		if (data) {
			if ((aDriverRecPtr->unknownFunc != NULL) && aDriverRecPtr->unknown55)
				aDriverRecPtr->unknownFunc(aDriverRecPtr->unknown55, 0x2000000, data);
		} else {
			aDriverRecPtr->aShare->driverFlags |= (1 << 22);
			ISTmember.member = 5;
		}
	}
	if (ISTmember.member == 5) aDriverRecPtr->member = 48;
	if (ISTmember.member == 10) aDriverRecPtr->member = 49;
	if ((aDriverRecPtr->currentInt != NULL) && (aDriverRecPtr->member == kIsrIsComplete)) {
		int_status_vals[aDriverRecPtr->currentInt->dispNum] &= ~aDriverRecPtr->currentInt->intFlags;
		if ((aDriverRecPtr->currentInt->addr2 == 0x6534) || (aDriverRecPtr->currentInt->addr2 == 0x6D34) || (aDriverRecPtr->currentInt->addr2 == 0x44))
			regw32(aDriverRecPtr, aDriverRecPtr->currentInt->addr2, aDriverRecPtr->currentInt->value2);
		else regw32(aDriverRecPtr, aDriverRecPtr->currentInt->addr2, regr32(aDriverRecPtr, aDriverRecPtr->currentInt->addr2) | aDriverRecPtr->currentInt->value2);
		if ((aDriverRecPtr->currentInt->member == 5) || (aDriverRecPtr->currentInt->member == 48)) SetUpFP1InterruptPolarity(aDriverRecPtr, 1);
		if ((aDriverRecPtr->currentInt->member == 10) || (aDriverRecPtr->currentInt->member == 49)) SetUpFP2InterruptPolarity(aDriverRecPtr, 1);
		aDriverRecPtr->currentInt = NULL;
	}
	RearmMSInterrupt(aDriverRecPtr);
	return ISTmember.member;
}

InterruptMemberNumber MyInterruptHandler1 (InterruptSetMember ISTmember, void *refCon, UInt32 theIntCount) {
	VSLDoInterruptService(((DriverGlobal *) refCon)->aShare->serviceID[0]);	//only by guessing
	return 0;
}

InterruptMemberNumber MyInterruptHandler2 (InterruptSetMember ISTmember, void *refCon, UInt32 theIntCount) {
	VSLDoInterruptService(((DriverGlobal *) refCon)->aShare->serviceID[1]);	//only by guessing
	return 0;
}

bool DetectNewConnectionEnabled(DriverGlobal *aDriverRecPtr) {
	if (!(aDriverRecPtr->driverFlags & (1 << 6))) return false;
	if (aDriverRecPtr->driverFlags & ((1 << 11) + (1 << 18))) return false;
	if (aDriverRecPtr->aShare->serviceID[aDriverRecPtr->dispNum] == NULL) return false;
	return true;
}

void PostConnectionInterrupt(DriverGlobal *aDriverRecPtr) {
	if (!DetectNewConnectionEnabled(aDriverRecPtr)) return;
	if (!(aDriverRecPtr->driverFlags & (1 << 12))) return;
	if (aDriverRecPtr->driverFlags & (1 << 23)) return;
	if (aDriverRecPtr->dispNum >= aDriverRecPtr->connectorNum) aDriverRecPtr->aShare->driverFlags &= ~(1 << 22);
	aDriverRecPtr->driverFlags |= (1 << 7);
	VSLDoInterruptService(aDriverRecPtr->aShare->serviceID[aDriverRecPtr->dispNum]);
}

InterruptMemberNumber FPInterruptHandle (DriverGlobal *aDriverRecPtr) {
	PostConnectionInterrupt(aDriverRecPtr);
	return 0;
}

InterruptMemberNumber FP1InterruptHandler (InterruptSetMember ISTmember, void *refCon, UInt32 theIntCount) {
	return FPInterruptHandle((DriverGlobal *)refCon);
}

InterruptMemberNumber FP2InterruptHandler (InterruptSetMember ISTmember, void *refCon, UInt32 theIntCount) {
	return FPInterruptHandle((DriverGlobal *)refCon);
}

OSStatus InstallATIFunction(UInt32 intNumber, InterruptSetMember *setMembers, InterruptSetID setID, ATIInterrupt *intSet, InterruptEnabler enableFunction, InterruptDisabler disableFunction) {
	OSStatus ret;
	UInt8 i;
	for (i = 0; i < intNumber; i++) {
		setMembers[i].setID = setID;
		setMembers[i].member = intSet[i].member;
		ret = InstallInterruptFunctions(setMembers[i].setID, setMembers[i].member, NULL, NULL, enableFunction, disableFunction);
		if (ret != noErr) break;
	}
	return ret;
}

OSStatus ExtendATIInterruptTree(DriverGlobal *aDriverRecPtr) {
	OSStatus ret = CreateInterruptSet(aDriverRecPtr->setMember.setID, aDriverRecPtr->setMember.member, 56, &aDriverRecPtr->setID, 1);
	if (ret != noErr) return ret;
	ret = InstallInterruptFunctions(aDriverRecPtr->setMember.setID, aDriverRecPtr->setMember.member, (void *)aDriverRecPtr, &ATIIsrDispatcher, NULL, NULL);
	if (ret != noErr) return ret;
	InterruptSetMember setMembers[57];
	return InstallATIFunction(maxIntSet, setMembers, aDriverRecPtr->setID, atiIntSet, &ATIDeviceEnabler, &ATIDeviceDisabler);
}

OSStatus InitInterrupts(DriverGlobal *aDriverRecPtr) {
	OSStatus ret;
	RegEntryID* parent;
	RegCStrEntryName* name;
	Boolean done;
	
	aDriverRecPtr->driverFlags &= ~(1 << 5); //bit5 = 0
	aDriverRecPtr->aShare->driverFlags &= (1 << 5); //bits = 0 except bit5
	InitInterruptMask(aDriverRecPtr);
	if (RegGet(&aDriverRecPtr->regIDDevice, "driver-ist", aDriverRecPtr->setMember.setID, 8) != noErr) return qErr;
	ret = GetInterruptFunctions(aDriverRecPtr->setMember.setID, aDriverRecPtr->setMember.member, &aDriverRecPtr->refCon,
								&aDriverRecPtr->handlerFunction, &aDriverRecPtr->enableFunction, &aDriverRecPtr->disablerFunction);
	if (aDriverRecPtr->dispNum == 0) {
		ret = ExtendATIInterruptTree(aDriverRecPtr);
		if (ret == noErr) RegPrint(&aDriverRecPtr->regIDDevice, "my_int", &aDriverRecPtr->setID, 4);
	} else ret = RegGet(&aDriverRecPtr->regIDDevice, "my_int", aDriverRecPtr->setID, 4);
	
	if (ret == noErr) {
		aDriverRecPtr->driverFlags |= (1 << 5);
		aDriverRecPtr->aShare->driverFlags |= (1 << 5);
		if (aDriverRecPtr->dispNum != 0)
			ret = InstallInterruptFunctions(aDriverRecPtr->setID, 8, (void *) aDriverRecPtr,
											&MyInterruptHandler2, &ATIDeviceEnabler, &ATIDeviceDisabler);
		else ret = InstallInterruptFunctions(aDriverRecPtr->setID, 1, (void *) aDriverRecPtr,
											 &MyInterruptHandler1, &ATIDeviceEnabler, &ATIDeviceDisabler);
		if (aDriverRecPtr->dispNum == 0)
			ret = InstallInterruptFunctions(aDriverRecPtr->setID, 5, (void *) aDriverRecPtr,
											&FP1InterruptHandler, &ATIDeviceEnabler, &ATIDeviceDisabler);
		else ret = InstallInterruptFunctions(aDriverRecPtr->setID, 48, (void *) aDriverRecPtr,
											 &FP1InterruptHandler, &ATIDeviceEnabler, &ATIDeviceDisabler);
		if (!aDriverRecPtr->dispNum)
			ret = InstallInterruptFunctions(aDriverRecPtr->setID, 10, (void *) aDriverRecPtr,
											&FP2InterruptHandler, &ATIDeviceEnabler, &ATIDeviceDisabler);
		else ret = InstallInterruptFunctions(aDriverRecPtr->setID, 49, (void *) aDriverRecPtr,
											 &FP2InterruptHandler, &ATIDeviceEnabler, &ATIDeviceDisabler);
	}
	
	if (ret == noErr) {
		aDriverRecPtr->aShare->driverFlags |= (1 << 6);
		ret = RegistryCStrEntryToName(&aDriverRecPtr->regIDNub, parent, name, &done);
		ret = VSLNewInterruptService(&aDriverRecPtr->regIDNub, ' lbv', &aDriverRecPtr->serviceID);
		if (ret != noErr) aDriverRecPtr->serviceID = NULL;
		ret = VSLNewInterruptService(&aDriverRecPtr->regIDNub, ' icd', &aDriverRecPtr->aShare->serviceID[aDriverRecPtr->dispNum]);
		if (ret != noErr) aDriverRecPtr->aShare->serviceID[aDriverRecPtr->dispNum] = NULL;
		if (aDriverRecPtr->dispNum == 0) {
			aDriverRecPtr->enableFunction(aDriverRecPtr->setMember, aDriverRecPtr->refCon);
			RearmMSIInterrupt(aDriverRecPtr);
		} 
	}
	
	return ret;
}

void RF_MiscSetup(DriverGlobal *aDriverRecPtr) {
	while (ExpMgrConfigReadLong(&aDriverRecPtr->regIDDevice, (void *)16, &aDriverRecPtr->aShare->configBAR0) != noErr) {};
	regw32(aDriverRecPtr, 0x4C, 1);
	InitPCIExpress(aDriverRecPtr);
#ifdef ATY_Caretta
	aDriverRecPtr->pciXState = false;
#endif
#ifdef ATY_Wormy
	aDriverRecPtr->pciXState = ((regr16(aDriverRecPtr, 0x5068) & 3) == 2);	//only L1 entry enabled or not?
#endif
	if (aDriverRecPtr->pciXState) regw32(aDriverRecPtr, 0x164, 0xFC381);
	else regw32(aDriverRecPtr, 0x164, 0xFCB81);
	regw32(aDriverRecPtr, 0xEC, 0x444F);
	regw32(aDriverRecPtr, 0x6414, 0xFFFFFFFF);
	regw32(aDriverRecPtr, 0x6410, 0x3F003F);
	regw32(aDriverRecPtr, 0x6C14, 0xFFFFFFFF);
	regw32(aDriverRecPtr, 0x6C10, 0x3F003F);
	regw32(aDriverRecPtr, 0x7854, aDriverRecPtr->memConfig->atySettings.daca_cntl);
	regw32(aDriverRecPtr, 0x7A54, aDriverRecPtr->memConfig->atySettings.dacb_cntl);
	regw32(aDriverRecPtr, 0x7828, 0x70000);
	regw32(aDriverRecPtr, 0x7A28, 0x70000);
	pllw32(aDriverRecPtr, 0xC, 0);
	rmsw32(aDriverRecPtr, 0xB00, 0x100, 8, 0);
	UInt32 host_path_cntl = RADEON_HDP_APER_CNTL;
	if (aDriverRecPtr->noHDPAperCntl) host_path_cntl = ~ RADEON_HDP_APER_CNTL;
	regw32(aDriverRecPtr, 0x130, host_path_cntl);
	DisableHWInterrupts(aDriverRecPtr);
	regw32(aDriverRecPtr, 0x40, 0);
	regw32(aDriverRecPtr, 0x44, regr32(aDriverRecPtr, 0x44));
	regw8(aDriverRecPtr, 0x7D08, 1);
	regw8(aDriverRecPtr, 0x7D18, 1);
	kdelay(1);
	regw32(aDriverRecPtr, 0x7D10, 1);
	regw32(aDriverRecPtr, 0x7D00, 1);
	kdelay(10);
	RearmMSIInterrupt(aDriverRecPtr);
#ifdef ATY_Caretta
	SetupLM63FanCntrlLookUP(aDriverRecPtr);
#endif
	pllw32(aDriverRecPtr, 0x25, 0x1F41644);
	pllw32(aDriverRecPtr, 0x24, 0x1F41DB0);
	pllw32(aDriverRecPtr, 1, 0x20);
	pllw32(aDriverRecPtr, 5, 1);
	regw32(aDriverRecPtr, 0x6548, 0x1100000);
	regw32(aDriverRecPtr, 0x6D48, 0x1100000);
	regw32(aDriverRecPtr, 0x654C, 0x1100000);
	regw32(aDriverRecPtr, 0x6D4C, 0x1100000);
	regw32(aDriverRecPtr, 0x70, 0x7F0000);
}

void HW_InitTMDS(DriverGlobal *aDriverRecPtr) {
	regw32(aDriverRecPtr, 0x7880, regr32(aDriverRecPtr, 0x7880) & ~(1 << 8));	//LVTMB_CNTL, 8: HPD_SELECT, set to HPD1?
}

void HW_InitTMDS2(DriverGlobal *aDriverRecPtr) {
	regw32(aDriverRecPtr, 0x7A80, regr32(aDriverRecPtr, 0x7A80) | (1 << 8));	//LVTMA_CNTL, 8: HPD_SELECT, set to HPD2
}

void HW_InitLVDS(DriverGlobal *aDriverRecPtr) {
	//not implemented
}

void HW_InitDisplay(DriverGlobal *aDriverRecPtr) {
#ifdef ATY_Caretta
	HW_InitTMDS2(aDriverRecPtr);
#endif
#ifdef ATY_Wormy
	HW_InitTMDS(aDriverRecPtr);
	HW_InitLVDS(aDriverRecPtr);
#endif
	SetUpFP1InterruptPolarity(aDriverRecPtr, 1);
	SetUpFP2InterruptPolarity(aDriverRecPtr, 1);
}

#ifdef ATY_Caretta
static UInt32 reduce_power_max[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_def[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_dd[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_0[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_1[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_2[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_3[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_min[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
static UInt32 reduce_power_vddc[9] = {
500, 600, 1, 1, 1100, 1, 1, 130, 2
};
#endif

#ifdef ATY_Wormy
static UInt32 reduce_power_max[9] = {
423, 450, 1, 1, 1100, 1, 1, 14, 3
};
static UInt32 reduce_power_def[9] = {
423, 450, 1, 1, 1100, 1, 1, 14, 3
};
static UInt32 reduce_power_dd[9] = {
423, 350, 1, 1, 1100, 1, 1, 14, 3
};
static UInt32 reduce_power_0[9] = {
423, 450, 1, 1, 1100, 1, 1, 14, 3
};
static UInt32 reduce_power_1[9] = {
309, 300, 1, 1, 1100, 1, 1, 10, 4
};
static UInt32 reduce_power_2[9] = {
160, 140, 1, 1, 1100, 1, 1, 7, 0
};
static UInt32 reduce_power_3[9] = {
130, 135, 1, 1, 1100, 1, 1, 6, 0
};
static UInt32 reduce_power_min[9] = {
130, 135, 1, 1, 1100, 1, 1, 6, 0
};
static UInt32 reduce_power_vddc[9] = {
325, 350, 1, 1, 1100, 1, 1, 6, 0
};
#endif

const static UInt32 platform_reduce_power[8*9*9] = {
//#ifdef ATY_Wormy
309, 300, 1, 1, 1100, 1, 1, 12, 4, 200, 300, 1, 1, 1100, 1, 1, 10, 0, 160, 140, 1, 1, 1100, 1, 1, 8, 0,
130, 135, 1, 1, 1100, 1, 1, 5, 0, 309, 300, 1, 1, 1100, 1, 1, 12, 4, 309, 300, 1, 1, 1100, 1, 1, 12, 4, 
130, 135, 1, 1, 1100, 1, 1, 5, 0, 309, 300, 1, 1, 1100, 1, 1, 12, 4, 325, 350, 1, 1, 1100, 1, 1, 12, 4,
309, 300, 1, 1, 1100, 1, 1, 12, 4, 309, 300, 1, 1, 1100, 1, 1, 12, 4, 309, 300, 1, 1, 1100, 1, 1, 12, 4,
309, 300, 1, 1, 1100, 1, 1, 12, 4, 309, 300, 1, 1, 1100, 1, 1, 12, 4, 309, 300, 1, 1, 1100, 1, 1, 12, 4,
309, 300, 1, 1, 1100, 1, 1, 12, 4, 309, 300, 1, 1, 1100, 1, 1, 12, 4, 325, 350, 1, 1, 1100, 1, 1, 12, 4,
400, 400, 1, 1, 1100, 1, 1, 17, 4, 350, 350, 1, 1, 1100, 1, 1, 15, 4, 350, 350, 1, 1, 1100, 1, 1, 15, 4,
350, 350, 1, 1, 1100, 1, 1, 15, 4, 400, 400, 1, 1, 1100, 1, 1, 17, 4, 400, 400, 1, 1, 1100, 1, 1, 17, 4,
350, 350, 1, 1, 1100, 1, 1, 15, 4, 400, 400, 1, 1, 1100, 1, 1, 17, 4, 325, 350, 1, 1, 1100, 1, 1, 12, 4,
423, 450, 1, 1, 1100, 1, 1, 19, 3, 309, 300, 1, 1, 1100, 1, 1, 12, 0, 160, 140, 1, 1, 1100, 1, 1, 8, 0,
130, 135, 1, 1, 1100, 1, 1, 5, 0, 423, 450, 1, 1, 1100, 1, 1, 12, 3, 423, 450, 1, 1, 1100, 1, 1, 19, 3,
130, 135, 1, 1, 1100, 1, 1, 5, 0, 423, 333, 1, 1, 1100, 1, 1, 17, 3, 325, 350, 1, 1, 1100, 1, 1, 12, 4,
475, 500, 1, 1, 1100, 1, 1, 22, 2, 475, 500, 1, 1, 1100, 1, 1, 22, 2, 475, 500, 1, 1, 1100, 1, 1, 22, 2,
475, 500, 1, 1, 1100, 1, 1, 22, 2, 475, 500, 1, 1, 1100, 1, 1, 22, 2, 475, 500, 1, 1, 1100, 1, 1, 22, 2,
475, 500, 1, 1, 1100, 1, 1, 22, 2, 475, 500, 1, 1, 1100, 1, 1, 22, 2, 325, 350, 1, 1, 1100, 1, 1, 12, 4,
340, 360, 1, 1, 1100, 1, 1, 15, 4, 340, 360, 1, 1, 1100, 1, 1, 15, 4, 340, 360, 1, 1, 1100, 1, 1, 15, 4,
340, 360, 1, 1, 1100, 1, 1, 15, 4, 340, 360, 1, 1, 1100, 1, 1, 15, 4, 340, 360, 1, 1, 1100, 1, 1, 15, 4,
340, 360, 1, 1, 1100, 1, 1, 15, 4, 340, 360, 1, 1, 1100, 1, 1, 15, 4, 350, 360, 1, 1, 1100, 1, 1, 12, 4,
423, 450, 1, 1, 1100, 1, 1, 19, 3, 309, 300, 1, 1, 1100, 1, 1, 12, 0, 160, 171, 1, 1, 1100, 1, 1, 8, 0,
130, 135, 1, 1, 1100, 1, 1, 5, 0, 423, 450, 1, 1, 1100, 1, 1, 12, 3, 423, 450, 1, 1, 1100, 1, 1, 19, 3,
130, 135, 1, 1, 1100, 1, 1, 5, 0, 423, 333, 1, 1, 1100, 1, 1, 17, 3, 325, 350, 1, 1, 1100, 1, 1, 12, 4,
472, 475, 1, 1, 1100, 1, 1, 22, 2, 309, 300, 1, 1, 1100, 1, 1, 12, 0, 160, 171, 1, 1, 1100, 1, 1, 8, 0,
130, 135, 1, 1, 1100, 1, 1, 5, 0, 472, 475, 1, 1, 1100, 1, 1, 22, 2, 475, 475, 1, 1, 1100, 1, 1, 22, 2,
130, 135, 1, 1, 1100, 1, 1, 5, 0, 472, 333, 1, 1, 1100, 1, 1, 17, 2, 325, 350, 1, 1, 1100, 1, 1, 12, 4
//#endif
};

void RF_DynamicClockGating(DriverGlobal *aDriverRecPtr, bool unknown1) {
	if (unknown1) {
		pllw32(aDriverRecPtr, 8, pllr32(aDriverRecPtr, 8) | 5);
		if ((aDriverRecPtr->chipID & 0xF000000) <= 0x3FFFFFF) {	//check revID version
			pllw32(aDriverRecPtr, 0xF, 0xF0FF01F);
			//......
		} else {
			pllw32(aDriverRecPtr, 0xF, (pllr32(aDriverRecPtr, 0xF) & 0xFCFF00A) | 0x1A);
			//......
		}
		pllw32(aDriverRecPtr, 0xB, 0x103FAC8F);
		//......
	} else pllw32(aDriverRecPtr, 8, pllr32(aDriverRecPtr, 8) & ~1);
}

void HW_EnableDynamicMode(DriverGlobal *aDriverRecPtr) {
	RF_DynamicClockGating(aDriverRecPtr, 1);
}

void RF_ResetPowerManager(DriverGlobal *aDriverRecPtr, UInt32 *powerState) {
	UInt8 i;
	for (i = 0;i < 7;i++) powerState[i] = 0xFFFFFFFF;
}

OSStatus RF_DefaultPowerManager(DriverGlobal *aDriverRecPtr, UInt32 *reducePower) {
	UInt8 i;
	for (i = 0;i < 9;i++) reducePower[i] = reduce_power_0[i];
	aDriverRecPtr->aShare->unknown13 = 0;
	aDriverRecPtr->aShare->clock2 = reduce_power_def[0] * 100;
	aDriverRecPtr->aShare->sysClock = aDriverRecPtr->aShare->clock2;
	aDriverRecPtr->aShare->clock1 = reduce_power_def[1] * 100;
	aDriverRecPtr->aShare->memClock = aDriverRecPtr->aShare->clock1;
	aDriverRecPtr->aShare->unknown15 = reduce_power_def[9];
	return noErr;
}

void HW_SetupPowerManager(DriverGlobal *aDriverRecPtr) {
#ifdef ATY_Wormy
	RF_ResetPowerManager(aDriverRecPtr, aDriverRecPtr->aShare->powerState);
	RF_DefaultPowerManager(aDriverRecPtr, aDriverRecPtr->aShare->reducePower);
#endif
}

OSStatus InitHardware(DriverGlobal *aDriverRecPtr) {
	if (aDriverRecPtr->dispNum) {	//already initialized before, just check video ram size
		if (aDriverRecPtr->aShare->totalVram != 0) return noErr;
		else return controlErr;
	}
	aDriverRecPtr->aShare->sysClock = HW_GetSClk(aDriverRecPtr);
	aDriverRecPtr->aShare->memClock = HW_GetMClk(aDriverRecPtr);
	aDriverRecPtr->aShare->clock1 = aDriverRecPtr->aShare->memClock;
	aDriverRecPtr->aShare->clock2 = aDriverRecPtr->aShare->sysClock;
	RegPrint(&aDriverRecPtr->regIDDevice, "ATY,SCLK", &aDriverRecPtr->aShare->sysClock, 4);
	RegPrint(&aDriverRecPtr->regIDDevice, "ATY,MCLK", &aDriverRecPtr->aShare->memClock, 4);
#ifdef ATY_Wormy
	SetPerformanceWeights(aDriverRecPtr);
#endif
	InitHWSetting(aDriverRecPtr);
	HW_InitMEMConfiguration(aDriverRecPtr);
	if (aDriverRecPtr->aShare->totalVram == 0) return controlErr;
	RF_MiscSetup(aDriverRecPtr);
	HW_InitDisplay(aDriverRecPtr);
	HW_EnableDynamicMode(aDriverRecPtr);
	HW_SetupPowerManager(aDriverRecPtr);
	RegPrint(&aDriverRecPtr->regIDDevice, "ATY,memsize", &aDriverRecPtr->aShare->configMemSize, 4);
	RegPrint(&aDriverRecPtr->regIDDevice, "VRAM,totalsize", &aDriverRecPtr->aShare->configMemSize, 4);
	return noErr;
}

void InitPlatformInfo(DriverGlobal *aDriverRecPtr) {
	switch (aDriverRecPtr->atyPlatformInf.unknown1) {
		case 0:
			aDriverRecPtr->unknown53 = false;
			aDriverRecPtr->unknown44 = 9;
			aDriverRecPtr->reducePowerIndex = 0;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->clamClose = true;
			aDriverRecPtr->aShare->unknown9 = 200;
			aDriverRecPtr->aShare->unknown10 = 500;
			break;
		case 1:
			aDriverRecPtr->unknown53 = true;
			aDriverRecPtr->unknown44 = 11;
			aDriverRecPtr->reducePowerIndex = 1;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->clamClose = false;
			aDriverRecPtr->aShare->unknown9 = 0;
			aDriverRecPtr->aShare->unknown10 = 0;
			break;
		case 2:
			aDriverRecPtr->unknown53 = true;
			aDriverRecPtr->unknown44 = 11;
			aDriverRecPtr->reducePowerIndex = 5;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->clamClose = false;
			aDriverRecPtr->aShare->unknown9 = 0;
			aDriverRecPtr->aShare->unknown10 = 0;
			break;
		case 3:
			aDriverRecPtr->unknown53 = true;
			aDriverRecPtr->unknown44 = 11;
			aDriverRecPtr->reducePowerIndex = 2;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->clamClose = false;
			aDriverRecPtr->aShare->unknown9 = 0;
			aDriverRecPtr->aShare->unknown10 = 0;
			break;
		case 4:
			aDriverRecPtr->unknown53 = true;
			aDriverRecPtr->unknown44 = 11;
			aDriverRecPtr->reducePowerIndex = 4;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->clamClose = false;
			aDriverRecPtr->aShare->unknown9 = 0;
			aDriverRecPtr->aShare->unknown10 = 0;
			break;
		case 5:
			aDriverRecPtr->unknown53 = false;
			aDriverRecPtr->unknown44 = 11;
			aDriverRecPtr->reducePowerIndex = 3;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->clamClose = true;
			aDriverRecPtr->aShare->unknown9 = 200;
			aDriverRecPtr->aShare->unknown10 = 500;
			break;
		case 6:
			aDriverRecPtr->unknown53 = false;
			aDriverRecPtr->unknown44 = 11;
			aDriverRecPtr->reducePowerIndex = 7;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->unknown9 = 200;
			aDriverRecPtr->clamClose = true;
			aDriverRecPtr->aShare->unknown9 = 400;
			aDriverRecPtr->aShare->unknown10 = 500;
			break;
		case 7:
			aDriverRecPtr->unknown53 = false;
			aDriverRecPtr->unknown44 = 9;
			aDriverRecPtr->reducePowerIndex = 6;
			aDriverRecPtr->unknown8 = 0;
			aDriverRecPtr->unknown9 = 200;
			aDriverRecPtr->clamClose = true;
			aDriverRecPtr->aShare->unknown9 = 400;
			aDriverRecPtr->aShare->unknown10 = 500;
			break;
		default:
			break;
	}
	UInt8 i;
	for (i = 0;i < 9;i++) {
		reduce_power_0[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i];
		reduce_power_1[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 9];
		reduce_power_2[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 18];
		reduce_power_3[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 27];
		reduce_power_def[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 36];
		reduce_power_max[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 45];
		reduce_power_min[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 54];
		reduce_power_dd[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 63];
		reduce_power_vddc[i] = platform_reduce_power[aDriverRecPtr->reducePowerIndex * 9 * 9 + i + 72];
	}
}

void kdelay(SInt32 time) {
	if (time < 0) IODelay(-time);
	else IODelay(time);
}

void regw8(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt8 value) {
	*(volatile UInt8 *)((UInt8 *) (aDriverRecPtr->baseIOMap) + addr) = value;
	SynchronizeIO();
}

void regw16(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt16 value) {
	*(volatile UInt16 *)((UInt8 *) (aDriverRecPtr->baseIOMap) + addr) = value;
}

void regw32(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt32 value) {
	*(volatile UInt32 *)((UInt8 *) (aDriverRecPtr->baseIOMap) + addr) = value;
}

UInt8 regr8(DriverGlobal *aDriverRecPtr, UInt16 addr) {
	SynchronizeIO();
	return *(volatile UInt8 *)((UInt8 *) (aDriverRecPtr->baseIOMap) + addr);
}

UInt16 regr16(DriverGlobal *aDriverRecPtr, UInt16 addr) {
	return *(volatile UInt16 *)((UInt8 *) (aDriverRecPtr->baseIOMap) + addr);
}

UInt32 regr32(DriverGlobal *aDriverRecPtr, UInt16 addr) {
	return *(volatile UInt32 *)((UInt8 *) (aDriverRecPtr->baseIOMap) + addr);
}

void rmsw32(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt32 mask, UInt8 offset, UInt32 value) {
	regw32(aDriverRecPtr, addr, regr32(aDriverRecPtr, addr) & ~mask | ((value << offset) & mask));
}

UInt32 rmsr32(DriverGlobal *aDriverRecPtr, UInt16 addr, UInt32 mask, UInt8 offset) {
	return (regr32(aDriverRecPtr, addr) & mask) >> offset;
}

void pllw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value) {
	regw8(aDriverRecPtr, 8, index | (1 << 7));
	regw32(aDriverRecPtr, 0xC, value);
}

UInt32 pllr32(DriverGlobal *aDriverRecPtr, UInt8 index) {
	regw8(aDriverRecPtr, 8, index);	//CLOCK_CNTL_INDEX
	return regr32(aDriverRecPtr, 0xC);	//CLOCK_CNTL_DATA
}

void pllmsw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 mask, UInt8 offset, UInt32 value) {
	pllw32(aDriverRecPtr, index, pllr32(aDriverRecPtr, index) & ~mask | ((value << offset) & mask));
}

UInt32 pllmsr32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 mask, UInt8 offset) {
	return (pllr32(aDriverRecPtr, index) & mask) >> offset;
}

void memws32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value, UInt32 mask) {
	regw32(aDriverRecPtr, 0x70, index | (mask & 0x7F0000) | 0x800000);
	regw32(aDriverRecPtr, 0x74, value);
}

#ifdef ATY_Caretta
void memw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value) {
	regw32(aDriverRecPtr, 0x70, index);
	regw32(aDriverRecPtr, 0x74, value);
}
#endif
#ifdef ATY_Wormy
void memw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value) {
	regw32(aDriverRecPtr, 0x70, index | (regr32(aDriverRecPtr, 0x70) & 0x7F0000) | 0x800000);
	regw32(aDriverRecPtr, 0x74, value);
}
#endif

UInt32 memrs32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 mask) {
	regw32(aDriverRecPtr, 0x70, index | (mask & 0x7F0000));
	return regr32(aDriverRecPtr, 0x74);
}

#ifdef ATY_Caretta
UInt32 memr32(DriverGlobal *aDriverRecPtr, UInt16 index) {
	regw32(aDriverRecPtr, 0x70, index);	//MC_INDEX
	return regr32(aDriverRecPtr, 0x74);	//MC_DATA
}
#endif
#ifdef ATY_Wormy
UInt32 memr32(DriverGlobal *aDriverRecPtr, UInt16 index) {
	regw32(aDriverRecPtr, 0x70, index | (regr32(aDriverRecPtr, 0x70) & 0x7F0000));	//MC_INDEX
	return regr32(aDriverRecPtr, 0x74);	//MC_DATA
}
#endif

UInt32 idx_regr32(DriverGlobal *aDriverRecPtr, UInt32 index) {
	regw32(aDriverRecPtr, 0, index);
	return regr32(aDriverRecPtr, 4);
}

void idx_regw32(DriverGlobal *aDriverRecPtr, UInt32 index, UInt32 value) {
	regw32(aDriverRecPtr, 0, index);
	regw32(aDriverRecPtr, 4, value);
}

void pciXw32(DriverGlobal *aDriverRecPtr, UInt16 index, UInt32 value) {
	regw32(aDriverRecPtr, 0x30, index);	//BUS_CNTL
	regw32(aDriverRecPtr, 0x34, value);	//BUS_CNTL2
}

UInt32 pciXr32(DriverGlobal *aDriverRecPtr, UInt16 index) {
	regw32(aDriverRecPtr, 0x30, index);
	return regr32(aDriverRecPtr, 0x34);
}

bool IsDetailMode(DriverGlobal *aDriverRecPtr, UInt32 modeID) {	//EDID contains a maxim of 4 detailed modes
	if ((modeID > 0x7FA) && (modeID < 0x800)) return true;		//reserved for CRT
	if ((modeID > 0x5FFA) && (modeID < 0x6000)) return true;	//reserved for DFP, LVDS
	return false;
}

UInt8 HALGetTimingFlagsRec(DriverGlobal *aDriverRecPtr, CrtcValues *mode, UInt8 connection) {
#ifdef ATY_Caretta
	const UInt32 TimingFlagsControlSize = 5;
	const TimingFlags TimingFlagsControl[5] = {
		{0x3000, 0x33FF, 1 << 0},			//none
		{0x400, 0x7FF, 1 << 4},				//CRT1
		{0x80000000, 0xFFFFFFFB, 1 << 4},	//CRT1
		{0x5C00, 0x5FFF, 1 << 3},			//DFP2
		{0x80000000, 0xFFFFFFFB, 1 << 3}	//DFP2
	};
#endif
#ifdef ATY_Wormy
	const UInt32 TimingFlagsControlSize = 8;
	const TimingFlags TimingFlagsControl[8] = {
		{0x3000, 0x33FF, 1 << 0},			//none
		{0xC00, 0x13FF, 1 << 1},			//TV
		{0x400, 0x7FF, 1 << 5},				//CRT2
		{0x80000000, 0xFFFFFFFB, 1 << 5},	//CRT2
		{0x5C00, 0x5FFF, 1 << 2},			//DFP1
		{0x80000000, 0xFFFFFFFB, 1 << 2},	//DFP1
		{0x5C00, 0x5FFF, 1 << 6},			//LVDS
		{0x80000000, 0xFFFFFFFB, 1 << 6},	//LVDS
	};
#endif
	
	if (connection == 0) return 0;
	if (mode->modeID == kDisplayModeIDInvalid) return connection;
#ifdef ATY_Wormy
	if (IsDetailMode(aDriverRecPtr, mode->modeID) && (mode != NULL) && (mode->mFlag && 0x2020)) {
		if (aDriverRecPtr->activeFlags & (1 << 1)) return (1 << 1);		//TV
		else return 0;
	} else
#endif
	{
		UInt8 i;
		for (i = 0;i < TimingFlagsControlSize;i++) {
			if ((TimingFlagsControl[i].minID <= mode->modeID) && (TimingFlagsControl[i].maxID >= mode->modeID)
				&& ((TimingFlagsControl[i].connection & connection) == TimingFlagsControl[i].connection))
				return TimingFlagsControl[i].connection;
		}
		return 0;
	}
}

#ifdef ATY_Caretta
const UInt8 TimingTableLen = 49;
const CrtcValues PredefinedTimingTable[49] = {
{0x407, 150, 0x0, 2518, 640, 800, 8, 96, 480, 525, 2, 2},
{0x408, 140, 0x0, 3024, 640, 864, 64, 64, 480, 525, 3, 3},
{0x409, 152, 0x0, 3150, 640, 832, 16, 40, 480, 520, 1, 3},
{0x40A, 154, 0x0, 3150, 640, 840, 16, 64, 480, 500, 1, 3},
{0x40B, 158, 0x0, 3600, 640, 832, 56, 56, 480, 509, 1, 3},
{0x40C, 0, 0x0, 3789, 640, 832, 32, 64, 480, 506, 1, 3},
{0x40D, 0, 0x0, 4316, 640, 848, 40, 64, 480, 509, 1, 3},
{0x40E, 0, 0x0, 5240, 640, 848, 40, 64, 480, 515, 1, 3},
{0x410, 180, 0x3, 3600, 800, 1024, 24, 72, 600, 625, 1, 2},
{0x411, 182, 0x3, 4000, 800, 1056, 40, 128, 600, 628, 1, 4},
{0x412, 184, 0x3, 5000, 800, 1040, 56, 120, 600, 666, 37, 6},
{0x413, 186, 0x3, 4950, 800, 1056, 16, 80, 600, 625, 1, 3},
{0x414, 188, 0x3, 5625, 800, 1048, 32, 64, 600, 631, 1, 3},
{0x415, 0, 0x3, 6006, 800, 1056, 40, 88, 600, 632, 1, 3},
{0x416, 0, 0x3, 6818, 800, 1072, 48, 88, 600, 636, 1, 3},
{0x417, 0, 0x3, 8395, 800, 1088, 56, 88, 600, 643, 1, 3},
{0x418, 170, 0x0, 5728, 832, 1152, 32, 64, 624, 667, 1, 3},
{0x41A, 190, 0x0, 6500, 1024, 1344, 24, 136, 768, 806, 3, 6},
{0x41B, 200, 0x0, 7500, 1024, 1328, 24, 136, 768, 806, 3, 6},
{0x41C, 204, 0x3, 7875, 1024, 1312, 16, 96, 768, 800, 1, 3},
{0x41E, 208, 0x3, 9450, 1024, 1376, 48, 96, 768, 808, 1, 3},
{0x41F, 0, 0x0, 10019, 1024, 1376, 64, 112, 768, 809, 1, 3},
{0x420, 0, 0x0, 11331, 1024, 1392, 72, 112, 768, 814, 1, 3},
{0x421, 0, 0x0, 13905, 1024, 1408, 80, 112, 768, 823, 1, 3},
{0x422, 220, 0x0, 10000, 1152, 1456, 32, 128, 870, 915, 3, 3},
{0x439, 252, 0x3, 10800, 1280, 1800, 96, 112, 960, 1000, 1, 3},
{0x423, 250, 0x3, 12600, 1280, 1680, 16, 144, 960, 1000, 1, 3},
{0x43A, 254, 0x3, 14850, 1280, 1728, 64, 160, 960, 1011, 1, 3},
{0x424, 260, 0x3, 10800, 1280, 1688, 48, 112, 1024, 1066, 1, 3},
{0x425, 262, 0x3, 13500, 1280, 1688, 16, 144, 1024, 1066, 1, 3},
{0x426, 268, 0x3, 15750, 1280, 1728, 64, 160, 1024, 1072, 1, 3},
{0x427, 280, 0x3, 16200, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x428, 282, 0x3, 17550, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x429, 284, 0x3, 18900, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x42A, 286, 0x3, 20250, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x42B, 289, 0x3, 22950, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x42C, 510, 0x0, 15984, 1920, 2368, 32, 144, 1080, 1125, 3, 3},
{0x42D, 520, 0x0, 21600, 1920, 2560, 48, 216, 1080, 1172, 3, 3},
{0x42E, 540, 0x0, 24320, 1920, 2560, 64, 224, 1200, 1250, 3, 3},
{0x430, 500, 0x0, 17045, 1600, 2096, 32, 160, 1024, 1070, 3, 3},
{0x431, 296, 0x2, 20475, 1792, 2448, 128, 200, 1344, 1394, 1, 3},
{0x432, 298, 0x2, 26100, 1792, 2456, 96, 216, 1344, 1417, 1, 3},
{0x433, 300, 0x2, 21825, 1856, 2528, 96, 224, 1392, 1439, 1, 3},
{0x434, 302, 0x2, 28800, 1856, 2560, 128, 224, 1392, 1500, 1, 3},
{0x435, 304, 0x2, 23400, 1920, 2600, 128, 208, 1440, 1500, 1, 3},
{0x436, 306, 0x2, 29700, 1920, 2640, 144, 224, 1440, 1500, 1, 3},
{0x437, 0, 0x2, 26695, 2048, 2800, 152, 224, 1536, 1589, 1, 3},
{0x438, 0, 0x2, 34047, 2048, 2832, 168, 224, 1536, 1603, 1, 3},		//Invalid mode
{0x3000, 550, 0x0, 0, 1, 0, 0, 0, 1, 0, 0, 0}						//Offline mode
};
#endif
#ifdef ATY_Wormy
const UInt8 TimingTableLen = 66;
const CrtcValues PredefinedTimingTable[66] = {
{0xC01, 232, 0x2000, 2880, 640, 800, 8, 32, 480, 600, 1, 2},
{0x1001, 238, 0x20, 2400, 640, 800, 8, 32, 480, 600, 1, 2},
{0xC02, 232, 0x2000, 3240, 720, 900, 8, 32, 480, 600, 1, 2},
{0xC05, 232, 0x2000, 3240, 856, 900, 8, 32, 480, 600, 1, 2},
{0x1006, 238, 0x20, 2700, 856, 900, 8, 32, 480, 600, 1, 2},
{0x1002, 238, 0x20, 3184, 720, 892, 8, 32, 576, 714, 1, 2},
{0x1003, 238, 0x20, 3184, 768, 892, 8, 32, 576, 714, 1, 2},
{0x1004, 238, 0x20, 3690, 800, 992, 8, 32, 600, 744, 1, 4},
{0xC03, 232, 0x2000, 4500, 800, 1000, 8, 32, 600, 750, 1, 2},
{0x1007, 238, 0x20, 4284, 1024, 1200, 8, 32, 576, 714, 1, 2},
{0xC06, 232, 0x2000, 5140, 1024, 1200, 8, 32, 576, 714, 1, 2},
{0x1005, 238, 0x20, 5416, 1024, 1344, 8, 32, 768, 806, 1, 2},
{0xC04, 232, 0x2000, 6500, 1024, 1344, 8, 32, 768, 806, 1, 2},
{0x1008, 238, 0x20, 7425, 1280, 1980, 110, 40, 720, 750, 5, 5},
{0xC07, 232, 0x2000, 7425, 1280, 1650, 110, 40, 720, 750, 5, 5},
{0xCFF, 232, 0x2008, 7425, 2262, 2730, 55, 180, 480, 525, 3, 6},
{0x10FF, 240, 0x28, 7425, 2768, 3405, 68, 250, 576, 625, 6, 3},
{0x407, 150, 0x0, 2518, 640, 800, 8, 96, 480, 525, 2, 2},
{0x408, 140, 0x0, 3024, 640, 864, 64, 64, 480, 525, 3, 3},
{0x409, 152, 0x0, 3150, 640, 832, 16, 40, 480, 520, 1, 3},
{0x40A, 154, 0x0, 3150, 640, 840, 16, 64, 480, 500, 1, 3},
{0x40B, 158, 0x0, 3600, 640, 832, 56, 56, 480, 509, 1, 3},
{0x40C, 0, 0x0, 3789, 640, 832, 32, 64, 480, 506, 1, 3},
{0x40D, 0, 0x0, 4316, 640, 848, 40, 64, 480, 509, 1, 3},
{0x40E, 0, 0x0, 5240, 640, 848, 40, 64, 480, 515, 1, 3},
{0x410, 180, 0x3, 3600, 800, 1024, 24, 72, 600, 625, 1, 2},
{0x411, 182, 0x3, 4000, 800, 1056, 40, 128, 600, 628, 1, 4},
{0x412, 184, 0x3, 5000, 800, 1040, 56, 120, 600, 666, 37, 6},
{0x413, 186, 0x3, 4950, 800, 1056, 16, 80, 600, 625, 1, 3},
{0x414, 188, 0x3, 5625, 800, 1048, 32, 64, 600, 631, 1, 3},
{0x415, 0, 0x3, 6006, 800, 1056, 40, 88, 600, 632, 1, 3},
{0x416, 0, 0x3, 6818, 800, 1072, 48, 88, 600, 636, 1, 3},
{0x417, 0, 0x3, 8395, 800, 1088, 56, 88, 600, 643, 1, 3},
{0x418, 170, 0x0, 5728, 832, 1152, 32, 64, 624, 667, 1, 3},
{0x41A, 190, 0x0, 6500, 1024, 1344, 24, 136, 768, 806, 3, 6},
{0x41B, 200, 0x0, 7500, 1024, 1328, 24, 136, 768, 806, 3, 6},
{0x41C, 204, 0x3, 7875, 1024, 1312, 16, 96, 768, 800, 1, 3},
{0x41E, 208, 0x3, 9450, 1024, 1376, 48, 96, 768, 808, 1, 3},
{0x41F, 0, 0x0, 10019, 1024, 1376, 64, 112, 768, 809, 1, 3},
{0x420, 0, 0x0, 11331, 1024, 1392, 72, 112, 768, 814, 1, 3},
{0x421, 0, 0x0, 13905, 1024, 1408, 80, 112, 768, 823, 1, 3},
{0x422, 220, 0x0, 10000, 1152, 1456, 32, 128, 870, 915, 3, 3},
{0x439, 252, 0x3, 10800, 1280, 1800, 96, 112, 960, 1000, 1, 3},
{0x423, 250, 0x3, 12600, 1280, 1680, 16, 144, 960, 1000, 1, 3},
{0x43A, 254, 0x3, 14850, 1280, 1728, 64, 160, 960, 1011, 1, 3},
{0x424, 260, 0x3, 10800, 1280, 1688, 48, 112, 1024, 1066, 1, 3},
{0x425, 262, 0x3, 13500, 1280, 1688, 16, 144, 1024, 1066, 1, 3},
{0x426, 268, 0x3, 15750, 1280, 1728, 64, 160, 1024, 1072, 1, 3},
{0x427, 280, 0x3, 16200, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x428, 282, 0x3, 17550, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x429, 284, 0x3, 18900, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x42A, 286, 0x3, 20250, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x42B, 289, 0x3, 22950, 1600, 2160, 64, 192, 1200, 1250, 1, 3},
{0x42C, 510, 0x0, 15984, 1920, 2368, 32, 144, 1080, 1125, 3, 3},
{0x42D, 520, 0x0, 21600, 1920, 2560, 48, 216, 1080, 1172, 3, 3},
{0x42E, 540, 0x0, 24320, 1920, 2560, 64, 224, 1200, 1250, 3, 3},
{0x430, 500, 0x0, 17045, 1600, 2096, 32, 160, 1024, 1070, 3, 3},
{0x431, 296, 0x2, 20475, 1792, 2448, 128, 200, 1344, 1394, 1, 3},
{0x432, 298, 0x2, 26100, 1792, 2456, 96, 216, 1344, 1417, 1, 3},
{0x433, 300, 0x2, 21825, 1856, 2528, 96, 224, 1392, 1439, 1, 3},
{0x434, 302, 0x2, 28800, 1856, 2560, 128, 224, 1392, 1500, 1, 3},
{0x435, 304, 0x2, 23400, 1920, 2600, 128, 208, 1440, 1500, 1, 3},
{0x436, 306, 0x2, 29700, 1920, 2640, 144, 224, 1440, 1500, 1, 3},
{0x437, 0, 0x2, 26695, 2048, 2800, 152, 224, 1536, 1589, 1, 3},
{0x438, 0, 0x2, 34047, 2048, 2832, 168, 224, 1536, 1603, 1, 3},		//Invalid mode
{0x3000, 550, 0x0, 0, 1, 0, 0, 0, 1, 0, 0, 0}						//Offline mode
};
#endif

CrtcValues * HALFindPredefinedCrtcValues(DriverGlobal *aDriverRecPtr, UInt32 modeID) {
	UInt8 i;
	for (i = 0;i < TimingTableLen;i++) {
		if (modeID == PredefinedTimingTable[i].modeID) return (CrtcValues *) &PredefinedTimingTable[i];
	}
	return NULL;
}

UInt32 HALGetRefreshRate(DriverGlobal *aDriverRecPtr, CrtcValues *mode) {
	if ((mode == NULL) || (mode->clock == NULL) || (mode->HTotal == NULL) || (mode->VTotal == NULL)) return 0;
	return (mode->clock * 20000 / mode->HTotal / mode->VTotal + 1) / 2;
}

void HALConvertAlias(DriverGlobal *aDriverRecPtr, UInt32 *modeID) {	//get modeID from aliasID
	if ((modeID == NULL) || (*modeID == 0)) return;
	UInt8 i;
	for (i = 0;i < 4;i++) {
		if (aDriverRecPtr->DTState[i].modeAlias == *modeID) {
			*modeID = aDriverRecPtr->DTState[i].modeID;
			return;
		}
	}
}

CrtcValues* HALGetNextCrtc(DriverGlobal *aDriverRecPtr, CrtcValues *mode) {
	if (aDriverRecPtr->modesNum == 0) return NULL; //no modes available
	if (mode == &aDriverRecPtr->modes[aDriverRecPtr->modesNum - 1]) return NULL; //reach last mode
	if (mode == &aDriverRecPtr->detailModes[3]) return NULL; //reach last detail mode
	return (CrtcValues *) (mode + sizeof(CrtcValues)); //next mode, each size 0x1C
}

CrtcValues* HALFindCrtcValues(DriverGlobal *aDriverRecPtr, UInt32 modeID) {
	UInt32 id = modeID;
	CrtcValues *mode = aDriverRecPtr->modes;
	HALConvertAlias(aDriverRecPtr, &id);
	if (IsDetailMode(aDriverRecPtr, id)) mode = aDriverRecPtr->detailModes;
	if (aDriverRecPtr->modesNum == 0) return NULL;
	while (mode != NULL) {
		if (mode->modeID == id) return mode;
		mode = HALGetNextCrtc(aDriverRecPtr, mode);
	}
	return NULL;
}

CrtcValues * HALFindNativeTiming(DriverGlobal *aDriverRecPtr, CrtcValues *mode, bool unknown1) {
	CrtcValues *newMode;
	
	if (unknown1 && (mode->mFlag & 0x20A0)) return mode;	//bit 5, 7, 13
	if (mode->mFlag & (1 << 13)) return HALFindPredefinedCrtcValues(aDriverRecPtr, 0xCFF);
	if (mode->mFlag & (1 << 5)) return HALFindPredefinedCrtcValues(aDriverRecPtr, 0x10FF);
	if (mode->mFlag & (1 << 7)) {
		UInt32 refRate = HALGetRefreshRate(aDriverRecPtr, mode);
		UInt32 modeID = 0;
		switch (mode->VTotal) {
			case 525:
				modeID = (mode->mFlag & (1 << 3))?0x5414:0x5802;	//interlaced?
				break;
			case 625:
				modeID = (mode->mFlag & (1 << 3))?0x5415:0x580C;	//interlaced?
				break;
			case 750:
				if (mode->mFlag & (1 << 3)) return NULL;	//interlaced?
				modeID = (refRate == 50)?0x580D:0x5808;
				break;
			case 1125:
				if (mode->mFlag & (1 << 3)) return NULL;	//interlaced?
				if (refRate == 50 || 25) modeID = 0x5412;
				if (refRate == 60 || 30) modeID = 0x5411;
				break;
		}
		if (modeID) return HALFindPredefinedCrtcValues(aDriverRecPtr, modeID);
	}
	UInt8 i;
	for (i = 0;i < aDriverRecPtr->aliasIDsNum;i++) {
		if (mode->modeID == aDriverRecPtr->aliasIDs[i].aliasModeID) {
			newMode = HALFindCrtcValues(aDriverRecPtr, aDriverRecPtr->aliasIDs[i].modeID);
			if (newMode == NULL) newMode = HALFindPredefinedCrtcValues(aDriverRecPtr, aDriverRecPtr->aliasIDs[i].modeID);
			if (newMode == NULL) newMode = mode;
			break;
		}
	}
	return newMode;
}

OSStatus HALGetDetailTimingState(DriverGlobal *aDriverRecPtr, UInt32 modeID, DetailTimingState* DTState) {
	DTState = aDriverRecPtr->DTState;
	UInt8 i;
	for(i = 0;i < 4;i++) {
		if (aDriverRecPtr->DTState[i].modeID == modeID) {
			DTState = &aDriverRecPtr->DTState[i];
			return noErr;
		}
	}
	return paramErr;
}

bool HALFillNativeCRTCParams(DriverGlobal *aDriverRecPtr, CrtcValues *mode, CrtcValues* newMode) {
	if ((mode == NULL) || (newMode == NULL)) return false;
	CrtcValues *nativeTiming = HALFindNativeTiming(aDriverRecPtr, mode, false);
	if (nativeTiming == NULL) return false;
	newMode->modeID = nativeTiming->modeID;
	newMode->timingMode = nativeTiming->timingMode;
	newMode->mFlag = nativeTiming->mFlag;
	newMode->clock = nativeTiming->clock;
	newMode->width = nativeTiming->width;
	newMode->HTotal = nativeTiming->HTotal;
	newMode->dHSyncS = nativeTiming->dHSyncS;
	newMode->dHSyncE = nativeTiming->dHSyncE;
	newMode->height = nativeTiming->height;
	newMode->VTotal = nativeTiming->VTotal;
	newMode->dVSyncS = nativeTiming->dVSyncS;
	newMode->dVSyncE = nativeTiming->dVSyncE;
	if (mode->mFlag & 0x20A0) return true;	//bit 5, 7, 13
	DetailTimingState *DTState;
	if (HALGetDetailTimingState(aDriverRecPtr, mode->modeID, DTState) == noErr) {
		if (DTState->modeState == kDMSModeFree) return false;
		newMode->width = DTState->width;
		newMode->height = DTState->height;
	}
	return true;
}

void HALFindUnderscan(DriverGlobal *aDriverRecPtr, UInt32 modeID, UInt32* usWidth, UInt32* usHeight) {
	DetailTimingState *DTState;
	if (HALGetDetailTimingState(aDriverRecPtr, modeID, DTState) != noErr) return;
	*usWidth -= DTState->horiInset * 2;
	*usHeight -= DTState->verInset * 2;
}

void CalculateScaledParams(DriverGlobal *aDriverRecPtr, CrtcValues *mode, CrtcValues *scaledMode, ScaledParameters* scaledPara) {
	scaledPara->width = mode->width;
	scaledPara->height = mode->height;
	UInt32 destWidth = scaledMode->width;		//var_18
	UInt32 destHeight = scaledMode->height;		//var_1C
	HALFindUnderscan(aDriverRecPtr, mode->modeID, &destWidth, &destHeight);
	destWidth = (scaledMode->width > destWidth)?destWidth:scaledMode->width;
	destHeight = (scaledMode->height > destHeight)?destHeight:scaledMode->height;
	scaledPara->width = destWidth;
	scaledPara->height = destHeight;
	
	if (mode->mFlag & 0x2020) {		//bit13, 5
		destWidth = mode->width;
		destHeight = mode->height;
		if (aDriverRecPtr->currentDispPara.unknown9) destWidth = destHeight * 4 / 3;
		else if (aDriverRecPtr->currentDispPara.unknown10) destWidth = destHeight * 16 / 9;
	}
	
	if (mode->mFlag & 0x80) {		//bit7
		destWidth = mode->width;
		destHeight = mode->height;
		if (!(mode->mFlag & (1 << 6))) {	//bit 6 stretch
			if ((scaledMode->height == 1080) || (scaledMode->height == 720)) destWidth = destHeight * 16 / 9;
			if (scaledMode->height == 576) destWidth = destHeight * 3 * 15 * 16 / 576;
			if ((scaledMode->height == 480) && (mode->height != 480)) destWidth = destHeight * 4 / 3;
		}
	}
	
	if (mode->mFlag & (1 << 6)) {			//bit 6 stretch
		float wRatio = (float) destWidth / (float) mode->width;
		float hRatio = (float) destHeight / (float) mode->height;
		
		float ratio = (wRatio > hRatio)?hRatio:wRatio;
		
		scaledPara->width = (UInt16) ((float) mode->width * ratio);
		scaledPara->height = (UInt16) ((float) mode->height * ratio);
		
		if (mode->mFlag & 0x20A0) {	//bit 13, 5, 7
			scaledPara->width = scaledMode->width;
			scaledPara->height = scaledMode->height;
			if (wRatio > 1.0) scaledPara->width = (UInt16) ((float) scaledMode->width / wRatio);
			if (wRatio > 1.0) scaledPara->height = (UInt16) ((float) scaledMode->height / wRatio);
		}
	}
	
	if (mode->mFlag & 0x20A0) {		//bit 13, 5, 7
		UInt32 unknown1 = scaledPara->width * 28;	//var_70
		if (unknown1 & (1 << 31)) unknown1 += 7;
		UInt32 unknown2 = unknown1 / 8 + 3;			//var_74
		if (unknown1 & (1 << 31)) unknown2 += 0xE0000000;
		if (unknown2 & (1 << 31)) unknown2 += 3;
		destWidth = unknown2 / 4;
		unknown1 = scaledPara->height * 28;
		if (unknown1 & (1 << 31)) unknown1 += 7;
		unknown2 = unknown1 / 8 + 3;
		if (unknown1 & (1 << 31)) unknown2 += 0xE0000000;
		if (unknown2 & (1 << 31)) unknown2 += 3;
		destHeight = unknown2 / 4;
		scaledPara->width = destWidth;
		scaledPara->height = destHeight;
	}
	scaledPara->width = (scaledPara->width + 1) & ~1;
	scaledPara->height = (scaledPara->height + 1) & ~1;
}

bool PixelFormat20bpp(DriverGlobal *aDriverRecPtr, UInt8 connection) {
#ifdef	ATY_Wormy
	if (connection & (1 << 1)) return true;	//TV
#endif
	return false;
}

UInt32 GetDisplayLBSize(DriverGlobal *aDriverRecPtr, UInt8 dispNum, UInt8 connection) {
	UInt32 size = 2880;		//2880 pixels for 32bpp
	if (aDriverRecPtr->displayLineBuffer & (1 << 2)) size = aDriverRecPtr->displayLineBuffer & 0x7FF0 >> 4 << 3;
	else if (aDriverRecPtr->displayLineBuffer < 1) size /= 2;
	else if (aDriverRecPtr->displayLineBuffer == 1) size = size * 3 / 4;
	else if (aDriverRecPtr->displayLineBuffer == 3) size /= 4;
	else if (aDriverRecPtr->displayLineBuffer == 4) size = 0;
	
	if (dispNum == 1) size = 2880 - size;
	if (!PixelFormat20bpp(aDriverRecPtr, connection)) return size;
	return size * 3 / 2;
}

bool GetScalerParameters(DriverGlobal *aDriverRecPtr, UInt32* sysClock, ScaledParameters* scaledPara, UInt16 clock, UInt32 width1,
						 UInt32 height1, UInt32 width2, UInt32 height2, UInt16 modeFlag, UInt8 connection) {
	UInt32 minRatioW = 2;
	UInt32 maxRatioW = 10;
	UInt32 minRatioH = 2;
	UInt32 maxRatioH = 6;
	
	if (modeFlag & (1 << 3)) height2 /= 2;		//interlaced
	UInt32 LBSize = GetDisplayLBSize(aDriverRecPtr, aDriverRecPtr->dispNum, connection);
	if (LBSize == 0) return false;
	UInt32 vClock = CalculateVClk(aDriverRecPtr, clock, aDriverRecPtr->dispNum, connection);
	if (vClock == 0) return false;
	UInt32 sClock = aDriverRecPtr->aShare->sysClock;
	if ((aDriverRecPtr->unknown8 & (1 << 1)) && aDriverRecPtr->unknown9) sClock -= (aDriverRecPtr->unknown9 * sClock / 10000 + 1);
	
	if ((float) height1 / (float) height2 > 2.0) minRatioH = 3;
	if ((float) height1 / (float) height2 > 3.0) minRatioH = 4;
	if ((float) width1 / (float) height1 > 2.0) minRatioW = 4;
	
	if ((height1 / height2 > 3) || (width1 / height1 > 3)) return false;
	if ((LBSize / (minRatioH + 1)) < width1) return false;
	maxRatioH = LBSize / width1 - 1;
	if (maxRatioH > 6) maxRatioH = 6;
	if ((vClock * minRatioW / 2) > sClock) return false;
	if ((vClock * width1 * height1 / width2 / height2) > sClock) return false;	
	if ((vClock * width1 * height2 / height1) > sClock) return false;
	do { maxRatioW -= 2; } while ((vClock * maxRatioW / 2) > sClock);
	while ((vClock * width1 * maxRatioH / height1) > sClock) maxRatioH--;
	if (maxRatioW < minRatioW) return false;
	if (maxRatioH < minRatioH) return false;
	
	if (scaledPara == NULL) return true;
	scaledPara->width = (UInt8) minRatioH;
	scaledPara->height = (UInt8) minRatioW;
	if (aDriverRecPtr->currentDispPara.unknown7) {
		scaledPara->width = ((maxRatioH - minRatioH) * aDriverRecPtr->currentDispPara.unknown8 * 2 / 4 + 1) / 2 + minRatioH;
		scaledPara->width = ((maxRatioW - minRatioW) * aDriverRecPtr->currentDispPara.unknown8 * 2 / 4 + 1) / 2 + minRatioW;
	}
	if (modeFlag & (1 << 3)) scaledPara->width = aDriverRecPtr->currentDispPara.unknown5 + minRatioH;
	if (scaledPara->width < minRatioH) scaledPara->width = minRatioH;
	if (scaledPara->height < minRatioW) scaledPara->height = minRatioW;
	if (scaledPara->width > maxRatioH) scaledPara->width = maxRatioH;
	if (scaledPara->height > maxRatioW) scaledPara->height = maxRatioW;
	
	if (sysClock == NULL) return true;
	sClock = scaledPara->height * vClock / 2;
	UInt32 newClock = vClock * width1 * height1 / width2 / height2;
	sClock = (newClock < sClock)?sClock:newClock;
	newClock = vClock * width1 * scaledPara->width / width2;
	sClock = (newClock < sClock)?sClock:newClock;
	if ((aDriverRecPtr->unknown8 & (1 << 1)) && aDriverRecPtr->unknown9) sClock = aDriverRecPtr->unknown9 * sClock / 10000 + sClock + 1;
	*sysClock = sClock;
	
	return true;
}

bool HW_CanUseHWScaler(DriverGlobal *aDriverRecPtr, UInt8 dispNum, UInt32 bitsPerPixel, UInt16 clock,
					   UInt32 modeW, UInt32 modeH, UInt32 scaledW, UInt32 scaledH, UInt16 modeFlag, UInt32 connection) {
	if (!(aDriverRecPtr->driverFlags & (1 << 24))) return false;		// not HWscalable
	if (!GetScalerParameters(aDriverRecPtr, 0, 0, clock, modeW, modeH, scaledW, scaledH, modeFlag, connection)) return false;
	return true;
}

bool HW_SWScalerAvailable(DriverGlobal *aDriverRecPtr, UInt8 dispNum, UInt8 bitsPerPixel) {
	if (!(aDriverRecPtr->driverFlags & (1 << 19))) return false;
	if (bitsPerPixel < 16) return false;
	return true;
}

UInt32 GetPitch(DriverGlobal *aDriverRecPtr, UInt32 width, UInt8 bytesPerPixel) {
	if (bytesPerPixel == 0) return 1;
	return (width * bytesPerPixel + 0xFF) & ~0xFF;
}

UInt32 GetDisplayVRAMSize(DriverGlobal *aDriverRecPtr, UInt32 width, UInt32 height, UInt8 bytesPerPixel) {
	if (!bytesPerPixel || !width || !height) return 0;
	UInt32 pitch = GetPitch(aDriverRecPtr, width, bytesPerPixel);
	return ((((height + 3) & ~3) * pitch + 0x7FF) & ~0x7FF);
}

UInt32 GetScreenOffset(DriverGlobal *aDriverRecPtr, UInt32 rowBytes, UInt32 height, UInt8 bytesPerPixel) {
	UInt32 offset = 0x20000;
	UInt32 offsetMask = bytesPerPixel << 11 - 1;
	return ((offset + offsetMask) & ~offsetMask); //still 0x20000 for 1,2,4,8 bytesPerPixel
}

bool HW_CanUseSWScaler(DriverGlobal *aDriverRecPtr, UInt8 dispNum, UInt8 bitsPerPixel, UInt32 modeW,
					   UInt32 modeH, UInt32 scaledW, UInt32 scaledH) {
	if (!HW_SWScalerAvailable(aDriverRecPtr, dispNum, bitsPerPixel)) return false;
	if ((modeW > 4096) || (scaledW > 4096)) return false;
	if ((modeH > 4096) || (scaledH > 4096)) return false;
	UInt32 size1 = GetDisplayVRAMSize(aDriverRecPtr, modeW, modeH, bitsPerPixel / 8)
					+ GetScreenOffset(aDriverRecPtr, bitsPerPixel / 8 * modeW, modeH, bitsPerPixel / 8);
	UInt32 size2 = GetDisplayVRAMSize(aDriverRecPtr, scaledW, scaledH, bitsPerPixel / 8)
					+ GetScreenOffset(aDriverRecPtr, bitsPerPixel / 8 * scaledW, scaledH, bitsPerPixel / 8);
	if ((size1 + size2) > aDriverRecPtr->totalVram) return false;
	return true;
}

OSStatus GetHWTimingRange(DriverGlobal *aDriverRecPtr, VDDisplayTimingRangeRec* timingRange, UInt8 connection, bool scaleable) {
	const VDDisplayTimingRangeRec theDisplayTimingRanges = {
		0xF0, 0x0, 0, 0,
		0, 0, 1, 0x0,
		7080000, 400000000,
		0, 0x0, 0, 4,
		10, 500, 10000, 500000,
		8192, 8192, 0, 0,
		2, 1, 1, 1,
		2, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 0,
		2, 4096, 1, 8192,
		1, 8191, 1, 4096,
		2, 4096, 1, 8192,
		1, 8192, 1, 4096,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};
	
	if (timingRange == NULL) return statusErr;
	memcpy(timingRange, &theDisplayTimingRanges, sizeof(VDDisplayTimingRangeRec));
	UInt64 clock = aDriverRecPtr->aShare->sysClock;
	if (!scaleable) clock = clock - clock * 1000 / 10000 - 1;
	else if ((aDriverRecPtr->unknown8 & (1 << 1)) && aDriverRecPtr->unknown9) clock = clock - clock * aDriverRecPtr->unknown9 / 10000 - 1;
#ifdef ATY_Wormy
	if (connection & (1 << 6)) clock = clock - clock * 75 / 10000 - 1;	//LVDS
#endif
	clock = (clock > 55000)?55000:clock;
	timingRange->csMinPixelClock = 708;
	timingRange->csMaxPixelClock = (clock > 110000)?110000:clock;
#ifdef ATY_Wormy
	if (connection & (1 << 2))		//DFP1
#endif
	{
		if (timingRange->csMinPixelClock < 708) timingRange->csMinPixelClock = 708;
		if (!aDriverRecPtr->unknown53 && (timingRange->csMaxPixelClock > 33000)) timingRange->csMaxPixelClock = 33000;
		else if (timingRange->csMaxPixelClock > 16500) timingRange->csMaxPixelClock = 16500;
		timingRange->csReserved2 = timingRange->csMinPixelClock;
		timingRange->csReserved3 = (timingRange->csMaxPixelClock > 16500)?16500:timingRange->csMaxPixelClock;
		timingRange->csReserved1 = 1;
		if (!aDriverRecPtr->unknown53) {
			timingRange->csReserved4 = timingRange->csReserved2;
			timingRange->csReserved5 = timingRange->csReserved3;
			timingRange->csReserved1 = 2;
		}
	}
#ifdef ATY_Caretta
	if (connection & (1 << 3))		//DFP2
#endif
#ifdef ATY_Wormy
	if (connection & (1 << 6))		//LVDS
#endif
	{
		if (timingRange->csMinPixelClock < 708) timingRange->csMinPixelClock = 708;
		if (!aDriverRecPtr->unknown20 && (timingRange->csMaxPixelClock > 33000)) timingRange->csMaxPixelClock = 33000;
		else if (timingRange->csMaxPixelClock > 16500) timingRange->csMaxPixelClock = 16500;
		timingRange->csReserved2 = timingRange->csMinPixelClock;
		timingRange->csReserved3 = (timingRange->csMaxPixelClock > 16500)?16500:timingRange->csMaxPixelClock;
		timingRange->csReserved1 = 1;
		if (!aDriverRecPtr->unknown20) {
			timingRange->csReserved4 = timingRange->csReserved2;
			timingRange->csReserved5 = timingRange->csReserved3;
			timingRange->csReserved1 = 2;
		}
	}
#ifdef ATY_Caretta
	if (connection & (1 << 4))				//CRT1
#endif
#ifdef ATY_Wormy
	if (connection & ((1 << 1) + (1 << 5)))	//CRT2 or TV
#endif
	{
		if (timingRange->csMinPixelClock < 708) timingRange->csMinPixelClock = 708;
		if (timingRange->csMaxPixelClock > 40000) timingRange->csMaxPixelClock = 40000;
	}
	if (timingRange->csMaxPixelClock > clock) timingRange->csMaxPixelClock = clock;
	if (timingRange->csReserved3 > clock) timingRange->csReserved3 = clock;
	if (timingRange->csReserved5 > clock) timingRange->csReserved5 = clock;
	timingRange->csMinPixelClock *= 10000;
	timingRange->csMaxPixelClock *= 10000;
	timingRange->csReserved2 *= 10000;
	timingRange->csReserved3 *= 10000;
	timingRange->csReserved4 *= 10000;
	timingRange->csReserved5 *= 10000;
	timingRange->csRangeBlockCount = 1;
	return noErr;
}

UInt8 HALNeedSWScaler(DriverGlobal *aDriverRecPtr, CrtcValues *mode, UInt32 bitsPerPixel) {
	UInt8 ret = 0;
	CrtcValues scaledMode;
	ScaledParameters scaledPara;
	
	if ((mode == NULL) || (bitsPerPixel == 0)) return 0;
	UInt8 connection = HALGetTimingFlagsRec(aDriverRecPtr, mode, aDriverRecPtr->activeFlags);
	if (!(connection & ~1)) return 0;	//no display connected
	if (!HALFillNativeCRTCParams(aDriverRecPtr, mode, &scaledMode)) return 0;
	if ((mode != NULL) && (mode->mFlag & 0xF00) && (aDriverRecPtr->driverFlags & (1 << 26))) ret |= 1;
	CalculateScaledParams(aDriverRecPtr, mode, &scaledMode, &scaledPara);
	if ((scaledPara.width < mode->width) && (scaledPara.height < mode->height)
		&& !HW_CanUseHWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, bitsPerPixel, scaledMode.clock,
							  mode->width, mode->height, scaledPara.width, scaledPara.height, scaledMode.mFlag, connection))
		ret |= 1;
	if (!ret && (scaledPara.width != mode->width || scaledPara.height != mode->height)
		&& !HW_CanUseHWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, bitsPerPixel, scaledMode.clock,
							  mode->width, mode->height, scaledPara.width, scaledPara.height, scaledMode.mFlag, connection)
		&& HW_CanUseSWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, bitsPerPixel, mode->width, mode->height, scaledPara.width, scaledPara.height))
		return 3;
	if ((bitsPerPixel >= 16) && ret
		&& HW_CanUseSWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, bitsPerPixel, mode->width, mode->height, scaledPara.width, scaledPara.height))
		ret |= 2;
	return ret;
}

UInt32 HALMaxPixelSize(DriverGlobal *aDriverRecPtr, CrtcValues *mode) {
	CrtcValues scaledMode;
	ScaledParameters scaledPara;
	UInt32 memSize;
	
	if ((mode->timingMode == timingApple_0x0_0hz_Offline) || (mode->clock == 0) || (mode->width == 0) || (mode->height == 0)) return 32;
	if (!HALFillNativeCRTCParams(aDriverRecPtr, mode, &scaledMode)) return 0;
	CalculateScaledParams(aDriverRecPtr, mode, &scaledMode, &scaledPara);
	UInt8 i;
	for (i = 8;i != 0;i /= 2) {
		memSize = GetDisplayVRAMSize(aDriverRecPtr, mode->width, mode->height, i);
		memSize += GetScreenOffset(aDriverRecPtr, mode->width * i, mode->height, i); //+128kb
		if (aDriverRecPtr->totalVram >= memSize) {
			if (HALNeedSWScaler(aDriverRecPtr, mode, 8 * i) != 3) break;
			memSize += GetDisplayVRAMSize(aDriverRecPtr, scaledPara.width, scaledPara.height, i);
			if (aDriverRecPtr->totalVram >= memSize) break;
		}
	}
	return (8 * i);
}

bool HALValidCRTCParameters(DriverGlobal *aDriverRecPtr, CrtcValues *mode, UInt8 connection) {
	ScaledParameters scaledPara;//var_2C
	scaledPara.unknown1 = 40000;
	scaledPara.unknown2 = 708;
	bool isRotate = false;		//var_1B
	bool HWscaleable = false;	//var_1A
	UInt8 theConnect;			//var_1C
	CrtcValues scaledMode;		//var_48
	
	if (mode == NULL) return false;
	if (mode->timingMode == timingApple_0x0_0hz_Offline) return true;
	if (connection == 1) {
		if (mode->timingMode == timingApple_0x0_0hz_Offline) return true;
		else return false;
	}
	theConnect = HALGetTimingFlagsRec(aDriverRecPtr, mode, connection);
	if (theConnect == 0) return false;
	if (!HALFillNativeCRTCParams(aDriverRecPtr, mode, &scaledMode)) return false;
	CalculateScaledParams(aDriverRecPtr, mode, &scaledMode, &scaledPara);
	if (mode->mFlag & 0xF00) isRotate = true;
	if (GetDisplayLBSize(aDriverRecPtr, aDriverRecPtr->dispNum, theConnect) < (2 * mode->width)) return false;
	if ((mode->width != scaledPara.width) || (mode->height != scaledPara.height) || (scaledMode.mFlag & (1 << 3)) || isRotate) {		//bit3:interlaced
		bool scaleable = false;
		if (!scaleable && HW_CanUseHWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, 32, scaledMode.clock,
											mode->width, mode->height, scaledPara.width, scaledPara.height,
											scaledMode.mFlag, theConnect)) {
			scaleable = true;
			HWscaleable = true;
			if (isRotate && !HW_CanUseSWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, 32, mode->width, mode->height, mode->width, mode->height))
				scaleable = false;
		}
		if (!scaleable && HW_CanUseSWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, 32, mode->width, mode->height, scaledPara.width, scaledPara.height)) {
			if (!(scaledMode.mFlag & (1 << 3))) scaleable = true;		//not interlaced
			else if (HW_CanUseHWScaler(aDriverRecPtr, aDriverRecPtr->dispNum, 32, scaledMode.clock,
									   scaledPara.width, scaledPara.height, scaledPara.width, scaledPara.height,
									   scaledMode.mFlag, theConnect))  {
				scaleable = true;
				HWscaleable = true;
			}
		}
		if (!scaleable) return false;
	}
	UInt32 ratio;
	if (mode->width > scaledPara.width || mode->height > scaledPara.height) {
#ifdef ATY_Caretta
		if (!(mode->mFlag & 108))		//not interlaced and not rotate
#endif
#ifdef ATY_Wormy
		if (!(mode->mFlag & 0x2128))	//not interlaced and not rotate unclear about the meaning of bits 5, 13
#endif
		{	ratio = scaledPara.width * 100 / mode->width;
			if (ratio < 50) {
				ratio = scaledPara.height * 100 / mode->height;
				if (ratio < 50) return false;
			}
		}
	}
	VDDisplayTimingRangeRec timingRange;	//var_138
	if (GetHWTimingRange(aDriverRecPtr, &timingRange, theConnect, HWscaleable) != noErr) return false;
	if ((mode->mFlag & (1 << 12)) && (timingRange.csMinHorizontalActiveClocks <= 1)) return false;
	if (((mode->dHSyncS * 420) > (timingRange.csMaxPixelClock >> 32)) || ((mode->dHSyncS * 2100) > (timingRange.csMaxPixelClock & 0xFFFFFFFF))) return false;
	if (mode->width > timingRange.csMaxHorizontalActiveClocks) return false;
	if (mode->width % timingRange.csCharSizeHorizontalActive != 0) return false;
	if (mode->height > timingRange.csMaxVerticalActiveClocks) return false;
	if (mode->height % timingRange.csCharSizeVerticalActive != 0) return false;
	if (mode->HTotal > timingRange.csMaxHorizontalTotal) return false;
	if (mode->VTotal > timingRange.csMaxVerticalTotal) return false;
	if (mode->dHSyncE > timingRange.csMaxHorizontalPulseWidthClocks) return false;
	if (mode->dVSyncE > timingRange.csMaxVerticalPulseWidthClocks) return false;
	if (((mode->dHSyncS * 420) < (timingRange.csMinPixelClock >> 32)) || ((mode->dHSyncS * 2100) < (timingRange.csMinPixelClock & 0xFFFFFFFF))) return false;
	if (mode->width < timingRange.csMinHorizontalActiveClocks) return false;
	if (mode->height < timingRange.csMinVerticalActiveClocks) return false;
	if (mode->HTotal == 0) return false;
	if (mode->VTotal == 0) return false;
	if (mode->dHSyncE < timingRange.csMinHorizontalPulseWidthClocks) return false;
	if (mode->dVSyncE < timingRange.csMinVerticalPulseWidthClocks) return false;
	if (mode->dHSyncS < timingRange.csMinHorizontalSyncOffsetClocks) return false;
	if (mode->dVSyncS < timingRange.csMinVerticalSyncOffsetClocks) return false;
	
	UInt32 maxPixelSize = HALMaxPixelSize(aDriverRecPtr, mode);
	if (maxPixelSize  < 8) return false;
	return true;
}

UInt32 GetCRTCHFreq(DriverGlobal *aDriverRecPtr, CrtcValues *mode) {
	if ((mode == NULL) || (mode->clock == 0) || (mode->HTotal == 0)) return 0;
	return (mode->clock * 10000 / mode->HTotal + 1);
}

void HALSetMonitorLimits(DriverGlobal *aDriverRecPtr, monitorLimit* monitor, CrtcValues *mode) {
	if (monitor->clock == 0) { //not initialized yet
		monitor->clock = mode->clock;
		monitor->maxHFreq = GetCRTCHFreq(aDriverRecPtr, mode);
		monitor->minHFreq = GetCRTCHFreq(aDriverRecPtr, mode);
		monitor->maxRefRate = HALGetRefreshRate(aDriverRecPtr, mode);
		monitor->minRefRate = monitor->maxRefRate;
		monitor->maxWidth = mode->width;
		monitor->maxHeight = mode->height;
	} else {
		if (mode->clock > monitor->clock) monitor->clock = mode->clock;
		if (GetCRTCHFreq(aDriverRecPtr, mode) > monitor->maxHFreq) monitor->maxHFreq = GetCRTCHFreq(aDriverRecPtr, mode);
		if (GetCRTCHFreq(aDriverRecPtr, mode) < monitor->minHFreq) monitor->minHFreq = GetCRTCHFreq(aDriverRecPtr, mode);
		if (HALGetRefreshRate(aDriverRecPtr, mode) > monitor->maxRefRate) monitor->maxRefRate = HALGetRefreshRate(aDriverRecPtr, mode);
		if (HALGetRefreshRate(aDriverRecPtr, mode) < monitor->minRefRate) monitor->minRefRate = HALGetRefreshRate(aDriverRecPtr, mode);
		if (mode->width > monitor->maxWidth) monitor->maxWidth = mode->width;
		if (mode->height > monitor->maxHeight) monitor->maxHeight = mode->height;
	}
}

UInt32 HALFindNextAvailModeID(DriverGlobal *aDriverRecPtr, UInt8 connection) {
	UInt32 offset;
#ifdef ATY_Caretta
	if (connection == (1 << 3)) offset = 0x5C3E;
	else if (connection == (1 << 4)) offset = 0x43E;
#endif
#ifdef ATY_Wormy
	if ((connection == (1 << 2)) || (connection == (1 << 6))) offset = 0x5C3E;
	else if (connection == (1 << 5)) offset = 0x43E;
#endif
	else if (connection == 1) offset = 0x3000;
	else return 0;
	return (aDriverRecPtr->modesIndex + offset);
}

ModeFlag* HALGetMonitorMode(DriverGlobal *aDriverRecPtr, UInt32 modeID) {
	UInt8 i;
	for (i = 0;i < aDriverRecPtr->modesNum;i++)
		if (aDriverRecPtr->modeFlags[i].modeID == modeID) return &aDriverRecPtr->modeFlags[i];
	return NULL;
}

CrtcValues * HALAddMonitorMode(DriverGlobal *aDriverRecPtr, UInt32 modeID, CrtcValues *mode, UInt32 mFlag, UInt8 connection) {
	CrtcValues *newMode = NULL;
#ifdef ATY_Caretta
	if ((aDriverRecPtr->connection != connection) && (connection & (1 << 3)))
#endif
#ifdef ATY_Wormy
	if ((aDriverRecPtr->connection != connection) && (connection & ((1 << 2) + (1 << 6))))
#endif
		mFlag |= (1 << 10);
	ModeFlag *modeFlag = HALGetMonitorMode(aDriverRecPtr, modeID);
	if (aDriverRecPtr->modesNum >= aDriverRecPtr->maxModesNum) return NULL;
	if (modeFlag != NULL) return NULL;
	aDriverRecPtr->modeFlags[aDriverRecPtr->modesNum].modeID = modeID;
	aDriverRecPtr->modeFlags[aDriverRecPtr->modesNum].mFlag = mFlag;
	if (mode != NULL) memcpy(&aDriverRecPtr->modes[aDriverRecPtr->modesNum], mode, sizeof(CrtcValues));
	aDriverRecPtr->modes[aDriverRecPtr->modesNum].modeID = modeID;
	if (HALValidCRTCParameters(aDriverRecPtr, &aDriverRecPtr->modes[aDriverRecPtr->modesNum], connection)) {
		newMode = &aDriverRecPtr->modes[aDriverRecPtr->modesNum];
		aDriverRecPtr->modesNum++;
	}
	return newMode;
}

UInt32 ChoosePreferredPanelModeID(DriverGlobal *aDriverRecPtr, UInt32 modeID1, UInt32 modeID2) {
	CrtcValues *mode1 = HALFindCrtcValues(aDriverRecPtr, modeID1);
	CrtcValues *mode2 = HALFindCrtcValues(aDriverRecPtr, modeID2);
	if ((mode1 == NULL) && (mode2 == NULL)) return 0;
	if (mode1 == NULL) return modeID2;
	if (mode2 == NULL) return modeID1;
	if (mode1->mFlag & (1 << 3)) return modeID2;
	if (mode2->mFlag & (1 << 3)) return modeID1;
	if (mode1->width > mode2->width) return modeID1;
	if (mode2->width > mode1->width) return modeID2;
	if (mode1->height > mode2->height) return modeID1;
	if (mode2->height > mode1->height) return modeID2;
	if (mode1->clock > mode2->clock) return modeID1;
	else return modeID2;
}

bool HALAddEDIDMode(DriverGlobal *aDriverRecPtr, UInt32 modeID, CrtcValues *mode, UInt32* newModeID, UInt8 connection, bool isDigital, UInt32* aFlag) {
	CrtcValues newMode;
	memcpy(&newMode, mode, sizeof(CrtcValues));
	UInt32 mID = modeID;
	if (modeID == 0) mID = HALFindNextAvailModeID(aDriverRecPtr, connection);
	newMode.modeID = mID;
#ifdef ATY_Caretta
	if ((connection & (1 << 3)) && (newMode.clock > 16500)) newMode.mFlag |= (1 << 4);
#endif
#ifdef ATY_Wormy
	if ((connection & (1 << 2)) && (modeV2.clock > 16500)) newMode.mFlag |= (1 << 4);
	if ((connection & (1 << 6)) && (modeV2.clock > 16500)) newMode.mFlag |= (1 << 4);
#endif
	if (aDriverRecPtr->connection != connection) *aFlag |= 0x3;
	if (newMode.mFlag & (1 << 3)) *aFlag |= (1 << 6);		//bit3 interlaced
	mode = HALAddMonitorMode(aDriverRecPtr, mID, &newMode, *aFlag, connection);
	if (mode == NULL) return false;
	if (isDigital) mode->timingMode = timingApple_FixedRateLCD;
	if (newModeID == NULL) return true;
	if (*newModeID == 0) *newModeID = mID;
	else if (isDigital) *newModeID = ChoosePreferredPanelModeID(aDriverRecPtr, *newModeID, mID);
	return true;
}

#ifdef ATY_Caretta
const UInt32 PredefinedTimingTableLen = 49;
#endif
#ifdef ATY_Wormy
const UInt32 PredefinedTimingTableLen = 66;
#endif
CrtcValues* HALMatchPredefinedDDCMode(DriverGlobal *aDriverRecPtr, UInt32 width, UInt32 height, UInt32 refRate) {
	UInt8 i;
	for (i = 0;i < PredefinedTimingTableLen;i++) {
		if (PredefinedTimingTable[i].modeID > 0x7FE) continue;
		if (PredefinedTimingTable[i].width != width) continue;
		if (PredefinedTimingTable[i].height != height) continue;
		if (refRate != HALGetRefreshRate(aDriverRecPtr, (CrtcValues *)&PredefinedTimingTable[i])) continue;
		return (CrtcValues *)&PredefinedTimingTable[i];
	}
	return NULL;
}

bool HALGetMonitorModeFlags(DriverGlobal *aDriverRecPtr, UInt32 modeID, UInt32* mFlag) {
	ModeFlag* modeFlag = HALGetMonitorMode(aDriverRecPtr, modeID);
	if (modeFlag == NULL) return false;
	*mFlag = modeFlag->mFlag;
	return true;
}

void HALSetMonitorModeFlags(DriverGlobal *aDriverRecPtr, UInt32 modeID, UInt32 mFlag) {
	ModeFlag* modeFlag = HALGetMonitorMode(aDriverRecPtr, modeID);
	if (modeFlag != NULL) modeFlag->mFlag = mFlag;
}

void HALInitEDIDModes(DriverGlobal *aDriverRecPtr, UInt8 connection) {
	typedef struct _ET_ID {
		UInt8		offset;
		UInt8		Bit;
		UInt16		ModeID;
	} ET_ID;
	
	const ET_ID ET_IDs[14] = {		//0x23,0x24,0x25 EDID established timing offset
		{0x23, 5, 0x407},			//only listed modes are supported
		{0x23, 4, 0x408},
		{0x23, 3, 0x409},
		{0x23, 2, 0x40A},
		{0x23, 1, 0x410},
		{0x23, 0, 0x411},
		{0x24, 7, 0x412},
		{0x24, 6, 0x413},
		{0x24, 5, 0x418},
		{0x24, 3, 0x41A},
		{0x24, 2, 0x41B},
		{0x24, 1, 0x41C},
		{0x24, 0, 0x425},
		{0x25, 7, 0x422}
	};
	
	UInt8* edid = aDriverRecPtr->edid;
	CrtcValues *mode;
	CrtcValues modeDT;
	UInt32 aFlag = 0;
	UInt32 newModeID = 0;
	UInt32 tempModeID = 0;
	UInt32 offsetID = 0;
	bool isDigital = false;	//var_9
	monitorLimit mLimit;	//var_B4
	bzero(&mLimit, sizeof(monitorLimit));
	UInt32 temp;
	
	if (CheckLoadEDIDBlock(aDriverRecPtr, connection, 1, edid) != noErr) return;
	
	isDigital = EdidDigital(aDriverRecPtr, edid);
	bool isPreferred = (edid[0x18] & (1 << 1)); //Preferred Timing Mode
	//bool GTFSupport = (edid[0x18] & 1);			//Default GTF support
	UInt32 edidRev = edid[0x13];				//EDID Revision number
	mLimit.clock = 0;
	
	if (isDigital) offsetID = 0x5800;
	UInt8 i;
	for (i = 0;i < 14;i++) {					//get established timing
		if (!(edid[ET_IDs[i].offset]) & (1 << ET_IDs[i].Bit)) continue;
		mode = HALFindPredefinedCrtcValues(aDriverRecPtr, ET_IDs[i].ModeID);
		if (mode == NULL) continue;
		temp = offsetID + ET_IDs[i].ModeID;
		HALSetMonitorLimits(aDriverRecPtr, &mLimit, mode);
		HALAddEDIDMode(aDriverRecPtr, temp, mode, &newModeID, connection, isDigital, &aFlag);
	}
	for (i = 0;i < 8;i++) {					//get standard timing, each 2 bytes
		UInt16 stData = (edid[0x26 + 2 * i + 1] << 8) + edid[0x26 + 2 * i]; //0x26 EDID standard timing section offset
		if ((stData == 0) || (stData == 0x101)) continue;					//exclude fill values
		temp = (edid[0x26 + 2 * i] << 3) + 0xF8;								//get Horizontal Active
		UInt8 ratio = (edid[0x26 + 2 * i + 1] >> 6) & 0x3;						//get the width:height ratio bits 6,7
		if (ratio == 0) {
			if (edidRev > 2)							//prior to EDID1.3, 0 means 1:1 ratio
				temp = temp * 10 / 16;					//0 means 16:10
		}
		else if (ratio == 1)
			temp = temp * 3 / 4;						//1 means 4:3
		else if (ratio == 2)
			temp = temp * 4 / 5;						//2 means 5:4
		else if (ratio == 3)
			temp = temp * 9 / 16;						//3 means 16:9
		UInt32 width = (edid[0x26 + 2 * i] << 3) + 0xF8;		//get Horizontal Active again
		UInt32 height = temp;									//get Vertical Active
		UInt32 refRate = edid[0x26 + 2 * i + 1] & 0x3F + 60;	//get refreshRate
		mode = HALMatchPredefinedDDCMode(aDriverRecPtr, width, height, refRate);
		if (mode == NULL) continue;
		temp = mode->modeID + offsetID;
		HALSetMonitorLimits(aDriverRecPtr, &mLimit, mode);
		HALAddEDIDMode(aDriverRecPtr, temp, mode, &newModeID, connection, isDigital, &aFlag);
	}
	UInt32 j;
	for (j = 0;j < 255; j++) { //a limit of 255 edid extensions
		if ((j != 0) && (CheckLoadEDIDBlock(aDriverRecPtr, connection, j + 1, edid) != noErr)) break;
		UInt32 numDT = 0;
		UInt32 offsetDT = 0;
		if ((edidRev <= 2) && (j != 0)) break; //prior EDIDv1.3, extensions are not supported
		if (j == 0) {
			offsetDT = 0x36;
			numDT = 4;
		} else if (edid[0] == 2) {
			offsetDT = edid[2];
			if (offsetDT > 0x80) offsetDT = 0;
			if (offsetDT != 0) numDT = (0x80 - offsetDT) / 18 - ((0x80 - offsetDT) >> 0x1F);
		}
		for (i = 0;i < numDT;i++, offsetDT += 0x12) {		//get detailed timing, each size 0x12
			if (!EDID_StoreDetailTiming(aDriverRecPtr, &modeDT, &edid[offsetDT], isDigital, connection)) continue;
			modeDT.modeID = HALFindNextAvailModeID(aDriverRecPtr, connection);
			mode = HALMatchPredefinedDDCMode(aDriverRecPtr, modeDT.width, modeDT.height, HALGetRefreshRate(aDriverRecPtr, &modeDT));
			if (mode != NULL) modeDT.modeID = mode->modeID + offsetID;
			if (!HALValidCRTCParameters(aDriverRecPtr, &modeDT, connection)) continue;
			mode = HALFindCrtcValues(aDriverRecPtr, modeDT.modeID);
			if (mode != NULL) EDID_StoreDetailTiming(aDriverRecPtr, mode, &edid[offsetDT], isDigital, connection);
			else {
				if (!HALAddEDIDMode(aDriverRecPtr, modeDT.modeID, &modeDT, &newModeID, connection, isDigital, &aFlag)) continue;
				aDriverRecPtr->modesIndex++;
			}
			HALSetMonitorLimits(aDriverRecPtr, &mLimit, &modeDT);
			if (j || i || !isPreferred || tempModeID || (modeDT.mFlag & (1 << 3))) continue;
			tempModeID = modeDT.modeID;			
		}
	}
	if (CheckLoadEDIDBlock(aDriverRecPtr, connection, 1, edid) != noErr) return;
	if (isDigital && tempModeID) newModeID = tempModeID;
	if (!newModeID || (aDriverRecPtr->connection != connection)) return;
	HALGetMonitorModeFlags(aDriverRecPtr, newModeID, &temp);
	HALSetMonitorModeFlags(aDriverRecPtr, newModeID, temp | 7);
}

void HALSetupMonitorTable(DriverGlobal *aDriverRecPtr, UInt32 connection) {
	OSStatus ret = noErr;		//var_21
	ModeFlag* modeFlag = NULL;	//var_20
	MonitorParameter* mPara = NULL;	//var_1C
	SInt32 length = 0;			//var_18
	CrtcValues *mode;			//var_14
	UInt32 mFlag;				//var_10
	
	if (!(aDriverRecPtr->activeFlags & connection)) return;
	ret = CheckLoadEDIDBlock(aDriverRecPtr, connection, 1, NULL);
	if (ret == noErr) HALInitEDIDModes(aDriverRecPtr, connection);
	
	mPara = GetMonitorInfo(aDriverRecPtr, connection);
	if (mPara == NULL) return;
	modeFlag = mPara->modeFlag;
	length = mPara->length;
	if (modeFlag == NULL) return;
	if (length == 0) return;
	while (--length >= 0) {
		mode = HALFindPredefinedCrtcValues(aDriverRecPtr, modeFlag->modeID);
#ifdef ATY_Caretta
		if (mode != NULL) {
			mFlag = modeFlag->mFlag;
			if (ret != noErr) mFlag &= ~((1 << 1) | (1 << 0));
			if ((aDriverRecPtr->connection != connection) || (ret != noErr)) mFlag &= ~(1 << 1);
			if ((aDriverRecPtr->driverFlags & (1 << 11)) && (aDriverRecPtr->activeFlags & (1 << 4))) {
				mFlag &= ~(1 << 2);
				if (modeFlag->modeID != 0x407) //0x407, first predefined
					mFlag |= 1;		//bit0 = 1
				else mFlag |= 7;	//bit0-2 = 1
			}
#endif
#ifdef ATY_Wormy
			if (modeFlag->modeID != 0x3000) mFlag &= ~7;
#endif
			HALAddMonitorMode(aDriverRecPtr, modeFlag->modeID, mode, mFlag, connection);
		}
		modeFlag = &modeFlag[1];	//goes to next
	}
}

void HALInitSimulscanModes(DriverGlobal *aDriverRecPtr) {
}

