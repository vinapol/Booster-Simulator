#include "audio_callback.hpp"
#include "audioData.hpp"
#include <cstdlib>

void audio_callback(void* userdata, Uint8* stream, int len) {
    AudioData* data = (AudioData*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / 2;

    double volume = 0.0;
    if (data->has_fuel && !data->is_finished) {
        volume = data->throttle * 6500.0;
    }

    for (int i = 0; i < samples; ++i) {
        double white = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        double rumble = 0.06 * white + 0.94 * data->last_sample;
        data->last_sample = rumble;
        double mix = 0.2 * white + 0.8 * rumble;
        buffer[i] = (int16_t)(mix * volume);
    }
}
