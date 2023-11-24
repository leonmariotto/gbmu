/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Ppu.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/07 19:58:01 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 12:10:05 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Gameboy.hpp"
#include "TilePixels.hpp"
#include "Sprite.hpp"
#include "Ppu.hpp"
#include <algorithm>
#include <iostream>
#include "define.hpp"
#include <functional>

unsigned char Ppu::windowCounter = 0;

void Ppu::reset()
{
	windowCounter = 0;
}

std::array<short, PIXEL_PER_LINE> Ppu::getDefaultWhiteLine()
{
	std::array<short, PIXEL_PER_LINE> line;
	line.fill(!Gameboy::bIsCGB ? 0 : 0xFFFF);
	return line;
}

std::array<short, PIXEL_PER_LINE> Ppu::doOneLine()
{
	std::array<short, PIXEL_PER_LINE> renderedLine = getDefaultWhiteLine();
	auto pixelLine = getOamLine();

	auto backgroundLine = getBackgroundLine();

	for (int i = 0; i < PIXEL_PER_LINE; i++)
	{
		if (!Gameboy::bIsCGB) {
			// color code == 0 means sprite pixel is translucent
			if ((pixelLine[i].colorCode != 0 &&
						((pixelLine[i].bIsAboveBackground && !backgroundLine[i].bIsAboveOAM)|| backgroundLine[i].colorCode == 0)))
			{
				renderedLine[i] = pixelLine[i].color;
			}
			else if (BIT(M_LCDC, 0)) {
				renderedLine[i] = backgroundLine[i].color;
			}
		}
		
		else
		{
			bool bIsPixel = false;
			if (backgroundLine[i].colorCode == 0)
				bIsPixel = true;
			else if (!BIT(M_LCDC, 0))
				bIsPixel = true;
			else if (!backgroundLine[i].bIsAboveOAM && pixelLine[i].bIsAboveBackground)
				bIsPixel = true;

			if (bIsPixel && pixelLine[i].bIsSet)// bIsSet takes transparency into account
				renderedLine[i] = pixelLine[i].color;
			else
				renderedLine[i] = backgroundLine[i].color;
		}
	}
	return renderedLine;
}

struct TilePixels Ppu::getTile(int tileAddress, int tileIndex, unsigned short mapAddress)
{
	if (tileAddress == 0x8800)
	{
		tileAddress = 0x9000; // this is because we dont actually start at 0x8000
							  // but add a signed offset to 0x9000 instead
		tileIndex = (char)tileIndex;// make sure it is signed
	}
	// fetch the 64 pixels of a tile 
	return TilePixels(tileAddress + (tileIndex * 2 * 8), mapAddress);
}

