#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <SDL2/SDL.h>
#include <algorithm>

class InputManager {
private:
    SDL_Joystick* joystick = nullptr;
    double throttle = 0.0;
    double tilt_x = 0.0;
    double tilt_z = 0.0;

public:
    InputManager() {
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
        if (SDL_NumJoysticks() > 0) {
            joystick = SDL_JoystickOpen(0);
        }
    }

    void update(double dt) {
        if (joystick) {
            SDL_JoystickUpdate();
            // L'axe 2 sur le T.Flight Hotas est la manette des gaz
            int16_t val_throttle = SDL_JoystickGetAxis(joystick, 2);
            throttle = (32767 - val_throttle) / 65535.0;

            // Axe 0 : Roulis / Lacet sur le manche (inclinaison X)
            int16_t val_tilt_x = SDL_JoystickGetAxis(joystick, 0);
            tilt_x = (val_tilt_x / 32768.0) * (3.141592653589793 / 4.0);

            // Axe 1 : Tangage sur le manche (inclinaison Z)
            int16_t val_tilt_z = SDL_JoystickGetAxis(joystick, 1);
            tilt_z = (val_tilt_z / 32768.0) * (3.141592653589793 / 4.0);
        } else {
            const Uint8* state = SDL_GetKeyboardState(NULL);
            
            if (state[SDL_SCANCODE_Z]) {
                throttle += 0.6 * dt;
            }
            if (state[SDL_SCANCODE_S]) {
                throttle -= 0.6 * dt;
            }
            throttle = std::clamp(throttle, 0.0, 1.0);

            // Contrôles Pitch (tangage X)
            if (state[SDL_SCANCODE_Q] || state[SDL_SCANCODE_LEFT]) {
                tilt_x -= 0.6 * dt;
            }
            if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
                tilt_x += 0.6 * dt;
            }
            tilt_x = std::clamp(tilt_x, -3.141592653589793 / 4.0, 3.141592653589793 / 4.0);

            // Contrôles Yaw (lacet Z)
            if (state[SDL_SCANCODE_UP]) {
                tilt_z -= 0.6 * dt;
            }
            if (state[SDL_SCANCODE_DOWN]) {
                tilt_z += 0.6 * dt;
            }
            tilt_z = std::clamp(tilt_z, -3.141592653589793 / 4.0, 3.141592653589793 / 4.0);
        }
    }

    bool isResetPressed() const {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        bool keyboard_reset = state[SDL_SCANCODE_R] != 0;

        bool joystick_reset = false;
        if (joystick) {
            SDL_JoystickUpdate();
            joystick_reset = (SDL_JoystickGetButton(joystick, 1) != 0) || (SDL_JoystickGetButton(joystick, 7) != 0);
        }
        return keyboard_reset || joystick_reset;
    }

    bool isResetEasyPressed() const {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        return state[SDL_SCANCODE_T] != 0;
    }

    bool isResetVeryEasyPressed() const {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        return state[SDL_SCANCODE_Y] != 0;
    }

    bool isQuitPressed() const {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        return state[SDL_SCANCODE_ESCAPE] != 0;
    }

    bool isAutopilotToggled() const {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        bool keyboard_toggle = state[SDL_SCANCODE_A] != 0;

        bool joystick_toggle = false;
        if (joystick) {
            SDL_JoystickUpdate();
            joystick_toggle = (SDL_JoystickGetButton(joystick, 3) != 0);
        }
        return keyboard_toggle || joystick_toggle;
    }

    void reset() {
        throttle = 0.0;
        tilt_x = 0.0;
        tilt_z = 0.0;
    }

    double getThrottle() const {
        return throttle;
    }

    double getTiltX() const {
        return tilt_x;
    }

    double getTiltZ() const {
        return tilt_z;
    }

    SDL_Joystick* getJoystick() const {
        return joystick;
    }

    bool hasJoystick() const {
        return joystick != nullptr;
    }

    const char* getJoystickName() const {
        if (joystick) {
            return SDL_JoystickName(joystick);
        }
        return "Aucun";
    }

    int16_t getRawAxis2() {
        if (joystick) {
            SDL_JoystickUpdate();
            return SDL_JoystickGetAxis(joystick, 2);
        }
        return 0;
    }

    int16_t getRawAxis0() {
        if (joystick) {
            SDL_JoystickUpdate();
            return SDL_JoystickGetAxis(joystick, 0);
        }
        return 0;
    }

    int16_t getRawAxis1() {
        if (joystick) {
            SDL_JoystickUpdate();
            return SDL_JoystickGetAxis(joystick, 1);
        }
        return 0;
    }

    ~InputManager() {
        if (joystick) {
            SDL_JoystickClose(joystick);
        }
    }
};

#endif