/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UserInterface.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmariott <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/06 22:44:23 by lmariott          #+#    #+#             */
/*   Updated: 2023/02/03 14:49:12 by lmariott         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UserInterface.hpp"
#include "Debugger.hpp"
#include <thread>
#include "Joypad.hpp"
#include "Screen.hpp"
#include <fstream>
#include "imgui/imgui.h"
#include <iostream>
#include "Gameboy.hpp"
#include "define.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

bool UserInterface::showVram = false;
bool UserInterface::showBG = false;
bool UserInterface::showHexdump = false;
bool UserInterface::showRegisters = false;
bool UserInterface::showPalettes = false;
bool UserInterface::bIsError = false;
bool UserInterface::bIsFatalError = false;
bool UserInterface::forceMode = false;
bool UserInterface::forceCGB = true;
int UserInterface::volume = 100;
SDL_Window*	UserInterface::uiWindow = nullptr;
SDL_Renderer*	UserInterface::uiRenderer = nullptr;
std::string	UserInterface::romFolderPath = "";
std::string	UserInterface::errMsg = "";
float UserInterface::framesToSkipRender = 0;
float UserInterface::framesToSkipUpdate = 0;

#define DEFAULT_ROM_PATH "./roms/42roms/Super Mario Land 2"

void UserInterface::TexturetoImage(SDL_Texture * Texture)
{
	int width;
	int height;

	SDL_SetRenderTarget(uiRenderer, Texture);
	SDL_UnlockTexture(Texture);
	SDL_QueryTexture(Texture, nullptr, nullptr, &width, &height);
	ImGui::Image((void*)(intptr_t)Texture, ImVec2(width, height));
	//	SDL_RenderCopy(uiRenderer, Texture, NULL, NULL);
	SDL_SetRenderTarget(uiRenderer, nullptr);
}

void UserInterface::destroy()
{
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	// Debugger::destroyTexture();
	SDL_DestroyRenderer(uiRenderer);
	SDL_DestroyWindow(uiWindow);
	SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
	SDL_Quit();
	SDL_TLSCleanup();
}

void UserInterface::reset()
{
	UserInterface::framesToSkipRender = 0;
	UserInterface::framesToSkipUpdate = 0;
}

bool UserInterface::create()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		std::cerr <<"Error SDL_Init! "<< SDL_GetError() << std::endl;
		return false;
	}

	auto window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	uiWindow = SDL_CreateWindow("GBMU",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			1980,
			1024,
			window_flags);
	if (!uiWindow) {
		std::cerr <<"Error SDL_CreateWindow! "<< SDL_GetError() << std::endl;
		return (false);
	}


	uiRenderer = SDL_CreateRenderer(uiWindow, -1,  SDL_RENDERER_ACCELERATED); //SDL_RENDERER_PRESENTVSYNC
	if (!uiRenderer)
	{
		std::cerr <<"Error SDL_CreateRenderer : "<< SDL_GetError() << std::endl;
		return false;
	}

	// if (!Debugger::createTexture(bIsCGB, uiRenderer))
	// 	return false;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(uiWindow, uiRenderer);
	ImGui_ImplSDLRenderer_Init(uiRenderer);
	SDL_RenderClear(uiRenderer);

	char *home;
	if (!(home = getenv("HOME")))
	{
		std::cerr <<"Error : env $HOME missing"<< std::endl;
		return false;
	}
	romFolderPath = home;

	return (true);
}

void	UserInterface::newFrame()
{
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	SDL_RenderClear(uiRenderer);
	if (Gameboy::bIsInit) {
		Screen::lockTexture();
		Debugger::lockTexture();
	}
}

void	UserInterface::clear()
{
	const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImGui::Render();
	SDL_SetRenderDrawColor(uiRenderer,
			clear_color.x * 255, clear_color.y * 255,
			clear_color.z * 255, clear_color.w * 255);
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(UserInterface::uiRenderer);
}

void UserInterface::showGameboyWindow()
{
	ImGui::Begin("GBMU");
	if (ImGui::Button(showBG ? "Hide BG" : "Show BG")) {
		showBG = !showBG;
	}
	ImGui::SameLine();
	if (ImGui::Button(showVram ? "Hide Vram" : "Show Vram")) {
		showVram = !showVram;
	}
	ImGui::SameLine();
	if (ImGui::Button(showHexdump ? "Hide hexdump" : "Show hexdump")) {
		showHexdump = !showHexdump;
	}
	ImGui::SameLine();
	if (ImGui::Button(showRegisters ? "Hide registers" : "Show registers")) {
		showRegisters = !showRegisters;
	}
	if (Gameboy::bIsCGB) {
		ImGui::SameLine();
		if (ImGui::Button(showPalettes ? "Hide palettes" : "Show palettes")) {
			showPalettes = !showPalettes;
		}
	}
	if (ImGui::Button("Reset")) {
		Gameboy::clear();
		ImGui::End();
		return ;
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Rom")) {
		fileExplorer();
		ImGui::End();
		return ;
	}
	ImGui::Text("Change this settings reset game : ");
	ImGui::SameLine();
	if (ImGui::Button("Autodetect CGB/DMG mode")) {
		Gameboy::clear();
		forceMode = false;
		ImGui::End();
		return ;
	}
	ImGui::SameLine();
	if (ImGui::Button("Force CGB mode")) {
		forceMode = true;
		forceCGB = true;
		Gameboy::clear();
		ImGui::End();
		return ;
	}
	ImGui::SameLine();
	if (ImGui::Button("Force DMG mode")) {
		forceMode = true;
		forceCGB = false;
		Gameboy::clear();
		ImGui::End();
		return ;
	}
	if (forceMode) {
		ImGui::Text("Forcing");
		ImGui::SameLine();
		ImGui::Text((forceCGB ? "CGB mode." : "DMG mode."));
	} else {
		ImGui::Text("Using autodetect DMG/CGB mode.");
	}
	ImGui::NewLine();
	if (ImGui::Button("Reset FPS")) {
		Debugger::fps = 60;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(180);
	if (ImGui::SliderInt("FPS", &Debugger::fps, 1, 300))
	{
		framesToSkipRender = 0;
		framesToSkipUpdate = 0;
		if (Debugger::fps > 300)
			Debugger::fps = 300;
		if (Debugger::fps < 1)
			Debugger::fps = 1;
	}
	if (ImGui::SliderInt("Volume", &UserInterface::volume, 0, 100))
	{
		if (volume > 100)
			volume = 100;
		if (volume < 0)
			volume = 0;
	}
	if (ImGui::Button(  Debugger::state == DebuggerState::RUNNING ? "PAUSE" : "RUN")) {
		Debugger::state = (Debugger::state == DebuggerState::PAUSED) ? DebuggerState::RUNNING : DebuggerState::PAUSED;
	}
	ImGui::SameLine();
	if (ImGui::Button("Next step")) {
		Debugger::state = DebuggerState::ONCE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Next frame")) {
		Debugger::state = DebuggerState::ONCE_FRAME;
	}
	ImGui::SameLine();
	if (ImGui::Button("Next line")) {
		Debugger::state = DebuggerState::ONCE_LINE;
	}
	if (ImGui::Button("Save State")) {
		if (Gameboy::bIsInit) {
			Gameboy::saveState();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Load State")) {
		if (Gameboy::bIsInit) {
			Gameboy::loadSaveState();
		}
	}
	if (ImGui::Button("++day")) {
		if (Gameboy::bIsInit && Gameboy::getMem().mbc->type == 3) {
			MBC3 *ptr = dynamic_cast<MBC3*>(Gameboy::getMem().mbc);
      ptr->addDay();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("++hour")) {
		if (Gameboy::bIsInit && Gameboy::getMem().mbc->type == 3) {
			MBC3 *ptr = dynamic_cast<MBC3*>(Gameboy::getMem().mbc);
      ptr->addHour();
		}
	}
	ImGui::End();
}

void UserInterface::showSubWindows(bool bShouldComputeScreen)
{
	if (!Gameboy::bIsInit)
		return ;
	if (showVram)
	{
		ImGui::Begin("Vram");
		if (bShouldComputeScreen)
			Debugger::drawVRam(Gameboy::bIsCGB);
		UserInterface::TexturetoImage(Debugger::VRamTexture);
		ImGui::End();
	}
	if (showBG)
	{
		ImGui::Begin("Background/Window Map");
		ImGui::Text("Displaying map address: %04x", Debugger::mapAddr);
		ImGui::Text("BG map address:         %04x", (BIT(M_LCDC, 3) ? 0x9C00 : 0x9800));
		ImGui::Text("Window map address:     %04x", (BIT(M_LCDC, 6) ? 0x9C00 : 0x9800));
		if (ImGui::Button("Draw Window")) {
			Debugger::mapAddr = (BIT(M_LCDC, 6) ? 0x9C00 : 0x9800);
		}
		ImGui::SameLine();
		if (ImGui::Button("Draw Background")) {
			Debugger::mapAddr = (BIT(M_LCDC, 3) ? 0x9C00 : 0x9800);
		}
		ImGui::SameLine();
		if (ImGui::Button("Draw 0x9800")) {
			Debugger::mapAddr = 0x9800;
		}
		ImGui::SameLine();
		if (ImGui::Button("Draw 0x9C00")) {
			Debugger::mapAddr = 0x9C00;
		}
		ImGui::SameLine();
		if (ImGui::Button("Draw 0x8000")) {
			Debugger::mapAddr = 0x8000;
		}
		if (bShouldComputeScreen)
			Debugger::drawBG(Debugger::mapAddr);
		UserInterface::TexturetoImage(Debugger::BGTexture);
		ImGui::End();
	}
	if (showPalettes)
	{
		if (Gameboy::bIsCGB) {
			ImGui::Begin("Palettes");
			Debugger::drawPalettes();
			ImGui::End();
		}
		else {
			showPalettes = false;
		}
	}

	if (showRegisters)
	{
		Debugger::registers();
	}

	if (showHexdump)
	{
		Debugger::hexdump();
	}
	if (Gameboy::bIsPathValid) {
		ImGui::Begin("PPU");
		UserInterface::TexturetoImage(Screen::ppuTexture);
		ImGui::End();
	}
}

bool UserInterface::loop()
{
	std::chrono::time_point<std::chrono::system_clock> lastTimeGameboyUpdated;
	std::chrono::microseconds hardwareFrameTime(1'000'000 / 60);

	while (!Gameboy::quit)
	{
		std::chrono::microseconds frametime(1'000'000 / Debugger::fps);
		auto beginFrameTime = std::chrono::system_clock::now();

		UserInterface::newFrame();

		if (bIsError) {
			errorWindow();
		}
		else
		{
			UserInterface::showGameboyWindow();
			if (!bIsError && !Gameboy::bIsInit) {
				if (Gameboy::bIsPathValid) {
					Gameboy::bIsInit = Gameboy::loadRom();
					if (!Gameboy::bIsInit) {
						Gameboy::bIsPathValid = false;
					}
				}
			}
			Gameboy::Step step = Gameboy::Step::full;
			if (Gameboy::bIsInit && !UserInterface::bIsError)
			{
				bool bShouldUpdateGameboy = Debugger::state != DebuggerState::PAUSED;
				//printf("framesToSkipUpdate: %f\n", framesToSkipUpdate);
				if (bShouldUpdateGameboy)
				{
					step = Gameboy::Step::full;
					if (Debugger::state == DebuggerState::ONCE)
						step = Gameboy::Step::oneInstruction;
					if (Debugger::state == DebuggerState::ONCE_LINE)
						step = Gameboy::Step::oneLine;
					//printf("rendering with updateGameboy: %d and skipping:%f\n",
							//bShouldUpdateGameboy, framesToSkipRender);
					if (step == Gameboy::Step::full && Debugger::state != DebuggerState::ONCE_FRAME)
					{
						if (framesToSkipUpdate < 1)
						{
							do
							{
								framesToSkipRender -= 1;
								Gameboy::execFrame(step, framesToSkipRender < 1);
								//printf("execframe %s\n", framesToSkipRender < 1 ? "WITH RENDER" : "WITHOUT RENDER");
							} while (framesToSkipRender >= 1);
							lastTimeGameboyUpdated = std::chrono::system_clock::now();
							framesToSkipUpdate += (60.f / (float)Debugger::fps) - 1;
							framesToSkipRender += ((float)Debugger::fps / 60.f);
						}
						else
							framesToSkipUpdate -= 1;
					}
					else
					{
						framesToSkipUpdate = 0;
						framesToSkipRender = 0;
						Gameboy::execFrame(step, true);
					}
				}
				if (Debugger::state == DebuggerState::ONCE ||
						Debugger::state == DebuggerState::ONCE_FRAME ||
						Debugger::state == DebuggerState::ONCE_LINE) {
					Debugger::state = DebuggerState::PAUSED;
				}
				if (!UserInterface::bIsError)
				{
					UserInterface::showSubWindows(bShouldUpdateGameboy);
				}
			}
		}


		auto handleEvents = [](){
			SDL_Event event;
			while (SDL_PollEvent(&event))
				UserInterface::handleEvent(&event);
		};
		handleEvents();
		std::chrono::microseconds timeTakenForFrame = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - beginFrameTime);
		/* Sleep : TODO calculate compute time to have a frame rate ~60fps*/
		if (timeTakenForFrame.count() < hardwareFrameTime.count())
		{
			//std::cout << std::dec <<  "timeTakenForFrame : " << timeTakenForFrame.count() << std::endl;
			//std::cout << "sleeping for: " << hardwareFrameTime.count() - timeTakenForFrame.count() << std::endl;
			std::this_thread::sleep_for(hardwareFrameTime - timeTakenForFrame);
			//while (hardwareFrameTime.count() > timeTakenForFrame.count())
			{
				//timeTakenForFrame = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - beginFrameTime);
				//std::this_thread::sleep_for(std::min(hardwareFrameTime - timeTakenForFrame, std::chrono::microseconds(500)));
			}
		}

		UserInterface::clear();
	}
	Gameboy::clear();
	destroy();
	return (true);
}

