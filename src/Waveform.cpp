#include "Waveform.hpp"
#include <iostream>
#include "Gameboy.hpp"

void Waveform::tick() {
    // if (trigger)
    //     return;


    length_enable = BIT(mem[NR34], 6);
    wavelength = mem[NR33] | ((mem[NR34] & 0b111) << 8);
    DACenable = BIT(mem[NR30], 7);
    volume = (mem[NR32] & 0b01100000) >> 5;

    switch (volume)
    {
    case 0:
    case 2:
    case 4:
        break;
    case 1:
        volume = 4;
        break;
    case 3:
        volume = 1;
        break;
    
    default:
        std::cout << "Volume : " << std::dec << (int)volume <<"\n";
        throw "Wrong volume specified for Waveform channel\n";
    }
}

void Waveform::triggerChannel() {
    trigger = true;
    current_length_timer = length_timer;
    memcpy(waveform, &mem[0xFF30], 16);
    wavelength = mem[NR33] | ((mem[NR34] & 0b111) << 8);
    length_timer = mem[NR31];

    regulator = 0;

    volume = (mem[NR32] & 0b01100000) >> 5;

    switch (volume)
    {
    case 0:
    case 2:
    case 4:
        break;
    case 1:
        volume = 4;
        break;
    case 3:
        volume = 1;
        break;
    
    default:
        std::cout << "Volume : " << std::dec << (int)volume <<"\n";
        throw "Wrong volume specified for Waveform channel\n";
    }
    // std::cout << "Channel 3 triggered\n";
    // std::cout << std::hex << (int)mem[NR31] << " - " << (int)mem[NR32] << " - " << (int)mem[NR33] << " - " << (int)mem[NR34] << "\n";
    // std::cout << "\tlength enable : " << std::dec << (int)length_enable << "\n";
    // std::cout << "\tlength timer : " << std::dec << (int)length_timer << "\n";
    // std::cout << "\tvolume : " << std::dec << (int)volume << "\n";
}

void Waveform::clear() {
    trigger = false;
    mem[NR30] = 0;
    mem[NR31] = 0;
    mem[NR32] = 0;
    mem[NR33] = 0;
    mem[NR34] = 0;
}

Waveform* Waveform::loadWaveform(int chan)
{
	return new Waveform(chan);
}

Waveform::Waveform(int chan)
: channel(chan), step(0), volume(0), trigger(false), iterations(0), length_count(0)
{
    std::cout << "Waveform channel " << chan << " was created\n";
    if (chan != 3) {
        throw "Wrong channel was specified for Waveform channel";
	}
	DACenable = 0;
}

Waveform::~Waveform()
{
}
