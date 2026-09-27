#pragma once

#include <SDL2/SDL.h>
#include "audioData.hpp"

SDL_AudioDeviceID audio_init(AudioData* data);
