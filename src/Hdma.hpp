/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Hdma.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/06 01:22:58 by nathan            #+#    #+#             */
/*   Updated: 2023/01/12 02:02:01 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HDMA_CLASS_H
# define HDMA_CLASS_H

/*
** HDMA
** It implement HDMA for CGB in normal mode and in H-BLANK mode
** Use Clock for timing.
** It handle interruption of HDMA transfert.
*/

#include <cstdint>

class Hdma {
public:
	static void writeInHdma(uint16_t dstAddr, uint16_t srcAddr, uint8_t newValue);
	static void reset();
	static int update();
	static int updateHBlank();
private:
	static uint16_t src;
	static uint16_t dst;
	static uint16_t len;
	static uint8_t vbank;
	static bool bIsWritting;
	static bool bIsInHBlankMode;
	static bool bJustStarted;
};

#endif
