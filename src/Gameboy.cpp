#include "UserInterface.hpp"
#include "Gameboy.hpp"
#include "Hdma.hpp"
#include "Debugger.hpp"
#include "APU.hpp"
#include <sys/stat.h>
#include "Joypad.hpp"
#include "define.hpp"
#include <fstream>
#include "Common.hpp"
#include "Screen.hpp"
#include "Cpu.hpp"
#include "Ppu.hpp"
#include <iostream>
#include <functional>

Mem*		Gameboy::gbMem = nullptr;
Clock		Gameboy::gbClock = Clock();
int		Gameboy::currentState = 0;
uint8_t		Gameboy::internalLY = 0;
int		Gameboy::clockLine = 0;
bool		Gameboy::bShouldRenderFrame = true;
bool		Gameboy::quit = false;
bool		Gameboy::bIsCGB = false;
bool		Gameboy::bCGBIsInCompatMode = false;
bool		Gameboy::bIsInit = false;
bool		Gameboy::bIsPathValid = false;
bool		Gameboy::lcdcWasOff = false;
std::string	Gameboy::path = "";
float		Gameboy::clockRest = 0;
Gameboy::saveBufferStruct	Gameboy::saveBuffer = {nullptr, nullptr};
unsigned short	Gameboy::saveBufferSize = 0;
bool		Gameboy::bLYIs0AndOAM = true;

Clock& Gameboy::getClock()
{
	return (gbClock);
}

void	Gameboy::init()
{
	//gbMem = nullptr;
	gbClock.reset();
	currentState = 0;
	internalLY = 0;
	clockLine = 0;
	bShouldRenderFrame = true;
	quit = false;
	bIsCGB = false;
	bCGBIsInCompatMode = false;
	clockRest = 0;
	clockLine = 0;
	Hdma::reset();
	Ppu::reset();
	UserInterface::reset();
	Debugger::reset();
}

void	Gameboy::updateLY(int iter)
{
	if (!BIT(M_LCDC, 7)) {
		// special case LCD disabled : don't update LY
		// And set it to 0
		mem.supervisorWrite(LY, 0);
		lcdcWasOff = true;
		bLYIs0AndOAM = true;
		return;
	}
	else if (lcdcWasOff) {
		// If lcdc was off a return back to on we must update internalLY to 0
		bShouldRenderFrame = false;
		lcdcWasOff = false;
		internalLY = 0;
	}
	if (M_LY == 0)
	{
		// SPECIAL undocumented case
		// about LY not being 99 but actually
		// 0 during vblank for 2 'line' cycles
		if (bLYIs0AndOAM == true)
		{
			mem.supervisorWrite(LY, 1);
			bLYIs0AndOAM = false;
		}
		else
			bLYIs0AndOAM = true;
	}
	else
		mem.supervisorWrite(LY, ((M_LY + iter) % 153));
	if (M_LY == M_LYC) {
		SET(M_LCDC_STATUS, 2);
		if (BIT(M_LCDC_STATUS, 6)) {
			Cpu::requestInterrupt(IT_LCD_STAT);
		}
	}
	else {
		RES(M_LCDC_STATUS, 2);
	}
}

bool Gameboy::loadRom()
{
	init();
	gbMem = Mem::loadFromFile(path);
	if (!gbMem || !gbMem->isValid)
	{
		if (gbMem)
		{
			delete gbMem;
			gbMem = nullptr;
		}
		bIsPathValid = false;
		return false;
	}
	bool bCartIsCGB = gbMem->isCGB();
	bIsCGB = (!UserInterface::forceMode ? bCartIsCGB : UserInterface::forceCGB);

	std::cout << (bIsCGB ? "cartridge is CGB" : "cartridge is DMG") << std::endl;

	if (bIsCGB && !bCartIsCGB) {
		bCGBIsInCompatMode = true;
		std::cout << "CGB is in compatibility mode for DMG" << std::endl;
	}
	if (mem.extraRamBanksNb)
	{
		saveBufferSize = mem.extraRamBanksNb * mem.getRamBankSize();
		std::cout << "saveBufferSize is: " << saveBufferSize << std::endl;
		saveBuffer.value = new unsigned char[saveBufferSize];
		saveBuffer.bHasBeenWritten = new bool[saveBufferSize];
		for (int i = 0; i < saveBufferSize; i++)
		{
			saveBuffer.value[i] = 0;
			saveBuffer.bHasBeenWritten[i] = false;
		}
	}
	else
		saveBufferSize = 0;
	Cpu::reset();
	APU::reset();
	Screen::createTexture(bIsCGB, UserInterface::uiRenderer);
	Debugger::createTexture(bIsCGB, UserInterface::uiRenderer);
	return gbMem->isValid;
}

