/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InstructionsMisc.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 21:48:16 by nallani           #+#    #+#             */
/*   Updated: 2022/11/11 16:09:49 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"

unsigned char Cpu::daa()
{
    // Opcode: 0x27
    // Symbol: DAA
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: Z - 0 CY
    // Description
    // Adjust the accumulator (register A) to a binary-coded decimal (BCD) number after BCD addition and subtraction operations.
    // https://stackoverflow.com/questions/8119577/z80-daa-instruction
    // http://z80-heaven.wikidot.com/instructions-set:daa
	unsigned char magic = 0;

	if (getHalfCarryFlag() || (!getSubtractFlag() && ((A & 0xF) > 9)))
		magic = 0x06;
	if (getCarryFlag() || (!getSubtractFlag() && A > 0x99))
	{
		magic |= 0x60;
		setCarryFlag(true);
	}

	A = (getSubtractFlag()) ? (A - magic) : (A + magic);

	setZeroFlag(A == 0);
	setHalfCarryFlag(0);
	return 1;
}

unsigned char Cpu::cpl()
{
    // Opcode: 0x2F
    // Symbol: CPL
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: - 1 1 -
    // Description
    // Take the one's complement (i.e., flip all bits) of the contents of register A.

	setSubtractFlag(1);
	setHalfCarryFlag(1);

	A = ~A;
    return 1;
}

unsigned char Cpu::scf()
{
    // Opcode: 0x37
    // Symbol: SCF
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: - 0 0 1
    // Description
    // Set the carry flag CY.

	setSubtractFlag(0);
	setHalfCarryFlag(0);
	setCarryFlag(1);
    return 1;
}

unsigned char Cpu::ccf()
{
    // Opcode: 0x3F
    // Symbol: CCF
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: - 0 0 !CY
    // Description
    // Flip the carry flag CY.

	setSubtractFlag(0);
	setHalfCarryFlag(0);
	setCarryFlag(!getCarryFlag());
    return 1;
}
