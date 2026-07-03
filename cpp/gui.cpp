#include "gui.hpp"
#include "font.hpp"
#include "aerodynamique.hpp"
#include <GL/gl.h>
#include <cstdio>
#include <cmath>
#include <string>

// Fonctions utilitaires de dessin 2D en OpenGL
inline void gl_fill_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
}

inline void gl_draw_line(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b) {
    glColor3ub(r, g, b);
    glBegin(GL_LINES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
}

// Rendu complet de l'interface de télémétrie HUD (Volet Droit)
void draw_telemetry(SDL_Renderer* renderer, const Rocket& myRocket, double time, double throttle, double tilt_x, double tilt_z, const Vector3D& wind, double current_wind_x, bool simulation_finished, bool landed_successfully, double impact_speed, InputManager& inputs, int control_mode) {
    // 1. Passage en projection orthographique 2D pour dessiner le HUD
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1000, 700, 0, -1, 1); // Résolution virtuelle 1000x700
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 2. Dessiner le fond translucide gris foncé du HUD (de X=600 à 1000)
    gl_fill_rect(600, 0, 400, 700, 18, 20, 25, 240);
    
    // Ligne verticale de séparation entre la vue 3D et la télémétrie
    gl_draw_line(600, 0, 600, 700, 44, 62, 80);

    // Titre Principal
    draw_string(renderer, "=== TELEMETRIE BOOSTER 3D ===", 622, 25, 2, 52, 152, 219);
    
    // Chronomètre
    char time_str[32];
    sprintf(time_str, "TEMPS DE SIM : %.1f s", time);
    draw_string(renderer, time_str, 630, 65, 1, 149, 165, 166);

    // Détermination de la dérive hors cible en 3D
    bool off_target = (std::abs(myRocket.position.x) >= 20.0 || std::abs(myRocket.position.z) >= 20.0);

    // --- STATUT DE LA SIMULATION ---
    if (simulation_finished) {
        if (landed_successfully) {
            gl_fill_rect(630, 95, 340, 35, 39, 174, 96); // Vert
            draw_string(renderer, "ATTERRISSAGE REUSSI !", 650, 105, 1, 255, 255, 255);
        } else {
            gl_fill_rect(630, 95, 340, 35, 192, 57, 43); // Rouge
            if (impact_speed <= 2.0 && off_target) {
                draw_string(renderer, "CRASH: HORS DU PAD (PAD=20m) !", 640, 105, 1, 255, 255, 255);
            } else {
                draw_string(renderer, "CRASH DETECTE !", 695, 105, 1, 255, 255, 255);
            }
        }
    } else {
        // Alertes en vol
        if (myRocket.position.y < 200.0 && myRocket.velocity.y < -5.0) {
            if ((int)(time * 5) % 2 == 0) {
                gl_fill_rect(630, 95, 340, 35, 241, 196, 15); // Jaune alerte
                draw_string(renderer, "ALERTE: VITESSE D'IMPACT ELEVEE", 640, 105, 1, 0, 0, 0);
            } else {
                gl_fill_rect(630, 95, 340, 35, 44, 62, 80);
                draw_string(renderer, "ALERTE: VITESSE D'IMPACT ELEVEE", 640, 105, 1, 241, 196, 15);
            }
        } else if (myRocket.position.y < 300.0 && off_target) {
            gl_fill_rect(630, 95, 340, 35, 230, 126, 34); // Orange dérive
            draw_string(renderer, "ATTENTION: HORS DU PAD D'ATT.", 640, 105, 1, 255, 255, 255);
        } else {
            gl_fill_rect(630, 95, 340, 35, 41, 128, 185); // Bleu vol normal
            draw_string(renderer, "BOOSTER EN VOL", 710, 105, 1, 255, 255, 255);
        }
    }

    // --- GRANDS COMPTEURS PRINCIPAUX (ALTITUDE ET VITESSE) ---
    char alt_str[64];
    sprintf(alt_str, "ALTITUDE : %.2f m", myRocket.position.y);
    draw_string(renderer, alt_str, 630, 145, 2, 236, 240, 241);

    char vy_str[64];
    sprintf(vy_str, "VIT VERT : %.2f m/s", myRocket.velocity.y);
    uint8_t vy_r = 236, vy_g = 240, vy_b = 241;
    if (myRocket.position.y < 250.0 && myRocket.velocity.y < -2.0) {
        vy_r = 231; vy_g = 76; vy_b = 60; // Rouge alerte
    } else if (myRocket.velocity.y > 0.1) {
        vy_r = 46; vy_g = 204; vy_b = 113; // Vert montée
    }
    draw_string(renderer, vy_str, 630, 175, 2, vy_r, vy_g, vy_b);

    // --- COMPTEURS SECONDAIRES 3D ---
    // Positions horizontales X et Z
    char px_str[64];
    sprintf(px_str, "Pos X : %6.2f m | Pos Z : %6.2f m", myRocket.position.x, myRocket.position.z);
    uint8_t pos_r = 189, pos_g = 195, pos_b = 199;
    if (off_target) {
        pos_r = 230; pos_g = 126; pos_b = 34; // Orange si hors pad
    }
    draw_string(renderer, px_str, 630, 215, 1, pos_r, pos_g, pos_b);

    // Vitesses horizontales Vx et Vz
    char vx_str[64];
    sprintf(vx_str, "Vit X : %6.2f m/s | Vit Z : %6.2f m/s", myRocket.velocity.x, myRocket.velocity.z);
    draw_string(renderer, vx_str, 630, 233, 1, 189, 195, 199);

    // Inclinaisons tangage (pitch) et lacet (yaw) réels du corps
    char ang_str[64];
    sprintf(ang_str, "Pitch (X) : %4.1f deg | Yaw (Z) : %4.1f deg", myRocket.theta_x * 180.0 / 3.141592653589793, myRocket.theta_z * 180.0 / 3.141592653589793);
    draw_string(renderer, ang_str, 630, 245, 1, 189, 195, 199);

    // Orientation de tuyère réelle (gimbal)
    char gim_str[64];
    sprintf(gim_str, "Gimbal X  : %4.1f deg | Gimbal Z : %4.1f deg", myRocket.gimbal_x * 180.0 / 3.141592653589793, myRocket.gimbal_z * 180.0 / 3.141592653589793);
    draw_string(renderer, gim_str, 630, 263, 1, 189, 195, 199);

    // --- BARRES DE CARBURANT ET DE COMMANDE ---
    // Carburant (Propergol)
    double fuel_ratio = myRocket.fuel_mass / 4000.0;
    char fuel_lbl[32];
    sprintf(fuel_lbl, "PROPERGOL : %.1f kg (%.0f%%)", myRocket.fuel_mass, fuel_ratio * 100.0);
    draw_string(renderer, fuel_lbl, 630, 292, 1, 46, 204, 113);
    
    // Conteneur jauge fuel
    gl_fill_rect(630, 307, 340, 14, 44, 62, 80);
    if (fuel_ratio > 0.0) {
        uint8_t fr = 46, fg = 204, fb = 113;
        if (fuel_ratio <= 0.2) { fr = 231; fg = 76; fb = 60; } // Rouge réserve
        gl_fill_rect(630, 307, (int)(fuel_ratio * 340.0), 14, fr, fg, fb);
    }

    // Commande des gaz (Throttle)
    char thr_lbl[32];
    sprintf(thr_lbl, "POUSSEE GAZ MOTEUR : %.1f%%", throttle * 100.0);
    draw_string(renderer, thr_lbl, 630, 335, 1, 52, 152, 219);
    
    // Conteneur jauge throttle
    gl_fill_rect(630, 350, 340, 14, 44, 62, 80);
    if (throttle > 0.0) {
        gl_fill_rect(630, 350, (int)(throttle * 340.0), 14, 52, 152, 219);
    }

    // --- FORCES PHYSIQUES ---
    draw_string(renderer, "=== STATISTIQUES DES FORCES ===", 630, 390, 1, 149, 165, 166);
    
    double current_thrust = (myRocket.fuel_mass > 0) ? (myRocket.max_mass_flow_rate * throttle * myRocket.effective_exhaust_velocity) : 0.0;
    Vector3D drag = get_drag_force(myRocket.position.y, myRocket.velocity, myRocket.theta_x, myRocket.theta_z, wind);

    char force_p[64];
    sprintf(force_p, "Poussee moteur : %6.0f N", current_thrust);
    draw_string(renderer, force_p, 630, 410, 1, 236, 240, 241);

    char force_dy[64];
    sprintf(force_dy, "Trainee Y (air): %6.0f N", drag.y);
    draw_string(renderer, force_dy, 630, 428, 1, 236, 240, 241);

    char force_dxz[64];
    sprintf(force_dxz, "Trainee Horiz  : X=%4.0f N | Z=%4.0f N", drag.x, drag.z);
    draw_string(renderer, force_dxz, 630, 446, 1, 236, 240, 241);

    char mass_str[64];
    sprintf(mass_str, "Masse totale   : %6.0f kg", myRocket.dry_mass + myRocket.fuel_mass);
    draw_string(renderer, mass_str, 630, 464, 1, 236, 240, 241);

    // Affichage des vecteurs de vent
    char wind_str[64];
    sprintf(wind_str, "Vent X : %5.1f m/s | Vent Z : %5.1f m/s", wind.x, wind.z);
    draw_string(renderer, wind_str, 630, 482, 1, 241, 196, 15);

    // --- SYSTÈME DE CONTRÔLE ---
    draw_string(renderer, "=== DISPOSITIF DE PILOTAGE ===", 630, 515, 1, 149, 165, 166);
    if (control_mode == 1) {
        draw_string(renderer, "MODE : AUTO (CASCADE PID)", 630, 535, 1, 46, 204, 113); // Vert
    } else if (control_mode == 2) {
        draw_string(renderer, "MODE : AUTO (IA NEURONALE)", 630, 535, 1, 155, 89, 182); // Violet premium
    } else {
        draw_string(renderer, "MODE : MANUEL (PILOTE HUMAIN)", 630, 535, 1, 52, 152, 219); // Bleu
    }

    if (inputs.hasJoystick()) {
        draw_string(renderer, inputs.getJoystickName(), 630, 558, 1, 241, 196, 15);
        char joy_axes[64];
        sprintf(joy_axes, "Axe0 (Pitch): %d | Axe1 (Yaw): %d", inputs.getRawAxis0(), inputs.getRawAxis1());
        draw_string(renderer, joy_axes, 630, 576, 1, 189, 195, 199);
        char joy_throttle[64];
        sprintf(joy_throttle, "Axe2 (Gaz)  : %d", inputs.getRawAxis2());
        draw_string(renderer, joy_throttle, 630, 592, 1, 189, 195, 199);

        // Diagnostic boutons
        std::string pressed_buttons_str = "Boutons press.: ";
        bool first = true;
        for (int b = 0; b < 15; ++b) {
            if (SDL_JoystickGetButton(inputs.getJoystick(), b)) {
                if (!first) pressed_buttons_str += ", ";
                pressed_buttons_str += std::to_string(b);
                first = false;
            }
        }
        draw_string(renderer, pressed_buttons_str.c_str(), 630, 612, 1, 241, 196, 15);
    } else {
        draw_string(renderer, "CLAVIER CONFIGURATION ACTIVE", 630, 558, 1, 241, 196, 15);
        draw_string(renderer, "Direction : FL. GAUCHE/DROITE (Pitch)", 630, 576, 1, 189, 195, 199);
        draw_string(renderer, "Profondeur: FL. HAUT/BAS (Yaw)", 630, 592, 1, 189, 195, 199);
        draw_string(renderer, "Moteur    : Z/S (Augmenter/Reduire Gaz)", 630, 608, 1, 189, 195, 199);
    }

    // 3. Restauration des matrices OpenGL
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
