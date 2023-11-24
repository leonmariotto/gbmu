/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mem.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 20:49:00 by nallani           #+#    #+#             */
/*   Updated: 2023/02/03 12:37:51 by lmariott         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UserInterface.hpp"
#include "Mem.hpp"
#include "Ppu.hpp"
#include "APU.hpp"
#include "Hdma.hpp"
#include <fstream>
#include <iostream>
#include "Joypad.hpp"
#include "Gameboy.hpp"

#include "Bootrom.hpp"

const std::map<unsigned short, unsigned char> Mem::readOnlyBits = {
 {0xFF00, 0b1100'1111}, // 0xCF, 0xFF00 is input register, first 4 bit are
						// set to 0 when pressed, last 2 are unused
 {0xFF02, 0b0111'1110}, // serial control : TODO bit freq speed is CGB only, we don't need it as we don't support this
 {0xFF07, 0b1111'1000}, // TAC. unused last 5 bits
 {0xFF0F, 0b1110'0000}, // IF, unusued last 7 bits
 {0xFF41, 0b1000'0111}, // LCDC Stat, unused bit7, first 3 bit are readOnly (set by ppu)
 {0xFF4D, 0b0111'1110}, // Key1
 {0xFF4F, 0b1111'1110}, // VBK, Vbank spec, only 1rst bit used
 {0xFF52, 0b0000'1111}, // HDMA2
 {0xFF53, 0b1110'0000}, // HDMA3
 {0xFF54, 0b0000'1111}, // HDMA4
 {0xFF68, 0b0100'0000}, // BCPS
 {0xFF6A, 0b0100'0000}, // OCPS
 {0xFF70, 0b1111'1000}, // SVBK, only 3 first bits used
 {0xFF26, 0b0000'1111}, // Sound on/off : required by some game which reading it
				
};

Mem* Mem::loadFromFile(const std::string& pathToRom)
{
	std::ifstream file = std::ifstream(pathToRom, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "file not found" << std::endl;
		UserInterface::throwError("File not found", false);
		return nullptr;
	}
	// read some values in the
	file.seekg(0, std::ifstream::end);
	int fileLen = file.tellg();
	if (fileLen < 32'768)
	{
			std::cerr << "Error with rom at wrong format" << std::endl;
			UserInterface::throwError("File is not valid, please select a valid Rom file", false);
			file.close();
			return nullptr;
			// throw("");
	}
	file.seekg(0x143, std::ifstream::beg);
	char CGBCode;
	file.read(&CGBCode, 1);
	std::cout << std::hex << "CGBCode: " << +CGBCode << std::endl;
	file.close();
	Mem* newMem;
	// boot roms are stored in byte array in Bootrom.hpp
	if (((CGBCode & 0x80) || (UserInterface::forceMode && UserInterface::forceCGB))
			&& !(UserInterface::forceMode && !UserInterface::forceCGB))
	{
		newMem = new CGBMem(pathToRom);
		if (newMem->isValid)
			memcpy(newMem->internalArray, cgbBootRom, 2304);
	}
	else
	{
		newMem = new Mem(pathToRom);
		if (newMem->isValid)
			memcpy(newMem->internalArray, dmgBootRom, 256);
	}
	return newMem;
}

void Mem::supervisorWrite(unsigned int addr, unsigned char value)
{
	Mem::internalArray[addr] = value;
}

std::vector<unsigned char> Mem::readFile(const std::string& filename)
{
    std::ifstream infile(filename, std::ios::binary);
    std::vector<unsigned char> fileContent((std::istreambuf_iterator<char>(infile)),
                                           std::istreambuf_iterator<char>());
    infile.close();
    return fileContent;
}

unsigned short Mem::getRamBankSize() const
{
	return mbc->getRamUpperAddress() - 0xA000 + 1;
}

Mem::Mem(const std::string& pathToRom)
{
	std::ifstream file = std::ifstream(pathToRom, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "file not found" << std::endl;
		UserInterface::throwError("File not found", false);
		isValid = false;
		return;
	}

	file.seekg(0, std::ifstream::end);

	long fileLen = file.tellg();
	file.seekg(0x148, std::ifstream::beg);
    char romSizeCode;
   	file.read(&romSizeCode, 1);
	int romBanksNb = getRomBanksNb(romSizeCode);
    std::cout<<"rombank: "<< std::dec<<(int)romBanksNb << " filelen: "<< (int)fileLen<<std::hex<<std::endl;
	if (fileLen != romBanksNb * 1024 * 16) //32768
	{
        file.close();
		UserInterface::throwError("File is not valid, please select a valid Rom file", false);
		isValid = false;
        UserInterface::bIsError = true;
		return ;
	}
	char ramSizeCode;
   	file.read(&ramSizeCode, 1);
	file.seekg(0, std::ifstream::beg);

	for (int i = 0; i < romBanksNb; i++) {
		romBanks.push_back(new unsigned char[ROM_BANK_SIZE]);
		bzero(romBanks[i], ROM_BANK_SIZE);
	}
	std::cout << "created " << romBanksNb << " rom banks" << std::endl;

	internalArray = new unsigned char[MEM_SIZE];
	bzero(internalArray, MEM_SIZE);

    for (int i = 0; i < romBanksNb; i++) {
        file.read((char *) romBanks[i], ROM_BANK_SIZE);
    }

//    for (int i = 0; i < extraRamBanksNb; i++) {
//        file.read((char *) extraRamBanks[i], ROM_BANK_SIZE);
//    }
    file.close();
	isValid = true;
	std::cout << "loaded rom with title: " << getTitle() << std::endl;
	std::cout << std::hex << "CartRidge type: " << (int)getCartridgeType() << std::endl;
	mbc = MBC::createMBC(getCartridgeType());

	extraRamBanksNb = getExtraRamBanksNb(ramSizeCode);
	if (extraRamBanksNb == 0 && mbc->type == 2)
		extraRamBanksNb = 1;
	unsigned short ramBankSize = getRamBankSize();
	for (int i = 0; i < extraRamBanksNb; i++) {
		extraRamBanks.push_back(new unsigned char[ramBankSize]);
		bzero(extraRamBanks[i], ramBankSize);
	}
	std::cout << "created " << +extraRamBanksNb << " extra ram banks" << std::endl;

	// Check if there is a save
	std::ifstream saveFile(pathToRom + ".save");
	if (saveFile.good())
	{
		std::cout << "Save was detected" << std::endl;
		// std::vector<unsigned char> saveContent = readFile(pathToRom + ".save");
		saveFile.seekg(0, std::ifstream::end);
		long unsigned int fileLen = saveFile.tellg();

		if (fileLen != ramBankSize * extraRamBanksNb + (mbc->hasTimer ? sizeof(time_t) : 0))
		{
			saveFile.close();
			UserInterface::throwError("Corrupted save file has been found with that game, please delete it manually", true);
			return;
		}

		saveFile.seekg(0, std::ifstream::beg);
		std::vector<unsigned char> saveContent((std::istreambuf_iterator<char>(saveFile)),
				std::istreambuf_iterator<char>());
		saveFile.close();

		// TODO do more check with size for extraRam
		if (mbc->hasTimer) {
			// Fetching timer save
			MBC3 *ptr = dynamic_cast<MBC3*>(mbc);
			memcpy(&ptr->start, saveContent.data(), sizeof(time_t));
			std::cout << "Loaded timer : " << std::dec << (int)ptr->start << std::hex << std::endl;
		}

		for (int i = 0; i < extraRamBanksNb; i++)
				memcpy(extraRamBanks[i], saveContent.data() + (mbc->hasTimer ? sizeof(time_t) : 0) + (i * ramBankSize), ramBankSize);
	} else
		std::cout << "No saves were detected" << std::endl;


	init();
}

void Mem::init()
{
	for (auto const& it: readOnlyBits)
	{
		unsigned short address = it.first;
		unsigned char value = it.second;
		internalArray[address] = value;
	}
}

Mem::~Mem()
{
	if (!isValid)
		return;

	delete[] internalArray;
	for (unsigned char* ramBank : extraRamBanks)
		delete[] ramBank;
	extraRamBanks.clear();
	for (unsigned char* romBank : romBanks)
		delete[] romBank;
	romBanks.clear();
	delete mbc;
}

MemWrap Mem::operator[](unsigned int i)
{
	return MemWrap(i, getRefWithBanks(i));
}

const MemWrap Mem::operator[](unsigned int i) const
{
	return MemWrap(i, getRefWithBanks(i));
}

unsigned char& Mem::getRefWithBanks(unsigned short addr) const
{
	//0xff50 is used to know if bootrom should be loaded or not
	// address from 0x100 to 0x200 are not considered because that's
	// where the nintendo logo should be in cartridges
	if (internalArray[0xFF50] == 0 && (addr < 0x100 || (addr >= 0x200 && addr <= 0x08FF)))
			return internalArray[addr];
	if (addr <= 0x7FFF)
	{
		unsigned char romBankNb = mbc->getRomBank(addr);
        romBankNb &= romBanks.size() - 1; // make sure it's in range
		//std::cout << "at address: " << addr << " i muse use romBank: " << (int)romBankNb << std::endl;
		if (addr <= 0x3FFF)
			return romBanks[romBankNb][addr];
		else
			return romBanks[romBankNb][addr - 0x4000];
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		unsigned char ramBankNb = mbc->getRamBank();
		if (ramBankNb == 0xFF) { // 0xFF stands for no extra ram used
			if (mbc->type == 3) {
				MBC3 *ptr = dynamic_cast<MBC3*>(mbc);
				if (ptr) {
					return *(((unsigned char *)&ptr->rtc_register) + ptr->rtcBindNb);
				} else
					throw "Could not dynamically cast MBC3";
			}
			// mbc2 exception : use only first extraRamBank
			// only the 9 bottom bits are used for address
			if (mbc->type == 2) {
				return extraRamBanks[0][addr & 0x1FF];
			}
			// else return 0
			internalArray[addr] = 0;
			return internalArray[addr];
		}
		else
		{
			if (extraRamBanks.size() == 0) {
				throw ("error: trying to acces extra unallocated ram");
			}
			ramBankNb &= extraRamBanks.size() - 1;
			return extraRamBanks[ramBankNb][addr - 0xA000];
		}
	}
	else
		return internalArray[addr];
}

void	Mem::saveByteInSave(unsigned short addr, unsigned char value) const
{
	unsigned char ramBankNb = mbc->getRamBank();
	unsigned short ramBankSize = getRamBankSize();
	if (ramBankNb != 0xFF)
	{
		Gameboy::saveByteInSave((ramBankNb & (extraRamBanks.size() - 1)) * ramBankSize
				+ (addr - 0xA000), value);
	}
	else if (mbc->type == 2 && addr - 0xA000 < 512)
	{
		Gameboy::saveByteInSave(addr - 0xA000, value);
	}
}

unsigned char& MemWrap::operator=(unsigned char newValue)
{
	if (UserInterface::bIsError) {
		return (value); // TODO what we should return here ?
	}
	if (addr >= 0xA000 && addr < 0xC000)
	{
		//Writing in external ram
		mem.saveByteInSave(addr, newValue);
	}
	if (addr == 0xFF1A) {
		mem.supervisorWrite(0xFF26, mem[0xff26] & ~(1 << 2));
	}
	if (addr <= 0x7FFF) // This is a special case to write in special register for bank switching
	{
		mem.mbc->writeInRom(addr, newValue);
		return value;// XXX that might pose aproblem or not
	}
	
	// make sure the new value doesnt override read only bits
	if (Mem::readOnlyBits.count(addr))
		newValue = newValue | Mem::readOnlyBits.at(addr);
	if (addr == LCDC_STATUS) {
		// Ignore bit 7, 1, 2 and 3
		// XXX nallani: that has to be uncorrect, did you mean bit 0, 1, 2 ?
		// fix comment and add it to readOnlyBits
		value &= 0x87;
		value |= (newValue & (~0x87));
		return (value);
	}
	if (addr == 0xFF01) {
		value = 0xFF;
		return (value);
	}
	if (addr == 0xFF02) {
		value = newValue;
		if ((value & 0x81) == 0x81) {
			mem[0xFF0F] = mem[0xFF0F] | (1 << 3);
		}
		return (value);
	}
	if (addr == 0xFF26) {
		APU::turnOnOff(value, newValue);
	}

	value = newValue;

	// if (addr == NR22)
	// 	if ((value & ~0b111) && !(newValue & ~0b111)) {
	// 		std::cout << "Turning off DAC\n";
	// 	}
	// if (addr == NR51)
	// 	std::cout << "Panning : " << (int)newValue << "\n";
	// if (addr == NR50)
	// 	std::cout << "MasterVolume : " << (int)newValue << "\n";
	// if (addr == NR52)
	// 	std::cout << "Sound on/off : " << (int)newValue << "\n";
	// if (addr == NR21)
	// 	std::cout << "NR21 : " << std::hex << (int)value << "\n";
	// if (addr == NR22)
	// 	std::cout << "NR22 : " << std::hex << (int)value << "\n";
	// if (addr == NR23)
	// 	std::cout << "NR23 : " << std::hex << (int)value << "\n";
	// if (addr == NR24)
	// 	std::cout << "NR24 : " << std::hex << (int)value << "\n";

	if (addr == 0xFF00) //JOYPAD register is 0xFF00
		Joypad::refresh();
	else if (addr == NR14) {
		if (BIT(value, 7))
			APU::channel1->triggerChannel();
	}
	else if (addr == NR24) {
		if (BIT(value, 7))
			APU::channel2->triggerChannel();
	}
	else if (addr == NR34) {
		if (BIT(value, 7)) {
			APU::channel3->triggerChannel();
		}
	}
	else if (addr == NR44) {
		if (BIT(value, 7))
			APU::channel4->triggerChannel();
	}
    /* recursive call
//	if (addr == LCDC) {
//		if ((value & 0x80) == 0) {
//			M_LY = 0;
//		}
//	}
//	if (addr == 0xFF02 && newValue == 0x81)
//	{
//		if (mem[0xFF01] == ' ')
//		{
//			std::cout << std::endl;
//		}
//		else
//		{
//			std::cout << (char)(mem[0xFF01]);
//		}
//	}
*/
    // rework too
	if (addr == LYC ) {
		if (value == M_LY)
			SET(M_LCDC_STATUS, 2);
		else
			RES(M_LCDC_STATUS, 2);
	}
	if (addr == LY) {
		std::cout << "WARNING riting to LY value: " << (int)value << std::endl;
		std::cout << "but LY is read only !!" << std::endl;
		if (value == M_LYC)
			SET(M_LCDC_STATUS, 2);
		else
			RES(M_LCDC_STATUS, 2);
	}
	if (addr == 0xff50 && value != 0 && Gameboy::bIsCGB) {
		const CGBMem& asCGB = dynamic_cast<const CGBMem&>(mem);
		if (Gameboy::bCGBIsInCompatMode)
		{
			memcpy(asCGB.CGBCompatPaletteSaveBG.data(), asCGB.BGPalettes.data(), 8);
			memcpy(asCGB.CGBCompatPaletteSaveOBJ.data(), asCGB.OBJPalettes.data(), 16);
		}
	}
	
    if (addr == 0xFF46) {
		if (newValue <= 0xF1) {
			memcpy(&mem[0xFE00], &mem[(newValue << 8)], 0xa0);// TODO change with banks
			// DMA transfert to OAM : This take 160 cycle and CPU can access only HRAM while TODO
		}
	}
	try // try block because this is only for CGB registers
	{
		const CGBMem& asCGB = dynamic_cast<const CGBMem&>(mem);
		// Save palettes for CGB in compatibility mode
		// Use ff50 to happen only when bootrom is done
		if (addr == 0xFF55) // CGB DMA transfert// CGB ONLY
		{
			// TODO WIP
			// NOTE : most of the registers are in readOnlyBits
			// TODO need to create HDMA class and do it clock by clock in order
			// to be able to stop it
			unsigned short srcAddr = (mem[0xFF51] << 8) | (mem[0xFF52] & 0xF0);
			unsigned short dstAddr = (mem[0xFF53] << 8) | (mem[0xFF54] & 0xF0);
			dstAddr = (dstAddr | 0x8000) & 0x9FF0; 
			//unsigned short len = ((newValue & 0x7F) + 1) * 0x10;// this breaks crystal 
			Hdma::writeInHdma(dstAddr, srcAddr, newValue);
			/*
			unsigned short srcAddr = (mem[0xFF51] << 8) | (mem[0xFF52] & 0xF0);
			unsigned short dstAddr = (mem[0xFF53] << 8) | (mem[0xFF54] & 0xF0);
			dstAddr = (dstAddr | 0x8000) & 0x9FF0; 
			unsigned short len = ((newValue & 0x7F) + 1) * 0x10;// this breaks crystal 
			std::cout << "writing to HMDA5: " << +newValue << std::endl;
			std::cout << "CGB HDMA requested ! with source: " << srcAddr << " and destination: " << dstAddr << " with a len of: " << len << std::endl;
			memcpy(&mem[dstAddr], &mem[srcAddr], len);
			value = 0xFF;// lets say it finished instantly
			std::cout << "HDMA done and was a " << (newValue & (1 << 7) ? "basic DMA": "HBLANK DMA (heavy one)")<< std::endl;
			*/
		}
		if (addr == 0xFF4F && Gameboy::bIsCGB && !Gameboy::bCGBIsInCompatMode) // VBK
		{
			//std::cout << "changed VBK value !" << std::endl;
			asCGB.bIsUsingCGBVram = newValue & 1;
			//std::cout << (newValue ? "Enabling CGB VRBank" : "Disabling CGB VRBank") << std::endl;
		}
		if (addr == 0xFF70 && Gameboy::bIsCGB && !Gameboy::bCGBIsInCompatMode) // SVBK
			asCGB.CGBextraRamBankNb = newValue & 7;
		if (addr == 0xff47 && Gameboy::bCGBIsInCompatMode) { // BGP for CGB in compat mode
			int index;
			index = value & 0b11;
			memcpy(&asCGB.BGPalettes.data()[0], &asCGB.CGBCompatPaletteSaveBG.data()[index * 2], 2);
			index = (value >> 2) & 0b11;
			memcpy(&asCGB.BGPalettes.data()[2], &asCGB.CGBCompatPaletteSaveBG.data()[index * 2], 2);
			index = (value >> 4) & 0b11;
			memcpy(&asCGB.BGPalettes.data()[4], &asCGB.CGBCompatPaletteSaveBG.data()[index * 2], 2);
			index = (value >> 6) & 0b11;
			memcpy(&asCGB.BGPalettes.data()[6], &asCGB.CGBCompatPaletteSaveBG.data()[index * 2], 2);
		}
		if (addr == 0xff48 && Gameboy::bCGBIsInCompatMode) { // OBJ0 for CGB in compat mode
			int index;
			// index = value & 0b11;
			// memcpy(&asCGB.OBJPalettes.data()[0], &asCGB.CGBCompatPaletteSaveOBJ.data()[index * 2], 2);
			index = (value >> 2) & 0b11;
			memcpy(&asCGB.OBJPalettes.data()[2], &asCGB.CGBCompatPaletteSaveOBJ.data()[index * 2], 2);
			index = (value >> 4) & 0b11;
			memcpy(&asCGB.OBJPalettes.data()[4], &asCGB.CGBCompatPaletteSaveOBJ.data()[index * 2], 2);
			index = (value >> 6) & 0b11;
			memcpy(&asCGB.OBJPalettes.data()[6], &asCGB.CGBCompatPaletteSaveOBJ.data()[index * 2], 2);
		}
		if (addr == 0xff49 && Gameboy::bCGBIsInCompatMode) { // OBJ1 for CGB in compat mode
			int index;
			// index = value & 0b11;
			// memcpy(&asCGB.OBJPalettes.data()[0], &asCGB.CGBCompatPaletteSaveOBJ.data()[index * 2], 2);
			index = (value >> 2) & 0b11;
			memcpy(&asCGB.OBJPalettes.data()[8 + 2], &asCGB.CGBCompatPaletteSaveOBJ.data()[8 + index * 2], 2);
			index = (value >> 4) & 0b11;
			memcpy(&asCGB.OBJPalettes.data()[8 + 4], &asCGB.CGBCompatPaletteSaveOBJ.data()[8 + index * 2], 2);
			index = (value >> 6) & 0b11;
			memcpy(&asCGB.OBJPalettes.data()[8 + 6], &asCGB.CGBCompatPaletteSaveOBJ.data()[8 + index * 2], 2);
		}
		if (addr == 0xFF69) // BCPD/BGPD
		{
			unsigned char BCPSVal = mem[0xFF68];
			bool bShouldIncrement = BCPSVal & (1 << 7);
			unsigned char BGPAddress = BCPSVal & 0x3F;
			asCGB.BGPalettes[BGPAddress] = newValue;
			if (bShouldIncrement)
			{
				mem[0xFF68] = (mem[0xFF68] & (1 << 7)) | ((BGPAddress + 1) & 0x3F);
			}
			//std::cout << "wrote " << +newValue << " to BG palette nb: " << +(BGPAddress / 8) << " at color nb: " << +(BGPAddress % 8) << std::endl;
			return asCGB.BGPalettes[BGPAddress];
		}
		if (addr == 0xFF6B) // OPCS/OBPD
		{
			unsigned char OCPSVal = mem[0xFF6A];
			bool bShouldIncrement = OCPSVal & (1 << 7);
			unsigned char OBJPAddress = OCPSVal & 0x3F;
			//std::cout << "wrote in FF6B at addr: " << (int)OBJPAddress << std::endl;
			asCGB.OBJPalettes[OBJPAddress] = newValue;
			if (bShouldIncrement)
			{
				mem.supervisorWrite(0xFF6A, (mem[0xFF6A] & (1 << 7)) | ((OBJPAddress + 1) & 0x3F) | (1 << 6));
				//std::cout << "incremented 0xFF6A: " << (int)mem[0xFF6A] << std::endl;
			}
			//std::cout << "wrote color: " << +newValue << " to OBJ Palette nb: " << +(OBJPAddress / 8) << " at color nb: " << +(OBJPAddress % 8) << std::endl;
			return asCGB.OBJPalettes[OBJPAddress];
		}
		/*
		if (addr == 0xFF6A)
			std::cout << "wrote in 0xFF6A: " << (int)value << std::endl;
			*/
	}
	catch (...)
	{
		// nothing should happen
	}
	return value;
}

