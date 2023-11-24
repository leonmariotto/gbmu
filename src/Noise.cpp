#include "Noise.hpp"
#include "Gameboy.hpp"
#include <iostream>


void Noise::tick() {
    // if (trigger)
    //     return ;

    length_enable = BIT(mem[NR44], 6);
    DACenable = (~(0b111) & mem[NR42]) != 0;
}

void Noise::triggerChannel() {

    trigger = true;
    length_count = 0;
    length_enable = BIT(mem[NR44], 6);
    length_timer = (mem[NR41] & 0b111111);
    clockShift = (mem[NR43] & 0b11110000) >> 4;
    LFSRwidth = BIT(mem[NR43], 3);
    clockDivider = (mem[NR43] & 0b111);
    current_length_timer = length_timer;
    initial_volume = (mem[NR42] & 0b11110000) >> 4;
    envelopeDirection = BIT(mem[NR42], 3);
    sweepPace = mem[NR42] & 0b111;
    volumeReduction = 0;
    regulator = 0;


    // std::cout << "Channel 4 triggered mais " << (int)(!(!BIT(mem[NR51], 3) && !BIT(mem[NR51], 7))) << "\n";
    // std::cout << "\tlength enable : " << length_enable << "\n";
    // std::cout << "\tlength timer : " << length_timer << "\n";
    // std::cout << "\tvolume : " << initial_volume << "\n";
    // std::cout << "\tvolume sweep direction : " << envelopeDirection << "\n";
    // std::cout << "\tvolume sweep pace : " << sweepPace << "\n";
}

void Noise::clear() {
    trigger = false;

    mem[NR41] = 0;
    mem[NR42] = 0;
    mem[NR43] = 0;
    mem[NR44] = 0;
}

Noise* Noise::loadNoise(int chan)
{
	return new Noise(chan);
}

Noise::Noise(int chan)
: channel(chan), initial_volume(0), trigger(false), iterations(0), sample(rand() % 101), length_count(0)
{
    std::cout << "Noise channel " << chan << " was created\n";
    if (chan != 4) {
        throw "Wrong channel was specified for Noise channel";
	}
	DACenable = 0;
}

Noise::~Noise()
{
}
