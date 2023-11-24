#include "SquareWave.hpp"
#include "Gameboy.hpp"
#include <iostream>

void SquareWave::tick() {

    switch (channel)
    {
    case 1:
        channel_1_tick();
        break;
    case 2:
        channel_2_tick();
        break;
    default:
        throw "Trying to tick wrong SquareWave channel";
    }
}

void SquareWave::channel_2_tick() {

    lengthEnable = BIT(mem[NR24], 6);
    // waveLength = ((mem[NR24] & 0b111) << 8) | mem[NR23];

    DACenable = (~(0b111) & mem[NR22]) != 0;

    // if (lengthEnable)
        // std::cout << "Channel 2 tick : " << waveLength << "\n";
    // std::cout << "CHANNEL 2 TICK : " << lengthEnable << " et " << volumeSweepPace << " avec " << envelopeDirection << "\n";


    switch (waveDuty)
    {
    case 0:
        wave = 0b00000001;
        break;
    case 1:
        wave = 0b10000001;
        break;
    case 2:
        wave = 0b10000111;
        break;
    case 3:
        wave = 0b11100111;
        break;
    
    default:
        throw "Wrong waveDuty specified for SquareWave";
    }
}

void SquareWave::popEntry(entry val) {
    step = 0;

    if (channel == 1) {
        waveSweepPace = val.waveSweepPace;
        waveSweepDirection = val.waveSweepDirection;
        // waveSweepSlope = val.waveSweepSlope;
    }

    waveDuty = val.waveDuty;
    length_timer = val.length_timer;
    current_length_timer = length_timer;

    initialVolume = val.initialVolume;
    envelopeDirection = val.envelopeDirection;
    volumeSweepPace = val.volumeSweepPace;

    trigger = true;
    // if (channel == 1)
    //     lengthEnable = BIT(mem[NR14], 6);
    // else
    //     lengthEnable = BIT(mem[NR24], 6);

    waveLength = val.waveLength;
    wavelengthSweepValue = 0;

    ticks = 0;
    iterations = 0;

    length_count = 0;
    volume = initialVolume;
    volumeReduction = 0;

    switch (waveDuty)
    {
    case 0:
        wave = 0b00000001;
        break;
    case 1:
        wave = 0b10000001;
        break;
    case 2:
        wave = 0b10000111;
        break;
    case 3:
        wave = 0b11100111;
        break;
    
    default:
        throw "Wrong waveDuty specified for SquareWave";
    }
    
}

void SquareWave::clear() {
    step = 0;

    trigger = false;
    
    if (channel == 1) {
        mem[NR10] = 0;
        mem[NR11] = 0;
        mem[NR12] = 0;
        mem[NR13] = 0;
        mem[NR14] = 0;
    } else {
        mem[NR21] = 0;
        mem[NR22] = 0;
        mem[NR23] = 0;
        mem[NR24] = 0;
    }

    queue = std::queue<entry>();
}

void SquareWave::triggerChannel() {
    // if (trigger) return;

    entry tmp;
    if (channel == 2) {
        // std::cout << "Channel " << channel << " triggered : \n";

        tmp.length_timer = (mem[NR21] & 0b00111111);
        tmp.volumeSweepPace = (mem[NR22] & 0b00000111);
        tmp.initialVolume = (mem[NR22] & 0b11110000) >> 4;
        tmp.envelopeDirection = BIT(mem[NR22], 3);
        tmp.waveLength = ((mem[NR24] & 0b111) << 8) | mem[NR23];
        tmp.waveDuty = (mem[NR21] & 0b11000000) >> 6;
        tmp.length_enable = BIT(mem[NR14], 6);
        
        // std::cout << std::hex << (int)mem[NR21] << " - " << (int)mem[NR22] << " - " << (int)mem[NR23] << " - " << (int)(mem[NR24] | 0b00111000) << "\n";
    }
    else {
        // std::cout << "Channel " << channel << " triggered : \n";
        tmp.length_timer = (mem[NR11] & 0b00111111);
        tmp.volumeSweepPace = (mem[NR12] & 0b00000111);
        tmp.initialVolume = (mem[NR12] & 0b11110000) >> 4;
        tmp.envelopeDirection = BIT(mem[NR12], 3);
        tmp.waveLength = ((mem[NR14] & 0b111) << 8) | mem[NR13];
        tmp.waveDuty = (mem[NR11] & 0b11000000) >> 6;
        tmp.length_enable = BIT(mem[NR24], 6);

        tmp.waveSweepDirection = BIT(mem[NR10], 3);
        tmp.waveSweepPace = (mem[NR10] & 0b01110000) >> 4;
        tmp.waveSweepSlope = (mem[NR10] & 0b00000111);
        
        // std::cout << std::hex << (int)mem[NR10] << " - " << (int)mem[NR11] << " - " << (int)mem[NR12] << " - " << (int)mem[NR13] << " - " << (int)mem[NR14] << "\n";

    }

    queue.push(tmp);
    // std::cout << "\tlength enable : " << tmp.length_enable << "\n";
    // std::cout << "\tlength timer : " << tmp.length_timer << "\n";
    // std::cout << "\tvolume : " << tmp.initialVolume << "\n";
    // std::cout << "\tvolume sweep direction : " << tmp.envelopeDirection << "\n";
    // std::cout << "\tvolume sweep pace : " << tmp.volumeSweepPace << "\n";
    // std::cout << "\twave length : " << tmp.waveLength << "\n";
    // std::cout << "\twave sweep direction : " << tmp.waveSweepDirection << "\n";
    // std::cout << "\twave sweep pace : " << tmp.waveSweepPace << "\n";
    // std::cout << "\twave sweep slope : " << tmp.waveSweepSlope << "\n";
}

void SquareWave::channel_1_tick() {
    // if (trigger)
    //     return ;

    lengthEnable = BIT(mem[NR14], 6);
    // waveLength = ((mem[NR14] & 0b111) << 8) | mem[NR13];
    // volume = (mem[NR12] & 0b11110000) >> 4;

    waveSweepSlope = (mem[NR10] & 0b00000111);
    // volume = (mem[NR12] & 0b11110000) >> 4;

    DACenable = (~(0b111) & mem[NR12]) != 0;
}

void SquareWave::changeWavelength(float val) {
    waveLength = val;
    if (channel == 1) {
        mem[NR13] = ((int)val & 0xFF);
        mem[NR14] &= ~0b111;
        mem[NR14] |= ((int)val >> 8) & 0b111;
    } else {
        mem[NR23] = ((int)val & 0xFF);
        mem[NR24] &= ~0b111;
        mem[NR24] |= ((int)val >> 8) & 0b111;
    }
}

SquareWave* SquareWave::loadSquareWave(int chan)
{
	return new SquareWave(chan);
}

SquareWave::SquareWave(int chan)
: channel(chan), step(0), initialVolume(0), trigger(false), iterations(0), length_count(0)
{
    std::cout << "SquareWave channel " << chan << " was created\n";
    if (chan != 1 && chan != 2)
        throw "Wrong channel was specified for SquareWave channel";
    ticks = 0;
	waveDuty = 0;
	DACenable = 0;
}

SquareWave::~SquareWave()
{
}
