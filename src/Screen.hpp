/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Screen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmariott <lmariott@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 20:46:17 by lmariott          #+#    #+#             */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCREEN_CLASS_H
# define SCREEN_CLASS_H

/*
** Handle all sub window in the UI
** This module manage texture and update it to draw different thing :
** Background, VRAM, Palettes, Sprites, and finally the Ppu output line.
*/

#include <SDL2/SDL.h>
#include <array>
#include "Ppu.hpp"

class Screen
{
public:
	static void		destroyTexture();

	static bool 		drawPoint(unsigned short x, unsigned short y,
					const unsigned short& color, void *pixels,
					int pitch, int pixelScale, bool bIsCGB);
    	static void		lockTexture();

	static int 	 	convertColorFromCGB(int colo,
					bool bConvertForImGUI = false);
	static void		updatePpuLine(const std::array<short,
					PIXEL_PER_LINE>& lineData,
					unsigned char currentLine, bool bIsCGB);
	static bool		createTexture(bool bIsCGB, SDL_Renderer* uiRenderer);


//	static int inline 	convertColorFromCGB(int colo,
//					bool bConvertForImGUI = false);

	static SDL_Texture*	ppuTexture;
	static void*		ppuPixels;
	static int		ppuPitch;
	static bool		bIsInit;

private:
};

#endif
