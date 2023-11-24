/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Instructions16BitLoad.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 15:57:55 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:51:00 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"
#include "Gameboy.hpp"

void	Cpu::internalPush(unsigned short valueToPush)
{
    // Subtract 1 from the stack pointer SP, and put the contents of the higher portion of register pair on the stack.
    // Subtract 2 from SP, and put the lower portion of register pair on the stack.
    // Decrement SP by 2.
//    std::cout<<(unsigned short)valueToPush << std::endl;
	mem[--SP] = valueToPush >> 8;
	mem[--SP] = valueToPush & 0xFF;
}

unsigned short	Cpu::internalPop()
{
    // Load the contents of memory specified by stack pointer SP into the lower portion of BC.
    // Add 1 to SP and load the contents from the new memory location into the upper portion of BC.
    // By the end, SP should be 2 more than its initial value.
	unsigned short value = mem[SP++];
	value |= (mem[SP++] << 8);
	return value;
}

unsigned char Cpu::push(unsigned short opcode)
{
    // Opcode: [0xC5, 0xD5, 0xE5, 0xF5]
    // Symbol: PUSH
    // Operands: [(BC), (DE), (HL), (AF)]
    // Number of Bytes: 1
    // Number of Cycles: 4
    // Flags: - - - -
    // Description
    // Push the contents of register pair onto the memory stack by doing the following:

    // Subtract 1 from the stack pointer SP, and put the contents of the higher portion of register pair on the stack.
    // Subtract 2 from SP, and put the lower portion of register pair on the stack.
    // Decrement SP by 2.

	if (opcode == 0xC5)
		internalPush(BC);
    else if (opcode == 0xD5)
		internalPush(DE);
    else if (opcode == 0xE5)
		internalPush(HL);
    else if (opcode == 0xF5)
		internalPush(AF);
    else
		logErr("called push with wrong opcode");

    return 4;
}

unsigned char Cpu::pop(unsigned short opcode)
{
    // Opcode: [0xC1, 0xD1, 0xE1, 0xF1]
    // Symbol: POP
    // Operands: [BC, DE, HL, AF]
    // Number of Bytes: 1
    // Number of Cycles: 3
    // Flags: - - - -
    // Description
    // Pop the contents from the memory stack into register pair into register pair BC by doing the following:

    // Load the contents of memory specified by stack pointer SP into the lower portion of BC.
    // Add 1 to SP and load the contents from the new memory location into the upper portion of BC.
    // By the end, SP should be 2 more than its initial value.

	if (opcode == 0xC1)
		BC = internalPop();
	else if (opcode == 0xD1)
		DE = internalPop();
	else if (opcode == 0xE1)
		HL = internalPop();
	else if (opcode == 0xF1)
		//special case with F register
		AF = (internalPop() & 0xFFF0);
	else
        logErr("called pop with the wrong opcode !");
    return 3;
}

unsigned char Cpu::load_sp_to_a16()
{
    // Opcode: 0x08
    // Symbol: LD
    // Operands: [(A16, SP)]
    // Number of Bytes: 3
    // Number of Cycles: 5
    // Flags: - - - -
    // Description
    // Store the lower byte of stack pointer SP at the address specified by the 16-bit immediate operand a16, and store the upper byte of SP at address a16 + 1.

	unsigned short a16 = readShort();
	mem[a16] = SP & 0xFF;
	mem[a16 + 1] = SP >> 8;

    return 5;
}

unsigned char Cpu::load_a16_a()
{
    // Opcode: 0xEA
    // Symbol: LD
    // Operands: [(A16, A)]
    // Number of Bytes: 3
    // Number of Cycles: 4
    // Flags: - - - -
    // Description
    // Store the contents of register A in the internal RAM or register specified by the 16-bit immediate operand a16.

	unsigned short a16 = readShort();
	mem[a16] = A;
    return 4;
}

unsigned char Cpu::load_hl_from_sp_plus_s8()
{
	//Opcode: 0xF8
    // Symbol: LD
    // Operands: [(HL, SP, S8)]
    // Number of Bytes: 2
    // Number of Cycles: 3
    // Flags: 0 0 16-bit 16-bit
    // Description
    // Add the 8-bit signed operand s8 (values -128 to +127) to the stack pointer SP, and store the result in register pair HL.

	unsigned char s8 = readByte();

	// should be evaluated as unsigned 8 bit operation
	setFlags(0, 0, getHalfCarry8Bit(s8, (unsigned char)(SP & 0xFF)), overFlow((unsigned char)(SP & 0xFF), s8));

	// but is added as signed
	HL = SP + (char)s8;

    return 3;
}

unsigned char Cpu::load_sp_from_hl()
{
    // Opcode: 0xF9
    // Symbol: LD
    // Operands: [(SP, HL)]
    // Number of Bytes: 1
    // Number of Cycles: 2
    // Flags: - - - -
    // Description
    // Load the contents of register pair HL into the stack pointer SP.

	SP = HL;
    return 2;
}

unsigned char Cpu::load_a_a16()
{
    // Opcode: 0xFA
    // Symbol: LD
    // Operands: [(A, A16)]
    // Number of Bytes: 3
    // Number of Cycles: 4
    // Flags: - - - -
    // Description
    // Load into register A the contents of the internal RAM or register specified by the 16-bit immediate operand a16.

    unsigned short addr = readShort();
	A = mem[addr];
    return 4;
}

unsigned char Cpu::load_r16_from_d16(unsigned short opcode)
{
    // Opcode: [0x01, 0x11, 0x21, 0x31]
    // Symbol: LD
    // Operands: [(BC, D16), (DE, D16), (HL, D16), (SP, D16)]
    // Number of Bytes: 3
    // Number of Cycles: 3
    // Flags: - - - -
    // Description
    // Load the 2 bytes of immediate data into register pair.

    // The first byte of immediate data is the lower byte (i.e., bits 0-7), and the second byte of immediate data is the higher byte (i.e., bits 8-15).

    switch (opcode) {
        case 0x01:
            BC = readShort();
            break;
        case 0x11:
            DE = readShort();
            break;
        case 0x21:
            HL = readShort();
            break;
        case 0x31:
            SP = readShort();
            break;
    }
    return 3;
}
