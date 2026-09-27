#include <iostream>
#include <chrono>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "inputManager.hpp"
#include "graphics.hpp"
#include "boosterVisual.hpp"
#include "draw_telemetry.hpp"
#include "audio.hpp"
#include "simulation.hpp"

int main(int argc, char* argv[])
{
    (void)argc;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Simulation Booster SpaceX Falcon 3D",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1000, 700,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (!window) {
        std::cerr << "Erreur de creation de fenetre : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "Erreur de creation du contexte OpenGL : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_SetSwapInterval(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    srand(std::chrono::system_clock::now().time_since_epoch().count());

    InputManager inputs;
    SimState sim(argv[0]);
    sim.wind_base = ((rand() % 200) / 100.0 - 1.0) * 35.0;

    AudioData audio_data;
    SDL_AudioDeviceID audio_device = audio_init(&audio_data);

    std::vector<Star> stars = generate_stars(300);
    std::cout << "Simulation 3D OpenGL lancee avec succes." << std::endl;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        if (inputs.isQuitPressed()) {
            running = false;
        }

        if (inputs.isResetPressed()) {
            reset_hard(sim, inputs, audio_data);
        } else if (inputs.isResetEasyPressed()) {
            reset_easy(sim, inputs, audio_data);
        } else if (inputs.isResetVeryEasyPressed()) {
            reset_very_easy(sim, inputs, audio_data);
        }

        cycle_control_mode(sim, inputs);
        update_wind(sim);

        if (!sim.simulation_finished) {
            inputs.update(sim.dt);
            update_controls(sim, inputs);
            step_physics(sim, audio_data);
        }

        // --- Rendu ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, 600, 700);
        setup_projection(600.0 / 700.0);
        setup_camera(sim.rocket, sim.time);
        draw_sky(sim.rocket.position.y);
        draw_stars(sim.rocket.position.y, stars);
        draw_ground_and_pad_3d();
        draw_booster_and_flame_3d(sim.rocket, sim.controls.tilt_x, sim.controls.tilt_z, sim.controls.throttle);
        draw_forces_vectors_3d(sim.rocket, sim.controls.tilt_x, sim.controls.tilt_z,
                               sim.wind, sim.controls.throttle, sim.gravity);

        glViewport(0, 0, 1000, 700);
        draw_telemetry(nullptr, sim.rocket, sim.time, sim.controls.throttle,
                       sim.controls.tilt_x, sim.controls.tilt_z, sim.wind, sim.current_wind_x,
                       sim.simulation_finished, sim.landed_successfully, sim.impact_speed,
                       inputs, sim.control_mode);

        SDL_GL_SwapWindow(window);
    }

    audio_shutdown(audio_device);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Simulation terminee proprement." << std::endl;
    return 0;
}
