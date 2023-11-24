/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FlagOp.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/08 16:25:02 by nallani           #+#    #+#             */
/*   Updated: 2022/11/10 19:34:49 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cpu.hpp"

bool Cpu::getHalfCarry8Bit(unsigned char a, unsigned char b, unsigned char c)
{
	a &= 0xF;
	b &= 0xF;
	c &= 0xF;
	return (a + b + c) > 0xF;
}

bool Cpu::getHalfCarry16Bit(unsigned short a, unsigned short b)
{
	a &= 0xFFF;
	b &= 0xFFF;
	return (a + b) > 0xFFF;
}

bool Cpu::getHalfBorrow8Bit(unsigned char a, unsigned char b, unsigned char c)
{
	a &= 0xF;
	b &= 0xF;
	c &= 0xF;

	return ((b + c) > a);
}

bool Cpu::getHalfBorrow16Bit(unsigned short a, unsigned short b)
{
	a &= 0xFFF;
	b &= 0xFFF;

	return (b > a);
}

bool Cpu::overFlow(unsigned char a, unsigned char b, unsigned char c)
{
	if (b != 0 || c != 0)
		return (unsigned char)(a + b + c) <= a;
	else
		return false;
}

bool Cpu::underFlow(unsigned char a, unsigned char b, unsigned char c)
{
	return ((int)(b + c)) > a;
}
