/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CpuStackTrace.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/20 16:33:18 by nallani           #+#    #+#             */
/*   Updated: 2023/01/04 00:52:49 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CPUSTACKTRACE_CLASS_H
# define CPUSTACKTRACE_CLASS_H
#include <vector>
#include <string>

struct StackData {
	StackData();
	void print();
	unsigned short PC;
	unsigned short SP;
	unsigned short AF;
	unsigned short BC;
	unsigned short DE;
	unsigned short HL;
	unsigned short opcode;
	unsigned char if_reg;
	unsigned char ie_reg;
	unsigned char ly_reg;
	unsigned char lcdc;
	unsigned char lcdc_stat;
	bool ime;
	std::string		customData;
};

class CpuStackTrace {
public:
	CpuStackTrace(void);
	void add(StackData stackData);
	void print();
	virtual ~CpuStackTrace(void);
	unsigned int	maxSize;
	unsigned int printSize;
	unsigned short PCBreak;
	unsigned short opcodeBreak;
	bool breakActive;
	static bool autoPrint;
private:
	std::vector<StackData> queue;
};

#endif
