#include "APU.hpp"
#include <iostream>
#include "Ppu.hpp"
#include "Gameboy.hpp"
#include "define.hpp"
#include "UserInterface.hpp"
#include "Debugger.hpp"


#define CHANNEL1 true
#define CHANNEL1_FACTOR 1
#define CHANNEL1_VOLUME_SPEED 0.2

#define CHANNEL2 true
#define CHANNEL2_FACTOR 1
#define CHANNEL2_VOLUME_SPEED 0.2

#define CHANNEL3 true
#define CHANNEL3_FACTOR 1

#define CHANNEL4 true
#define CHANNEL4_FACTOR 1
#define CHANNEL4_VOLUME_SPEED 2.5

unsigned char APU::triggers[4] = {0,0,0,0};
int APU::ticks = 0;
int APU::masterVolume = 0;

SquareWave* APU::channel1 = nullptr;
SquareWave* APU::channel2 = nullptr;
Waveform* APU::channel3 = nullptr;
Noise* APU::channel4 = nullptr;

SDL_AudioSpec APU::obtainedSpec;
SDL_AudioSpec APU::desiredSpec;

float getBetween(float val, float min, float max) {
    if (val < min)
        return min;
    else if (val > max)
        return max;
    return val;
}

void APU::clear() {
	if (Gameboy::bIsInit) {
		SDL_CloseAudio();
		delete channel1;
		delete channel2;
		delete channel3;
		delete channel4;
	}
}

void APU::reset()
{
	channel1 = SquareWave::loadSquareWave(1);
	channel2 = SquareWave::loadSquareWave(2);
	channel3 = Waveform::loadWaveform(3);
	channel4 = Noise::loadNoise(4);
	bzero(triggers, 4);
	ticks = 0;
	masterVolume = 0;
	init();
}

void APU::init() {
    SDL_Init(SDL_INIT_AUDIO);
    std::cout << "INIT AUDIO\n";

    desiredSpec.freq = SAMPLING_RATE;
    desiredSpec.format = AUDIO_U16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = APU::sound_callback;
    desiredSpec.userdata = NULL;

    // you might want to look for errors here
    SDL_OpenAudio(&desiredSpec, &obtainedSpec);

    // start play audio
    SDL_PauseAudio(0);
}

void APU::tick(int n) {
    ticks += n;
    if (ticks >= 190) {
        masterVolume = ((mem[0xFF24] & 0b111) + ((mem[0xFF24] & 0b1110000) >> 4)) / 2;
	masterVolume++; // TODO LMA 7 should be 8 and 0 should be 1
        // if (!masterVolume)
        //     masterVolume = 1;

        channel2->tick();
        channel1->tick();
        channel3->tick();
        channel4->tick();
    }
    ticks %= 190;
}

APU::APU(/* args */)
{
}

APU::~APU()
{
}

void APU::turnOnOff(unsigned char old, unsigned char val) {
    if (BIT(old, 7) && !BIT(val, 7)) {
        // Turning off APU
        channel1->clear();
        channel2->clear();
        channel3->clear();
        channel4->clear();

        // mem[NR50] = 0;
        // mem[NR51] = 0;
        // mem[NR52] = 0;
    }
}

