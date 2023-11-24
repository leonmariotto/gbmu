/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CpuUtility.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/09 20:25:14 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:50:08 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"
#include <iomanip>
#include <iostream>
#include "Utility.hpp"
#include "Gameboy.hpp"

void	Cpu::printRegisters()
{
		std::cout << std::setfill('0') << "Current opcode: 0x";
		if (mem[PC] == 0xCB)
			std::cout << std::setw(4) << std::hex << +(((unsigned short)(mem[PC]) << 8) + mem[PC + 1]) << " with PC: 0x" << std::setw(4) << PC << std::endl;
		else
			std::cout << std::setw(2) << std::hex << +(int)(mem[PC]) << " with PC: 0x" << std::setw(4) << std::hex << PC << std::endl;

		std::cout << std::setfill('0') << std::uppercase;
		//std::cout << "mem[HL]: 0x" << std::setw(2) << (int)mem[HL] << std::endl;
		std::cout << "AF: 0x" << std::setw(2) << +A << std::setw(2) << +F << std::endl;
		std::cout << "BC: 0x" << std::setw(4) << +BC << std::endl;
		std::cout << "DE: 0x" << std::setw(4) << +DE << std::endl;
		std::cout << "HL: 0x" << std::setw(4) << +HL << std::endl;
		std::cout << "SP: 0x" << std::setw(4) << +SP << std::endl;
		//std::cout << "FF44: 0x" << std::setw(2) << +(memory[0xFF44]) << std::endl;
		//std::cout << "clock: " << std::dec << clock * 2 << std::endl;
		//std::cout << "0xff44: " << std::hex << +memory[0xFF44] << std::endl;
		std::cout << std::endl << std::endl;

}

unsigned char	Cpu::getTargetBit(unsigned short opcode)
{
    // opcode % 40 removes the offset for first operation that need this function (0x40-0x7f)
    // & 0xF0 >> 4 to get the higher bit, multiplied by 2 to get the result
    unsigned char nb = (((opcode % 0x40) & 0xF0) >> 4) * 2;
    if (((opcode & 0xF) - 0x7) > 0)
        nb++;
    return nb;
}

unsigned char&	Cpu::getTargetRegister(unsigned short opcode)
{
	if (opcode >= 0x40 && opcode <= 0x8f)
	{
		switch ((opcode - 0x40) / 0x8)
		{
			case 0x07:
				return A;
			case 0x00:
				return B;
			case 0x01:
				return C;
			case 0x02:
				return D;
			case 0x03:
				return E;
			case 0x04:
				return H;
			case 0x05:
				return L;
			//case 0x06:
				//return PHL;  no more PHL in target !!
			default:
				logErr("Opcode is too big, shouldn't be more than 0x7F");
		}
	}
	else if ((opcode & 0x0F) == 0x0E)
	{
		switch (opcode)
		{
			case 0x0E:
				return C;
			case 0x1E:
				return E;
			case 0x2E:
				return L;
			case 0x3E:
				return A;
			default:
				logErr("wrong opcode sent to getTargetRegister");
		}
	}
	else if ((opcode & 0x0F) == 0x06)
	{
		switch (opcode)
		{
			case 0x06:
				return B;
			case 0x16:
				return D;
			case 0x26:
				return H;
			//case 0x36:
				//return PHL; no more PHL in target !!
			default:
				logErr("unkown opcode sent to getTargetRegister");
		}
	}
	else {
		logErr(string_format("Opcode is too little, should at least 0x40 and is 0x%X", opcode));
	}
	return (A); // Error state, don't care about what is returned
}

unsigned char	Cpu::getSourceRegister(unsigned short opcode)
{
	switch (opcode & 0x07)
	{
		case 0x07:
			return A;
		case 0x00:
			return B;
		case 0x01:
			return C;
		case 0x02:
			return D;
		case 0x03:
			return E;
		case 0x04:
			return H;
		case 0x05:
			return L;
		default:
			logErr("called getSourceRegister with wrong opcode");
	}
	return (A);  // Error state, don't care about what is returned
}

unsigned char&	Cpu::getSourceRegisterRef(unsigned short opcode)
{
	switch (opcode & 0x07)
	{
		case 0x07:
			return A;
		case 0x00:
			return B;
		case 0x01:
			return C;
		case 0x02:
			return D;
		case 0x03:
			return E;
		case 0x04:
			return H;
		case 0x05:
			return L;
		default:
			logErr("called getSourceRegisterRef with wrong opcode");
	}
	return (A);  // Error state, don't care about what is returned
}

void	Cpu::setZeroFlag(bool value)
{
	if (value)
		F |= (1 << 7);
	else
		F &= ~(1 << 7);
}

void	Cpu::setSubtractFlag(bool value)
{
	if (value)
		F |= (1 << 6);
	else
		F &= ~(1 << 6);
}

void	Cpu::setHalfCarryFlag(bool value)
{
	if (value)
		F |= (1 << 5);
	else
		F &= ~(1 << 5);
}

void	Cpu::setCarryFlag(bool value)
{
	if (value)
		F |= (1 << 4);
	else
		F &= ~(1 << 4);
}

void	Cpu::setFlags(bool zero, bool sub, bool halfCarry, bool carry)
{
	setZeroFlag(zero);
	setSubtractFlag(sub);
	setHalfCarryFlag(halfCarry);
	setCarryFlag(carry);
}

bool	Cpu::getZeroFlag()
{
	return (F >> 7) & 1;
}

bool	Cpu::getSubtractFlag()
{
	return (F >> 6) & 1;
}

bool	Cpu::getHalfCarryFlag()
{
	return (F >> 5) & 1;
}

bool	Cpu::getCarryFlag()
{
	return (F >> 4) & 1;
}

unsigned char Cpu::readByte()
{
	unsigned char val = mem[PC++];
	return val;
}

unsigned short Cpu::readShort()
{
	unsigned short shortVal = readByte();
	shortVal += ((unsigned short)readByte() << 8);
	return shortVal;
}

void Cpu::logErr(std::string msg)
{
	// Throw a fatal error window
	Gameboy::throwError(msg.c_str());
}