bool Gameboy::launchUserInterface()
{
	if (UserInterface::create())
	    return UserInterface::loop();
    return false;
}

void Gameboy::clear()
{
	if (bIsInit) {
		saveRam();
		if (mem.extraRamBanksNb)
		{
			delete [] saveBuffer.value;
			delete [] saveBuffer.bHasBeenWritten;
		}
		delete gbMem;
		APU::clear();
		Screen::destroyTexture();
		Debugger::destroyTexture();
		bIsInit = false;
	}
}

bool Gameboy::execFrame(Gameboy::Step step, bool bRefreshScreen)
{
	if (!bIsInit || UserInterface::bIsError) {
		return (false);
	}

	// render a white screen if LCD is off
	// normal render wont be called since we wont enter pxl transfer state
	std::function<bool()> loopFunc = [&]()
	{
		if (Gameboy::executeLine(step == Step::oneInstruction, internalLY < 144, bRefreshScreen))
		{
			updateLY(1);
			if (!(M_LY == 0 && bLYIs0AndOAM == false))
				internalLY++;
		}
		if (step == Step::oneLine || step == Step::oneInstruction)
			return false;
		return true;
	};

	//it returns when whole 144 lines have been rendered
	//implementation: if lcdc is off then count clocks in order to give the screen
	//(even if it's white), otherwise always return when ly > 144 because of Screen Tearing
	//when lcdc was disabled and came back on internalLY is update to 0 thanks to lcdcWasOff
	while (!UserInterface::bIsError && internalLY < 144)
	{
		if (!loopFunc())
			break;
	}
	while (!UserInterface::bIsError && internalLY >= 144 && internalLY < 153)
	{
		Gameboy::setState(GBSTATE_V_BLANK, bRefreshScreen);
		if (!loopFunc())
			break;
	}
	if (!UserInterface::bIsError && internalLY >= 153) {
		Ppu::resetWindowCounter();
		bShouldRenderFrame = true;
		internalLY = 0;
	}
	// internalLY %= 154;
	return true;
}

void Gameboy::doHblankHdma()
{
	int clockHblankForHdma = Hdma::updateHBlank();
	if (clockHblankForHdma)
	{
		// update g_clock/clock here instead of cpu
		// because it has to be done once per hblank
		g_clock += clockHblankForHdma;
		APU::tick(clockHblankForHdma);
		clockLine += clockHblankForHdma;
	}
}

