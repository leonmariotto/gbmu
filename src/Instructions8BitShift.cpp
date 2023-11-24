/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Instructions8BitShift.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:06:47 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:51:22 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"
#include "Gameboy.hpp"

unsigned char Cpu::rca(unsigned short opcode)
{
    // Opcode: [0x07, 0x0F]
    // Symbol: RCA
    // Operands: [(A), (A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: 0 0 0 A7
    // Description

    // FOR 0x07
    // Rotate the contents of register A to the left.
    // That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy) are copied to bit 2.
    // The same operation is repeated in sequence for the rest of the register.
    // The contents of bit 7 are placed in both the CY flag and bit 0 of register A.

    // FOR 0x0F
    // Rotate the contents of register A to the right.
    // That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy) are copied to bit 5.
    // The same operation is repeated in sequence for the rest of the register.
    // The contents of bit 0 are placed in both the CY flag and bit 7 of register A.

    bool shiftedBitValue = false;
	if (opcode == 0x07)
	{
		shiftedBitValue = A >> 7;
		A = (A << 1) | shiftedBitValue;
	}
	else if (opcode == 0x0F)
	{
		shiftedBitValue = A & 1;
		A = (A >> 1) | ((unsigned char)shiftedBitValue << 7);
	}
	else
		logErr("called RCA with wrong opcode");
	setFlags(0, 0, 0, shiftedBitValue);
	return 1;
}

unsigned char Cpu::ra(unsigned short opcode)
{
	// Opcode: [0x17, 0x1F]
	// Symbol: RA
	// Operands: [(A), (A)]
	// Number of Bytes: 1
	// Number of Cycles: 1
	// Flags: 0 0 0 A7
	// Description

	// FOR 0x17
	// Rotate the contents of register A to the left, through the carry (CY) flag.
	// That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy) are copied to bit 2.
	// The same operation is repeated in sequence for the rest of the register. The previous contents of the carry flag are copied to bit 0.

	// FOR 0x1F
	// Rotate the contents of register A to the right, through the carry (CY) flag.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the register. The previous contents of the carry flag are copied to bit 7.

	bool shiftedBitValue = false;
	if (opcode == 0x17)
	{
		shiftedBitValue = A >> 7;
		A = (A << 1) | getCarryFlag();
	}
	else if (opcode == 0x1F)
	{
		shiftedBitValue = A & 1;
		A = (A >> 1) | ((unsigned char)getCarryFlag() << 7);
	}
	else
		logErr("called RA with wrong opcode");

	setZeroFlag(false);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(shiftedBitValue);
	return 1;
}

unsigned char Cpu::rlc_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB00, 0xCB01, 0xCB02, 0xCB03, 0xCB04, 0xCB05, 0xCB06, 0xCB07]
	// Symbol: RLC
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 r7
	// Description
	// Rotate the contents of register r to the left. That is, the contents of bit 0 are copied to bit 1, and
	// the previous contents of bit 1 (before the copy operation) are copied to bit 2.
	// The same operation is repeated in sequence for the rest of the register.
	// The contents of bit 7 are placed in both the CY flag and bit 0 of register r.

	// FOR 0xCB06 :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Rotate the contents of memory specified by register pair HL to the left.
	// That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are copied to bit 2.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The contents of bit 7 are placed in both the CY flag and bit 0 of (HL).
	// let old_carry_flag: u8 = cpu.get_carry_flag() as u8;
	bool bitSevenValue = targetRegister >> 7;

	setZeroFlag(targetRegister << 1 == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(bitSevenValue);
	targetRegister = (targetRegister << 1) | bitSevenValue;
	return 2;
}

unsigned char Cpu::rrc_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB08, 0xCB09, 0xCB0A, 0xCB0B, 0xCB0C, 0xCB0D, 0xCB0E, 0xCB0F]
	// Symbol: RRC
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 r0
	// Description
	// Rotate the contents of register r to the right. That is, the contents of bit 7 are copied to bit 6, and
	// the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the register.
	// The contents of bit 0 are placed in both the CY flag and bit 7 of register r.

	// FOR 0xCB0E :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Rotate the contents of memory specified by register pair HL to the right.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The contents of bit 0 are placed in both the CY flag and bit 7 of (HL).

	bool bitZeroValue = targetRegister & 1;

	targetRegister = (targetRegister >> 1) | ((unsigned char) bitZeroValue << 7);

	setFlags(targetRegister == 0, 0, 0, bitZeroValue);


	return 2;
}

