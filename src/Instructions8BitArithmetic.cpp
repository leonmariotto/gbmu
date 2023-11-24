/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Instructions8BitArithmetic.cpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:06:02 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:51:11 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"
#include "Gameboy.hpp"

unsigned char Cpu::inc_r8(unsigned short opcode)
{
	// Opcode: [0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x3C, 0x34]
	// Symbol: INC
	// Operands: [(B), (C), (D), (E), (H), (L), (A)]
	// Number of Bytes: 1
	// Number of Cycles: 1
	// Flags: Z 1 8-bit -
	// Description
	// Increment the contents of memory specified by register by 1.

	unsigned char* reg = nullptr;
	switch (opcode) {
		case 0x04:
			reg = &B;
			break;
		case 0x0C:
			reg = &C;
			break;
		case 0x14:
			reg = &D;
			break;
		case 0x1C:
			reg = &E;
			break;
		case 0x24:
			reg = &H;
			break;
		case 0x2C:
			reg = &L;
			break;
		case 0x3C:
			reg = &A;
			break;
		case 0x34:
			setSubtractFlag(false);
			setHalfCarryFlag(getHalfCarry8Bit(mem[HL], 1));

			mem[HL] = mem[HL] + 1;
			setZeroFlag(mem[HL] == 0);
			return 3;
		default:
			logErr("Error calling inc_r8 with wrong opcode");
	}

	setSubtractFlag(false);
	setHalfCarryFlag(getHalfCarry8Bit(*reg, 1));

	*reg += 1;
	setZeroFlag(*reg == 0);

	return 1;
}

unsigned char Cpu::dec_r8(unsigned short opcode)
{
    // Opcode: [0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D, 0x3D, 0x35]
    // Symbol: DEC
    // Operands: [(B), (C), (D), (E), (H), (L), (A)]
    // Number of Bytes: 1
    // Number of Cycles: 3
    // Flags: Z 1 8-bit -
    // Description
    // Decrement the contents of the register by 1.
    
    unsigned char *reg = nullptr;
	switch (opcode)
	{
		case 0x05:
			reg = &B;
			break;
		case 0x0D:
			reg = &C;
			break;
		case 0x15:
			reg = &D;
			break;
		case 0x1D:
			reg = &E;
			break;
		case 0x25:
			reg = &H;
			break;
		case 0x2D:
			reg = &L;
			break;
		case 0x3D:
			reg = &A;
			break;
		case 0x35:
			setSubtractFlag(true);
			setHalfCarryFlag(getHalfBorrow8Bit(mem[HL], 1));

			mem[HL] = mem[HL] - 1;
			setZeroFlag(mem[HL] == 0);
			return 3;
		default:
			reg = nullptr;
			logErr("Error calling dec_r8 with wrong opcode");
	};


	setSubtractFlag(true);
	setHalfCarryFlag(getHalfBorrow8Bit(*reg, 1));

	*reg -= 1;
	setZeroFlag(*reg == 0);

	return 1;
}

unsigned char Cpu::add_a_r8(unsigned char reg)
{
    // Opcode: [0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x87]
    // Symbol: ADD
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z 0 8-bit 8-bit
    // Description
    // Add the contents of register X to the contents of register A, and store the results in register A.

	setFlags((unsigned char)(A + reg) == 0, 0, getHalfCarry8Bit(A, reg), overFlow(A, reg));
	A += reg;
	return 1;
}

