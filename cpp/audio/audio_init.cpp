#include "audio_init.hpp"
#include "audio_callback.hpp"
#include <iostream>
#include <cstring>

SDL_AudioDeviceID audio_init(AudioData* data) {
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    want.userdata = data;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (device != 0) {
        SDL_PauseAudioDevice(device, 0);
    } else {
        std::cerr << "Attention: impossible d'ouvrir l'audio : " << SDL_GetError() << std::endl;
    }
    return device;
}
