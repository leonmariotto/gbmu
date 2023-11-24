/* ************************************************************************** */ /*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cpu.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 20:46:17 by nallani           #+#    #+#             */
/*   Updated: 2023/01/03 21:14:15 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UserInterface.hpp"
#include "Gameboy.hpp"
#include "Cpu.hpp"
#include "Ppu.hpp"
#include "APU.hpp"
#include <functional>
#include <iostream>
#include <iomanip>
#include "Hdma.hpp"
#include "Utility.hpp"

CpuStackTrace	Cpu::stackTrace;

/* Interrupts */
bool		Cpu::IME = false;
bool		Cpu::setIMEFlag = false;
bool		Cpu::halted = false;
uint32_t	Cpu::halt_counter = 0;

/* Registers */
unsigned short 	Cpu::PC = 0;
unsigned short 	Cpu::SP = 0;
unsigned short 	Cpu::registers[4] = {};
unsigned char&	Cpu::A = reinterpret_cast<unsigned char*>(registers)[1];
unsigned char&	Cpu::F = reinterpret_cast<unsigned char*>(registers)[0];
unsigned char&	Cpu::B = reinterpret_cast<unsigned char*>(registers)[3];
unsigned char&	Cpu::C = reinterpret_cast<unsigned char*>(registers)[2];
unsigned char&	Cpu::D = reinterpret_cast<unsigned char*>(registers)[5];
unsigned char&	Cpu::E = reinterpret_cast<unsigned char*>(registers)[4];
unsigned char&	Cpu::H = reinterpret_cast<unsigned char*>(registers)[7];
unsigned char&	Cpu::L = reinterpret_cast<unsigned char*>(registers)[6];
unsigned short&	Cpu::AF = registers[0];
unsigned short&	Cpu::BC = registers[1];
unsigned short&	Cpu::DE = registers[2];
unsigned short&	Cpu::HL = registers[3];

#define IT_VBLANK 0x40
#define IT_LCD_STAT 0x48
#define IT_TIMER 0x50
#define IT_SERIAL 0x58
#define IT_JOYPAD 0x60

// TODO LMA CGB Compatibility mode init value
//
void Cpu::reset()
{
	PC = 0;
	IME = false;
	setIMEFlag = false;
	halted = false;
	halt_counter = 0;
}
void Cpu::loadBootRom()
{
	PC = 0;
	IME = false;
	setIMEFlag = false;
	halted = false;
	halt_counter = 0;
	return;
	PC = 0x100;
	mem.supervisorWrite(LCDC_STATUS, 0x85);
	mem.supervisorWrite(HDMA1, 0xFF);
	mem.supervisorWrite(HDMA2, 0xFF);
	mem.supervisorWrite(HDMA3, 0xFF);
	mem.supervisorWrite(HDMA4, 0xFF);
	mem.supervisorWrite(HDMA5, 0xFF);
	M_LCDC = 0x91;
	//M_LY = 0x00;
	//M_LCDC = 0x80;
	
	if (!Gameboy::bIsCGB)
	{
		A = 0x01;
		F = 0;
		B = 0x00;
		C = 0x13;
		D = 0;
		E = 0xD8;
		H = 0x01;
		L = 0x4D;
	}
	else if (Gameboy::bCGBIsInCompatMode)
	{
		A = 0x11;
		F = 0;
		B = 0x43; // TODO may change
		C = 0;
		setZeroFlag(1);
		D = 0x00;
		E = 0x08;
		H = 0x99; // TODO may change
		L = 0x1A; // TODO may change
		// TODO Fill palette DEBUG PALETTES
	    if (Gameboy::bIsCGB && Gameboy::bCGBIsInCompatMode) {
			const CGBMem& asCGB = dynamic_cast<const CGBMem&>(mem);
			
			asCGB.BGPalettes[0] = 0xFF;
			asCGB.BGPalettes[1] = 0xFF;
			asCGB.BGPalettes[2] = 0xAA;
			asCGB.BGPalettes[3] = 0xAA;
			asCGB.BGPalettes[4] = 0x55;
			asCGB.BGPalettes[5] = 0x55;
			asCGB.BGPalettes[6] = 0x00;
			asCGB.BGPalettes[7] = 0x00;
			
			asCGB.OBJPalettes[0] = 0xFF;
			asCGB.OBJPalettes[1] = 0xFF;
			asCGB.OBJPalettes[2] = 0x24;
			asCGB.OBJPalettes[3] = 0x24;
			asCGB.OBJPalettes[4] = 0x42;
			asCGB.OBJPalettes[5] = 0x42;
			asCGB.OBJPalettes[6] = 0x00;
			asCGB.OBJPalettes[7] = 0x00;
			
			asCGB.OBJPalettes[8+0] = 0xFF;
			asCGB.OBJPalettes[8+1] = 0xFF;
			asCGB.OBJPalettes[8+2] = 0x35;
			asCGB.OBJPalettes[8+3] = 0x35;
			asCGB.OBJPalettes[8+4] = 0x53;
			asCGB.OBJPalettes[8+5] = 0x53;
			asCGB.OBJPalettes[8+6] = 0x00;
			asCGB.OBJPalettes[8+7] = 0x00;
			asCGB.bIsUsingCGBVram = 0;
	    }
	}
	else
	{
		A = 0x11;
		F = 0;
		BC = 0;
		setZeroFlag(1);
		D = 0xFF;
		E = 0x56;
		H = 0;
		L = 0x0D;
	}

	stackTrace.PCBreak = 0x021D;
	stackTrace.breakActive = false;
	IME = false;
	setIMEFlag = false;
	halted = false;
	halt_counter = 0;
	//stackTrace.opcodeBreak = 0xCB27;
}