void Gameboy::setState(int newState, bool bRefreshScreen)
{
	if (BIT(M_LCDC, 7) && currentState != newState)
	{
		if (newState == GBSTATE_V_BLANK)
		{
			if (BIT(M_LCDC_STATUS, 4))
			{
				//std::cout << "request interrupt VBLANK" << std::endl;
				Cpu::requestInterrupt(IT_LCD_STAT);
			}
			Cpu::requestInterrupt(IT_VBLANK);
		}
		// should refresh screen
		if (newState == GBSTATE_H_BLANK)
		{
			if (bRefreshScreen)
			{
				// we need to have the ternary because the
				// frame after the lcd was put on is still white
				Screen::updatePpuLine( bShouldRenderFrame ? Ppu::doOneLine() : Ppu::getDefaultWhiteLine()
						, internalLY, bIsCGB);
			}
		}
		if (newState == GBSTATE_H_BLANK) {
			// need to check hblank hdma
			doHblankHdma();
		}
		if (newState == GBSTATE_H_BLANK && BIT(M_LCDC_STATUS, 3)) {
			//std::cout << "request interrupt HBLANK" << std::endl;
			Cpu::requestInterrupt(IT_LCD_STAT);
		}
		if (newState == GBSTATE_OAM_SEARCH && BIT(M_LCDC_STATUS, 5)) {
			//std::cout << "request interrupt OAM" << std::endl;
			Cpu::requestInterrupt(IT_LCD_STAT);
		}
		// unsigned char lcdcs = M_LCDC_STATUS & ~0x07;
		unsigned char lcdcs = M_LCDC_STATUS & ~0x03;
		lcdcs |= newState;
		mem.supervisorWrite(LCDC_STATUS, lcdcs);
	}
	currentState = newState;
}

