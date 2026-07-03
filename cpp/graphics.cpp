#include "graphics.hpp"
#include <GL/gl.h>
#include <cmath>
#include <cstdlib>

// Configuration de la projection perspective OpenGL
void setup_projection(double aspect) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Matrice de projection perspective faite à la main (évite GLU)
    double fov_degrees = 45.0;
    double fov_radians = fov_degrees * 3.141592653589793 / 180.0;
    double f = 1.0 / std::tan(fov_radians / 2.0);
    double z_near = 0.5;
    double z_far = 15000.0;
    
    double m[16] = {
        f / aspect, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, (z_far + z_near) / (z_near - z_far), -1.0,
        0.0, 0.0, (2.0 * z_far * z_near) / (z_near - z_far), 0.0
    };
    glMultMatrixd(m);
}

// Positionnement de la caméra de poursuite orbitale
void setup_camera(const Rocket& rocket, double time) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Reculer la caméra de 80 mètres pour observer le booster de l'extérieur
    glTranslated(0.0, 0.0, -80.0);
    
    // Orbite automatique lente autour du booster (vitesse angulaire douce)
    double orbit_angle = time * 0.12; 
    
    // Inclinaison de 12 degrés vers le bas pour observer l'aire d'atterrissage
    glRotated(12.0, 1.0, 0.0, 0.0);
    // Rotation orbitale sur l'axe Y
    glRotated(-orbit_angle * 180.0 / 3.14159265, 0.0, 1.0, 0.0);
    // Translation pour centrer la caméra sur le milieu du booster
    glTranslated(-rocket.position.x, -rocket.position.y - 15.0, -rocket.position.z);
}

// Dessin du ciel dégradé dépendant de l'altitude
void draw_sky(double rocket_y) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    double h_factor = rocket_y / 3000.0;
    if (h_factor > 1.0) h_factor = 1.0;
    if (h_factor < 0.0) h_factor = 0.0;

    // Dégradé : Bleu atmosphérique en bas, noir spatial en haut
    uint8_t r_top = (uint8_t)(8.0 * (1.0 - h_factor));
    uint8_t g_top = (uint8_t)(10.0 * (1.0 - h_factor));
    uint8_t b_top = (uint8_t)(20.0 * (1.0 - h_factor));

    uint8_t r_bot = (uint8_t)(20.0 + 70.0 * (1.0 - h_factor));
    uint8_t g_bot = (uint8_t)(30.0 + 120.0 * (1.0 - h_factor));
    uint8_t b_bot = (uint8_t)(50.0 + 190.0 * (1.0 - h_factor));

    glBegin(GL_QUADS);
    glColor3ub(r_bot, g_bot, b_bot);
    glVertex2d(-1.0, -1.0);
    glVertex2d(1.0, -1.0);
    glColor3ub(r_top, g_top, b_top);
    glVertex2d(1.0, 1.0);
    glVertex2d(-1.0, 1.0);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

// Dessin des étoiles en 3D
void draw_stars(double rocket_y, const std::vector<Star>& stars) {
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    for (const auto& star : stars) {
        glColor3ub(star.brightness, star.brightness, star.brightness);
        glVertex3d(star.x, star.y, star.z);
    }
    glEnd();
}

void draw_wind_indicator_3d(const Vector3D& wind) {
    // Le vent est dessiné plus lisiblement dans le HUD 2D
}

// Dessin de la grille de sol et du pad d'atterrissage 3D
void draw_ground_and_pad_3d() {
    // Grille 3D du sol
    glColor3ub(35, 38, 45);
    glBegin(GL_LINES);
    for (int i = -300; i <= 300; i += 20) {
        glVertex3d(i, 0.0, -300.0);
        glVertex3d(i, 0.0, 300.0);
        glVertex3d(-300.0, 0.0, i);
        glVertex3d(300.0, 0.0, i);
    }
    glEnd();

    // Zone verte du Pad d'atterrissage (40m x 40m)
    glColor3ub(39, 174, 96);
    glBegin(GL_QUADS);
    glVertex3d(-20.0, 0.01, -20.0);
    glVertex3d(20.0, 0.01, -20.0);
    glVertex3d(20.0, 0.01, 20.0);
    glVertex3d(-20.0, 0.01, 20.0);
    glEnd();

    // Bordure blanche du Pad
    glColor3ub(236, 240, 241);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3d(-20.0, 0.02, -20.0);
    glVertex3d(20.0, 0.02, -20.0);
    glVertex3d(20.0, 0.02, 20.0);
    glVertex3d(-20.0, 0.02, 20.0);
    glEnd();
    
    // Cible circulaire blanche
    glBegin(GL_LINE_LOOP);
    for (int a = 0; a < 36; ++a) {
        double rad = a * 10.0 * 3.141592653589793 / 180.0;
        glVertex3d(10.0 * std::cos(rad), 0.02, 10.0 * std::sin(rad));
    }
    glEnd();
    glLineWidth(1.0f);
}

