#ifndef NOISE_HPP
#define NOISE_HPP

#include "define.hpp"
#include <SDL2/SDL_audio.h>


class Noise
{
public:
    int channel;

    // NR41
    int length_timer;

    int current_length_timer;

    // NR42
    int initial_volume;
    bool envelopeDirection;
    int sweepPace;

    // NR43
    int clockShift;
    bool LFSRwidth;
    int clockDivider;

    // NR44
    bool trigger;
    bool length_enable;

    SDL_AudioSpec obtainedSpec;
    SDL_AudioSpec desiredSpec;

    int volumeReduction;
    int iterations;

    int sample;
    bool DACenable;

    int regulator;

    void tick();
    void triggerChannel();

    void clear();

    const static int samples_per_length = SAMPLING_RATE/256;

    int length_count;

	static Noise* loadNoise(int chan);
    Noise(int chan);
    ~Noise();
};


#endif
