/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Instructions8BitLoad.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 15:56:06 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:50:52 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"
#include "Gameboy.hpp"

unsigned char Cpu::load_a8_a()
{
    // Opcode: 0xE0
    // Symbol: LD
    // Operands: [(A8, A)]
    // Number of Bytes: 2
    // Number of Cycles: 3
    // Flags: - - - -
    // Description
    // Store the contents of register A in the internal RAM, port register, or mode register at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.

    // Note: Should specify a 16-bit address in the mnemonic portion for a8, although the immediate operand only has the lower-order 8 bits.

    // 0xFF00-0xFF7F: Port/Mode registers, control register, sound register
    // 0xFF80-0xFFFE: Working & Stack RAM (127 bytes)
    // 0xFFFF: Interrupt Enable Register

    mem[0xFF00 | readByte()] = A;
    return 3;
}

unsigned char Cpu::load_c_a()
{
    // Opcode: 0xE2
    // Symbol: LD
    // Operands: [(C, A)]
    // Number of Bytes: 1
    // Number of Cycles: 2
    // Flags: - - - -
    // Description
    // Store the contents of register A in the internal RAM, port register, or mode register at the address in the range 0xFF00-0xFFFF specified by register C.

    // 0xFF00-0xFF7F: Port/Mode registers, control register, sound register
    // 0xFF80-0xFFFE: Working & Stack RAM (127 bytes)
    // 0xFFFF: Interrupt Enable Register

    mem[0xFF00 | C] = A;
    return 2;
}

unsigned char Cpu::load_a_a8()
{
    // Opcode: 0xF0
    // Symbol: LD
    // Operands: [(A, A8)]
    // Number of Bytes: 2
    // Number of Cycles: 3
    // Flags: - - - -
    // Description
    // Load into register A the contents of the internal RAM, port register, or mode register at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.

    // Note: Should specify a 16-bit address in the mnemonic portion for a8, although the immediate operand only has the lower-order 8 bits.

    // 0xFF00-0xFF7F: Port/Mode registers, control register, sound register
    // 0xFF80-0xFFFE: Working & Stack RAM (127 bytes)
    // 0xFFFF: Interrupt Enable Register

	/*
	std::cout << "read at address: " << (0xFF00 | (unsigned short)mem[PC])
		<< " value: " << (int)mem[(0xFF00 |(unsigned short)mem[PC])]  << std::endl;
		*/
	unsigned char lowAddr = readByte();
	//if (Gameboy::frameNb > DBG::stopAtFrame)
		//printf("reading at address %X, which has value: %X\n", (0xFF00 | lowAddr), (int)mem[0xFF00|lowAddr]);
    A = mem[0xFF00 | lowAddr];
    return 3;
}

unsigned char Cpu::load_a_c()
{
    // Opcode: 0xF2
    // Symbol: LD
    // Operands: [(C, A)]
    // Number of Bytes: 1
    // Number of Cycles: 2
    // Flags: - - - -
    // Description
    // Load into register A the contents of the internal RAM, port register, or mode register at the address in the range 0xFF00-0xFFFF specified by register C.

    // 0xFF00-0xFF7F: Port/Mode registers, control register, sound register
    // 0xFF80-0xFFFE: Working & Stack RAM (127 bytes)
    // 0xFFFF: Interrupt Enable Register

    A = mem[0xFF00 | C];
    return 2;
}

unsigned char Cpu::load_r_r(unsigned char& loadTarget, unsigned char loadSource)
{
    // Opcode: [0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x57, 0x58, 0x59,
    //          0x5a, 0x5b, 0x5c, 0x5d, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6f, 0x78, 0x79, 0x7a, 0x7b,
    //          0x7c, 0x7d, 0x7f]
    // Symbol: LD
    // Operands: [(B, B), (B, C), (B, D), (B, E), (B, H), (B, L), (B, A), (C, B), (C, C), (C, D), (C, E), (C, H), (C, L), (C, A),
    //            (D, B), (D, C), (D, D), (D, E), (D, H), (D, L), (D, A), (E, B), (E, C), (E, D), (E, E), (E, H), (E, L), (E, A),
    //            (H, B), (H, C), (H, D), (H, E), (H, H), (H, L), (H, A), (L, B), (L, C), (L, D), (L, E), (L, H), (L, L), (L, A),
    //            (A, B), (A, C), (A, D), (A, E), (A, H), (A, L), (A, A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: - - - -
    // Description
    // Load the contents of register X into register Y.
    loadTarget = loadSource;
    return 1;
}

