#include "Joypad.hpp"
#include "Gameboy.hpp"
#include "define.hpp"
#include "CpuStackTrace.hpp"

// TODO We need to have 8 bit for each input
// so we cannot write into memory directly, need a temp buffer,
// we need to update everytime 0xFF00 bit 0-3 with respect to bit 4-5
// the update of bit 4-5 is done by sw ROM
// so, at every write to bit 4 or 5, cpy bit 0-3 accordingly

#define A_BUTTON SDLK_s
#define B_BUTTON SDLK_a
#define SELECT_BUTTON SDLK_LSHIFT
#define START_BUTTON SDLK_RETURN
#define JOYPAD_MSK_START	0x80
#define JOYPAD_MSK_SELECT 	0x40
#define JOYPAD_MSK_B 		0x20
#define JOYPAD_MSK_A 		0x10
#define JOYPAD_MSK_DOWN	    0x08
#define JOYPAD_MSK_UP 		0x04
#define JOYPAD_MSK_LEFT 	0x02
#define JOYPAD_MSK_RIGHT 	0x01

unsigned char Joypad::input = 0;

void	Joypad::refresh()
{
	// bit = 0 is select, not 1
	if (BIT(mem[0xFF00], 5) == 0)
	{
		// Action button
		unsigned char newValue = (mem[0xFF00] & 0xF0) | (~(input >> 4) & 0x0F);
		mem.supervisorWrite(0xFF00, newValue);
	}
	else if (BIT(mem[0xFF00], 4) == 0)
	{
		// Direction buttons
		unsigned char newValue = (mem[0xFF00] & 0xF0) | (~input & 0x0F);
		mem.supervisorWrite(0xFF00, newValue);
	}
}

unsigned char Joypad::get()
{
	return (input);
}

void Joypad::handleEvent(SDL_Event *ev)
{
	if (!(ev->type == SDL_KEYUP || ev->type == SDL_KEYDOWN))
		return;
	const SDL_KeyboardEvent keyboardEv = ev->key;

	bool bSomethingChanged = false;
	if (keyboardEv.type == SDL_KEYDOWN && keyboardEv.repeat == 0) {
		switch (keyboardEv.keysym.sym) {
			case SDLK_DOWN:
			case SDLK_KP_5:
				input |= JOYPAD_MSK_DOWN;
				bSomethingChanged = true;
				break;
			case SDLK_UP:
			case SDLK_KP_8:
				input |= JOYPAD_MSK_UP;
				bSomethingChanged = true;
				break;
			case SDLK_LEFT:
			case SDLK_KP_4:
				input |= JOYPAD_MSK_LEFT;
				bSomethingChanged = true;
				break;
			case SDLK_RIGHT:
			case SDLK_KP_6:
				input |= JOYPAD_MSK_RIGHT;
				bSomethingChanged = true;
				break;
			case A_BUTTON: // A
				input |= JOYPAD_MSK_A;
				bSomethingChanged = true;
				break;
			case B_BUTTON: // B
				input |= JOYPAD_MSK_B;
				bSomethingChanged = true;
				break;
			case START_BUTTON: // Start
				input |= JOYPAD_MSK_START;
				bSomethingChanged = true;
				break;
			case SELECT_BUTTON: // Select
				input |= JOYPAD_MSK_SELECT;
				bSomethingChanged = true;
				break;
			default:
				break;
		}
	}
	if (keyboardEv.type == SDL_KEYUP) {
		switch (keyboardEv.keysym.sym) {
			case SDLK_DOWN:
			case SDLK_KP_5:
				input &= ~JOYPAD_MSK_DOWN;
				bSomethingChanged = true;
				break;
			case SDLK_UP:
			case SDLK_KP_8:
				input &= ~JOYPAD_MSK_UP;
				bSomethingChanged = true;
				break;
			case SDLK_LEFT:
			case SDLK_KP_4:
				input &= ~JOYPAD_MSK_LEFT;
				bSomethingChanged = true;
				break;
			case SDLK_RIGHT:
			case SDLK_KP_6:
				input &= ~JOYPAD_MSK_RIGHT;
				bSomethingChanged = true;
				break;
			case A_BUTTON: // A
				input &= ~JOYPAD_MSK_A;
				bSomethingChanged = true;
				break;
			case B_BUTTON: // B
				input &= ~JOYPAD_MSK_B;
				bSomethingChanged = true;
				break;
			case START_BUTTON: // Start
				input &= ~JOYPAD_MSK_START;
				bSomethingChanged = true;
				break;
			case SELECT_BUTTON: // Select
				input &= ~JOYPAD_MSK_SELECT;
				bSomethingChanged = true;
				break;
			default:
				break;
		}
	}
	if (bSomethingChanged)
		refresh();
	if (keyboardEv.type == SDL_KEYDOWN && keyboardEv.keysym.sym == SDLK_l)
		CpuStackTrace::autoPrint = true;
	if (keyboardEv.type == SDL_KEYDOWN && keyboardEv.keysym.sym == SDLK_k)
		CpuStackTrace::autoPrint = false;
}
