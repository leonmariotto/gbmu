/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 20:52:08 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 10:35:31 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Gameboy.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

#define DEFAULT_PATH_TO_FILE "./roms/42roms/Mystic_Quest.gb"


int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	std::cout << std::setfill('0') << std::uppercase;
	std::cerr << std::setfill('0') << std::uppercase;
	std::cerr << std::hex;
	std::cout << std::hex;
	if (argc >= 2)
	{
		Gameboy::path = argv[1];
		Gameboy::bIsPathValid = true;
	}

	try {
		Gameboy::launchUserInterface();
	}
	catch (const char* e) {
		std::cerr << e << std::endl;
	}
	return (0);
}
