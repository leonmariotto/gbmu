/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clock.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 14:55:38 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 10:15:16 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Clock.hpp"
#include "Gameboy.hpp"

bool Clock::cgbMode = false;
bool Clock::reloadTMA = false;
int Clock::timaClock = 0;
int Clock::divClock = 0;

#define M_DIV	mem[0xFF04]
#define M_TIMA	mem[0xFF05]
#define M_TMA	mem[0xFF06]
#define M_TAC	mem[0xFF07]

Clock::Clock()
{
	clock = 0;
} 

Clock::~Clock()
{}

void Clock::reset()
{
	cgbMode = false;
	reloadTMA = false;
	timaClock = 0;
	divClock = 0;
}

// NOTE : addValue is in cycle in this context
int&	Clock::operator+=(int addValue)
{
	const int clocks_array[4] = {1024, 16, 64, 256};
	int timaFreqDivider = 0;
	// TODO cgb double speed Mode impl
	uint8_t clockMul = (cgbMode ? 2 : 4);

	clock += addValue * clockMul;

	/* Handle DIV */
	divClock += addValue * clockMul;
	M_DIV += divClock / 256;
	divClock %= 256;

	/* Handle TIMA */
	if ((M_TAC & (1 << 2)) != 0) {
		/* Calculate TIMA frequency divider TODO CGB Mode is not handled */
		timaFreqDivider = clocks_array[M_TAC & 3];
		timaClock += addValue * clockMul;
		if (reloadTMA) {
			/* Reset TIMA to TMA and add the rest */
			M_TIMA = (uint8_t)((int)M_TMA + (256 - ((int)mem[0xFF05] + timaClock / timaFreqDivider)));
			timaClock %= timaFreqDivider;
			reloadTMA = false;
		}
		else if ((int)M_TIMA + timaClock / timaFreqDivider > 0xFF) {
			/* Request Interrupt */
			mem[0xFF0F] |= (1 << 2);
			M_TIMA = 0;
			reloadTMA = true;
			timaClock %= timaFreqDivider;
		}
		else {
			M_TIMA += timaClock / timaFreqDivider;
			// TODO DEBUG mem_timing
			//if ((Cpu::PC >= 0xC39E && Cpu::PC <= 0xC3C9)
			//	|| (Cpu::PC >= 0xC3C5 && Cpu::PC <= 0xC3E6)
			//	|| (Cpu::PC >= 0xC2AF && Cpu::PC <= 0xC2FF)
			//	) {
			//	printf("PC=%04x TIMA=%02x AF=%04x BC=%04x DE=%04x HL=%04x timaClock=%d timaFreqDivider=%d\n", Cpu::PC, (uint8_t)M_TIMA,Cpu::AF, Cpu::BC, Cpu::DE, Cpu::HL, timaClock, timaFreqDivider);
			//}
			timaClock %= timaFreqDivider;
		}
	}
	if (clock >= 17556) {
		// 144 line + V-BLANK
		// We reset the counter here, we dont need to do it at this value but why not
		clock = clock - 17556;
	}
	return clock;
}

Clock::operator int()
{
	return clock;
}
