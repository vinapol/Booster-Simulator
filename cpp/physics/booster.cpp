#include "booster.hpp"
#include <algorithm>
#include <cmath>

State State::operator+(const State& other) const {
    return {
        pos + other.pos,
        vel + other.vel,
        theta_x + other.theta_x,
        theta_y + other.theta_y,
        theta_z + other.theta_z,
        omega_x + other.omega_x,
        omega_y + other.omega_y,
        omega_z + other.omega_z
    };
}

State State::operator*(double scalar) const {
    return {
        pos * scalar,
        vel * scalar,
        theta_x * scalar,
        theta_y * scalar,
        theta_z * scalar,
        omega_x * scalar,
        omega_y * scalar,
        omega_z * scalar
    };
}

Rocket::Rocket(double dry, double fuel, double ve, double m_dot,
               const Vector3D& pos, const Vector3D& vel)
    : position(pos), velocity(vel),
      dry_mass(dry), fuel_mass(fuel),
      effective_exhaust_velocity(ve), max_mass_flow_rate(m_dot) {}

void Rocket::update_mass(double dt, double throttle) {
    double current_flow = max_mass_flow_rate * throttle;
    fuel_mass -= current_flow * dt;
    if (fuel_mass < 0)
        fuel_mass = 0;
}

double Rocket::get_thrust(double throttle) {
    return (fuel_mass > 0) ? (max_mass_flow_rate * throttle * effective_exhaust_velocity) : 0.0;
}

State Rocket::get_derivative(const State& s, double gravity_sea_level, double thrust,
                             double gimbal_x, double gimbal_z, double roll_cmd,
                             double mass, const Vector3D& wind)
{
    // 1. Force de traînée aérodynamique
    Vector3D drag = get_drag_force(s.pos.y, s.vel, s.theta_x, s.theta_z, wind);

    // 2. Gravité variable avec l'altitude : g(h) = g0 * (R / (R + h))^2
    const double Earth_Radius = 6371000.0;
    double gravity = gravity_sea_level * (Earth_Radius / (Earth_Radius + s.pos.y))
                                      * (Earth_Radius / (Earth_Radius + s.pos.y));

    // 3. Composantes de la poussée en 3D
    double thrust_angle_x = s.theta_x + gimbal_x;
    double thrust_angle_z = s.theta_z + gimbal_z;

    double sin_ax = std::sin(thrust_angle_x);
    double sin_az = std::sin(thrust_angle_z);
    double cos_ax = std::cos(thrust_angle_x);
    double cos_az = std::cos(thrust_angle_z);

    Vector3D u_thrust = {
        sin_ax,
        cos_ax * cos_az,
        sin_az
    };
    u_thrust = u_thrust.normalized();

    Vector3D thrust_vec = u_thrust * thrust;

    // Couples et forces des Grid Fins actifs (avec Roulis)
    double F_fin_x = 0.0;
    double F_fin_z = 0.0;
    double torque_roll_fin = 0.0;
    double torque_roll_damp = 0.0;

    Vector3D rel_vel = s.vel - wind;
    double rel_speed = rel_vel.length();
    double rho = get_air_density(s.pos.y);

    double torque_aero_x = 0.0;
    double torque_aero_z = 0.0;

    if (rel_speed > 0.01) {
        double phi_x = std::atan2(rel_vel.x, rel_vel.y);
        double phi_z = std::atan2(rel_vel.z, rel_vel.y);

        double aoa_x = s.theta_x - phi_x;
        double aoa_z = s.theta_z - phi_z;

        const double C_stabil = 15.0;
        const double C_damp = 20.0;

        double dynamic_pressure_factor = 0.5 * rho * rel_speed;

        torque_aero_x = -C_stabil * aoa_z * dynamic_pressure_factor - C_damp * s.omega_z * dynamic_pressure_factor;
        torque_aero_z = -C_stabil * aoa_x * dynamic_pressure_factor - C_damp * s.omega_x * dynamic_pressure_factor;

        double C_fin_lift = 8.0;
        F_fin_x = -dynamic_pressure_factor * rel_speed * C_fin_lift * gimbal_x;
        F_fin_z = -dynamic_pressure_factor * rel_speed * C_fin_lift * gimbal_z;

        double d_fins = 15.0;
        torque_aero_x += d_fins * F_fin_z;
        torque_aero_z += -d_fins * F_fin_x;

        double C_roll_fin = 2.0;
        double C_roll_damp = 5.0;
        torque_roll_fin = dynamic_pressure_factor * rel_speed * C_roll_fin * roll_cmd;
        torque_roll_damp = -C_roll_damp * s.omega_y * dynamic_pressure_factor;
    }

    double acc_x = (thrust_vec.x + drag.x + F_fin_x) / mass;
    double acc_y = (thrust_vec.y - (mass * gravity) + drag.y) / mass;
    double acc_z = (thrust_vec.z + drag.z + F_fin_z) / mass;

    double I_xz = mass * 77.25;
    double I_y = mass * 4.5;

    double d_engine = -15.0;
    double torque_thrust_x = d_engine * thrust * std::sin(gimbal_z);
    double torque_thrust_z = -d_engine * thrust * std::sin(gimbal_x);

    double alpha_x = (torque_thrust_z + torque_aero_z) / I_xz;
    double alpha_y = (torque_roll_fin + torque_roll_damp) / I_y;
    double alpha_z = (torque_thrust_x + torque_aero_x) / I_xz;

    return {
        s.vel,
        {acc_x, acc_y, acc_z},
        s.omega_x,
        s.omega_y,
        s.omega_z,
        alpha_x,
        alpha_y,
        alpha_z
    };
}

