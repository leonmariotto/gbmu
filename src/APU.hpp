#ifndef APU_HPP
#define APU_HPP

#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>


#include "SquareWave.hpp"
#include "Waveform.hpp"
#include "Noise.hpp"

class APU
{
private:
    static unsigned char triggers[4];
    static int ticks;
    
    static int masterVolume;
public:
    static void tick(int n);
    
    static void init();
    static void reset();
    static void clear();

    static void turnOnOff(unsigned char old, unsigned char val);

    static SquareWave *channel1;
    static SquareWave *channel2;
    static Waveform *channel3;
    static Noise *channel4;

    static SDL_AudioSpec obtainedSpec;
    static SDL_AudioSpec desiredSpec;

    static void sound_callback(void *arg, Uint8 *stream, int length);

    APU(/* args */);
    ~APU();
};

void channel4_callback(void *_beeper, Uint8 *_stream, int _length);
void channel3_callback(void *_beeper, Uint8 *_stream, int _length);
void channel2_callback(void *_beeper, Uint8 *_stream, int _length);
void channel1_callback(void *_beeper, Uint8 *_stream, int _length);

#endif
