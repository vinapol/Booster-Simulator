#include "audio_shutdown.hpp"

void audio_shutdown(SDL_AudioDeviceID device) {
    if (device != 0) {
        SDL_CloseAudioDevice(device);
    }
}
