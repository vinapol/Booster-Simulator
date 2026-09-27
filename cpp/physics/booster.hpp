#pragma once
#include "physique.hpp"
#include "aerodynamique.hpp"

// État 8 dimensions (6 DOF complets)
struct State
{
    Vector3D pos;
    Vector3D vel;
    double theta_x = 0.0; // Orientation Pitch (rad)
    double theta_y = 0.0; // Orientation Roll (rad)
    double theta_z = 0.0; // Orientation Yaw (rad)
    double omega_x = 0.0; // Vitesse angulaire Pitch (rad/s)
    double omega_y = 0.0; // Vitesse angulaire Roll (rad/s)
    double omega_z = 0.0; // Vitesse angulaire Yaw (rad/s)

    State operator+(const State& other) const;
    State operator*(double scalar) const;
};

class Rocket
{
public:
    Vector3D position;
    Vector3D velocity;
    double dry_mass;
    double fuel_mass;
    double effective_exhaust_velocity; // v_e (ex: 3000 m/s)
    double max_mass_flow_rate;         // m_dot (ex: 50 kg/s)

    // Attributs angulaires physiques 6 DOF
    double theta_x = 0.0;
    double theta_y = 0.0; // Roll
    double theta_z = 0.0;
    double omega_x = 0.0;
    double omega_y = 0.0; // Roll rate
    double omega_z = 0.0;

    // Actuateurs réels
    double gimbal_x = 0.0;
    double gimbal_z = 0.0;
    double roll_cmd = 0.0; // Commande de roulis active

    Rocket(double dry, double fuel, double ve, double m_dot,
           const Vector3D& pos = {50.0, 3000.0, 0.0},
           const Vector3D& vel = {0.0, -150.0, 0.0});

    void update_mass(double dt, double throttle);
    double get_thrust(double throttle);

    static State get_derivative(const State& s, double gravity_sea_level, double thrust,
                                double gimbal_x, double gimbal_z, double roll_cmd,
                                double mass, const Vector3D& wind);

    void update_rk4(double dt, double gravity, double thrust,
                    double target_gimbal_x, double target_gimbal_z, double target_roll,
                    const Vector3D& wind = {0.0, 0.0, 0.0});
};