// Dessin du corps du booster et de sa flamme en 3D OpenGL
void draw_booster_and_flame_3d(const Rocket& myRocket, double tilt_x, double tilt_z, double throttle) {
    glPushMatrix();
    
    // Positionner le booster à ses coordonnées physiques 3D
    glTranslated(myRocket.position.x, myRocket.position.y, myRocket.position.z);
    
    // Appliquer les inclinaisons d'orientation du corps du booster (Pitch, Roll & Yaw réels 6 DOF)
    glRotated(myRocket.theta_z * 180.0 / 3.141592653589793, 1.0, 0.0, 0.0);
    glRotated(myRocket.theta_y * 180.0 / 3.141592653589793, 0.0, 1.0, 0.0); // Roulis (autour de Y)
    glRotated(-myRocket.theta_x * 180.0 / 3.141592653589793, 0.0, 0.0, 1.0);

    int segments = 12;
    double radius = 3.0; // Rayon = 3 mètres (diamètre 6m)
    double height = 30.0; // Hauteur = 30 mètres

    // 1. Fuselage (Cylindre gris métallique)
    glColor3ub(180, 185, 190);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        double cx = radius * std::cos(angle);
        double cz = radius * std::sin(angle);
        glVertex3d(cx, 0.0, cz);
        glVertex3d(cx, height, cz);
    }
    glEnd();

    // 2. Ogive aérodynamique supérieure (Gris sombre)
    glColor3ub(44, 62, 80);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0.0, height + 4.0, 0.0); // Apex
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        glVertex3d(radius * std::cos(angle), height, radius * std::sin(angle));
    }
    glEnd();

    // 3. Anneaux de marquage (Orange et Noir)
    glColor3ub(230, 126, 34); // Orange
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        double cx = radius * std::cos(angle);
        double cz = radius * std::sin(angle);
        glVertex3d(cx, height - 5.0, cz);
        glVertex3d(cx, height - 4.2, cz);
    }
    glEnd();

    // 4. Ailettes de grille de contrôle (Grid Fins) orientables différentiellement
    glColor3ub(52, 73, 78);
    for (int j = 0; j < 4; ++j) {
        double a_fin = j * 3.141592653589793 / 2.0;
        double fx = radius * std::cos(a_fin);
        double fz = radius * std::sin(a_fin);
        
        glPushMatrix();
        // Déplacer l'origine locale vers la charnière de la grille sur la coque du booster
        glTranslated(fx, height - 2.1, fz);
        
        // Commande de déflexion différentielle (Tangage, Lacet et Roulis)
        double defl = 0.0;
        if (j == 0) defl = myRocket.gimbal_z + myRocket.roll_cmd * 0.26;
        else if (j == 2) defl = myRocket.gimbal_z - myRocket.roll_cmd * 0.26;
        else if (j == 1) defl = myRocket.gimbal_x + myRocket.roll_cmd * 0.26;
        else if (j == 3) defl = myRocket.gimbal_x - myRocket.roll_cmd * 0.26;

        // Inclinaison de la grille (pivot vertical Y local) avec amplification visuelle
        glRotated(defl * 180.0 / 3.14159265 * 1.5, 0.0, 1.0, 0.0);

        // Dessin de la grille locale (centrée verticalement sur la charnière, s'étendant vers l'extérieur)
        glBegin(GL_QUADS);
        glVertex3d(0.0, -0.9, 0.0);
        glVertex3d(fx * 0.5, -0.9, fz * 0.5);
        glVertex3d(fx * 0.5, 0.9, fz * 0.5);
        glVertex3d(0.0, 0.9, 0.0);
        glEnd();
        
        glPopMatrix();
    }

    // 5. Tuyère orientable (gimbal)
    glPushMatrix();
    // La tuyère est articulée au centre bas du booster (0.0, 0.0, 0.0)
    glRotated(myRocket.gimbal_z * 180.0 / 3.141592653589793, 1.0, 0.0, 0.0);
    glRotated(-myRocket.gimbal_x * 180.0 / 3.141592653589793, 0.0, 0.0, 1.0);
    
    // Cône métallique de tuyère
    glColor3ub(50, 50, 50);
    double nozzle_rad = radius * 0.4;
    double nozzle_len = 2.0;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        double cx = nozzle_rad * std::cos(angle);
        double cz = nozzle_rad * std::sin(angle);
        glVertex3d(cx, 0.0, cz); 
        glVertex3d(cx * 1.3, -nozzle_len, cz * 1.3); 
    }
    glEnd();

    // Flamme de propulsion sous la tuyère pivotée
    if (throttle > 0.01 && myRocket.fuel_mass > 0.0) {
        double flame_len = 5.0 + 15.0 * throttle + (rand() % 100) / 100.0 * 2.0;
        double flame_rad = nozzle_rad * (0.8 + 0.6 * throttle);

        glBegin(GL_TRIANGLE_FAN);
        // Centre de flamme blanc/jaune chaud
        glColor4ub(255, 230, 100, 240);
        glVertex3d(0.0, -nozzle_len, 0.0);

        // Déperdition rouge translucide
        glColor4ub(231, 76, 60, 40);
        for (int i = 0; i <= segments; ++i) {
            double angle = i * 2.0 * 3.141592653589793 / segments;
            glVertex3d(flame_rad * 1.3 * std::cos(angle), -nozzle_len - flame_len, flame_rad * 1.3 * std::sin(angle));
        }
        glEnd();
    }
    glPopMatrix();

    glPopMatrix();
}

