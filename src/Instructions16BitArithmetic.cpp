/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Instructions16BitArithmetic.cpp                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:03:50 by nallani           #+#    #+#             */
/*   Updated: 2022/11/10 18:20:28 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"

unsigned char Cpu::dec_r16(unsigned short opcode)
{
	// Opcode: [0x0B, 0x1B, 0x2B, 0x3B]
	// Symbol: DECL
	// Operands: [(BC), (DE), (HL), (SP)]
	// Number of Bytes: 1
	// Number of Cycles: 2
	// Flags: - - - -
	// Description
	// Decrement the contents of register pair reg by 1.

	unsigned short* reg = nullptr;
	switch (opcode)
	{
		case 0x0B:
			reg = &BC;
			break;
		case 0x1B:
			reg = &DE;
			break;
		case 0x2B:
			reg = &HL;
			break;
		case 0x3B:
			reg = &SP;
			break;
		default:
			logErr("dec_r16 received wrong opcode");
	}

	*reg -= 1;

	return 2;
}

unsigned char Cpu::inc_r16(unsigned short opcode)
{
	// Opcode: [0x03, 0x13, 0x23, 0x33]
	// Symbol: INCL
	// Operands: [(BC), (DE), (HL), (SP)]
	// Number of Bytes: 1
	// Number of Cycles: 2
	// Flags: - - - -
	// Description
	// Increment the contents of register pair reg by 1.

	unsigned short* reg = nullptr;
   	switch (opcode)
	{
		case 0x03:
			reg = &BC;
			break;
		case 0x13:
			reg = &DE;
			break;
		case 0x23:
			reg = &HL;
			break;
		case 0x33:
			reg = &SP;
			break;
		default:
			logErr("inc_r16 received wrong opcode");
	}

	*reg += 1;

	return 2;
}

unsigned char Cpu::add_hl_r16(unsigned short opcode)
{
	// Opcode: [0x09, 0x19, 0x29, 0x39]
	// Symbol: ADDL
	// Operands: [(BC), (DE), (HL), (SP)]
	// Number of Bytes: 1
	// Number of Cycles: 2
	// Flags: - 0 16-bit 16-bit
	// Description :
	// Add the contents of register pair reg to the contents of register pair HL, and store the results in register pair HL.

	unsigned short* reg = nullptr;
	switch (opcode)
	{
		case 0x09:
			reg = &BC;
			break;
		case 0x19:
			reg = &DE;
			break;
		case 0x29:
			reg = &HL;
			break;
		case 0x39:
			reg = &SP;
			break;
		default:
			logErr("add_hl_r16 received wrong opcode");
	}

	setSubtractFlag(0);
	setHalfCarryFlag(getHalfCarry16Bit(*reg, HL));// TODO
	setCarryFlag((unsigned short)(*reg + HL) < *reg);
													  
	HL += *reg;
	return 2;
}

unsigned char Cpu::add_sp_s8()
{
	// Opcode: [0xE8]
	// Symbol: ADDL
	// Operands: [(SP, S8)]
	// Number of Bytes: 2
	// Number of Cycles: 4
	// Flags: 0 0 16-bit 16-bit
	// Description
	// Add the contents of the 8-bit signed (2's complement) immediate operand s8 and the stack pointer SP and store the results in SP.


	unsigned char byte = readByte();
	char signedByte = (char)byte;
	unsigned char SP_low = SP & 0xFF;

	// special case evaluating operation as two unsigned char
	setFlags(0, 0, getHalfCarry8Bit(byte, SP_low), overFlow(SP_low, byte));

	SP = SP + signedByte;
	return 4;
}
