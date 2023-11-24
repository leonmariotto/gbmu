/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InstructionsJumpCalls.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 15:53:28 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:49:27 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"
#include "Utility.hpp"

unsigned char Cpu::ret_flag(unsigned short opcode)
{
    // Opcode: [0xC0, 0xC8, 0xD0, 0xD8]
    // Symbol: RET NZ, RET Z, RET NC, RET C
    // Number of Bytes: 1
    // Number of Cycles: 5/2
    // Flags: - - - -
    // Description
    // 0xC0 0xC8
    // If the Z flag is [0, 1], control is to the source program by popping from the memory stack the program counter PC valu
    // that was pushed to the stack when the subroutine was called.
    // 0xD0 0xD8
    // If the CY flag is [0, 1], control is to the source program by popping from the memory stack the program counter PC valu
    // that was pushed to the stack when the subroutine was called.

    // The contents of the address specified by the stack pointer SP are loaded in the lower-order byte of PC,
    // and the contents of SP are incremented by 1. The contents of the address specified by the new SP value are then
    // loaded in the higher-order byte of PC, and the contents of SP are incremented by 1 again.
    // (The value of SP is 2 larger than before instruction execution.)
    // The next instruction is fetched from the address specified by the content of PC (as usual).

	bool isFlagSet = ((opcode & 0xF0) == 0xD0) ? getCarryFlag() : getZeroFlag();

	if ((opcode & 0x0F) == 0)
	{
		if (!isFlagSet) {
			ret();
			return 5;
		}
	}
	else if ((opcode & 0x0F) == 0x08)
	{
		if (isFlagSet) {
			ret();
			return 5;
		}
	}
	else
		logErr("ret_flag called with wrong opcode !");
	return 2;
}

unsigned char Cpu::jp_flag_a16(unsigned short opcode)
{
    // Opcode: [0xC2, 0xCA, 0xD2, 0xDA]
    // Symbol: JP NZ, JP Z, JP NC, JP C
    // Operands: [(A16), (A16), (A16), (A16)]
    // Number of Bytes: 3
    // Number of Cycles: 4/3
    // Flags: - - - -
    // Description
    // FOR 0xC2 :
    // Load the 16-bit immediate operand a16 into the program counter PC if the Z flag is 0. If the Z flag is 0,
    // then the subsequent instruction starts at address a16.
    // If not, the contents of PC are incremented, and the next instruction following the current JP instruction is executed (as usual).
    // FOR 0xCA :
    // Load the 16-bit immediate operand a16 into the program counter PC if the Z flag is 1. If the Z flag is 1,
    // FOR 0xD2 :
    // Load the 16-bit immediate operand a16 into the program counter PC if the CY flag is 0. If the CY flag is 0,
    // FOR 0xDA :
    // Load the 16-bit immediate operand a16 into the program counter PC if the CY flag is 1. If the CY flag is 1,

    // The second byte of the object code (immediately following the opcode) corresponds to the lower-order byte of a16 (bits 0-7),
    // and the third byte of the object code corresponds to the higher-order byte (bits 8-15).

    bool is_flag_set = ((opcode & 0xF0) == 0xD0) ? getCarryFlag() : getZeroFlag();

    if ((opcode & 0x0F) == 0x02)
	{
        if (!is_flag_set)
            return jp_a16();
    }
	else if ((opcode & 0x0F) == 0x0A)
	{
        if (is_flag_set)
            return jp_a16();
    }
	else
        logErr(string_format("called jp_flag_a16 with the wrong opcode: 0x%X", opcode));

	readShort(); // Used to make sure the PC is correct.

    return 3;
}

unsigned char Cpu::jp_a16()
{
    // Opcode: 0xC3
    // Symbol: JP
    // Operands: [(A16)]
    // Number of Bytes: 3
    // Number of Cycles: 4
    // Flags: - - - -
    // Description
    // Load the 16-bit immediate operand a16 into the program counter (PC). a16 specifies the address of the subsequently executed instruction.

    // The second byte of the object code (immediately following the opcode) corresponds to the lower-order byte of a16 (bits 0-7),
    // and the third byte of the object code corresponds to the higher-order byte (bits 8-15).

	PC = readShort();

    return 4;
}