void Gameboy::loadSaveState()
{
	s_state tmp;
	std::cout << "Loading game state\n";
	std::cout << "Thread ID : " << pthread_self() << "\n";
	
	std::string savePath;
    if (bIsCGB)
        savePath = Gameboy::path + ".cbg.state";
    else
        savePath = Gameboy::path + ".dmg.state";
    std::ifstream infile(savePath, std::ios::binary);
	if (!infile.is_open()) {
		UserInterface::throwError("No save state found", false);
		return ;
	}
    	std::vector<unsigned char> content((std::istreambuf_iterator<char>(infile)),
    	                                       std::istreambuf_iterator<char>());
    	infile.close();
	if (content.size() < sizeof(tmp)) {
		UserInterface::throwError("Corrupted save state, please delete it manually.", false);
		return ;
	}
	memcpy(&tmp, content.data(), sizeof(tmp));

	size_t romHash = 0;
	for (size_t i = 0; i < mem.romBanks.size(); i++)
		romHash += ft_hash(mem.romBanks.data()[i], ROM_BANK_SIZE);
	if (romHash != tmp.romHash) {
		bIsInit = false;
		UserInterface::throwError("Could not load save state from a different game", false);
		return ;
	}

	// Load CPU state
	Cpu::IME = tmp.cpu.IME;
	Cpu::setIMEFlag = tmp.cpu.setIMEFlag;
	Cpu::halted = tmp.cpu.halted;
	Cpu::halt_counter = tmp.cpu.halt_counter;
	Cpu::PC = tmp.cpu.PC;
	Cpu::SP = tmp.cpu.SP;
	memcpy(&Cpu::registers, tmp.cpu.registers, sizeof(unsigned short[4]));

	// Load Clock state
	Gameboy::getClock().setClock(tmp.clock.val);
	Gameboy::getClock().setDivClock(tmp.clock.divClock);
	Gameboy::getClock().setReloadTMA(tmp.clock.reloadTMA);
	Gameboy::getClock().setTimaClock(tmp.clock.timaClock);

	// Load PPU state
	Ppu::setWindowCounter(tmp.ppu.windowCounter);

	// Load Gameboy state
	Gameboy::clockLine = tmp.gameboy.clockLine;
	Gameboy::currentState = tmp.gameboy.currentState;
	Gameboy::bIsCGB = tmp.gameboy.bIsCGB;
	Gameboy::internalLY = tmp.gameboy.internalLY;
	// Gameboy::path = tmp.gameboy.path;
	Gameboy::quit = tmp.gameboy.quit;
	Gameboy::bLYIs0AndOAM = tmp.gameboy.bLYIs0AndOAM;
	
	// Load MBC state
	switch (Gameboy::getMem().mbc->type)
	{
		case 0:
			break;
		case 1: {
			MBC1 *ptr = dynamic_cast<MBC1*>(Gameboy::getMem().mbc);
			ptr->setAdvancedBankingMode(tmp.mbc.bank.mbc1.bAdvancedBankingMode);
			ptr->setEnableRam(tmp.mbc.bank.mbc1.bEnableRam);
			ptr->setHighBitsRomBankNumberOrRam(tmp.mbc.bank.mbc1.highBitsRomBankNumberOrRam);
			ptr->setLowBitsRomBankNumber(tmp.mbc.bank.mbc1.lowBitsRomBankNumber);
		} break;
		case 2: {
			MBC2 *ptr = dynamic_cast<MBC2*>(Gameboy::getMem().mbc);
			ptr->setEnableRam(tmp.mbc.bank.mbc2.bEnableRam);
			ptr->setRomBankNb(tmp.mbc.bank.mbc2.romBankNb);
		} break;
		case 3: {
			MBC3 *ptr = dynamic_cast<MBC3*>(Gameboy::getMem().mbc);
			ptr->setEnableRam(tmp.mbc.bank.mbc3.bEnableRam);
			ptr->setStart(tmp.mbc.bank.mbc3.start);
			ptr->setRTC(tmp.mbc.bank.mbc3.rtc_register);
			ptr->setRTCBind(tmp.mbc.bank.mbc3.rtcBindNb);
			ptr->setRomBankNb(tmp.mbc.bank.mbc3.romBankNb);
			ptr->setRamBankNb(tmp.mbc.bank.mbc3.ramBankNb);
			ptr->setLastVal(tmp.mbc.bank.mbc3.lastVal);
			ptr->setLatch(tmp.mbc.bank.mbc3.latched);
		} break;
		case 5: {
			MBC5 *ptr = dynamic_cast<MBC5*>(Gameboy::getMem().mbc);
			ptr->setEnableRam(tmp.mbc.bank.mbc5.bEnableRam);
			ptr->setLeastSignificantRomByte(tmp.mbc.bank.mbc5.leastSignificantRomByte);
			ptr->setBit9(tmp.mbc.bank.mbc5.bit9);		
			ptr->setRamBankNb(tmp.mbc.bank.mbc5.ramBankNb);
		} break;
		
		default:
			bIsInit = false;
			UserInterface::throwError("Incorrect MBC type for save state loading", true);
			return ;
	}

	size_t offset = sizeof(s_state);
	
	if (mem.mbc->hasTimer) {
		// Fetching timer save
		MBC3 *ptr = dynamic_cast<MBC3*>(mem.mbc);
		if (ptr)
		{
			if (content.size() - offset < sizeof(time_t)) {
				UserInterface::throwError("Missing data in state (MBC3)", true);
				return;
			}
			memcpy(&ptr->start, content.data() + offset, sizeof(time_t));
			offset += sizeof(time_t);
		}
	}

	if (content.size() - offset < MEM_SIZE) {
		UserInterface::throwError("Missing data in state (Internal Array)", true);
		return;
	}
	memcpy(Gameboy::getMem().getInternalArray(), content.data() + offset, MEM_SIZE);

	offset += MEM_SIZE;

	{
		unsigned short ramBankSize = mem.getRamBankSize();
		for (size_t i = 0; i < Gameboy::getMem().extraRamBanks.size(); i++) {
			if (content.size() - offset < ramBankSize) {
				UserInterface::throwError("Missing data in state (RAM BANK)", true);
				return;
			}
			memcpy(Gameboy::getMem().extraRamBanks[i], content.data() + offset, ramBankSize);
			offset += ramBankSize;
		}
	}

	CGBMem *ptr = dynamic_cast<CGBMem*>(&Gameboy::getMem());
	if (ptr) {
		// CGB
		ptr->CGBextraRamBankNb = tmp.cgb.CGBextraRamBankNb;
		
		for (int i = 0; i < 64; i++) {
			ptr->BGPalettes[i] = tmp.cgb.BGPalettes[i];
			ptr->OBJPalettes[i] = tmp.cgb.OBJPalettes[i];
		}
		for (int i = 0; i < 8; i++) {
			ptr->CGBCompatPaletteSaveBG[i] = tmp.cgb.CGBCompatPaletteSaveBG[i];
		}
		for (int i = 0; i < 16; i++) {
			ptr->CGBCompatPaletteSaveOBJ[i] = tmp.cgb.CGBCompatPaletteSaveOBJ[i];
		}
		ptr->bIsUsingCGBVram = tmp.cgb.bIsUsingCGBVram;
		
		if (content.size() - offset < 0x2000) {
			UserInterface::throwError("Missing data in state (CGBVram)", true);
			return;
		}
		memcpy(ptr->getCGBVram(), content.data() + offset, 0x2000);
		offset += 0x2000;

		for (int i = 0; i < 8; i++) {
			if (ptr->getCGBExtraRamBanks()[i]) {
				if (content.size() - offset < 0x1000) {
					UserInterface::throwError("Missing data in state (CGBExtraRam)", true);
					return;
				}
				memcpy(ptr->getCGBExtraRamBanks()[i], content.data() + offset, 0x1000);
			}
			offset += 0x1000;
		}
	}
}