std::array<BackgroundData, PIXEL_PER_LINE> Ppu::getBackgroundLine()
{
	std::array<BackgroundData, PIXEL_PER_LINE> backgroundLine{};
	bool bWindowEnabled = BIT(M_LCDC, 5);
	bool bBackgroundEnabled = BIT(M_LCDC, 0);
	int xPosInLine = 0;
	bool bDrawWindow = bWindowEnabled && M_LY >= M_WY && xPosInLine >= (M_WX - WX_OFFSET);

	while (xPosInLine < PIXEL_PER_LINE)
	{
		TilePixels tilePixels;
		if (bDrawWindow)
			tilePixels = getWindowTile((xPosInLine + WX_OFFSET - M_WX) / 8,  windowCounter / 8);// should not underflow/panic because of windowDraw bool
		else if (bBackgroundEnabled || (Gameboy::bIsCGB && !Gameboy::bCGBIsInCompatMode))// because in CGB bit 0 of LCDC only matters for superposition
		{
			//std::cout << std::dec << "creating a tilePixel at LY: " << (int)M_LY << " number of tile: " << (int)(xPosInLine + M_SCX) / 8
				//<< std::hex << std::endl;
			tilePixels = getBackgroundTile((xPosInLine + M_SCX) / 8, (M_LY + M_SCY) / 8);//[(M_LY + M_SCY) % 8];
		}
		const unsigned char yLine = (bDrawWindow ? (windowCounter % 8) : ((M_LY + M_SCY) % 8));
		auto tilePixelColorLine = tilePixels.getColorLine(yLine);
		auto tilePixelColorCodeLine = tilePixels.getLineColorCode(yLine);
		for (int i = 0; i < 8; i++)
		{
			if (!bDrawWindow && xPosInLine == 0 && i < (M_SCX % 8))// skip pixel if SCX % 8 != 0
				continue;				 // if scx == 3 then skip the
									 // first 3 pixels
			backgroundLine[xPosInLine].color = tilePixelColorLine[i];
			backgroundLine[xPosInLine].colorCode = tilePixelColorCodeLine[i];
			backgroundLine[xPosInLine].bIsAboveOAM = tilePixels.isAboveOAM();
			xPosInLine++;
			if (xPosInLine >= PIXEL_PER_LINE)
				break;

			// check if window should be enabled,
			// if the condition is met restart draw at that pos
			// make sure the window has to be rendered with its WX/WY
			// make sure window is in on x axis,
			// WX == 0x07 and WY == 0x00 means the window will be at the top left of the screen
			if (!bDrawWindow && bWindowEnabled && M_LY >= M_WY && xPosInLine >= (M_WX - WX_OFFSET))
			{
				bDrawWindow = true;
				break;
			}
		}
	}
	if (bDrawWindow)// increment internal window counter if window was drawn
		windowCounter++;
	return backgroundLine;
}

std::array<SpriteData, PIXEL_PER_LINE> Ppu::getOamLine()
{
	std::vector<struct OAM_entry> spritesFound, spritesFound2;
	std::array<SpriteData, PIXEL_PER_LINE> spriteLine{};
	spriteLine.fill({0, false, 0, false}); // Init first the sprite line
	if (!BIT(M_LCDC, 1)) { // if OBJ flag isnt enabled, return empty array
		return spriteLine;
	}
	const int OAM_Addr = 0xFE00;
	unsigned char spriteHeight = BIT(M_LCDC, 2) ? 16 : 8; // type of sprite from flag

	struct OAM_entry *entry = (struct OAM_entry *)(&mem[OAM_Addr]);
	// 1 - fetch the sprites needed for that line
	for (int i = 0; i < SPRITES_IN_OAM; i++)
	{
		// verify if the sprite should be rendered on this line
		// if posX == 0 then sprite is totally off the screen.
		// same goes for posY that starts at 0x10 (as it can be 16 height)
		if ((M_LY + 16 >= entry->posY && (M_LY + 16 < entry->posY + spriteHeight))
				&& entry->posX > 0)
		{
			spritesFound.push_back(*entry);
			if (spritesFound.size() >= 10) // exit if we already found 10 sprites to render
				break;
		}
		entry++;
	}

	spritesFound2 = spritesFound;
	// 2 -reverse sort sprites so that the first (in X drawn order) will be drawn fully
	// CHANGE : Priorities : we will draw first the greatest X so the lowest X overlap them
	std::function<bool(struct OAM_entry& a, struct OAM_entry& b)> sortFunction;
	if (!Gameboy::bIsCGB || Gameboy::bCGBIsInCompatMode)
		sortFunction = [&spritesFound2](struct OAM_entry& a, struct OAM_entry& b){
			if (a.posX != b.posX)
				return a.posX > b.posX;
			else {
				// if same X, we pick the sprites earliest in OAM
				auto ndxA = std::find(spritesFound2.begin(), spritesFound2.end(), a) - spritesFound2.begin();
				auto ndxB = std::find(spritesFound2.begin(), spritesFound2.end(), b) - spritesFound2.begin();
				return ndxA > ndxB;
			}
		};
	else
		sortFunction = [&spritesFound2](struct OAM_entry& a, struct OAM_entry& b){
			// if same X, we pick the sprites earliest in OAM
			auto ndxA = std::find(spritesFound2.begin(), spritesFound2.end(), a) - spritesFound2.begin();
			auto ndxB = std::find(spritesFound2.begin(), spritesFound2.end(), b) - spritesFound2.begin();
			return ndxA > ndxB;
		};
	std::sort(spritesFound.begin(), spritesFound.end(), sortFunction);

	// 3 - copy sprite color into the whole line
	for (struct OAM_entry spriteEntry : spritesFound)
	{
		bool bIsAboveBG = !spriteEntry.getBGOverWindow();
		Sprite sprite = Sprite(spriteEntry, spriteHeight);

		unsigned char yOffset = M_LY - (spriteEntry.posY - 16); // (posY - 16) is where the first line of the sprite should be drawn, this is the current offset inside the sprite, can be higher than 8 if spriteHeight == 16
		if (spriteEntry.getFlipY()) // reverse offset if flipped
			sprite.flipY();
		if (spriteEntry.getFlipX())
			sprite.flipX();

		// fetch the 8 pixel of the sprite in a tmp buffer
		std::array<short, 8> coloredSpriteLine = sprite.getColoredLine(yOffset);
		std::array<short, 8> colorCodeSpriteLine = sprite.getLineColorCode(yOffset);
		// copy the sprite on the line
		for (int x=spriteEntry.posX - 8, i=0; (x < spriteEntry.posX) && (x < PIXEL_PER_LINE); x++, i++)
		{
			if (x >= 0)
			{
				if (spriteLine[x].bIsSet && colorCodeSpriteLine[i] == 0)
					continue;
				spriteLine[x] = {coloredSpriteLine[i], bIsAboveBG,
				colorCodeSpriteLine[i], colorCodeSpriteLine[i] != 0}; // might need to check color 0 
														   // which is not winning over BG
														   // is it after or before palette ?
														   // (i think its after, then what about 
														   // CGB)
			}
		}
	}
	return spriteLine;
}