int		Mem::getRomBanksNb(char romSizeCode)
{
	int value = 2;
	switch (romSizeCode)
	{
		case 8:
			value *= 2;
			[[fallthrough]];
		case 7:
			value *= 2;
			[[fallthrough]];
		case 6:
			value *= 2;
			[[fallthrough]];
		case 5:
			value *= 2;
			[[fallthrough]];
		case 4:
			value *= 2;
			[[fallthrough]];
		case 3:
			value *= 2;
			[[fallthrough]];
		case 2:
			value *= 2;
			[[fallthrough]];
		case 1:
			value *= 2;
			[[fallthrough]];
		case 0:
			return value;
		default:
			std::cerr << "wrong romSizeCodeReceived" << std::endl;
			return 0xFF;
	}
}

int		Mem::getExtraRamBanksNb(char ramSizeCode)
{
	switch (ramSizeCode)
	{
		case 0:
			return 0;
		case 2:
			return 1;
		case 3:
			return 4;
		case 4:
			return 16;
		case 5:
			return 8;
		default:
			std::cerr << "wrong ramSizeCode received" << std::endl;
			throw("");
	}
}

std::string Mem::getTitle()
{
	int i = 0x134;
	std::string title;
	while (romBanks[0][i] != '\0' && i < 0x143)
	{
		title += (char)romBanks[0][i];
		i++;
	}
	return title;
}