unsigned char Cpu::rl_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB10, 0xCB11, 0xCB12, 0xCB13, 0xCB14, 0xCB15, 0xCB16, 0xCB17]
	// Symbol: RL
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 r7
	// Description
	// Rotate the contents of register r to the left. That is, the contents of bit 0 are copied to bit 1, and
	// the previous contents of bit 1 (before the copy operation) are copied to bit 2.
	// The same operation is repeated in sequence for the rest of the register.
	// The previous contents of the carry (CY) flag are copied to bit 0 of register r.

	// FOR 0xCB16 :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Rotate the contents of memory specified by register pair HL to the left, through the carry flag.
	// That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are copied to bit 2.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The previous contents of the CY flag are copied into bit 0 of (HL).

	bool oldCarryFlag = getCarryFlag();
	unsigned char lostBit = targetRegister & (1 << 7);


	targetRegister = (targetRegister << 1) | oldCarryFlag;
	setFlags(targetRegister == 0, 0, 0, lostBit);

	return 2;
}

unsigned char Cpu::rr_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB18, 0xCB19, 0xCB1A, 0xCB1B, 0xCB1C, 0xCB1D, 0xCB1E, 0xCB1F]
	// Symbol: RR
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 r0
	// Description
	// Rotate the contents of register r to the right. That is, the contents of bit 7 are copied to bit 6, and
	// the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the register.
	// The previous contents of the carry (CY) flag are copied to bit 7 of register r.

	// FOR 0xCB1E :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Rotate the contents of memory specified by register pair HL to the right, through the carry flag.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The previous contents of the CY flag are copied into bit 7 of (HL).

	bool oldCarryFlag = getCarryFlag();
	bool lostBit = targetRegister & 1;

	targetRegister = (targetRegister >> 1) | (((unsigned char)oldCarryFlag) << 7);
	setFlags(targetRegister == 0, 0, 0, lostBit);

	return 2;
}

unsigned char Cpu::sla_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB20, 0xCB21, 0xCB22, 0xCB23, 0xCB24, 0xCB25, 0xCB26, 0xCB27]
	// Symbol: SLA
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 r7
	// Description
	// Shift the contents of register B to the left.
	//  That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are copied to bit 2.
	//  The same operation is repeated in sequence for the rest of the register.
	//  The contents of bit 7 are copied to the CY flag, and bit 0 of register B is reset to 0.

	// FOR 0xCB26 :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Shift the contents of memory specified by register pair HL to the left.
	// That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are copied to bit 2.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The contents of bit 7 are copied to the CY flag, and bit 0 of (HL) is reset to 0.

	bool lostBit = targetRegister & (1 << 7);

	targetRegister <<= 1;
	setFlags(targetRegister == 0, 0, 0, lostBit);

	return 2;
}

unsigned char Cpu::sra_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB28, 0xCB29, 0xCB2A, 0xCB2B, 0xCB2C, 0xCB2D, 0xCB2E, 0xCB2F]
	// Symbol: SRA
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 r0
	// Description
	// Shift the contents of register B to the right.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the register.
	// The contents of bit 0 are copied to the CY flag, and bit 7 of register B is unchanged.

	// FOR 0xCB2E :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Shift the contents of memory specified by register pair HL to the right.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The contents of bit 0 are copied to the CY flag, and bit 7 of (HL) is unchanged.


	setZeroFlag(targetRegister >> 1 == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(targetRegister & 1);

	targetRegister = (targetRegister >> 1) + (targetRegister & (1 << 7));

	return 2;
}

unsigned char Cpu::swap_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB30, 0xCB31, 0xCB32, 0xCB33, 0xCB34, 0xCB35, 0xCB36, 0xCB37]
	// Symbol: SWAP
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 0
	// Description
	// Shift the contents of the lower-order four bits (0-3) of register B to the higher-order four bits (4-7) of the register,
	// and shift the higher-order four bits to the lower-order four bits.

	// FOR 0xCB36 :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Shift the contents of the lower-order four bits (0-3) of the contents of memory specified by register pair HL
	// to the higher-order four bits (4-7) of that memory location, and shift the contents of the higher-order four bits to the lower-order four bits.
	unsigned char low = (targetRegister & 0x0F) << 4;
	unsigned char high = (targetRegister & 0xF0) >> 4;

	targetRegister = low | high;

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(false);
	setZeroFlag(targetRegister == 0);

	return 2;
}