unsigned char Cpu::call_flag_a16(unsigned short opcode)
{
    // Opcode: [0xC4, 0xCC, 0xD4, 0xDC]
    // Symbol: CALL NZ, CALL A, CALL NC, CALL C
    // Operands: [(A16), (A16), (A16), (A16)]
    // Number of Bytes: 3
    // Number of Cycles: 6/3
    // Flags: - - - -
    // Description   [0xC4, 0xCC]
    // If the Z flag is [0, 1], the program counter PC value corresponding to the memory location
    // ...
    // Description   [0xD4, 0xDC]
    // If the CY flag is [0, 1], the program counter PC value corresponding to the memory location
    // of the instruction following the CALL instruction is pushed to the 2 bytes following the memory byte specified by the stack pointer SP.
    // The 16-bit immediate operand a16 is then loaded into PC.

    // The lower-order byte of a16 is placed in byte 2 of the object code, and the higher-order byte is placed in byte 3.

    bool is_flag_set = ((opcode == 0xD4) || (opcode == 0xDC)) ? getCarryFlag() : getZeroFlag();

    if ((opcode == 0xCC) || (opcode == 0xDC))
	{
        if (is_flag_set)
            return call_a16(); // returns 6
    }
	else if ((opcode == 0xC4) || (opcode == 0xD4))
	{
        if (!is_flag_set)
			return call_a16(); // returns 6
	}
	else
        logErr("called 'call' opcode with wrong opcode !");

    readShort(); // Set PC to correct value

    return 3;
}

unsigned char Cpu::ret()
{
    // Opcode: 0xC9
    // Symbol: RET
    // Number of Bytes: 1
    // Number of Cycles: 4
    // Flags: - - - -
    // Description
    // Pop from the memory stack the program counter PC value pushed when the subroutine was called, contorl to the source program

    PC = internalPop();
    // The contents of the address specified by the stack pointer SP are loaded in the lower-order byte of PC, and the contents of SP are incremented by 1.
    // The contents of the address specified by the new SP value are then loaded in the higher-order byte of PC, and the contents of SP are incremented by 1 again.
    // (THe value of SP is 2 larger than before instruction execution.)
    // The next instruction is fetched from the address specified by the content of PC (as usual).
    return 4;
}

unsigned char Cpu::call_a16()
{
    // Opcode: 0xCD
    // Symbol: CALL
    // Operands: [(A16)]
    // Number of Bytes: 3
    // Number of Cycles: 6
    // Flags: - - - -
    // Description
    // In memory, push the program counter PC value corresponding to the address following the CALL instruction to the 2 bytes following the byte
    // specified by the current stack pointer SP. Then load the 16-bit immediate operand a16 into PC.

    // The subroutine is placed after the location specified by the new PC value.
    // When the subroutine finishes, control is to the source program using a return instruction an
    // by popping the starting address of the next instruction (which was just pushed) and moving it to the PC.

    // With the push, the current value of SP is decremented by 1, and the higher-order byte of PC is loaded in the memory address specified by the new SP value.
    // The value of SP is then decremented by 1 again, and the lower-order byte of PC is loaded in the memory address specified by that value of SP.

    // The lower-order byte of a16 is placed in byte 2 of the object code, and the higher-order byte is placed in byte 3.
	unsigned short a16 = readShort();
	internalPush(PC);
	PC = a16;
    return 6;
}

unsigned char Cpu::jp_hl()
{
    // Opcode: 0xE9
    // Symbol: JP
    // Operands: [(HL)]
    // Number of Bytes: 1
    // Number of Cycles: 1
    // Flags: - - - -
    // Description
    // Load the contents of register pair HL into the program counter PC. The next instruction is fetched from the location specified by the new value of PC.
	PC = HL;
    return 1;
}

