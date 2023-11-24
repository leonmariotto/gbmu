/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CpuStackTrace.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/20 16:33:15 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 09:34:57 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CpuStackTrace.hpp"
#include <iostream>
#include <iomanip>

bool CpuStackTrace::autoPrint = false;

CpuStackTrace::CpuStackTrace()
{
	printSize = 3000;
	maxSize = 1;
	PCBreak = 0;
	opcodeBreak = 0xFF;
	breakActive = false;
	queue.resize(maxSize + 1);
}

CpuStackTrace::~CpuStackTrace()
{
}

void CpuStackTrace::print()
{
	for (int i = queue.size() - 1; i >= 0; i--)
	{
		queue[i].print();
		std::cout << std::endl;
	}
}

void CpuStackTrace::add(StackData stackData)
{
	return;
	if (autoPrint)
	{
		stackData.print();
		std::cout << std::endl;
	}
	queue.insert(queue.begin(), stackData);
	if (queue.size() > maxSize)
		queue.pop_back();
	if (breakActive && (stackData.PC == PCBreak || stackData.opcode == opcodeBreak || queue.size() > printSize))
	{
		print();
		exit(0);
	}
}

StackData::StackData()
{
}

void StackData::print()
{
	std::cout << std::setfill('0');
	std::cout << "PC: 0x" << std::setw(4) << PC << "\topcode: 0x" << std::setw(4) <<  opcode << std::endl;
	std::cout << "AF: 0x" << std::setw(4) << AF << "\tBC: 0x" << std::setw(4) << BC << std::endl;
	std::cout << "DE: 0x" << std::setw(4) << DE << "\tHL: 0x" << std::setw(4) << HL << std::endl;
	std::cout << "SP: 0x" << std::setw(4) << SP << std::endl;
	std::cout << "IE: 0x" << std::setw(2) << (int)ie_reg << "\tIF: 0x" << std::setw(2) << (int)if_reg << std::endl;
	std::cout << "LCDC: 0x" << std::setw(2) << (int)lcdc << "\tLY: 0x" << std::setw(2) << (int)ly_reg  << std::endl;
	std::cout << "LY: 0x" << std::setw(2) << (int)ly_reg << "\tIME: " << ime << std::endl;
	std::cout << "STAT: 0x" << std::setw(2) << (int)lcdc_stat << std::endl;
	if (!customData.empty())
		std::cout << customData << std::endl;
}
