#ifndef WAVEFORM_HPP
#define WAVEFORM_HPP

#include <SDL2/SDL_audio.h>
#include "define.hpp"

class Waveform
{
public:
    int channel;
    int step;

    // NR30
    bool DACenable;

    // NR31
    unsigned char length_timer;

    int current_length_timer;

    // NR32
    unsigned char volume;

    // NR33 - NR34
    bool trigger;
    int wavelength;
    bool length_enable;
    
    unsigned char waveform[16];

    int iterations;

    int regulator;


    SDL_AudioSpec obtainedSpec;
    SDL_AudioSpec desiredSpec;

    void tick(void);

    const static int samples_per_length = SAMPLING_RATE/64;

    int length_count;

    void triggerChannel();
    void clear();

	static Waveform* loadWaveform(int chan);
    Waveform(int chan);
    ~Waveform();
};


#endif