unsigned char Cpu::add_a_phl()
{
	add_a_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::adc_a_r8(unsigned char reg)
{
    // Opcode: [0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8F]
    // Symbol: ADC
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z 0 8-bit 8-bit
    // Description
    // Add the contents of register r8 and the CY flag to the contents of register A, and store the results in register A.

	unsigned char carryFlag = getCarryFlag();

	setFlags((unsigned char)(A + reg + carryFlag) == 0, 0, getHalfCarry8Bit(A, reg, carryFlag), overFlow(A, reg, carryFlag));

	A += reg + carryFlag;

	return 1;
}

unsigned char Cpu::adc_a_phl()
{
	adc_a_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::sub_r8(unsigned char reg)
{
    // Opcode: [0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x97]
    // Symbol: SUB
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z 1 8-bit 8-bit
    // Description
    // Subtract the contents of register B from the contents of register A, and store the results in register A.

	setFlags(A == reg, 1, getHalfBorrow8Bit(A, reg), underFlow(A, reg));
	A -= reg;
	return 1;
}

unsigned char Cpu::sub_phl()
{
	sub_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::sbc_r8(unsigned char reg)
{
    // Opcode: [0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9F]
    // Symbol: SBC
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z 1 8-bit 8-bit
    // Description
    // Subtract the contents of register and the CY flag from the contents of register A, and store the results in register A.

	unsigned char carryFlag = getCarryFlag();
	setFlags(A == (unsigned char)(reg + carryFlag), 1, getHalfBorrow8Bit(A, reg, carryFlag), underFlow(A, reg, carryFlag));
	A -= (reg + carryFlag);
	return 1;
}

unsigned char Cpu::sbc_phl()
{
	sbc_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::and_r8(unsigned char reg)
{
    // Opcode: [0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA7]
    // Symbol: AND
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z 0 1 0
    // Description
    // Take the logical AND for each bit of the contents of register and the contents of register A, and store the results in register A.

	A &= reg;
	setFlags(A == 0, 0, 1, 0);
	return 1;
}

unsigned char Cpu::and_phl()
{
	and_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::xor_r8(unsigned char reg)
{
    // Opcode: [0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAF, 0xAE]
    // Symbol: XOR
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z 0 0 0
    // Description
    // Take the logical exclusive-OR for each bit of the contents of register and the contents of register A, and store the results in register A.

	A ^= reg;
	setFlags(A == 0, 0, 0, 0);
	return 1;
}

unsigned char Cpu::xor_phl()
{
	xor_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::or_r8(unsigned char reg)
{
    // Opcode: [0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB7, 0xB6]
    // Symbol: OR
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z 0 0 0
    // Description
    // Take the logical OR for each bit of the contents of register and the contents of register A, and store the results in register A.

	A |= reg;
	setFlags(A == 0, 0, 0, 0);
	return 1;
}

unsigned char Cpu::or_phl()
{
	or_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::cp_r8(unsigned char reg)
{
    // Opcode: [0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBF, 0xBE]
    // Symbol: CP
    // Operands: [(A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 1 8-bit 8-bit
    // Description
    // Compare the contents of register r8 and the contents of register A by calculating A - r8, and set the Z flag if they are equal.
    // The execution of this instruction does not affect the contents of register A.

	setFlags(A == reg, 1, getHalfBorrow8Bit(A, reg), underFlow(A, reg));
	return 1;
}

unsigned char Cpu::cp_phl()
{
	cp_r8(mem[HL]);
	return 2;
}

unsigned char Cpu::add_a_d8()
{
    // Opcode: 0xC6
    // Symbol: ADD
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 0 8-bit 8-bit
    // Description
    // Add the contents of the 8-bit immediate operand d8 to the contents of register A, and store the results in register A.

	unsigned char d8 = readByte();
	setFlags((unsigned char)(A + d8) == 0, 0, getHalfCarry8Bit(A, d8), overFlow(A, d8));
	A += d8;
    return 2;
}

unsigned char Cpu::sub_d8()
{
    // Opcode: 0xD6
    // Symbol: SUB
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 1 8-bit 8-bit
    // Description
    // Subtract the contents of the 8-bit immediate operand d8 from the contents of register A, and store the results in register A.

	unsigned char d8 = readByte();
	setFlags(A == d8, 1, getHalfBorrow8Bit(A, d8), underFlow(A, d8));
	A -= d8;
    return 2;
}

unsigned char Cpu::adc_a_d8()
{
    // Opcode: 0xCE
    // Symbol: ADC
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 0 8-bit 8-bit
    // Description
    // Add the contents of the 8-bit immediate operand d8 and the CY flag to the contents of register A, and store the results in register A.

	unsigned char d8 = readByte();
	unsigned char carryFlag = getCarryFlag();
	setFlags((unsigned char)(A + d8 + carryFlag) == 0, 0, getHalfCarry8Bit(A, d8, carryFlag), overFlow(A, d8, carryFlag));
	A += d8 + carryFlag;
    
    return 2;
}

unsigned char Cpu::sbc_d8()
{
    // Opcode: 0xDE
    // Symbol: SBC
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 1 8-bit 8-bit
    // Description
    // Subtract the contents of the 8-bit immediate operand d8 and the carry flag CY from the contents of register A, and store the results in register A.

	unsigned char d8 = readByte();
	unsigned char carryFlag = getCarryFlag();

	setFlags(A == (unsigned char)(d8 + carryFlag), 1, getHalfBorrow8Bit(A, d8, carryFlag), underFlow(A, d8, carryFlag));

	A -= (unsigned char)(d8 + carryFlag);
    
    return 2;
}

unsigned char Cpu::and_d8()
{
    // Opcode: 0xE6
    // Symbol: AND
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 0 1 0
    // Description
    // Take the logical AND for each bit of the contents of 8-bit immediate operand d8 and the contents of register A, and store the results in register A.


	A &= readByte();
	setFlags(A == 0, 0, 1, 0);
    return 2;
}

unsigned char Cpu::xor_d8()
{
    // Opcode: 0xEE
    // Symbol: XOR
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 0 0 0
    // Description
    // Take the logical exclusive-OR for each bit of the contents of the 8-bit immediate operand d8 and the contents of register A, and store the results in register A.

	A ^= readByte();
	setFlags(A == 0, 0, 0 ,0);
    return 2;
}

unsigned char Cpu::or_d8()
{
    // Opcode: 0xF6
    // Symbol: OR
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 0 0 0
    // Description
    // Take the logical OR for each bit of the contents of the 8-bit immediate operand d8 and the contents of register A, and store the results in register A.

	A |= readByte();
	setFlags(A == 0, 0, 0, 0);
    return 2;
}

unsigned char Cpu::cp_d8()
{
    // Opcode: 0xFE
    // Symbol: CP
    // Operands: [(A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: Z 1 8-bit 8-bit
    // Description
    // Compare the contents of register A and the contents of the 8-bit immediate operand d8 by calculating A - d8, and set the Z flag if they are equal.
    // The execution of this instruction does not affect the contents of register A.

	unsigned char d8 = readByte();
	setFlags(A == d8, 1, getHalfBorrow8Bit(A, d8), underFlow(A, d8));
    return 2;
}