unsigned char Cpu::load_r_hl(unsigned char& loadTarget)
{
	loadTarget = mem[HL];
	return 2;
}

unsigned char Cpu::load_hl_r(unsigned char loadSource)
{
    // Opcode: [0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77]
    // Symbol: LD
    // Operands: [((HL), B), ((HL), C), ((HL), D), ((HL), E), ((HL), H), ((HL), L), ((HL), A)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: - - - -
    // Description
    // Load the contents of register X into register Y.
    // TODO think about verifying if register is HL
    mem[HL] = loadSource;
    return 2;
}

unsigned char Cpu::load_hl_d8()
{
    // Opcode: [0x36]
    // Symbol: LD
    // Operands: [(B, D8), (D, D8), (H, D8), (C, D8), (E, D8), (L, D8), (A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: - - - -
    // Description
    // Load the 8-bit immediate operand d8 into register r.

    mem[HL] = readByte();
	return 3;
}

unsigned char Cpu::load_r_d8(unsigned char& loadTarget)
{
    // Opcode: [0x06, 0x16, 0x26, 0x0E, 0x1E, 0x2E, 0x3E]
    // Symbol: LD
    // Operands: [(B, D8), (D, D8), (H, D8), (C, D8), (E, D8), (L, D8), (A, D8)]
    // Number of Bytes: 2
    // Number of Cycles: 2
    // Flags: - - - -
    // Description
    // Load the 8-bit immediate operand d8 into register r.

    loadTarget = readByte();
	return 2;
}

unsigned char Cpu::load_a_r16(unsigned short opcode)
{
    // Opcode: [0x0A, 0x1A, 0x2A, 0x3A]
    // Symbol: LD
    // Operands: [(A, BC), (A, DE), (A, HL), (A, SP)]
    // Number of Bytes: 1
    // Number of Cycles: 2
    // Flags: - - - -
    // Description
    // Load the 8-bit contents of memory specified by register pair BC into register A.

    // For 0x2A : Load the contents of memory specified by register pair HL into register A, and simultaneously increment the contents of HL.
    // For 0x3A : Load the contents of memory specified by register pair HL into register A, and simultaneously decrement the contents of HL.
    switch (opcode) {
        case 0x0A:
            A = mem[BC];
            break;
        case 0x1A:
            A = mem[DE];
            break;
        case 0x2A:
            A = mem[HL];
			HL++;
            break;
        case 0x3A:
            A = mem[HL];
			HL--;
            break;
    }
    return 2;
}

unsigned char Cpu::load_r16_a(unsigned short opcode)
{
    // Opcode: [0x02, 0x12, 0x22, 0x32]
    // Symbol: LD
    // Operands: [(BC, A), (DE, A), (HL, A), (HL, A)]
    // Number of Bytes: 1
    // Number of Cycles: 2
    // Flags: - - - -
    // Description
    // Store the contents of register A in the memory location specified by register pair rr.

    // For 0x22 : Store the contents of register A into the memory location specified by register pair HL, and simultaneously increment the contents of HL.
    // For 0x32 : Store the contents of register A into the memory location specified by register pair HL, and simultaneously decrement the contents of HL.

    switch (opcode) {
        case 0x02:
            mem[BC] = A;
            break;
        case 0x12:
            mem[DE] = A;
            break;
        case 0x22:
            mem[HL] = A;
			HL++;
            break;
        case 0x32:
            mem[HL] = A;
			HL--;
            break;
    }
    return 2;
}