void	Cpu::requestInterrupt(int i)
{
	unsigned char bit;
	switch (i)
	{
		case IT_VBLANK:
			bit = 0;
			break;
		case IT_LCD_STAT:
			bit = 1;
			break;
		case IT_TIMER:
			bit = 2;
			break;
		case IT_SERIAL:
			bit = 3;
			break;
		case IT_JOYPAD:
			bit = 4;
			break;

		default:
			std::cerr << "Incorrect request ask\n";
			exit(2);
			break;
	}
	SET(M_IF, bit);
}

bool  Cpu::isCpuHalted(void)
{
	if (Cpu::halted)
	{
		// TODO we should exit halt even when IME is not set but the behavior is different
		if (M_EI & M_IF & 0x1F)
		{
			Cpu::halted = false;
			Cpu::halt_counter = 0;
			return false;
		}
		else if (Cpu::halt_counter != 0)
		{
			Cpu::halt_counter++;
			if (Cpu::halt_counter > 2050)
			{
				Cpu::halted = false;
				Cpu::halt_counter = 0;
				return false;
			}
		}
		else {
		    return true;
		}
	}
	return false;
}

std::pair<unsigned char, int> Cpu::executeInstruction()
{
	// std::cout << "Execute instruction : " << std::hex << Cpu::PC << "\n";
	unsigned char opcode = 0;
	int clock = 0;
	std::function<unsigned char()> instruction = [](){stackTrace.print(); return 0;};

	// debug(readByte(false));
	opcode = readByte();
	if (Cpu::setIMEFlag && opcode != 0xf3) {
	    Cpu::IME = true;
	}
	Cpu::setIMEFlag = false;
	bool bIsPHL = ((opcode & 0x0F) == 0x06) | ((opcode & 0x0F) == 0x0E);
	switch (opcode)
	{
		case 0x00:
			instruction = [&](){ return nop();};
			break;
		case 0x10:
			instruction = [&](){ return stop();};
			break;
		case 0x27:
			instruction = [&](){ return daa();};
			break;
		case 0x2F:
			instruction = [&](){ return cpl();};
			break;
		case 0x37:
			instruction = [&](){ return scf();};
			break;
		case 0x3F:
			instruction = [&](){ return ccf();};
			break;
		case 0x76:
			instruction = [&](){ return halt();};
			break;
		case 0xF3:
			instruction = [&](){ return di();};
			break;
		case 0xFB:
			instruction = [&](){ return ei();};
			break;

		case 0x01:
		case 0x11:
		case 0x21:
		case 0x31:
			instruction = [&](){ return load_r16_from_d16(opcode);};
			break;
		case 0x02:
		case 0x12:
		case 0x22:
		case 0x32:
			instruction = [&](){ return load_r16_a(opcode);};
			break;
		case 0x03:
		case 0x13:
		case 0x23:
		case 0x33:
			instruction = [&](){ return inc_r16(opcode);};
			break;
		case 0x04:
		case 0x0C:
		case 0x14:
		case 0x1C:
		case 0x24:
		case 0x2C:
		case 0x3C:
		case 0x34:
			instruction = [&](){ return inc_r8(opcode);};
			break;
		case 0x05:
		case 0x0D:
		case 0x15:
		case 0x1D:
		case 0x25:
		case 0x2D:
		case 0x3D:
		case 0x35:
			instruction = [&](){ return dec_r8(opcode);};
			break;
		case 0x06:
		case 0x16:
		case 0x26:
		case 0x0E:
		case 0x1E:
		case 0x2E:
		case 0x3E:
			instruction = [&](){ return load_r_d8(getTargetRegister(opcode));};
			break;
		case 0x36:
			instruction = [&](){ return load_hl_d8(); };
			break;
		case 0x07:
		case 0x0F:
			instruction = [&](){ return rca(opcode);};
			break;
		case 0x08:
			instruction = [&](){ return load_sp_to_a16();};
			break;
		case 0x09:
		case 0x19:
		case 0x29:
		case 0x39:
			instruction = [&](){ return add_hl_r16(opcode);};
			break;
		case 0x0A:
		case 0x1A:
		case 0x2A:
		case 0x3A:
			instruction = [&](){ return load_a_r16(opcode);};
			break;
		case 0x0B:
		case 0x1B:
		case 0x2B:
		case 0x3B:
			instruction = [&](){ return dec_r16(opcode);};
			break;
		case 0x17:
		case 0x1F:
			instruction = [&](){ return ra(opcode);};
			break;
		case 0x18:
			instruction = [&](){ return jr_s8();};
			break;
		case 0x20:
		case 0x28:
		case 0x30:
		case 0x38:
			instruction = [&](){ return jr_s8_flag(opcode);};
			break;
		case 0x40 ... 0x45:
		case 0x47 ... 0x4D:
		case 0x4F ... 0x55:
		case 0x57 ... 0x5D:
		case 0x5F ... 0x65:
		case 0x67 ... 0x6D:
		case 0x6F:
		case 0x78 ... 0x7D:
		case 0x7F:
			instruction = [&](){ return load_r_r(getTargetRegister(opcode), getSourceRegister(opcode));};
			break;
		case 0x46:
		case 0x56:
		case 0x66:
		case 0x4E:
		case 0x5E:
		case 0x6E:
		case 0x7E:
			instruction = [&](){ return load_r_hl(getTargetRegister(opcode));};
			break;
		case 0x70 ... 0x75:
		case 0x77:
			instruction = [&](){ return load_hl_r(getSourceRegister(opcode));};
			break;
		case 0x80 ... 0x87:
			if (bIsPHL)
				instruction = [&](){ return add_a_phl();};
			else
				instruction = [&](){ return add_a_r8(getSourceRegister(opcode));};
			break;
		case 0x88 ... 0x8F:
			if (bIsPHL)
				instruction = [&](){ return adc_a_phl();};
			else
				instruction = [&](){ return adc_a_r8(getSourceRegister(opcode));};
			break;
		case 0x90 ... 0x97:
			if (bIsPHL)
				instruction = [&](){ return sub_phl();};
			else
				instruction = [&](){ return sub_r8(getSourceRegister(opcode));};
			break;
		case 0x98 ... 0x9F:
			if (bIsPHL)
				instruction = [&](){ return sbc_phl();};
			else
				instruction = [&](){ return sbc_r8(getSourceRegister(opcode));};
			break;
		case 0xA0 ... 0xA7:
			if (bIsPHL)
				instruction = [&](){ return and_phl();};
			else
				instruction = [&](){ return and_r8(getSourceRegister(opcode));};
			break;
		case 0xA8 ... 0xAF:
			if (bIsPHL)
				instruction = [&](){ return xor_phl();};
			else
				instruction = [&](){ return xor_r8(getSourceRegister(opcode));};
			break;
		case 0xB0 ... 0xB7:
			if (bIsPHL)
				instruction = [&](){ return or_phl();};
			else
				instruction = [&](){ return or_r8(getSourceRegister(opcode));};
			break;
		case 0xB8 ... 0xBF:
			if (bIsPHL)
				instruction = [&](){ return cp_phl();};
			else
				instruction = [&](){ return cp_r8(getSourceRegister(opcode));};
			break;
		case 0xC0:
		case 0xC8:
		case 0xD0:
		case 0xD8:
			instruction = [&](){ return ret_flag(opcode);};
			break;
		case 0xC1:
		case 0xD1:
		case 0xE1:
		case 0xF1:
			instruction = [&](){ return pop(opcode);};
			break;
		case 0xC2:
		case 0xCA:
		case 0xD2:
		case 0xDA:
			instruction = [&](){ return jp_flag_a16(opcode);};
			break;
		case 0xC3:
			instruction = [&](){ return jp_a16();};
			break;
		case 0xC4:
		case 0xCC:
		case 0xD4:
		case 0xDC:
			instruction = [&](){ return call_flag_a16(opcode);};
			break;
		case 0xC5:
		case 0xD5:
		case 0xE5:
		case 0xF5:
			instruction = [&](){ return push(opcode);};
			break;
		case 0xC6:
			instruction = [&](){ return add_a_d8();};
			break;
		case 0xC7:
		case 0xD7:
		case 0xE7:
		case 0xF7:
		case 0xCF:
		case 0xDF:
		case 0xEF:
		case 0xFF:
			instruction = [&](){ return rst_n(opcode);};
			break;
		case 0xC9:
			instruction = [&](){ return ret();};
			break;
		case 0xCD:
			instruction = [&](){ return call_a16();};
			break;
		case 0xCE:
			instruction = [&](){ return adc_a_d8();};
			break;
		case 0xD6:
			instruction = [&](){ return sub_d8();};
			break;
		case 0xD9:
			instruction = [&](){ return reti();};
			break;
		case 0xDE:
			instruction = [&](){ return sbc_d8();};
			break;
		case 0xE0:
			instruction = [&](){ return load_a8_a();};
			break;
		case 0xE2:
			instruction = [&](){ return load_c_a();};
			break;
		case 0xE6:
			instruction = [&](){ return and_d8();};
			break;
		case 0xE8:
			instruction = [&](){ return add_sp_s8();};
			break;
		case 0xE9:
			instruction = [&](){ return jp_hl();};
			break;
		case 0xEA:
			instruction = [&](){ return load_a16_a();};
			break;
		case 0xEE:
			instruction = [&](){ return xor_d8();};
			break;
		case 0xF0:
			instruction = [&](){ return load_a_a8();};
			break;
		case 0xF2:
			instruction = [&](){ return load_a_c();};
			break;
		case 0xF6:
			instruction = [&](){ return or_d8();};
			break;
		case 0xF8:
			instruction = [&](){ return load_hl_from_sp_plus_s8();};
			break;
		case 0xF9:
			instruction = [&](){ return load_sp_from_hl();};
			break;
		case 0xFA:
			instruction = [&](){ return load_a_a16();};
			break;
		case 0xFE:
			instruction = [&](){ return cp_d8();};
			break;

		case 0xCB:
			{
				opcode = readByte();

				bIsPHL = ((opcode & 0x0F) == 0x06) | ((opcode & 0x0F) == 0x0E);
				unsigned char targetBit = getTargetBit(opcode);
				switch (opcode) {
					case 0x00 ... 0x07:
						if (bIsPHL)
							instruction = [&](){ return rlc_phl();};
						else
							instruction = [&](){ return rlc_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x08 ... 0x0F:
						if (bIsPHL)
							instruction = [&](){ return rrc_phl();};
						else
							instruction = [&](){ return rrc_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x10 ... 0x17:
						if (bIsPHL)
							instruction = [&](){ return rl_phl();};
						else
							instruction = [&](){ return rl_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x18 ... 0x1F:
						if (bIsPHL)
							instruction = [&](){ return rr_phl();};
						else
							instruction = [&](){ return rr_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x20 ... 0x27:
						if (bIsPHL)
							instruction = [&](){ return sla_phl();};
						else
							instruction = [&](){ return sla_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x28 ... 0x2F:
						if (bIsPHL)
							instruction = [&](){ return sra_phl();};
						else
							instruction = [&](){ return sra_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x30 ... 0x37:
						if (bIsPHL)
							instruction = [&](){ return swap_phl();};
						else
							instruction = [&](){ return swap_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x38 ... 0x3F:
						if (bIsPHL)
							instruction = [&](){ return srl_phl();};
						else
							instruction = [&](){ return srl_r8(getSourceRegisterRef(opcode));};
						break;
					case 0x40 ... 0x7F:
						if (bIsPHL)
							instruction = [&](){ return bit_n_phl(targetBit);};
						else
							instruction = [&](){ return bit_n_r8(targetBit, getSourceRegisterRef(opcode));};
						break;
					case 0x80 ... 0xBF:
						if (bIsPHL)
							instruction = [&](){ return res_n_phl(targetBit);};
						else
							instruction = [&](){ return res_n_r8(targetBit, getSourceRegisterRef(opcode));};
						break;
					case 0xC0 ... 0xFF:
						if (bIsPHL)
							instruction = [&](){ return set_n_phl(targetBit);};
						else
							instruction = [&](){ return set_n_r8(targetBit, getSourceRegisterRef(opcode));};
						break;
					default:
						stackTrace.print();
						logErr(string_format("Unkown opcode encountered: 0xCB%X, a gameboy would either crash/freeze", opcode));
				}
				// NOTE need to exit here because of targetBit/Register references
				// Cpu::debug(opcode);
				if (!UserInterface::bIsError) {
					clock = instruction();
				}
				return std::pair<unsigned char, int>((int)opcode, clock);
			}
			break;
		default:
			{
				std::cerr << "previuous opcode: 0x" << std::hex << ((int)mem[PC - 2]) << std::endl;
				stackTrace.print();
				logErr(string_format("Unkown opcode encountered: 0x%X, a gameboy would either crash/freeze", opcode));
			}
	}
	// Cpu::debug(opcode);
	if (!UserInterface::bIsError) {
		clock = instruction();
	}
	return std::pair<unsigned char, int>((int)opcode, clock);
}

StackData	Cpu::captureCurrentState(std::string customData)
{
	return StackData();
	StackData stackData;

	stackData.PC = PC;
	stackData.SP = SP;
	stackData.AF = AF;
	stackData.BC = BC;
	stackData.DE = DE;
	stackData.HL = HL;
	stackData.opcode = mem[PC];
	stackData.ie_reg = M_EI;
	stackData.if_reg = M_IF;
	stackData.ly_reg = M_LY;
	stackData.lcdc = M_LCDC;
	stackData.lcdc_stat = M_LCDC_STATUS;
	stackData.ime = IME;
	if (mem[PC] == 0xCB)
	{
		stackData.opcode <<= 8;
		stackData.opcode|= mem[PC + 1];
	}
	//customData = string_format("SCX: %2X    mem[0xC49D]: %2X\nmem[0xC497]: %2X    mem[HL]: %2X\n", (int)mem[0xFF43], (int)mem[0xC49D], (int)mem[0xC497], (int)mem[HL]) + customData;
	stackData.customData = customData;
	return stackData;
}

void Cpu::debug(int opcode)
{
	static int count = 1;

	std::cout << std::dec << count++ << "\n";
	std::cout << std::hex << std::setw(2) << std::setfill('0') << opcode << ": ";
	std::cout << std::hex << std::setw(2) << std::setfill('0') << "PC = " << PC << "\tLY = " << (int)M_LY << "\t\tLCDC = " << (int)M_LCDC << "\tLCDCS = " << (int)M_LCDC_STATUS << "\n";
	printf("IF=%02x EI=%02x JOY=%02x SC=%02x\n", (uint8_t)M_IF, (uint8_t)M_EI, (uint8_t)mem[0xFF00], (uint8_t)mem[0xFF02]);
	std::cout << std::hex << "AF = " << std::setw(4) << std::setfill('0') << AF << "\tBC = " << std::setw(4) << std::setfill('0') << BC << "\tDE = " << std::setw(4) << std::setfill('0') << DE << "\tHL = " << std::setw(4) << std::setfill('0') << HL << "\n";
	std::cout << (getZeroFlag() ? "Z" : "-") << (getSubtractFlag() ? "N" : "-") << (getHalfCarryFlag() ? "H" : "-") << (getCarryFlag() ? "C" : "-") << "\n\n";
}

void 		Cpu::doInterrupt(unsigned int addr, unsigned char bit)
{
	mem[--Cpu::SP] = Cpu::PC >> 8; //internalpush
	mem[--Cpu::SP] = Cpu::PC & 0xFF;
	Cpu::PC = addr;
	RES(M_IF, bit);
	Cpu::IME = false;
	Cpu::setIMEFlag = false;// overkill
}

unsigned char	Cpu::doMinimumStep()
{
	// the cpu is halted during hdma
	int cycleForHdma = Hdma::update();
	if (!UserInterface::bIsError && cycleForHdma)
	{
		if (cycleForHdma == -1) // special case startup
		{
			if (Clock::cgbMode) {
				g_clock += 1;
				APU::tick(1);
			}
		}
		return 1;
	}
	if (!UserInterface::bIsError && isCpuHalted()) {
		stackTrace.add(captureCurrentState("IM HALTED"));
	    	/* Increment one cycle */
		return 1;
	}
	else if (!UserInterface::bIsError && handleInterrupt()) {
		// interrupts takes 5 cycles;
		return 5;
	}
	else if (!UserInterface::bIsError) {
		stackTrace.add(captureCurrentState());
		return executeInstruction().second;
	}
	else {
		return (0); // Errot state
	}
}

bool Cpu::handleInterrupt()
{
	if (Cpu::IME)
	{
		if (M_EI & M_IF & 0x1f)
		{
			if (BIT(M_EI, 0) && BIT(M_IF, 0))
			{
				stackTrace.add(captureCurrentState("INTERRUPT VBLANK"));
				doInterrupt(IT_VBLANK, 0);
				return true;
			}
			else if (BIT(M_EI, 1) && BIT(M_IF, 1))
			{
				stackTrace.add(captureCurrentState("INTERRUPT LCD_STAT"));
				doInterrupt(IT_LCD_STAT, 1);
				return true;
			}
			else if (BIT(M_EI, 2) && BIT(M_IF, 2))
			{
				stackTrace.add(captureCurrentState("INTERRUPT TIMER"));
				doInterrupt(IT_TIMER, 2);
				return true;
			}
			else if (BIT(M_EI, 3) && BIT(M_IF, 3))
			{
				stackTrace.add(captureCurrentState("INTERRUPT SERIAL"));
				doInterrupt(IT_SERIAL, 3);
				return true;
			}
			else if (BIT(M_EI, 4) && BIT(M_IF, 4))
			{
				stackTrace.add(captureCurrentState("INTERRUPT JOYPAD"));
				doInterrupt(IT_JOYPAD, 4);
				return true;
			}
		}
	}
	return false;
}