void Gameboy::saveState()
{
	std::cout << "Saving game state\n";
	s_state tmp;

	tmp.romHash = 0;
	for (size_t i = 0; i < mem.romBanks.size(); i++)
		tmp.romHash += ft_hash(mem.romBanks.data()[i], ROM_BANK_SIZE);

	std::cout << "Rom hash : " << std::dec << tmp.romHash << "\n";
	
	// Save CPU state
	tmp.cpu.IME = Cpu::IME;
	tmp.cpu.setIMEFlag = Cpu::setIMEFlag;
	tmp.cpu.halted = Cpu::halted;
	tmp.cpu.halt_counter = Cpu::halt_counter;
	tmp.cpu.PC = Cpu::PC;
	tmp.cpu.SP = Cpu::SP;
	memcpy(&tmp.cpu.registers, Cpu::registers, sizeof(Cpu::registers));
	
	// Save clock state
	tmp.clock.divClock = Gameboy::getClock().getDivClock();
	tmp.clock.timaClock = Gameboy::getClock().getTimaClock();
	tmp.clock.reloadTMA = Gameboy::getClock().getReloadTMA();
	tmp.clock.val = Gameboy::getClock().getClock();
	tmp.clock.cgbMode = Gameboy::getClock().cgbMode;

	tmp.mbc.type = Gameboy::getMem().mbc->type;

	// Save PPU state
	tmp.ppu.windowCounter = Ppu::getWindowCounter();

	// Save Gameboy state
	tmp.gameboy.currentState = Gameboy::currentState;
	// tmp.gameboy.path = Gameboy::path;
	tmp.gameboy.bIsCGB = Gameboy::bIsCGB;
	tmp.gameboy.clockLine = Gameboy::clockLine;
	tmp.gameboy.internalLY = Gameboy::internalLY;
	tmp.gameboy.quit = Gameboy::quit;
	tmp.gameboy.bLYIs0AndOAM = bLYIs0AndOAM;

	// Save MBC state
	switch (tmp.mbc.type)
	{
	case 0:
		break;
	case 1: {
		MBC1 *ptr = dynamic_cast<MBC1*>(Gameboy::getMem().mbc);
		tmp.mbc.bank.mbc1.bAdvancedBankingMode = ptr->getAdvancedBankingMode();
		tmp.mbc.bank.mbc1.bEnableRam = ptr->getEnableRam();
		tmp.mbc.bank.mbc1.highBitsRomBankNumberOrRam = ptr->getHighLowBitsRomBankNumber();
		tmp.mbc.bank.mbc1.lowBitsRomBankNumber = ptr->getLowBitsRomBankNumber();
	} break;
	case 2: {
		MBC2 *ptr = dynamic_cast<MBC2*>(Gameboy::getMem().mbc);
		tmp.mbc.bank.mbc2.bEnableRam = ptr->getEnableRam();
		tmp.mbc.bank.mbc2.romBankNb = ptr->getRomBankNb();
	} break;
	case 3: {
		MBC3 *ptr = dynamic_cast<MBC3*>(Gameboy::getMem().mbc);
		tmp.mbc.bank.mbc3.bEnableRam = ptr->getEnableRam();
		tmp.mbc.bank.mbc3.lastVal = ptr->getLastVal();
		tmp.mbc.bank.mbc3.latched = ptr->getLatch();
		tmp.mbc.bank.mbc3.ramBankNb = ptr->getRamBankNb();
		tmp.mbc.bank.mbc3.romBankNb = ptr->getRomBankNb();
		tmp.mbc.bank.mbc3.rtc_register = ptr->getRTC();
		tmp.mbc.bank.mbc3.rtcBindNb = ptr->getRTCBind();
		tmp.mbc.bank.mbc3.start = ptr->getStart();
	} break;
	case 5: {
		MBC5 *ptr = dynamic_cast<MBC5*>(Gameboy::getMem().mbc);
		tmp.mbc.bank.mbc5.bEnableRam = ptr->getEnableRam();
		tmp.mbc.bank.mbc5.bit9 = ptr->getBit9();
		tmp.mbc.bank.mbc5.leastSignificantRomByte = ptr->getLeastSignificantRomByte();
		tmp.mbc.bank.mbc5.ramBankNb = ptr->getRamBankNb();
	} break;
	
	default:
		break;
	}


	// std::cout << "\tIME = " << Cpu::IME << "\n";
	// std::cout << "\tIF = " << Cpu::setIMEFlag << "\n";
	// std::cout << "\tHalted = " << Cpu::halted << "\n";
	// std::cout << "\tHalt_counter = " << Cpu::halt_counter << "\n";
	// std::cout << "\tPC = " << Cpu::PC << "\n";
	// std::cout << "\tSP = " << Cpu::SP << "\n";
	// std::cout << "\tRegisters = " << Cpu::registers[0] << Cpu::registers[1] << Cpu::registers[2] << Cpu::registers[3] << "\n";
	// std::cout << "\tLY = " << (int)M_LY << "\n";

    std::string savePath;
    if (bIsCGB) {
        savePath = Gameboy::path + ".cbg.state";
    } else {
        savePath = Gameboy::path + ".dmg.state";
    }
	std::ofstream outfile(savePath, std::ios::binary);
	CGBMem *ptr = dynamic_cast<CGBMem*>(&Gameboy::getMem());
	if (ptr) {
		// CGB
		tmp.cgb.bIsUsingCGBVram = ptr->bIsUsingCGBVram;
		tmp.cgb.CGBextraRamBankNb = ptr->CGBextraRamBankNb;
		memcpy(tmp.cgb.BGPalettes, ptr->BGPalettes.data(), sizeof(unsigned char) * 64);
		memcpy(tmp.cgb.OBJPalettes, ptr->OBJPalettes.data(), sizeof(unsigned char) * 64);

		memcpy(tmp.cgb.CGBCompatPaletteSaveBG, ptr->CGBCompatPaletteSaveBG.data(), sizeof(unsigned char) * 8);
		memcpy(tmp.cgb.CGBCompatPaletteSaveOBJ, ptr->CGBCompatPaletteSaveOBJ.data(), sizeof(unsigned char) * 16);
	}

	outfile.write(reinterpret_cast<char*>(&tmp), sizeof(tmp));


	if (mem.mbc->hasTimer) {
		MBC3 *ptr = dynamic_cast<MBC3*>(mem.mbc);
		if (ptr) {
			outfile.write(reinterpret_cast<char *>(&ptr->start), sizeof(time_t));
		}
	}

	outfile.write(reinterpret_cast<char*>(mem.getInternalArray()), MEM_SIZE);

	{
		unsigned short ramBankSize = mem.getRamBankSize();
		for (unsigned char *elem : mem.extraRamBanks) {
			outfile.write(reinterpret_cast<char*>(elem), ramBankSize);
		}
	}

	if (ptr) {
		outfile.write(reinterpret_cast<char*>(ptr->getCGBVram()), 0x2000);

		for (int i = 0; i < 8; i++) {
			if (ptr->getCGBExtraRamBanks()[i])
				outfile.write(reinterpret_cast<char*>(ptr->getCGBExtraRamBanks()[i]), 0x1000);
		}
	}

	outfile.close();
}