struct TilePixels Ppu::getBackgroundTile(unsigned char xOffsetInMap, unsigned char yOffsetInMap)
{
    unsigned int BGMap  = BIT(M_LCDC, 3) ? 0x9C00 : 0x9800;
    unsigned int BGDataAddress = BIT(M_LCDC, 4) ? 0x8000 : 0x8800;

	// this is to loop back to 0 when we overflow the background map with the viewport
	yOffsetInMap %= 32;
	xOffsetInMap %= 32;

    unsigned int addrInMap = BGMap + xOffsetInMap + (yOffsetInMap * 32);
    int tileNumber = mem.getVram()[addrInMap - 0x8000];
    return getTile(BGDataAddress, tileNumber, addrInMap);
}

// TODO this has been broken
TilePixels Ppu::getWindowTile(unsigned int xOffsetInMap, unsigned int yOffsetInMap)
{
  unsigned int windowMap = BIT(M_LCDC, 6) ? 0x9C00 : 0x9800;
  unsigned int windowDataAddress = BIT(M_LCDC, 4) ? 0x8000 : 0x8800;

  unsigned int addressInMap = windowMap + xOffsetInMap + (yOffsetInMap * 32);

  // condition is there to see if we need to loop over 1024 it should never happen if i understood correctly
  if (xOffsetInMap > 0x3ff || yOffsetInMap > 0x3ff)
	  std::cerr << "offset in window tile is superior to 1024 to fetch the tile data and is: " <<  xOffsetInMap + yOffsetInMap << std::endl;
  int tileNumber = mem.getVram()[addressInMap - 0x8000];
  return getTile(windowDataAddress, tileNumber, addressInMap);
}

void	Ppu::resetWindowCounter()
{
	windowCounter = 0;
}