unsigned char Cpu::reti()
{
    // Opcode: 0xD9
    // Symbol: RETI
    // Number of Bytes: 1
    // Number of Cycles: 4
    // Flags: - - - -
    // Description
    // Used when an interrupt-service routine finishes. The address for the from the interrupt is loaded in the program counter PC
    // The master interrupt enable flag is to its pre-interrupt status

    // The contents of the address specified by the stack pointer SP are loaded in the lower-order byte of PC, and the contents of SP are incremented by 1.
    // The contents of the address specified by the new SP value are then loaded in the higher-order byte of PC, and the contents of SP are incremented by 1 again.
    // (THe value of SP is 2 larger than before instruction execution.)
    // The next instruction is fetched from the address specified by the content of PC (as usual).

    Cpu::IME = true;
    return ret(); // returns 4
}

unsigned char Cpu::jr_s8()
{
    // Opcode: 0x18
    // Symbol: JR
    // Number of Bytes: 2
    // Number of Cycles: 3
    // Flags: - - - -
    // Description
    // Jump s8 steps from the current address in the program counter (PC). (Jump relative.)

	PC += (char)readByte();
    return 3;
}

unsigned char Cpu::jr_s8_flag(unsigned short opcode)
{
    // Opcode: [0x20, 0x28, 0x30, 0x38]
    // Symbol: JR NZ, JR Z, JR NC, JR C
    // Operands: [(S8), (S8), (S8), (S8)]
    // Number of Bytes: 2
    // Number of Cycles: 3/2
    // Flags: - - - -
    // Description
    // FOR 0x20 : If the Z flag is 0, jump s8 steps from the current address stored in the program counter (PC).
    // If not, the instruction following the current JP instruction is executed (as usual).
    // FOR 0x28 : If the Z flag is 1, jump s8 steps from the current address stored in the program counter (PC).
    // If not, the instruction following the current JP instruction is executed (as usual).
    // FOR 0x30 : If the CY flag is 0, jump s8 steps from the current address stored in the program counter (PC).
    // If not, the instruction following the current JP instruction is executed (as usual).
    // FOR 0x38 : If the CY flag is 1, jump s8 steps from the current address stored in the program counter (PC).
    // If not, the instruction following the current JP instruction is executed (as usual).

    bool is_flag_set = ((opcode & 0xF0) == 0x30) ? getCarryFlag() : getZeroFlag();
	if ((opcode & 0x0F) == 0)
	{
		if (!is_flag_set) 
			return jr_s8();
	}
	else if ((opcode & 0x0F) == 0x08)
	{
		if (is_flag_set)
			return jr_s8();
	}
	else
		logErr("jr_s8_flag called with wrong opcode");

	readByte(); // Used to make sure the PC is correct.;
	return 2;
}

unsigned char Cpu::rst_n(unsigned short opcode)
{
    // Opcode: [0xC7, 0xD7, 0xE7, 0xF7, 0xCF, 0xDF, 0xEF, 0xFF]
    // Symbol: RST
    // Operands: [(0), (2), (4), (6), (1), (3), (5), (7)]
    // Number of Bytes: 1
    // Number of Cycles: 4
    // Flags: - - - -
    // Description
    // Push the current value of the program counter PC onto the memory stack, and load into PC the (n + 1)th byte of page 0 memory addresses, [0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38].
    // The next instruction is fetched from the address specified by the new content of PC (as usual).

    // With the push, the contents of the stack pointer SP are decremented by 1,
    // and the higher-order byte of PC is loaded in the memory address specified by the new SP value.
    // The value of SP is then again decremented by 1, and the lower-order byte of the PC is loaded in the memory address specified by that value of SP.

    // The RST instruction can be used to jump to 1 of 8 addresses.
    // Because all of the addresses are held in page 0 memory, 0x00 is loaded in the higher-orderbyte of the PC,
    // and [0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38] is loaded in the lower-order byte.
    unsigned char targetByte = '\0';
	if (opcode == 0xC7)
		targetByte = 0;
	else if (opcode == 0xD7)
		targetByte = 0x10;
	else if (opcode == 0xE7)
		targetByte = 0x20;
	else if (opcode == 0xF7)
		targetByte = 0x30;
	else if (opcode == 0xCF)
		targetByte = 0x08;
	else if (opcode == 0xDF)
		targetByte = 0x18;
	else if (opcode == 0xEF)
		targetByte = 0x28;
	else if (opcode == 0xFF)
		targetByte = 0x38;
	else
		logErr("called rst_n with wrong opcode");

    internalPush(PC);
	PC = targetByte;
    return 4;
}