void Rocket::update_rk4(double dt, double gravity, double thrust,
                        double target_gimbal_x, double target_gimbal_z, double target_roll,
                        const Vector3D& wind)
{
    // Lag actuateur : limité à 30 degrés/s (~0.52 rad/s)
    double max_gimbal_speed = 0.52;
    gimbal_x += std::clamp(target_gimbal_x - gimbal_x, -max_gimbal_speed * dt, max_gimbal_speed * dt);
    gimbal_z += std::clamp(target_gimbal_z - gimbal_z, -max_gimbal_speed * dt, max_gimbal_speed * dt);
    roll_cmd = target_roll;

    double total_mass = dry_mass + fuel_mass;
    State s0 = {position, velocity, theta_x, theta_y, theta_z, omega_x, omega_y, omega_z};

    State k1 = get_derivative(s0, gravity, thrust, gimbal_x, gimbal_z, roll_cmd, total_mass, wind);

    State s_k2 = {
        s0.pos + k1.pos * (dt / 2.0),
        s0.vel + k1.vel * (dt / 2.0),
        s0.theta_x + k1.theta_x * (dt / 2.0),
        s0.theta_y + k1.theta_y * (dt / 2.0),
        s0.theta_z + k1.theta_z * (dt / 2.0),
        s0.omega_x + k1.omega_x * (dt / 2.0),
        s0.omega_y + k1.omega_y * (dt / 2.0),
        s0.omega_z + k1.omega_z * (dt / 2.0)
    };
    State k2 = get_derivative(s_k2, gravity, thrust, gimbal_x, gimbal_z, roll_cmd, total_mass, wind);

    State s_k3 = {
        s0.pos + k2.pos * (dt / 2.0),
        s0.vel + k2.vel * (dt / 2.0),
        s0.theta_x + k2.theta_x * (dt / 2.0),
        s0.theta_y + k2.theta_y * (dt / 2.0),
        s0.theta_z + k2.theta_z * (dt / 2.0),
        s0.omega_x + k2.omega_x * (dt / 2.0),
        s0.omega_y + k2.omega_y * (dt / 2.0),
        s0.omega_z + k2.omega_z * (dt / 2.0)
    };
    State k3 = get_derivative(s_k3, gravity, thrust, gimbal_x, gimbal_z, roll_cmd, total_mass, wind);

    State s_k4 = {
        s0.pos + k3.pos * dt,
        s0.vel + k3.vel * dt,
        s0.theta_x + k3.theta_x * dt,
        s0.theta_y + k3.theta_y * dt,
        s0.theta_z + k3.theta_z * dt,
        s0.omega_x + k3.omega_x * dt,
        s0.omega_y + k3.omega_y * dt,
        s0.omega_z + k3.omega_z * dt
    };
    State k4 = get_derivative(s_k4, gravity, thrust, gimbal_x, gimbal_z, roll_cmd, total_mass, wind);

    position = position + (k1.pos + (k2.pos + k3.pos) * 2.0 + k4.pos) * (dt / 6.0);
    velocity = velocity + (k1.vel + (k2.vel + k3.vel) * 2.0 + k4.vel) * (dt / 6.0);

    theta_x = theta_x + (k1.theta_x + (k2.theta_x + k3.theta_x) * 2.0 + k4.theta_x) * (dt / 6.0);
    theta_y = theta_y + (k1.theta_y + (k2.theta_y + k3.theta_y) * 2.0 + k4.theta_y) * (dt / 6.0);
    theta_z = theta_z + (k1.theta_z + (k2.theta_z + k3.theta_z) * 2.0 + k4.theta_z) * (dt / 6.0);

    omega_x = omega_x + (k1.omega_x + (k2.omega_x + k3.omega_x) * 2.0 + k4.omega_x) * (dt / 6.0);
    omega_y = omega_y + (k1.omega_y + (k2.omega_y + k3.omega_y) * 2.0 + k4.omega_y) * (dt / 6.0);
    omega_z = omega_z + (k1.omega_z + (k2.omega_z + k3.omega_z) * 2.0 + k4.omega_z) * (dt / 6.0);
}
