/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Debugger.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmariott <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/08 19:04:02 by lmariott          #+#    #+#             */
/*   Updated: 2023/02/02 18:19:41 by lmariott         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GBMU_DEBUGGER_H
#define GBMU_DEBUGGER_H

/*
** Debugger implementation
** Used to show some debug information in UserInterface
** Also used for controlling Gameboy for break/step
*/

#define MAIN_SCREEN_SCALE 4
#define VRAM_SCREEN_SCALE 2
#define BG_SCREEN_SCALE 2


#include <SDL2/SDL.h>

enum class DebuggerState : int {
    PAUSED,
    RUNNING,
	ONCE,
	ONCE_FRAME,
	ONCE_LINE
};

class Debugger {
public:
    static DebuggerState state;
    static int fps;
    static void registers();
    static void hexdump();

	static void		lockTexture();
	static void		destroyTexture();
	static bool		createTexture(bool, SDL_Renderer *);
	static void		drawBG(int mapAddr);
	static void		drawVRam(bool bIsCGB);
	static void		drawPalettes();
	static void		reset();

	static int		mapAddr;

	static SDL_Texture*	BGTexture;
	static void*		BGPixels;
	static int		BGPitch;

	static SDL_Texture*	VRamTexture;
	static void*		VramPixels;
	static int		VramPitch;

};


#endif //GBMU_DEBUGGER_H