unsigned char Cpu::srl_r8(unsigned char& targetRegister)
{
	// Opcode: [0xCB38, 0xCB39, 0xCB3A, 0xCB3B, 0xCB3C, 0xCB3D, 0xCB3E, 0xCB3F]
	// Symbol: SRL
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 B0
	// Description
	// Shift the contents of register B to the right.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the register.
	// The contents of bit 0 are copied to the CY flag, and bit 7 of register B is reset to 0.

	// FOR 0xCB3E :
	// Shift the contents of memory specified by register pair HL to the right.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The contents of bit 0 are copied to the CY flag, and bit 7 of (HL) is reset to 0.

	setZeroFlag((targetRegister >> 1) == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(targetRegister & 1);

	targetRegister >>= 1;
	return 2;
}

unsigned char Cpu::bit_n_r8(unsigned char targetBit, unsigned char& targetRegister)
{
	// Opcode: [0xCB40, 0xCB41, 0xCB42, 0xCB43, 0xCB44, 0xCB45, 0xCB46, 0xCB47, 0xCB48, 0xCB49, 0xCB4A, 0xCB4B,
	//          0xCB4C, 0xCB4D, 0xCB4F, 0xCB4E, 0xCB50, 0xCB51, 0xCB52, 0xCB53, 0xCB54, 0xCB55, 0xCB56, 0xCB57, 0xCB58,
	//          0xCB59, 0xCB5A, 0xCB5B, 0xCB5C, 0xCB5D, 0xCB5E, 0xCB5F, 0xCB60, 0xCB61, 0xCB62, 0xCB63, 0xCB64,
	//          0xCB65, 0xCB66, 0xCB67, 0xCB68, 0xCB69, 0xCB6A, 0xCB6B, 0xCB6C, 0xCB6D, 0xCB6E, 0xCB6F, 0xCB70, 0xCB71,
	//          0xCB72, 0xCB73, 0xCB74, 0xCB75, 0xCB76, 0xCB77, 0xCB78, 0xCB79, 0xCB7A, 0xCB7B, 0xCB7C, 0xCB7D, 0xCB7E, 0xCB7F]
	// Symbol: BIT
	// Operands: [(0, B), (0, C), (0, D), (0, E), (0, H), (0, L), (0, HL), (0, A), (1, B), (1, C), (1, D), (1, E), (1, H), (1, L), (1, HL), (1, A),
	//            (2, B), (2, C), (2, D), (2, E), (2, H), (2, L), (2, HL), (2, A), (3, B), (3, C), (3, D), (3, E), (3, H), (3, L), (3, HL), (3, A),
	//            (4, B), (4, C), (4, D), (4, E), (4, H), (4, L), (4, HL), (4, A), (5, B), (5, C), (5, D), (5, E), (5, H), (5, L), (5, HL), (5, A),
	//            (6, B), (6, C), (6, D), (6, E), (6, H), (6, L), (6, HL), (6, A), (7, B), (7, C), (7, D), (7, E), (7, H), (7, L), (7, HL), (7, A)]
	// Number of Bytes: 2
	// Number of Cycles: 2 / 3
	// Flags: !r(n) 0 1 -
	// Description
	// Copy the complement of the contents of bit n in register r to the Z flag of the program status word (PSW).

	bool complement = (targetRegister & (1 << targetBit));
	complement = !complement;
	setZeroFlag(complement);
	setSubtractFlag(false);
	setHalfCarryFlag(true);

	return 2;
}

unsigned char Cpu::res_n_r8(unsigned char targetBit, unsigned char& targetRegister)
{
	// Opcode: [0xCB80, 0xCB81, 0xCB82, 0xCB83, 0xCB84, 0xCB85, 0xCB86, 0xCB87, 0xCB88, 0xCB89, 0xCB8A, 0xCB8B,
	//          0xCB8C, 0xCB8D, 0xCB8E, 0xCB8F, 0xCB90, 0xCB91, 0xCB92, 0xCB93, 0xCB94, 0xCB95, 0xCB96, 0xCB97, 0xCB98,
	//          0xCB99, 0xCB9A, 0xCB9B, 0xCB9C, 0xCB9D, 0xCB9E, 0xCB9F, 0xCBA0, 0xCBA1, 0xCBA2, 0xCBA3, 0xCBA4,
	//          0xCBA5, 0xCBA6, 0xCBA7, 0xCBA8, 0xCBA9, 0xCBAA, 0xCBAB, 0xCBAC, 0xCBAD, 0xCBAE, 0xCBAF, 0xCBB0, 0xCBB1,
	//          0xCBB2, 0xCBB3, 0xCBB4, 0xCBB5, 0xCBB6, 0xCBB7, 0xCBB8, 0xCBB9, 0xCBBA, 0xCBBB, 0xCBBC, 0xCBBD, 0xCBBE, 0xCBBF]
	// Symbol: RES
	// Operands: [(0, B), (0, C), (0, D), (0, E), (0, H), (0, L), (0, HL), (0, A), (1, B), (1, C), (1, D), (1, E), (1, H), (1, L), (1, HL), (1, A),
	//            (2, B), (2, C), (2, D), (2, E), (2, H), (2, L), (2, HL), (2, A), (3, B), (3, C), (3, D), (3, E), (3, H), (3, L), (3, HL), (3, A),
	//            (4, B), (4, C), (4, D), (4, E), (4, H), (4, L), (4, HL), (4, A), (5, B), (5, C), (5, D), (5, E), (5, H), (5, L), (5, HL), (5, A),
	//            (6, B), (6, C), (6, D), (6, E), (6, H), (6, L), (6, HL), (6, A), (7, B), (7, C), (7, D), (7, E), (7, H), (7, L), (7, HL), (7, A)]
	// Number of Bytes: 2
	// Number of Cycles: 2 / 3
	// Flags: - - - -
	// Description
	// Reset bit n in register r to 0.

	targetRegister = (targetRegister & ~(1 << targetBit));

	return 2;
}

unsigned char Cpu::set_n_r8(unsigned char targetBit, unsigned char& targetRegister)
{
	// Opcode: [0xCBC0, 0xCBC1, 0xCBC2, 0xCBC3, 0xCBC4, 0xCBC5, 0xCBC6, 0xCBC7, 0xCBC8, 0xCBC9, 0xCBCA, 0xCBCB,
	//          0xCBCC, 0xCBCD, 0xCBCE, 0xCBCF, 0xCBD0, 0xCBD1, 0xCBD2, 0xCBD3, 0xCBD4, 0xCBD5, 0xCBD6, 0xCBD7, 0xCBD8,
	//          0xCBD9, 0xCBDA, 0xCBDB, 0xCBDC, 0xCBDD, 0xCBDE, 0xCBDF, 0xCBE0, 0xCBE1, 0xCBE2, 0xCBE3, 0xCBE4,
	//          0xCBE5, 0xCBE6, 0xCBE7, 0xCBE8, 0xCBE9, 0xCBEA, 0xCBEB, 0xCBEC, 0xCBED, 0xCBEE, 0xCBEF, 0xCBF0, 0xCBF1,
	//          0xCBF2, 0xCBF3, 0xCBF4, 0xCBF5, 0xCBF6, 0xCBF7, 0xCBF8, 0xCBF9, 0xCBFA, 0xCBFB, 0xCBFC, 0xCBFD, 0xCBFE, 0xCBFF]
	// Symbol: SET
	// Operands: [(0, B), (0, C), (0, D), (0, E), (0, H), (0, L), (0, HL), (0, A), (1, B), (1, C), (1, D), (1, E), (1, H), (1, L), (1, HL), (1, A),
	//            (2, B), (2, C), (2, D), (2, E), (2, H), (2, L), (2, HL), (2, A), (3, B), (3, C), (3, D), (3, E), (3, H), (3, L), (3, HL), (3, A),
	//            (4, B), (4, C), (4, D), (4, E), (4, H), (4, L), (4, HL), (4, A), (5, B), (5, C), (5, D), (5, E), (5, H), (5, L), (5, HL), (5, A),
	//            (6, B), (6, C), (6, D), (6, E), (6, H), (6, L), (6, HL), (6, A), (7, B), (7, C), (7, D), (7, E), (7, H), (7, L), (7, HL), (7, A)]
	// Number of Bytes: 2
	// Number of Cycles: 2 / 3
	// Flags: - - - -
	// Description
	// Set bit n in register r to 1.

	targetRegister = (targetRegister | (1 << targetBit));

	return 2;
}

unsigned char Cpu::rlc_phl()
{
	bool bitSevenValue = mem[HL] >> 7;

	setZeroFlag(mem[HL] << 1 == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(bitSevenValue);
	mem[HL] = (mem[HL] << 1) | bitSevenValue;
	return 4;
}

unsigned char Cpu::rrc_phl()
{
	// Opcode: [0xCB08, 0xCB09, 0xCB0A, 0xCB0B, 0xCB0C, 0xCB0D, 0xCB0E, 0xCB0F]
	// Symbol: RRC
	// Operands: [(B), (C), (D), (E), (H), (L), (HL), (A)]
	// Number of Bytes: 2
	// Number of Cycles: 2
	// Flags: Z 0 0 r0
	// Description
	// Rotate the contents of register r to the right. That is, the contents of bit 7 are copied to bit 6, and
	// the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the register.
	// The contents of bit 0 are placed in both the CY flag and bit 7 of register r.

	// FOR 0xCB0E :
	// Number of bytes: 2
	// Number of Cycles : 4
	// Rotate the contents of memory specified by register pair HL to the right.
	// That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5.
	// The same operation is repeated in sequence for the rest of the memory location.
	// The contents of bit 0 are placed in both the CY flag and bit 7 of (HL).

	bool bitZeroValue = mem[HL] & 1;

	mem[HL] = (mem[HL] >> 1) | ((unsigned char) bitZeroValue << 7);

	setFlags(mem[HL] == 0, 0, 0, bitZeroValue);


	return 4;
}

unsigned char Cpu::rl_phl()
{
	bool oldCarryFlag = getCarryFlag();
	unsigned char lostBit = mem[HL] & (1 << 7);


	mem[HL] = (mem[HL] << 1) | oldCarryFlag;
	setFlags(mem[HL] == 0, 0, 0, lostBit);

	return 4;
}

unsigned char Cpu::rr_phl()
{
	bool oldCarryFlag = getCarryFlag();
	bool lostBit = mem[HL] & 1;

	mem[HL] = (mem[HL] >> 1) | (((unsigned char)oldCarryFlag) << 7);
	setFlags(mem[HL] == 0, 0, 0, lostBit);

	return 4;
}

unsigned char Cpu::sla_phl()
{
	bool lostBit = mem[HL] & (1 << 7);

	mem[HL] <<= 1;
	setFlags(mem[HL] == 0, 0, 0, lostBit);

	return 4;
}

unsigned char Cpu::sra_phl()
{
	setZeroFlag(mem[HL] >> 1 == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(mem[HL] & 1);

	mem[HL] = (mem[HL] >> 1) + (mem[HL] & (1 << 7));

	return 4;
}

unsigned char Cpu::swap_phl()
{
	unsigned char low = (mem[HL] & 0x0F) << 4;
	unsigned char high = (mem[HL] & 0xF0) >> 4;

	mem[HL] = low | high;

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(false);
	setZeroFlag(mem[HL] == 0);

	return 4;
}

unsigned char Cpu::srl_phl()
{
	setZeroFlag((mem[HL] >> 1) == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(mem[HL] & 1);

	mem[HL] >>= 1;
	return 4;
}

unsigned char Cpu::bit_n_phl(unsigned char targetBit)
{
	bool complement = (mem[HL] & (1 << targetBit));
	complement = !complement;
	setZeroFlag(complement);
	setSubtractFlag(false);
	setHalfCarryFlag(true);

	return 3;
}

unsigned char Cpu::res_n_phl(unsigned char targetBit)
{
	mem[HL] = (mem[HL] & ~(1 << targetBit));
	return 4;
}

unsigned char Cpu::set_n_phl(unsigned char targetBit)
{
	mem[HL] = (mem[HL] | (1 << targetBit));

	return 4;
}
