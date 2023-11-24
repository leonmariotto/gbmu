/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Joypad.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 20:46:19 by nallani           #+#    #+#             */
/*   Updated: 2023/01/05 22:50:59 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef JOYPAD_CLASS_H
# define JOYPAD_CLASS_H

/*
** Joypad
** Use SDL to map the input Gameboy joypad on keyboard.
** It directly update the Joypad register in Mem.
*/

#include <SDL2/SDL.h>

class Joypad {
public:
	static void handleEvent(SDL_Event *ev);
	static unsigned char get();
	static void refresh();
private:
	static unsigned char input;
};

#endif
