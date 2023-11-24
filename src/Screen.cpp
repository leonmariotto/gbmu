#include <iostream>
#include "Screen.hpp"
#include "Cpu.hpp"
#include "Debugger.hpp"
#include <fstream>
#include "imgui/imgui.h"


SDL_Texture	*Screen::ppuTexture = nullptr;
void		*Screen::ppuPixels = nullptr;
int 		Screen::ppuPitch = 0;

void		Screen::destroyTexture()
{
	SDL_DestroyTexture(ppuTexture);
}

int		Screen::convertColorFromCGB(int colo, bool bConvertForImGUI)
{
	unsigned char r = (colo & 0x1F);
	unsigned char g = ((colo & (0x1F << 5)) >> 5);
	unsigned char b = ((colo & (0x1F << 10)) >> 10);

	r *= 8;
	g *= 8;
	b *= 8;
	if (bConvertForImGUI)
		return (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (0xFF << 24);
	else
		return (r << 24) | (g << 16) | (b << 8);
}

void	Screen::lockTexture()
{
	if (SDL_LockTexture(ppuTexture, nullptr, &ppuPixels, &ppuPitch)) {
		throw "Could not lock ppuTexture\n";
	}
}


bool	Screen::createTexture(bool bIsCGB, SDL_Renderer* uiRenderer)
{
	(void)bIsCGB;
	ppuTexture = SDL_CreateTexture(uiRenderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			160 * MAIN_SCREEN_SCALE,
			144 * MAIN_SCREEN_SCALE);
	if (!ppuTexture) {
		std::cerr << "Erreur SDL_CreateTexture Ppu : "<< SDL_GetError() << std::endl;
		return false;
	}
	if (SDL_LockTexture(ppuTexture, nullptr, &ppuPixels, &ppuPitch)) {
		throw "Could not lock ppuTexture\n";
	}
	// bzero(ppuPixels, (160 * MAIN_SCREEN_SCALE) * 144 * MAIN_SCREEN_SCALE);
	return true;
}

void	Screen::updatePpuLine(const std::array<short, PIXEL_PER_LINE>& lineData,
		unsigned char currentLine, bool bIsCGB)
{
	for (unsigned char i = 0; i < PIXEL_PER_LINE; i++) {
		Screen::drawPoint(i, currentLine, lineData[i],
				(int*)ppuPixels, ppuPitch, MAIN_SCREEN_SCALE, bIsCGB);
	}
}


bool	Screen::drawPoint(unsigned short x, unsigned short y, const unsigned short& color, void *pixels, int pitch, int pixelScale, bool bIsCGB)
{
	x *= pixelScale;
	y *= pixelScale;
	int colorForSDL;
	if (!bIsCGB) {
		//colorForSDL = convertRGBforGB(color);
		colorForSDL = 255 - color * (255 / 3);
		colorForSDL = (colorForSDL << 24) | (colorForSDL << 16) | (colorForSDL << 8);
	}
	else {
		colorForSDL = convertColorFromCGB(color);
	}
	int *p = (int*)pixels;
	for (int i = 0 ; i < pixelScale ; i++) {
		for (int j = 0 ; j < pixelScale ; j++) {
			p[(y + j) * (pitch / 4) + (i + x)] = colorForSDL | 0xFF;
		}
	}
	return (true);
}