void Gameboy::saveByteInSave(unsigned short addr, unsigned char value)
{
	saveBuffer.value[addr] = value;
	saveBuffer.bHasBeenWritten[addr] = true;
}

void Gameboy::saveRam()
{
	if (!bIsPathValid || !bIsInit || saveBufferSize == 0)
		return ;
	struct stat _;
	bool bSaveExists = stat((path + ".save").c_str(), &_) != -1;
	std::fstream saveFile(path + ".save", std::ios::binary | std::ios::out |
			(bSaveExists ? std::ios::in : std::ios::out));

	if (mem.mbc->hasTimer) {
		MBC3 *ptr = dynamic_cast<MBC3*>(mem.mbc);
		if (ptr)
			saveFile.write(reinterpret_cast<char *>(&ptr->start), sizeof(time_t));
	}

	if (mem.extraRamBanksNb && !bSaveExists)
	{
		saveFile.write((char*)saveBuffer.value, saveBufferSize);
	}
	else if (mem.extraRamBanksNb)
	{
		auto startPos = saveFile.tellp();
		unsigned char* finalBuffer = new unsigned char[saveBufferSize];
		saveFile.read((char*)finalBuffer, saveBufferSize);
		if (saveFile.tellg() - startPos != saveBufferSize)
		{
			std::cerr << "warning existant save had a bad size when writing to it" << std::endl;
			saveFile.write((char*)saveBuffer.value, saveBufferSize);// save is corrupted so rewrite it
		}
		else
		{
			for (int i = 0; i < saveBufferSize; i++)
			{
				if (saveBuffer.bHasBeenWritten[i]) {
					finalBuffer[i] = saveBuffer.value[i];
				}
			}
			saveFile.seekp(startPos);
			saveFile.write((char*)finalBuffer, saveBufferSize);
		}
		delete [] finalBuffer;
	}
	saveFile.close();
}