void	UserInterface::throwError(const char *msg, bool fatal)
{
	bIsError = true;
	bIsFatalError = fatal;
	errMsg = msg;
}

void	UserInterface::errorWindow()
{
	ImGui::Begin(bIsFatalError ? "FATAL ERROR" : "ERROR");
	ImGui::Text("%s", errMsg.c_str());
	if (ImGui::Button("OK")) {
		if (bIsFatalError) {
			Gameboy::quit = true;
		}
		UserInterface::bIsError = false;
	}
	ImGui::End();
}

void	UserInterface::fileExplorer()
{
	// Yes, it's big
	char filename[8192] = {0};
	FILE *f = popen("zenity --file-selection --file-filter=\"Gameboy Rom | *.gb *.gbc\" --filename=" DEFAULT_ROM_PATH, "r");
    while (!feof(f)) {
       if (fgets(filename, 8192, f) == nullptr) {
           if (ferror(f)) {
               throwError("Zenity: File Explorer can't open this file", true);
               pclose(f);
               return ;
           }
       }
    }
	if (filename[0] == 0) {
		// throwError("Please select a ROM", false);
		// Gameboy::bIsPathValid = false;
    	pclose(f);
		return ;
	}
	for (int i = 0 ; i < 8192 ; i++) {
		if (filename[i] == '\n') {
			filename[i] = 0;
			break ;
		}
	}
	Gameboy::clear();
	filename[8191] = 0; // Ensure it last 0
	Gameboy::path = filename;
	Gameboy::bIsPathValid = true;
    pclose(f);
/*
** TODO old file explorer , to remove ?
**	DIR *dir;
**	struct dirent *entry;
**	struct stat info;
**
**	dir = opendir(UserInterface::romFolderPath.c_str());
**	while ((entry = readdir(dir)) != NULL)
**	{
**		std::string path = UserInterface::romFolderPath + "/" + std::string(entry->d_name);
**		stat(path.c_str(),&info);
**		if ((entry->d_name[0] != '.' || !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) &&
**				(!strcmp(&entry->d_name[strlen(entry->d_name) - 4], ".gbc")
**				 || !strcmp(&entry->d_name[strlen(entry->d_name) - 3], ".gb")
**				 || S_ISDIR(info.st_mode)))
**		{
**
**			if (ImGui::Button(entry->d_name)) {
**				if (S_ISDIR(info.st_mode))
**				{
**					UserInterface::romFolderPath = path;
**				}
**				else
**				{
**					Gameboy::path = path;
**					Gameboy::bIsPathValid = true;
**				}
**			}
**		}
**	}
**	closedir(dir);
*/

}

void	UserInterface::handleEvent(SDL_Event *ev)
{
	ImGui_ImplSDL2_ProcessEvent(ev);
	if (ev->type == SDL_QUIT)
		Gameboy::quit = true;
	if (ev->type == SDL_WINDOWEVENT) {
		switch (ev->window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				if (ev->window.windowID
						== SDL_GetWindowID(uiWindow)) {
					Gameboy::quit = true;
				}
				break;
		}
	}
	if (ev->type == SDL_KEYDOWN)
	{
		if (ev->key.keysym.sym == SDLK_ESCAPE) {
			Gameboy::saveState();
			Gameboy::clear();
			Gameboy::bIsPathValid = false;
		}
	}
	if (ev->type == SDL_DROPFILE)
	{
		Gameboy::clear();
		Gameboy::path = ev->drop.file;
		Gameboy::bIsPathValid = true;
		SDL_free(ev->drop.file);
	}
	if (!bIsError && Gameboy::bIsInit)
		Joypad::handleEvent(ev);

}