bool	Mem::isCGB()
{
	return romBanks[0][0x143] & 0x80;
}

int		Mem::getCartridgeType()
{
	// might be unused with getRamSize / getRomSize instead
	return romBanks[0][0x147];
}

CGBMem::CGBMem(const std::string& pathToRom) : Mem(pathToRom)
{
	CGBVramBank = new unsigned char[0x2000];
	bzero(CGBVramBank, 0x2000);
	BGPalettes.fill(0xFF);
	for (int i = 0; i < 8; i++) {
		CGBextraRamBanks[i] = new unsigned char[0x1000];
		bzero(CGBextraRamBanks[i], 0x1000);
	}
	bIsUsingCGBVram = 0;
}

CGBMem::~CGBMem()
{
	delete[] CGBVramBank;
	for (int i = 0; i < 8; i++)
		delete[] CGBextraRamBanks[i];
}

unsigned char& CGBMem::getRefWithBanks(unsigned short addr) const
{
	if (addr == 0xFF69)
	{
		unsigned char BCPSVal = internalArray[0xFF68];
		unsigned char BGPAddress = BCPSVal & 0x3F;
		return BGPalettes[BGPAddress];
	}
	if (addr == 0xFF6B)
	{
		unsigned char OCPSVal = internalArray[0xFF6A];
		unsigned char OBJPAddress = OCPSVal & 0x3F;
		return OBJPalettes[OBJPAddress];
	}
	if (addr >= 0x8000 && addr <= 0x9FFF)
	{
		if (bIsUsingCGBVram && Gameboy::bIsCGB && !Gameboy::bCGBIsInCompatMode)
		{
			//std::cout << "accessing CGBVramBank !" << std::endl;
			return CGBVramBank[addr - 0x8000];
		}
		else
		{
			//std::cout << "accessing normalVramBank !" << std::endl;
			return internalArray[addr];
		}
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF && Gameboy::bIsCGB && !Gameboy::bCGBIsInCompatMode)
	{
		if (addr <= 0xCFFF)
			return CGBextraRamBanks[0][addr - 0xC000];
		else
		{
			unsigned char index = CGBextraRamBankNb & 0b111;
			if (index == 0) {
				index = 1;
			}
			return CGBextraRamBanks[index][addr - 0xD000];
		}
	}
	else
		return Mem::getRefWithBanks(addr);
}
