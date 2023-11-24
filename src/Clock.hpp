/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clock.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 14:55:36 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 11:56:16 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CLOCK_CLASS_H
# define CLOCK_CLASS_H

/*
** Clock : time management in GBMU
**
** Clock cycle freq is 4.194304 MHz for GB
** and 8.4 MHz for CGB in double speed mode
** That mean : for each cycle spent (which is returned by the instructions)
** we increment clock of 4 or 8 (CGB db speed)
**
** Timer registers :
**
** - FF04 : DIV (Divider Register)
** FF04 is incremented at rate of 16384Hz ----> it's clock cycle of gb / 256 
** and 32768Hz in CGB double speed mode. ------> same thing
** - FF05 : TIMA (Timer Counter)
** Incremented according to clock freq from TAC register (FF07)
** When the value overflow (> 0xFF, its a byte) it trigger
** an interrupt and it's reset to TMA (FF06)
** - FF06 : TMA
** This data is loaded to TIMA when its overflow
** - FF07 : TAC (Timer Control)
** Bit 2 : 1:Start 0:Stop
** Bits 1-0 : 
** 00 -> 4096Hz -> clock cycle / (1024 * CGBMODE?2:1)
** 01 -> 262144Hz -> clock cycle / (16 * CGBMODE?2:1)
** 10 -> 65536Hz -> clock cycle / (64 * CGBMODE?2:1)
** 11 -> 16384Hz -> clock cycle / (256 * CGBMODE?2:1)
*/


class Clock
{
public:
	Clock();
	virtual	~Clock(void);
	int& operator+=(int addValue);
	operator int();
	static bool	cgbMode;

	static bool	getReloadTMA() {return reloadTMA;}
	static int	getTimaClock() {return timaClock;}
	static int	getDivClock() {return divClock;}
	int		getClock() {return clock;}

	static void	setReloadTMA(bool val) {reloadTMA = val;}
	static void	setTimaClock(int val) {timaClock = val;}
	static void	setDivClock(int val) {divClock = val;}
	void 		setClock(int val) {clock = val;}
	void 		reset();

private:
	static bool	reloadTMA;
	static int	timaClock;
	static int	divClock;
	int 		clock;
};

#endif