// Dessin des vecteurs de forces 3D à l'écran
void draw_forces_vectors_3d(const Rocket& myRocket, double tilt_x, double tilt_z, const Vector3D& wind, double throttle, double gravity) {
    Vector3D com = myRocket.position + Vector3D{0.0, 15.0, 0.0}; // Centre de masse
    double scale = 0.00015; // Échelle de tracé

    // 1. Force de Gravité (Vert)
    double mass = myRocket.dry_mass + myRocket.fuel_mass;
    double f_grav = mass * gravity;
    glColor3ub(46, 204, 113);
    glBegin(GL_LINES);
    glVertex3d(com.x, com.y, com.z);
    glVertex3d(com.x, com.y - f_grav * scale, com.z);
    glEnd();

    // 2. Force de Poussée Moteur (Bleu)
    double thrust = myRocket.fuel_mass > 0 ? myRocket.max_mass_flow_rate * throttle * myRocket.effective_exhaust_velocity : 0.0;
    // Orientation réelle de la poussée = attitude + déviation gimbal
    double thrust_angle_x = myRocket.theta_x + myRocket.gimbal_x;
    double thrust_angle_z = myRocket.theta_z + myRocket.gimbal_z;
    double sin_ax = std::sin(thrust_angle_x);
    double sin_az = std::sin(thrust_angle_z);
    double cos_ax = std::cos(thrust_angle_x);
    double cos_az = std::cos(thrust_angle_z);
    Vector3D thrust_dir = { sin_ax, cos_ax * cos_az, sin_az };
    thrust_dir = thrust_dir.normalized();

    glColor3ub(52, 152, 219);
    glBegin(GL_LINES);
    glVertex3d(myRocket.position.x, myRocket.position.y, myRocket.position.z);
    glVertex3d(myRocket.position.x + thrust_dir.x * thrust * scale,
               myRocket.position.y + thrust_dir.y * thrust * scale,
               myRocket.position.z + thrust_dir.z * thrust * scale);
    glEnd();

    // 3. Force de Traînée Aérodynamique (Jaune)
    Vector3D drag = get_drag_force(myRocket.position.y, myRocket.velocity, myRocket.theta_x, myRocket.theta_z, wind);
    glColor3ub(241, 196, 15);
    glBegin(GL_LINES);
    glVertex3d(com.x, com.y, com.z);
    glVertex3d(com.x + drag.x * scale,
               com.y + drag.y * scale,
               com.z + drag.z * scale);
    glEnd();
}
