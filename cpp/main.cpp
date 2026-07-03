#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "booster.hpp"
#include "inputManager.hpp"
#include "graphics.hpp"
#include "gui.hpp"
#include "autopilot.hpp"
#include "neuralNetwork.hpp"

struct AudioData {
    volatile double throttle = 0.0;
    volatile bool has_fuel = true;
    volatile bool is_finished = false;
    double last_sample = 0.0;
};

// Callback SDL Audio pour synthétiser le grondement de combustion
void audio_callback(void* userdata, Uint8* stream, int len) {
    AudioData* data = (AudioData*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / 2;

    double volume = 0.0;
    if (data->has_fuel && !data->is_finished) {
        volume = data->throttle * 6500.0; // Volume calibré et confortable
    }

    for (int i = 0; i < samples; ++i) {
        double white = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        // Filtre passe-bas (Brownian noise) pour créer les basses lourdes
        double rumble = 0.06 * white + 0.94 * data->last_sample;
        data->last_sample = rumble;
        
        // Mélange : 80% grondement (basses) + 20% souffle (aigus)
        double mix = 0.2 * white + 0.8 * rumble;
        buffer[i] = (int16_t)(mix * volume);
    }
}

int main(int argc, char* argv[])
{
    // Initialisation globale de SDL (avec audio)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return -1;
    }

    // Fenêtre avec support OpenGL
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

    // Création du contexte OpenGL de SDL2
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "Erreur de creation du contexte OpenGL : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Synchronisation verticale (V-Sync)
    SDL_GL_SetSwapInterval(1);

    // Paramètres OpenGL globaux
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Joystick & Input manager
    InputManager inputs;
    // Initialisation du Booster : Position (X=50m, Y=3000m, Z=0m), Vitesse (Vx=0, Vy=-150m/s, Vz=0)
    Rocket myRocket(10000.0, 4000.0, 3000.0, 100.0, {50.0, 3000.0, 0.0}, {0.0, -150.0, 0.0});
    Autopilot autopilot;
    
    NeuralNetwork ai_net;
    bool ai_loaded = ai_net.load_weights("weights");
    int control_mode = 0; // 0 = Manuel, 1 = Cascade PID, 2 = IA

    // Initialisation du périphérique audio de SDL
    AudioData audio_data;
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    want.userdata = &audio_data;

    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_device != 0) {
        SDL_PauseAudioDevice(audio_device, 0); // Lance la lecture audio
    } else {
        std::cerr << "Attention: impossible d'ouvrir l'audio : " << SDL_GetError() << std::endl;
    }
    
    double gravity = 9.81;
    double dt = 0.016; // ~60 FPS
    double time = 0.0;
    
    srand(std::chrono::system_clock::now().time_since_epoch().count());
    double wind_base = ((rand() % 200) / 100.0 - 1.0) * 35.0; // Vent moyen de base
    double wind_noise = 0.0;
    
    double throttle = 0.0;
    double tilt_x = 0.0;
    double tilt_z = 0.0;
    double roll_cmd = 0.0;
    double current_thrust = 0.0;

    bool running = true;
    bool simulation_finished = false;
    bool landed_successfully = false;
    double impact_speed = 0.0;

    SDL_Event event;

    // Génération des étoiles en 3D
    std::vector<Star> stars;
    for (int i = 0; i < 300; ++i) {
        stars.push_back({
            (double)(rand() % 2000 - 1000),  // X position
            (double)(rand() % 8000 + 100),   // Y altitude
            (double)(rand() % 2000 - 1000),  // Z profondeur
            rand() % 155 + 100               // Luminosité
        });
    }
    
    std::cout << "Simulation 3D OpenGL lancee avec succes." << std::endl;

    while (running)
    {
        // 1. Gestion des événements de fenêtre
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        if (inputs.isQuitPressed()) {
            running = false;
        }

        // Si touche R enfoncée : réinitialisation complète de la simulation (Difficulté max : 3000m)
        if (inputs.isResetPressed()) {
            // Spawne à Z avec un petit décalage aléatoire pour tester le lacet de l'autopilote
            double random_z = ((rand() % 200) / 100.0 - 1.0) * 30.0;
            myRocket = Rocket(10000.0, 4000.0, 3000.0, 100.0, {50.0, 3000.0, random_z}, {0.0, -150.0, 0.0});
            inputs.reset();
            autopilot.setActive(false);
            control_mode = 2; // Activer directement l'IA !
            wind_base = ((rand() % 200) / 100.0 - 1.0) * 35.0;
            wind_noise = 0.0;
            throttle = 0.0;
            tilt_x = 0.0;
            tilt_z = 0.0;
            time = 0.0;
            simulation_finished = false;
            landed_successfully = false;
            impact_speed = 0.0;
            
            // Reinit Audio
            audio_data.throttle = 0.0;
            audio_data.has_fuel = true;
            audio_data.is_finished = false;
        }

        // Si touche T enfoncée : réinitialisation facile (Difficulté moyenne : 1000m)
        if (inputs.isResetEasyPressed()) {
            double random_z = ((rand() % 200) / 100.0 - 1.0) * 10.0;
            myRocket = Rocket(10000.0, 4000.0, 3000.0, 100.0, {15.0, 1000.0, random_z}, {0.0, -50.0, 0.0});
            inputs.reset();
            autopilot.setActive(false);
            control_mode = 2; // Activer directement l'IA !
            wind_base = 0.0; // Pas de vent pour ce niveau
            wind_noise = 0.0;
            throttle = 0.0;
            tilt_x = 0.0;
            tilt_z = 0.0;
            time = 0.0;
            simulation_finished = false;
            landed_successfully = false;
            impact_speed = 0.0;
            
            // Reinit Audio
            audio_data.throttle = 0.0;
            audio_data.has_fuel = true;
            audio_data.is_finished = false;
        }

        // Si touche Y enfoncée : réinitialisation ultra-facile (Difficulté min : 500m)
        if (inputs.isResetVeryEasyPressed()) {
            double random_z = ((rand() % 200) / 100.0 - 1.0) * 5.0;
            myRocket = Rocket(10000.0, 4000.0, 3000.0, 100.0, {5.0, 500.0, random_z}, {0.0, -30.0, 0.0});
            inputs.reset();
            autopilot.setActive(false);
            control_mode = 2; // Activer directement l'IA !
            wind_base = 0.0;
            wind_noise = 0.0;
            throttle = 0.0;
            tilt_x = 0.0;
            tilt_z = 0.0;
            time = 0.0;
            simulation_finished = false;
            landed_successfully = false;
            impact_speed = 0.0;
            
            // Reinit Audio
            audio_data.throttle = 0.0;
            audio_data.has_fuel = true;
            audio_data.is_finished = false;
        }

        // Changement de mode de contrôle (Cycle entre Manuel -> PID -> IA)
        static bool last_ap_pressed = false;
        bool ap_pressed = inputs.isAutopilotToggled();
        if (ap_pressed && !last_ap_pressed) {
            control_mode = (control_mode + 1) % 3;
            if (control_mode == 2 && !ai_loaded) {
                control_mode = 0; // IA non chargée, retour au mode manuel
            }
            autopilot.setActive(control_mode == 1);
        }
        last_ap_pressed = ap_pressed;

        // Calcul du vent dynamique en 3D
        double wind_alt_factor = 0.3 + 0.7 * (myRocket.position.y / 1000.0);
        if (wind_alt_factor < 0.3) wind_alt_factor = 0.3;
        
        if (!simulation_finished) {
            wind_noise = 0.98 * wind_noise + 0.02 * (((rand() % 200) / 100.0 - 1.0) * 12.0);
        }
        double current_wind_x = (wind_base * std::cos(time * 0.15) + wind_noise) * wind_alt_factor;
        // On rajoute un vent de travers Z oscillant
        double current_wind_z = (wind_base * 0.35 * std::sin(time * 0.25)) * wind_alt_factor;
        Vector3D wind = { current_wind_x, 0.0, current_wind_z };

        if (!simulation_finished) {
            inputs.update(dt);
            
            if (control_mode == 1) {
                autopilot.compute_controls(myRocket, wind, gravity, dt, throttle, tilt_x, tilt_z);
                roll_cmd = 0.0;
            } else if (control_mode == 2 && ai_loaded) {
                // Calcul des commandes par le réseau de neurones SAC
                std::vector<double> input = {
                    myRocket.position.x / 100.0,
                    myRocket.position.y / 1000.0, // Altitude relative (cible à 0m en C++)
                    myRocket.position.z / 100.0,
                    myRocket.velocity.x / 100.0,
                    myRocket.velocity.y / 100.0,
                    myRocket.velocity.z / 100.0,
                    myRocket.theta_x,
                    myRocket.theta_y, // Roll
                    myRocket.theta_z,
                    myRocket.omega_x,
                    myRocket.omega_y, // Roll rate
                    myRocket.omega_z,
                    myRocket.fuel_mass / 4000.0,
                    wind.x / 20.0,
                    wind.z / 20.0,
                    myRocket.position.y / 1000.0 // Altitude absolue
                };
                // Calculer d'abord les commandes du pilote automatique classique
                double ap_throttle = 0.0;
                double ap_gimbal_x = 0.0;
                double ap_gimbal_z = 0.0;
                autopilot.compute_controls(myRocket, wind, gravity, dt, ap_throttle, ap_gimbal_x, ap_gimbal_z);

                std::vector<double> action = ai_net.forward(input);

                // Limite maximale du gimbal (+/- 15 deg max)
                double max_gimbal = 15.0 * 3.141592653589793 / 180.0;

                // Combiner le pilote automatique avec la correction résiduelle de l'IA
                // action[0] (poussée) corrige à +/- 20%
                throttle = std::clamp(ap_throttle + 0.2 * action[0], 0.0, 1.0);
                
                // action[1] et action[2] (gimbals) corrigent à +/- 5 degrés (+/- 0.087 rad)
                tilt_x = std::clamp(ap_gimbal_x + 0.087 * action[1], -max_gimbal, max_gimbal);
                tilt_z = std::clamp(ap_gimbal_z + 0.087 * action[2], -max_gimbal, max_gimbal);
                roll_cmd = std::clamp(action[3], -1.0, 1.0); // IA a le contrôle direct en roulis
            } else {
                throttle = inputs.getThrottle();
                tilt_x = inputs.getTiltX();
                tilt_z = inputs.getTiltZ();
                roll_cmd = 0.0;
            }
            
            current_thrust = myRocket.get_thrust(throttle);
            
            // Envoyer la commande de gaz à la synthèse sonore
            audio_data.throttle = throttle;
            audio_data.has_fuel = (myRocket.fuel_mass > 0.0);
            audio_data.is_finished = false;
            
            // Physique 3D RK4
            myRocket.update_rk4(dt, gravity, current_thrust, tilt_x, tilt_z, roll_cmd, wind);
            myRocket.update_mass(dt, throttle);
            time += dt;

            // Vérification de l'impact
            if (myRocket.position.y <= 0.0) {
                myRocket.position.y = 0.0;
                simulation_finished = true;
                impact_speed = std::abs(myRocket.velocity.y);
                // Le pad d'atterrissage vert mesure 40x40 mètres (-20 à +20 sur X et Z)
                bool inside_pad = (std::abs(myRocket.position.x) < 20.0 && std::abs(myRocket.position.z) < 20.0);
                landed_successfully = (impact_speed <= 2.0 && inside_pad);
                
                // Couper le moteur sonore
                audio_data.throttle = 0.0;
                audio_data.is_finished = true;
            }
        }

        // 2. RENDU GRAPHIQUE OPENGL
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- VOLET GAUCHE : VUE DE SIMULATION 3D ---
        // Le volet gauche fait 600px de large
        glViewport(0, 0, 600, 700);
        setup_projection(600.0 / 700.0);
        setup_camera(myRocket, time);
        
        draw_sky(myRocket.position.y);
        draw_stars(myRocket.position.y, stars);
        draw_ground_and_pad_3d();
        draw_booster_and_flame_3d(myRocket, tilt_x, tilt_z, throttle);
        draw_forces_vectors_3d(myRocket, tilt_x, tilt_z, wind, throttle, gravity);

        // --- VOLET DROIT : HUD ET TELEMETRIE ---
        // On dessine l'overlay sur l'écran complet (1000px de large)
        glViewport(0, 0, 1000, 700);
        draw_telemetry(nullptr, myRocket, time, throttle, tilt_x, tilt_z, wind, current_wind_x, simulation_finished, landed_successfully, impact_speed, inputs, control_mode);

        // Swap des buffers
        SDL_GL_SwapWindow(window);
    }

    // Nettoyage complet
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
    }
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Simulation terminee proprement." << std::endl;
    return 0;
}