void APU::sound_callback(void *arg, Uint8 *stream, int length) {

    // TODO verify for ptr[i] overflow
    uint16_t *ptr = (uint16_t *)stream;
    static int wavelenRegu = 0;
    // std::cout << "Sound callback\n";

    if (!BIT(mem[NR52], 7) || !BIT(mem[LCDC], 7)) {
        memset(stream, 0, length);
        return ;
    }

    for (int i = 0; i < length/2; i++) {
        ptr[i] = 0;
        float decibels[4] = {0, 0, 0, 0};
        if (Debugger::state != DebuggerState::RUNNING)
            continue ;
        

        if (CHANNEL1)
        {
            if (channel1->queue.size()) {
                channel1->popEntry(channel1->queue.front());
                channel1->queue.pop();
                SET(mem[NR52], 0);
                channel1->regulator = 0;
                wavelenRegu = 0;

                // if (channel1->waveSweepSlope)
                //     channel1->waveSweepSlope = 7 - channel1->waveSweepSlope;

                // std::cout << "Channel 1 playing :\n";
                // std::cout << "\tlength enable : " << channel1->lengthEnable << "\n";
                // std::cout << "\tlength timer : " << channel1->length_timer << "\n";
                // std::cout << "\tvolume : " << channel1->initialVolume << "\n";
                // std::cout << "\tvolume sweep direction : " << channel1->envelopeDirection << "\n";
                // std::cout << "\tvolume sweep pace : " << channel1->volumeSweepPace << "\n";
                // std::cout << "\twave length : " << channel1->waveLength << "\n";
                // std::cout << "\twave sweep direction : " << channel1->waveSweepDirection << "\n";
                // std::cout << "\twave sweep pace : " << channel1->waveSweepPace << "\n";
                // std::cout << "\twave sweep slope : " << channel1->waveSweepSlope << "\n";
            }
            if (channel1->trigger && channel1->DACenable && !(!BIT(mem[NR51], 0) && !BIT(mem[NR51], 4))) {

                // Channel 1
                
                int freq = abs(1000000/(2048 - (int)((int)(channel1->waveLength + channel1->wavelengthSweepValue) == 2048 ? 2047 : channel1->waveLength)));
                int samples_per_step = (SAMPLING_RATE / freq);

                float val = getBetween((float)(channel1->volume)/(0xF/2), 0, 2);

                // ptr[i] = (BIT(channel1->wave, channel1->step) * (int)(val * MAX_VOLUME));
                if (val + channel1->volumeReduction) {
                    decibels[0] = (BIT(channel1->wave, channel1->step) * (getBetween(val + channel1->volumeReduction, 0, 2) * CHANNEL1_FACTOR));
                }
                

                channel1->iterations++;
                channel1->length_count++;

                if (channel1->iterations >= samples_per_step) {
                    channel1->step = (channel1->step + 1) % 8;
                    channel1->iterations = 0;
                    
                }
                
                if (channel1->length_count >= SquareWave::samples_per_length) {
                    // Each 64 Hz
            
                    if (channel1->waveSweepPace && !wavelenRegu) {

                        // std::cout << "wavelength 1 : " << channel1->waveLength << " avec " << (channel1->waveLength / pow(2, channel1->waveSweepSlope)) << "\n"; 
                        if (channel1->waveSweepDirection)
                            channel1->wavelengthSweepValue += (channel1->waveLength / pow(2, channel1->waveSweepSlope));
                            // channel1->changeWavelength(channel1->waveLength + (channel1->waveLength / pow(2, channel1->waveSweepSlope)));
                        else
                            channel1->wavelengthSweepValue -= (channel1->waveLength / pow(2, channel1->waveSweepSlope));
                            // channel1->changeWavelength(channel1->waveLength - (channel1->waveLength / pow(2, channel1->waveSweepSlope)));

                        if (channel1->waveLength > 0x07FF) {
                            channel1->trigger = false;
                            RES(mem[NR52], 0);
                        }
                        // std::cout << "wavelength 2 : " << channel1->waveLength << "\n";
                    }

                    if (channel1->volumeSweepPace && !channel1->regulator) {
                        if (channel1->envelopeDirection)
                            channel1->volumeReduction += CHANNEL1_VOLUME_SPEED;
                        else
                            channel1->volumeReduction -= CHANNEL1_VOLUME_SPEED;
                        
                        
                        if (channel1->volumeReduction > 2)
                            channel1->volumeReduction = 2;
                        else if (channel1->volumeReduction < -2)
                            channel1->volumeReduction = -2;
                    }

                    if (channel1->lengthEnable) {
                        channel1->current_length_timer--;
                        
                        if (channel1->current_length_timer <= 0) {
                            channel1->trigger = false;
                            RES(mem[NR52], 0);
                        }
                    }
                    channel1->length_count = 0;
                    if (channel1->volumeSweepPace)
                        channel1->regulator = (channel1->regulator + 1) % channel1->volumeSweepPace;
                    if (channel1->waveSweepPace)
                        wavelenRegu = (wavelenRegu + 1) % channel1->waveSweepPace;
                }
            }
        }

        if (CHANNEL2)
        {
            if (channel2->queue.size()) {
                channel2->popEntry(channel2->queue.front());
                channel2->queue.pop();
                SET(mem[NR52], 1);
                channel2->regulator = 0;

                // std::cout << "Channel 2 playing :\n";
                // std::cout << "\tlength enable : " << channel2->lengthEnable << "\n";
                // std::cout << "\tlength timer : " << channel2->length_timer << "\n";
                // std::cout << "\tvolume : " << channel2->initialVolume << "\n";
                // std::cout << "\tvolume sweep direction : " << channel2->envelopeDirection << "\n";
                // std::cout << "\tvolume sweep pace : " << channel2->volumeSweepPace << "\n";
                // std::cout << "\twave length : " << channel2->waveLength << "\n";
                // std::cout << "\twave sweep direction : " << channel2->waveSweepDirection << "\n";
                // std::cout << "\twave sweep pace : " << channel2->waveSweepPace << "\n";
                // std::cout << "\twave sweep slope : " << channel2->waveSweepSlope << "\n";

                
            }
            if (channel2->trigger && channel2->DACenable && !(!BIT(mem[NR51], 1) && !BIT(mem[NR51], 5))) {

                // Channel 1
                int freq = abs(1048576/(2048 - (int)((int)channel2->waveLength == 2048 ? 2047 : channel2->waveLength)));
                int samples_per_step = (SAMPLING_RATE / (freq * 1));

                float val = getBetween((float)(channel2->volume)/(0xF/2), 0, 2);

                // std::cout << "Channel 2 volume : " << std::dec << val << "\n";

                // ptr[i] = (BIT(channel1->wave, channel1->step) * (int)(val * MAX_VOLUME));
                if (val + channel2->volumeReduction)
                    decibels[1] = (BIT(channel2->wave, channel2->step) * (getBetween(val + channel2->volumeReduction, 0, 2) * CHANNEL2_FACTOR));

                channel2->iterations++;
                channel2->length_count++;

                if (channel2->iterations >= samples_per_step) {
                    channel2->step = (channel2->step + 1) % 8;
                    channel2->iterations = 0;
                }

                if (channel2->length_count >= SquareWave::samples_per_length) {
                    // Each 64 Hz

                    if (channel2->volumeSweepPace && !channel2->regulator) {
                        if (channel2->envelopeDirection)
                            channel2->volumeReduction += CHANNEL2_VOLUME_SPEED;
                        else
                            channel2->volumeReduction -= CHANNEL2_VOLUME_SPEED;
                        
                        if (channel2->volumeReduction > 2)
                            channel2->volumeReduction = 2;
                        else if (channel2->volumeReduction < -2)
                            channel2->volumeReduction = -2;
                    }

                    if (channel2->lengthEnable) {
                        channel2->current_length_timer--;
                        
                        if (channel2->current_length_timer <= 0) {
                            channel2->trigger = false;
                            RES(mem[NR52], 1);
                        }
                    }
                    channel2->length_count = 0;
                    if (channel2->volumeSweepPace)
                        channel2->regulator = (channel2->regulator + 1) % channel2->volumeSweepPace;
                }
            }
        }


        

        if (CHANNEL3 && channel3->trigger && channel3->DACenable && !(!BIT(mem[NR51], 2) && !BIT(mem[NR51], 6)))
        {
            // Channel 3
            int freq = abs(2097152/(2048 - (channel3->wavelength == 2048 ? 2047 : channel3->wavelength)));
            int samples_per_step = (SAMPLING_RATE / freq);
            float volumeFactor = ((float)channel3->volume)/(0x2 * 0xF);
            // std::cout << "chan3 : " << std::dec << volumeFactor << "\n";

            static char tmp = 0;
            if (!(tmp % 2))
                decibels[2] = getBetween(((float)(channel3->waveform[channel3->step/2] & 0x0F)) * volumeFactor, 0, 2) * CHANNEL3_FACTOR;
            else
                decibels[2] = getBetween((float)((channel3->waveform[channel3->step/2] & 0xF0) >> 4) * volumeFactor, 0, 2) * CHANNEL3_FACTOR;
            tmp++;
            
            channel3->iterations++;
            channel3->length_count++;

            if (channel3->iterations >= samples_per_step) {
                channel3->iterations = 0;
                channel3->step = (channel3->step + 1) % 32;
            }

            if (channel3->length_count >= Waveform::samples_per_length) {
                // Each 64 Hz
                channel3->length_count = 0;

                if (channel3->length_enable && !channel3->regulator) {
                    channel3->current_length_timer -= 2;
                    if (channel3->current_length_timer < 0)
                        channel3->current_length_timer = 0;

                    if (channel3->current_length_timer <= 0) {
                        channel3->trigger = false;
                    }
                }

                channel3->regulator = (channel3->regulator + 1) % 4;

            }
        }

        if (CHANNEL4 && channel4->trigger && BIT(mem[NR52], 3) && channel4->DACenable && !(!BIT(mem[NR51], 3) && !BIT(mem[NR51], 7)))
        {
            // Channel 4
            int freq = (262144 / (channel4->clockDivider * pow(2, channel4->clockShift)));

            int samples_per_step = (SAMPLING_RATE / freq);
            float volumeFactor = ((float)channel4->initial_volume)/(0xF * 100);

            float val = ((float)(getBetween(channel4->sample - channel4->volumeReduction, 0, 100)) * volumeFactor);
            // std::cout << "Channel 4 : " << std::dec << val << "\n";
            
            if (val) {
                decibels[3] = (val * CHANNEL4_FACTOR);
            }

            channel4->iterations++;
            channel4->length_count++;

            if (channel4->iterations >= samples_per_step) {
                channel4->iterations = 0;
                channel4->sample = rand() % 101;
            }

            if (channel4->length_count >= Noise::samples_per_length) {
                // Each 64 Hz

                if (channel4->sweepPace && !channel4->regulator) {
                    // std::cout << "volume 4-1 : " << channel4->initial_volume << "\n";
                    if (channel4->envelopeDirection)
                        channel4->volumeReduction -= CHANNEL4_VOLUME_SPEED;
                    else
                        channel4->volumeReduction += CHANNEL4_VOLUME_SPEED;

                    if (channel4->volumeReduction > 100)
                        channel4->volumeReduction = 100;
                    else if (channel4->volumeReduction < -100)
                        channel4->volumeReduction = -100;
                    // std::cout << "volume 4-2 : " << channel4->initial_volume << "\n";
                }

                if (channel4->length_enable) {
                    channel4->current_length_timer -= 200;
                    
                    if (channel4->current_length_timer <= 0)
                        channel4->trigger = false;
                }
                channel4->length_count = 0;
                if (channel4->sweepPace)
                    channel4->regulator = (channel4->regulator + 1) % channel4->sweepPace;
            }
        }
        for (int i = 0; i < 4; i++)
            decibels[i] *= ((float)1/2) * ((float)UserInterface::volume/100);


        ptr[i] = MAX_VOLUME * (log10(1 + decibels[0]) + log10(1 + decibels[1]) + log10(1 + decibels[2]) + log10(1 + decibels[3]));
        // ptr[i] = (unsigned char)(decibel/4 * (float)MAX_VOLUME * 2);
//        unsigned short max = 0; UNUSED Variable
//        float factor = 0;  UNUSED Variable
        
    }
    (void)arg; // Unused Variable
    // std::cout << "\n";
}
