/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MBC.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/28 22:13:43 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 09:19:59 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef MBC_CLASS_H
# define MBC_CLASS_H

/*
** Memory Bank Controller implementation
** This is the implementation of the 5 MBC present in different GB/GBC cartridge.
** It handle Bank switching for RAM and ROM.
*/

#include <ctime>

class MBC {
public:
	MBC(int mbcCode, unsigned char typeNb);
	virtual ~MBC(void){}
	virtual unsigned char writeInRom(unsigned short addr, unsigned char value) = 0;
	virtual unsigned short getRomBank(unsigned short addr) = 0;
	virtual unsigned char getRamBank() = 0;
	virtual unsigned short getRamUpperAddress() = 0;
	static MBC* createMBC(unsigned char mbcCode);

	void setTimer(bool val) {hasTimer = val;}

	bool hasTimer;
	const unsigned char type;
};

class RomOnly: public MBC {
public:
	RomOnly(int mbcCode)
	: MBC(mbcCode, 0) {}
	virtual unsigned char writeInRom(unsigned short addr, unsigned char value);
	virtual unsigned short getRomBank(unsigned short addr);
	virtual unsigned char getRamBank();
	virtual unsigned short getRamUpperAddress() {return 0;}
};

class MBC1: public MBC {
public:
	MBC1(int mbcCode)
	: MBC(mbcCode, 1)
	{bAdvancedBankingMode = false; bEnableRam = false;
		lowBitsRomBankNumber = 0; highBitsRomBankNumberOrRam = 0;}
	virtual unsigned char writeInRom(unsigned short addr, unsigned char value);
	virtual unsigned short getRomBank(unsigned short addr);
	virtual unsigned short getRamUpperAddress() {return 0xBFFF;}
	virtual unsigned char getRamBank();

	bool getAdvancedBankingMode() { return bAdvancedBankingMode; }
	bool getEnableRam() { return bEnableRam; }
	unsigned char getLowBitsRomBankNumber() {return lowBitsRomBankNumber; }
	unsigned char getHighLowBitsRomBankNumber() { return highBitsRomBankNumberOrRam; }

	void setAdvancedBankingMode(bool val) {bAdvancedBankingMode = val;}
	void setEnableRam(bool val) {bEnableRam = val;}
	void setLowBitsRomBankNumber(unsigned char val) {lowBitsRomBankNumber = val;}
	void setHighBitsRomBankNumberOrRam(unsigned char val) {highBitsRomBankNumberOrRam = val;}
private:
	// TODO LMA This cause warning for uninitialized value
	// with gcc -fanalizer
	bool bAdvancedBankingMode;
	bool bEnableRam;
	unsigned char lowBitsRomBankNumber;
	unsigned char highBitsRomBankNumberOrRam;
};

class MBC2: public MBC {
public:
	MBC2(int mbcCode)
	: MBC(mbcCode, 2)
	{romBankNb = 0; bEnableRam = false;}
	virtual unsigned char writeInRom(unsigned short addr, unsigned char value);
	virtual unsigned short getRomBank(unsigned short addr);
	virtual unsigned char getRamBank();
	virtual unsigned short getRamUpperAddress() {return 0xA1FF;}

	bool getEnableRam() {return bEnableRam;}
	unsigned char getRomBankNb() {return romBankNb;}
	
	void setRomBankNb(unsigned char val) {romBankNb = val;}
	void setEnableRam(bool val) {bEnableRam = val;}
private:
	unsigned char romBankNb;
	bool bEnableRam;
};

//  RTC S   Seconds   0-59 (0-3Bh)
//  RTC M   Minutes   0-59 (0-3Bh)
//  RTC H   Hours     0-23 (0-17h)
//  RTC DL  Lower 8 bits of Day Counter (0-FFh)
//  RTC DH  Upper 1 bit of Day Counter, Carry Bit, Halt Flag
//     	Bit 0  Most significant bit of Day Counter (Bit 8)
//      Bit 6  Halt (0=Active, 1=Stop Timer)
//      Bit 7  Day Counter Carry Bit (1=Counter Overflow)
typedef struct {
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hours;
	unsigned char DL;
	unsigned char DH;
} rtc;

class MBC3: public MBC {
public:
	MBC3(int mbcCode)
	: MBC(mbcCode, 3)
	{romBankNb = 0; bEnableRam = false;
	ramBankNb = 0; latched = false; lastVal = 0; time(&start);}
	virtual unsigned char writeInRom(unsigned short addr, unsigned char value);
	virtual unsigned short getRomBank(unsigned short addr);
	virtual unsigned char getRamBank();
	virtual unsigned short getRamUpperAddress() {return 0xBFFF;}
	rtc getCurrentTime();

	time_t getStart() {return start;}
	rtc getRTC() { return rtc_register; }
	unsigned char getRTCBind() {return rtcBindNb;}

	void setStart(time_t val) {start = val;}
	void setRTC(rtc val) {rtc_register = val;}
	void setRTCBind(unsigned char val) {rtcBindNb = val;}
  void addDay(void) {start -= (time_t)(3600 * 24);};
  void addHour(void) {start -= (time_t)(3600);};
	
	unsigned char getEnableRam() {return bEnableRam;}
	unsigned short getRomBankNb() {return romBankNb;}
	unsigned char getRamBankNb() {return ramBankNb;}
	unsigned char getLastVal() { return lastVal; }
	bool getLatch() {return latched;}

	void setRomBankNb(unsigned char val) {romBankNb = val;}
	void setRamBankNb(unsigned char val) {ramBankNb = val;}
	void setEnableRam(bool val) {bEnableRam = val;}
	void setLastVal(unsigned char val) {lastVal = val;}
	void setLatch(bool val) {latched = val;}


	time_t start;
	rtc rtc_register;
	unsigned char rtcBindNb;
private:
	unsigned char romBankNb;
	unsigned char ramBankNb;
	bool bEnableRam;
	unsigned char lastVal;
	bool latched;
};

class MBC5: public MBC {
public:
	MBC5(int mbcCode)
	: MBC(mbcCode, 5)
	{bEnableRam = 0; leastSignificantRomByte = 0;
	bit9 = 0; ramBankNb = 0;}
	virtual unsigned char writeInRom(unsigned short addr, unsigned char value);
	virtual unsigned short getRomBank(unsigned short addr);
	virtual unsigned char getRamBank();
	virtual unsigned short getRamUpperAddress() {return 0xBFFF;}

	bool getEnableRam() {return bEnableRam;}
	unsigned char getRamBankNb() {return ramBankNb;}
	unsigned char getLeastSignificantRomByte() {return leastSignificantRomByte;}
	bool getBit9() { return bit9; }

	void setEnableRam(bool val) {bEnableRam = val;}
	void setLeastSignificantRomByte(unsigned char val) { leastSignificantRomByte = val; }
	void setBit9(bool val) {bit9 = val;}
	void setRamBankNb(unsigned char val) {ramBankNb = val;}
private:
	bool bEnableRam;
	unsigned char leastSignificantRomByte;
	bool bit9;
	unsigned char ramBankNb;

};

#endif