int Gameboy::getState()
{
	return (currentState);
}

int	Gameboy::executeLine(bool step, bool updateState, bool bRefreshScreen)
{
	static const int nbClockLine = 114;
	std::pair<unsigned char, int>r;

	while (!UserInterface::bIsError && Gameboy::clockLine < nbClockLine)
	{
		if (!(M_LY == 0 && bLYIs0AndOAM == false))
		{
			if (updateState && Gameboy::clockLine < 20)
			{
				Gameboy::setState(GBSTATE_OAM_SEARCH, bRefreshScreen);
			}
			else if (updateState && Gameboy::clockLine < 20 + 43)
			{
				Gameboy::setState(GBSTATE_PX_TRANSFERT, bRefreshScreen);
			}
			else if (updateState && Gameboy::clockLine < 20 + 43 + 51)
			{
				Gameboy::setState(GBSTATE_H_BLANK, bRefreshScreen);
			}
		}

		int clockInc = Cpu::doMinimumStep();
		g_clock += clockInc;
		APU::tick(clockInc);
		if (Clock::cgbMode)
		{
			int entireClock = (clockInc / 2.0) + clockRest;
			if (entireClock >= 1)
				Gameboy::clockLine += entireClock;
			clockRest = ((clockInc / 2.0) + clockRest) - entireClock;
		}
		else
			Gameboy::clockLine += clockInc;

		if (step) {
			break;
		}
	}
	if (Gameboy::clockLine >= nbClockLine) {
		Gameboy::clockLine -= nbClockLine;
		return (true);
	}
	return (false);
}

void Gameboy::throwError(std::string errMsg)
{
	UserInterface::throwError((std::string("Fatal error encountered: ") + errMsg).c_str(), true);
}
