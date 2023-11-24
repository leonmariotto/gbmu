/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InstructionsControl.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 15:52:11 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:50:41 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"
#include "Clock.hpp"
#include "Gameboy.hpp"

unsigned char Cpu::nop()
{
	return 1;
}

unsigned char Cpu::stop()
{
	//if (!Gameboy::bIsCGB || Gameboy::bCGBIsInCompatMode) {
	//	return 2;
	//}
	Cpu::halted = true;
	Cpu::halt_counter = 1; // init halt counter
	// After a successfull stop the CPU is stopped for 2050 cycle
	mem[0xFF04] = 0; // reset DIV
	// if speed change requested apply it
	// TODO speed change stop is the only case supported here
	// TODO cpu will be halted for 2050 inst only (true?)
	printf("Speed change requested from %d to %d\n", (uint8_t)Clock::cgbMode, (uint8_t)(mem[0xFF4D] & 1));
	if ((mem[0xFF4D] & 1)) {
		mem[0xFF4D] = 0x80;
		Clock::cgbMode = true;
	}
	else {
		mem[0xFF4D] = 0x00;
		Clock::cgbMode = false;
	}
	return 2;
}

unsigned char Cpu::halt()
{
	Cpu::halted = true;
	return 1;
}

unsigned char Cpu::di()
{
	Cpu::IME = false;
	Cpu::setIMEFlag = false;
	return 1;
}

// The effect of EI is delayed by one instruction. This means that EI followed immediately by DI does not allow interrupts between the EI and the DI.
unsigned char Cpu::ei()
{
	// this is because if the IME is already working, there is no need to verify with a delay to enable it
	if (IME != true)
		Cpu::setIMEFlag = true;
	return 1;
}
