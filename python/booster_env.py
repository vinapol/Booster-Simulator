import gymnasium as gym
from gymnasium import spaces
import numpy as np

class Booster3DEnv(gym.Env):
    """
    Environnement Gymnasium 3D 6 DOF pour l'atterrissage vertical d'un booster SpaceX Falcon.
    """
    metadata = {"render_modes": ["human"]}

    def __init__(self):
        super(Booster3DEnv, self).__init__()

        # Actions : [Throttle (-1 à 1), Target Gimbal X, Target Gimbal Z, Roll Command]
        self.action_space = spaces.Box(
            low=np.array([-1.0, -1.0, -1.0, -1.0], dtype=np.float32),
            high=np.array([1.0, 1.0, 1.0, 1.0], dtype=np.float32),
            dtype=np.float32
        )

        # Observations (16 dimensions) :
        # [pos_x, relative_alt, pos_z, vel_x, vel_y, vel_z, theta_x, theta_y, theta_z, omega_x, omega_y, omega_z, fuel, wind_x, wind_z, absolute_alt]
        self.observation_space = spaces.Box(
            low=-np.inf,
            high=np.inf,
            shape=(16,),
            dtype=np.float32
        )

        # Constantes physiques par défaut (randomisées par Domain Randomization)
        self.dry_mass_default = 10000.0
        self.max_fuel_mass_default = 4000.0
        self.ve_default = 3000.0
        self.m_dot_default = 100.0
        
        self.dry_mass = self.dry_mass_default
        self.max_fuel_mass = self.max_fuel_mass_default
        self.ve = self.ve_default
        self.m_dot = self.m_dot_default
        
        self.gravity_sea = 9.81
        self.dt = 0.016 # 60 Hz physics

        # Niveau de difficulté pour le Curriculum Learning (0.0 à 1.0)
        self.difficulty = 0.0
        self.stage = 1
        self.current_episode_stage = 1
        self.curriculum_training = False

    def set_stage(self, stage_val):
        self.stage = int(np.clip(stage_val, 1, 6))
        self.current_episode_stage = self.stage

    def set_difficulty(self, value):
        self.difficulty = np.clip(value, 0.0, 1.0)

    def _get_air_density(self, altitude):
        rho0 = 1.225
        H = 8500.0
        if altitude < 0:
            return rho0
        return rho0 * np.exp(-altitude / H)

    def _get_drag_force(self, altitude, vel, theta_x, theta_z, wind):
        rel_vel = vel - wind
        speed = np.linalg.norm(rel_vel)
        if speed < 1e-3:
            return np.zeros(3)

        rho = self._get_air_density(altitude)

        # Vecteur unitaire de la vitesse relative
        u_rel = rel_vel / speed

        # Vecteur d'orientation du booster
        sin_tx, sin_tz = np.sin(theta_x), np.sin(theta_z)
        cos_tx, cos_tz = np.cos(theta_x), np.cos(theta_z)
        
        u_booster = np.array([
            sin_tx,
            cos_tx * cos_tz,
            sin_tz
        ])
        norm_booster = np.linalg.norm(u_booster)
        if norm_booster > 1e-9:
            u_booster = u_booster / norm_booster

        # Angle d'attaque
        cos_alpha = np.clip(np.dot(u_booster, u_rel), -1.0, 1.0)
        sin_alpha = np.sqrt(1.0 - cos_alpha**2)

        # Caractéristiques physiques (diamètre 6m, hauteur 30m)
        Cd_axial = 0.5
        Cd_lateral = 1.2
        Area_axial = 28.27
        Area_lateral = 180.0

        cos_a2 = cos_alpha**2
        sin_a2 = sin_alpha**2

        Cd = Cd_axial * cos_a2 + Cd_lateral * sin_a2
        Area = Area_axial * cos_a2 + Area_lateral * sin_a2

        drag_mag = 0.5 * rho * Cd * Area * (speed**2)
        return -u_rel * drag_mag

    def _get_derivative(self, pos, vel, theta_x, theta_y, theta_z, omega_x, omega_y, omega_z, gimbal_x, gimbal_z, roll_cmd, fuel_mass, wind):
        mass = self.dry_mass + fuel_mass
        
        # 1. Traînée
        drag = self._get_drag_force(pos[1], vel, theta_x, theta_z, wind)

        # 2. Gravité variable
        Earth_Radius = 6371000.0
        gravity = self.gravity_sea * (Earth_Radius / (Earth_Radius + pos[1]))**2

        # 3. Poussée moteur
        thrust_angle_x = theta_x + gimbal_x
        thrust_angle_z = theta_z + gimbal_z

        u_thrust = np.array([
            np.sin(thrust_angle_x),
            np.cos(thrust_angle_x) * np.cos(thrust_angle_z),
            np.sin(thrust_angle_z)
        ])
        norm_t = np.linalg.norm(u_thrust)
        if norm_t > 1e-9:
            u_thrust = u_thrust / norm_t

        thrust = 0.0
        if fuel_mass > 0.0:
            thrust = self.m_dot * self.throttle * self.ve

        thrust_vec = u_thrust * thrust

        # Couples et forces des Grid Fins actifs (avec Roulis)
        F_fin_x = 0.0
        F_fin_z = 0.0
        torque_roll_fin = 0.0
        torque_roll_damp = 0.0

        # Couples aérodynamiques
        rel_vel = vel - wind
        rel_speed = np.linalg.norm(rel_vel)
        rho = self._get_air_density(pos[1])

        torque_aero_x = 0.0
        torque_aero_z = 0.0

        if rel_speed > 0.01:
            phi_x = np.arctan2(rel_vel[0], rel_vel[1])
            phi_z = np.arctan2(rel_vel[2], rel_vel[1])

            aoa_x = theta_x - phi_x
            aoa_z = theta_z - phi_z

            C_stabil = 15.0
            C_damp = 20.0
            q = 0.5 * rho * rel_speed # Facteur pression dynamique

            torque_aero_x = -C_stabil * aoa_z * q - C_damp * omega_z * q
            torque_aero_z = -C_stabil * aoa_x * q - C_damp * omega_x * q

            # Le couplage est négatif pour que les forces générées en haut du booster coopèrent avec la poussée orientée en bas
            C_fin_lift = 8.0
            F_fin_x = -q * rel_speed * C_fin_lift * gimbal_x
            F_fin_z = -q * rel_speed * C_fin_lift * gimbal_z
            
            d_fins = 15.0
            torque_aero_x += d_fins * F_fin_z
            torque_aero_z += -d_fins * F_fin_x

            # Roulis contrôlé par les Grid Fins différentiels
            C_roll_fin = 2.0
            C_roll_damp = 5.0
            torque_roll_fin = q * rel_speed * C_roll_fin * roll_cmd
            torque_roll_damp = -C_roll_damp * omega_y * q

        # Accélérations linéaires (avec forces des grid fins)
        acc = (thrust_vec + drag + np.array([F_fin_x, 0.0, F_fin_z])) / mass
        acc[1] -= gravity

        # 4. Moments d'inertie
        I_xz = mass * 77.25
        I_y = mass * 4.5 # Moment d'inertie en roulis (0.5 * m * r^2 avec r=3m)

        # 5. Couples moteur
        d_engine = -15.0
        torque_thrust_x = d_engine * thrust * np.sin(gimbal_z) # -15.0 * thrust * sin(gimbal_z)
        torque_thrust_z = -d_engine * thrust * np.sin(gimbal_x) # 15.0 * thrust * sin(gimbal_x)

        # Accélérations angulaires
        alpha_x = (torque_thrust_z + torque_aero_z) / I_xz # Pitch
        alpha_y = (torque_roll_fin + torque_roll_damp) / I_y # Roll
        alpha_z = (torque_thrust_x + torque_aero_x) / I_xz # Yaw

        return vel, acc, omega_x, omega_y, omega_z, alpha_x, alpha_y, alpha_z

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        
        # Difficulté effective aléatoire pour éviter l'oubli catastrophique
        eff_difficulty = np.random.uniform(0.0, self.difficulty)
        
        # Choisir le stage pour cet épisode (Curriculum Sampling pour éviter l'oubli catastrophique)
        if self.curriculum_training and self.stage > 1:
            self.current_episode_stage = np.random.randint(1, self.stage + 1)
        else:
            self.current_episode_stage = self.stage
        
        # 1. Configuration de l'état initial selon le stage (1 à 6)
        if self.current_episode_stage == 1:
            # Stage 1 : Vol stationnaire à 200m
            self.dry_mass = self.dry_mass_default
            self.ve = self.ve_default
            self.max_fuel_mass = self.max_fuel_mass_default
            self.pos = np.array([0.0, 200.0, 0.0], dtype=np.float64)
            self.vel = np.array([0.0, 0.0, 0.0], dtype=np.float64)
            self.theta_x = self.theta_y = self.theta_z = 0.0
            self.wind_base = 0.0
            self.hover_steps = 0
        elif self.current_episode_stage == 2:
            # Stage 2 : Petite chute libre (300m), atterrissage n'importe où (pas de zone)
            self.dry_mass = self.dry_mass_default
            self.ve = self.ve_default
            self.max_fuel_mass = self.max_fuel_mass_default
            self.pos = np.array([0.0, 300.0, 0.0], dtype=np.float64)
            self.vel = np.array([0.0, -15.0, 0.0], dtype=np.float64)
            self.theta_x = self.theta_y = self.theta_z = 0.0
            self.wind_base = 0.0
        elif self.current_episode_stage == 3:
            # Stage 3 : Landing sur le pad (300m, dérive latérale)
            self.dry_mass = self.dry_mass_default
            self.ve = self.ve_default
            self.max_fuel_mass = self.max_fuel_mass_default
            spawn_offset = 20.0
            spawn_x = np.random.uniform(-spawn_offset, spawn_offset)
            spawn_z = np.random.uniform(-spawn_offset, spawn_offset)
            self.pos = np.array([spawn_x, 300.0, spawn_z], dtype=np.float64)
            self.vel = np.array([0.0, -15.0, 0.0], dtype=np.float64)
            self.theta_x = self.theta_y = self.theta_z = 0.0
            self.wind_base = 0.0
        elif self.current_episode_stage == 4:
            # Stage 4 : Lâché plus haut (1000m), sans vent
            self.dry_mass = self.dry_mass_default
            self.ve = self.ve_default
            self.max_fuel_mass = self.max_fuel_mass_default
            spawn_offset = 30.0
            spawn_x = np.random.uniform(-spawn_offset, spawn_offset)
            spawn_z = np.random.uniform(-spawn_offset, spawn_offset)
            self.pos = np.array([spawn_x, 1000.0, spawn_z], dtype=np.float64)
            self.vel = np.array([0.0, -50.0, 0.0], dtype=np.float64)
            spawn_tilt = 0.1
            self.theta_x = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.theta_y = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.theta_z = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.wind_base = 0.0
        elif self.current_episode_stage == 5:
            # Stage 5 : Lâché à 1000m, avec vent et domain randomization partielle
            mass_var = 0.05 * np.random.uniform(-1.0, 1.0)
            self.dry_mass = self.dry_mass_default * (1.0 + mass_var)
            self.ve = self.ve_default * (1.0 + mass_var)
            self.max_fuel_mass = self.max_fuel_mass_default * (1.0 + mass_var)
            
            spawn_offset = 30.0
            spawn_x = np.random.uniform(-spawn_offset, spawn_offset)
            spawn_z = np.random.uniform(-spawn_offset, spawn_offset)
            self.pos = np.array([spawn_x, 1000.0, spawn_z], dtype=np.float64)
            self.vel = np.array([0.0, -50.0, 0.0], dtype=np.float64)
            spawn_tilt = 0.1
            self.theta_x = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.theta_y = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.theta_z = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.wind_base = np.random.uniform(-20.0, 20.0)
        elif self.current_episode_stage == 6:
            # Stage 6 : Lâché à 3000m (conditions réelles)
            mass_var = eff_difficulty * 0.1 * np.random.uniform(-1.0, 1.0)
            thrust_var = eff_difficulty * 0.1 * np.random.uniform(-1.0, 1.0)
            self.dry_mass = self.dry_mass_default * (1.0 + mass_var)
            self.ve = self.ve_default * (1.0 + thrust_var)
            self.max_fuel_mass = self.max_fuel_mass_default * (1.0 + mass_var)
            
            spawn_offset = eff_difficulty * 40.0
            spawn_x = np.random.uniform(-spawn_offset, spawn_offset)
            spawn_z = np.random.uniform(-spawn_offset, spawn_offset)
            self.pos = np.array([spawn_x, 3000.0, spawn_z], dtype=np.float64)
            self.vel = np.array([0.0, -150.0, 0.0], dtype=np.float64)
            spawn_tilt = eff_difficulty * 0.2
            self.theta_x = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.theta_y = np.random.uniform(-spawn_tilt, spawn_tilt)
            self.theta_z = np.random.uniform(-spawn_tilt, spawn_tilt)
            wind_max = eff_difficulty * 35.0
            self.wind_base = np.random.uniform(-wind_max, wind_max)

        self.omega_x = 0.0
        self.omega_y = 0.0
        self.omega_z = 0.0
        
        self.gimbal_x = 0.0
        self.gimbal_z = 0.0
        self.roll_cmd = 0.0
        self.fuel_mass = self.max_fuel_mass
        self.throttle = 0.0
        self.time = 0.0
        self.wind_noise = 0.0
        self.initial_altitude = self.pos[1]

        self.action_history = []
        self.prev_action = np.zeros(4, dtype=np.float32)
        self.potential = self._compute_potential()

        return self._get_obs(), {}

    def _compute_potential(self):
        dist_xz = np.sqrt(self.pos[0]**2 + self.pos[2]**2)
        vel_norm = np.linalg.norm(self.vel)
        attitude_err = self.theta_x**2 + self.theta_y**2 + self.theta_z**2

        # Pas d'incitation à s'approcher du centre horizontal aux stages 1 et 2
        dist_weight = 0.0 if self.current_episode_stage in [1, 2] else 0.2
        target_alt = 200.0 if self.current_episode_stage == 1 else 0.0

        return (
            - 0.5    * abs(self.pos[1] - target_alt)
            - dist_weight * dist_xz
            - 4.0    * vel_norm
            - 15.0   * attitude_err
        )

    def _compute_autopilot(self):
        # 1. CONTRÔLE VERTICAL (POUSSÉE)
        target_alt = 200.0 if self.current_episode_stage == 1 else 0.0
        relative_alt = self.pos[1] - target_alt
        
        # Profil de vitesse cible : proportionnel pour le stationnaire, parabolique pour l'atterrissage
        if self.current_episode_stage == 1:
            target_vy = -0.5 * relative_alt
            target_vy = np.clip(target_vy, -15.0, 15.0)
        else:
            alt_err = max(0.0, relative_alt)
            target_vy = -np.sqrt(2.0 * 3.8 * alt_err) - 0.5
            if alt_err < 3.0:
                target_vy = -1.0
            
        error_vy = target_vy - self.vel[1]
        mass = self.dry_mass + self.fuel_mass
        max_thrust = self.m_dot * self.ve
        gravity_feedforward = (mass * 9.81) / max_thrust
        
        ap_throttle = np.clip(gravity_feedforward + 0.35 * error_vy, 0.0, 1.0)
        
        # 2. CONTRÔLE HORIZONTAL : BOUCLE CASCADE
        max_theta = 25.0 * np.pi / 180.0
        max_gimbal = 15.0 * np.pi / 180.0
        
        # AXE X (Pitch / Tangage)
        target_vx = -0.15 * self.pos[0]
        target_vx = np.clip(target_vx, -25.0, 25.0)
        error_vx = target_vx - self.vel[0]
        target_theta_x = 0.05 * error_vx
        target_theta_x = np.clip(target_theta_x, -max_theta, max_theta)
        
        error_theta_x = target_theta_x - self.theta_x
        target_gimbal_x = 1.2 * error_theta_x - 0.85 * self.omega_x
        ap_gimbal_x = np.clip(target_gimbal_x, -max_gimbal, max_gimbal)
        
        # AXE Z (Yaw / Lacet)
        target_vz = -0.15 * self.pos[2]
        target_vz = np.clip(target_vz, -25.0, 25.0)
        error_vz = target_vz - self.vel[2]
        target_theta_z = 0.05 * error_vz
        target_theta_z = np.clip(target_theta_z, -max_theta, max_theta)
        
        error_theta_z = target_theta_z - self.theta_z
        target_gimbal_z = -1.2 * error_theta_z + 0.85 * self.omega_z
        ap_gimbal_z = np.clip(target_gimbal_z, -max_gimbal, max_gimbal)
        
        return ap_throttle, ap_gimbal_x, ap_gimbal_z

    def _get_obs(self):
        # Composantes de vent au temps t
        wind_alt_factor = 0.3 + 0.7 * (self.pos[1] / 1000.0)
        if wind_alt_factor < 0.3:
            wind_alt_factor = 0.3
        
        current_wind_x = (self.wind_base * np.cos(self.time * 0.15) + self.wind_noise) * wind_alt_factor
        current_wind_z = (self.wind_base * 0.35 * np.sin(self.time * 0.25)) * wind_alt_factor

        target_alt = 200.0 if self.current_episode_stage == 1 else 0.0
        relative_alt = self.pos[1] - target_alt

        # Normalisation des valeurs pour le réseau de neurones (altitude relative à la cible + altitude absolue)
        return np.array([
            self.pos[0] / 100.0,
            relative_alt / 1000.0,
            self.pos[2] / 100.0,
            self.vel[0] / 100.0,
            self.vel[1] / 100.0,
            self.vel[2] / 100.0,
            self.theta_x,
            self.theta_y, # Roll
            self.theta_z,
            self.omega_x,
            self.omega_y, # Roll rate
            self.omega_z,
            self.fuel_mass / 4000.0,
            current_wind_x / 20.0,
            current_wind_z / 20.0,
            self.pos[1] / 1000.0
        ], dtype=np.float32)

    def step(self, action):
        # Enregistrer l'action courante dans l'historique
        self.action_history.append(action)

        # Calculer les commandes du pilote automatique classique
        ap_throttle, ap_gimbal_x, ap_gimbal_z = self._compute_autopilot()

        # Limite maximale du Gimbal (+/- 15 deg max)
        max_gim = 15.0 * np.pi / 180.0

        # 1. Combiner le pilote automatique avec la correction résiduelle de l'IA
        # action[0] (poussée) corrige à +/- 20%
        self.throttle = np.clip(ap_throttle + 0.2 * action[0], 0.0, 1.0)
        
        # action[1] et action[2] (gimbals) corrigent à +/- 5 degrés (+/- 0.087 rad)
        target_gimbal_x = np.clip(ap_gimbal_x + 0.087 * action[1], -max_gim, max_gim)
        target_gimbal_z = np.clip(ap_gimbal_z + 0.087 * action[2], -max_gim, max_gim)
        self.roll_cmd = np.clip(action[3], -1.0, 1.0)

        # 2. Retard physique sur les actuateurs de tuyère (gimbal speed limit 30 deg/s)
        max_gim_speed = 0.52 # rad/s
        self.gimbal_x += np.clip(target_gimbal_x - self.gimbal_x, -max_gim_speed * self.dt, max_gim_speed * self.dt)
        self.gimbal_z += np.clip(target_gimbal_z - self.gimbal_z, -max_gim_speed * self.dt, max_gim_speed * self.dt)

        # 3. Calcul du vent tridimensionnel
        wind_alt_factor = 0.3 + 0.7 * (self.pos[1] / 1000.0)
        if wind_alt_factor < 0.3:
            wind_alt_factor = 0.3
        self.wind_noise = 0.98 * self.wind_noise + 0.02 * np.random.uniform(-12.0, 12.0)
        
        current_wind_x = (self.wind_base * np.cos(self.time * 0.15) + self.wind_noise) * wind_alt_factor
        current_wind_z = (self.wind_base * 0.35 * np.sin(self.time * 0.25)) * wind_alt_factor
        wind = np.array([current_wind_x, 0.0, current_wind_z])

        # 4. Intégrateur RK4 en Python (8 dimensions d'état)
        s0 = (self.pos, self.vel, self.theta_x, self.theta_y, self.theta_z, self.omega_x, self.omega_y, self.omega_z)

        # k1
        v1, a1, o_x1, o_y1, o_z1, al_x1, al_y1, al_z1 = self._get_derivative(*s0, self.gimbal_x, self.gimbal_z, self.roll_cmd, self.fuel_mass, wind)

        # k2
        s_k2 = (
            s0[0] + v1 * (self.dt / 2.0),
            s0[1] + a1 * (self.dt / 2.0),
            s0[2] + o_x1 * (self.dt / 2.0),
            s0[3] + o_y1 * (self.dt / 2.0),
            s0[4] + o_z1 * (self.dt / 2.0),
            s0[5] + al_x1 * (self.dt / 2.0),
            s0[6] + al_y1 * (self.dt / 2.0),
            s0[7] + al_z1 * (self.dt / 2.0)
        )
        v2, a2, o_x2, o_y2, o_z2, al_x2, al_y2, al_z2 = self._get_derivative(*s_k2, self.gimbal_x, self.gimbal_z, self.roll_cmd, self.fuel_mass, wind)

        # k3
        s_k3 = (
            s0[0] + v2 * (self.dt / 2.0),
            s0[1] + a2 * (self.dt / 2.0),
            s0[2] + o_x2 * (self.dt / 2.0),
            s0[3] + o_y2 * (self.dt / 2.0),
            s0[4] + o_z2 * (self.dt / 2.0),
            s0[5] + al_x2 * (self.dt / 2.0),
            s0[6] + al_y2 * (self.dt / 2.0),
            s0[7] + al_z2 * (self.dt / 2.0)
        )
        v3, a3, o_x3, o_y3, o_z3, al_x3, al_y3, al_z3 = self._get_derivative(*s_k3, self.gimbal_x, self.gimbal_z, self.roll_cmd, self.fuel_mass, wind)

        # k4
        s_k4 = (
            s0[0] + v3 * self.dt,
            s0[1] + a3 * self.dt,
            s0[2] + o_x3 * self.dt,
            s0[3] + o_y3 * self.dt,
            s0[4] + o_z3 * self.dt,
            s0[5] + al_x3 * self.dt,
            s0[6] + al_y3 * self.dt,
            s0[7] + al_z3 * self.dt
        )
        v4, a4, o_x4, o_y4, o_z4, al_x4, al_y4, al_z4 = self._get_derivative(*s_k4, self.gimbal_x, self.gimbal_z, self.roll_cmd, self.fuel_mass, wind)

        # Mise à jour RK4
        self.pos = self.pos + (v1 + (v2 + v3) * 2.0 + v4) * (self.dt / 6.0)
        self.vel = self.vel + (a1 + (a2 + a3) * 2.0 + a4) * (self.dt / 6.0)
        self.theta_x = self.theta_x + (o_x1 + (o_x2 + o_x3) * 2.0 + o_x4) * (self.dt / 6.0)
        self.theta_y = self.theta_y + (o_y1 + (o_y2 + o_y3) * 2.0 + o_y4) * (self.dt / 6.0)
        self.theta_z = self.theta_z + (o_z1 + (o_z2 + o_z3) * 2.0 + o_z4) * (self.dt / 6.0)
        self.omega_x = self.omega_x + (al_x1 + (al_x2 + al_x3) * 2.0 + al_x4) * (self.dt / 6.0)
        self.omega_y = self.omega_y + (al_y1 + (al_y2 + al_y3) * 2.0 + al_y4) * (self.dt / 6.0)
        self.omega_z = self.omega_z + (al_z1 + (al_z2 + al_z3) * 2.0 + al_z4) * (self.dt / 6.0)

        # Consommation carburant
        flow = self.m_dot * self.throttle
        self.fuel_mass = max(0.0, self.fuel_mass - flow * self.dt)
        self.time += self.dt

        # 5. Conditions de terminaison
        terminated = False
        truncated = False
        is_tilt_crash = False
        is_flyaway = False
        success = False

        if self.current_episode_stage == 1:
            # Vol stationnaire : succès si reste à 200m [120m, 280m] avec attitude stable
            in_hover_range = (120.0 < self.pos[1] < 280.0)
            upright = (abs(self.theta_x) < 0.2 and abs(self.theta_z) < 0.2)
            
            if in_hover_range and upright:
                self.hover_steps += 1
            else:
                self.hover_steps = 0
                
            if self.hover_steps >= 350:
                terminated = True
                success = True
            elif self.pos[1] <= 0.0 or self.pos[1] > 400.0 or abs(self.theta_x) > 1.05 or abs(self.theta_z) > 1.05:
                terminated = True
                success = False
                if abs(self.theta_x) > 1.05 or abs(self.theta_z) > 1.05:
                    is_tilt_crash = True
        else:
            # Atterrissage standard
            if self.pos[1] <= 0.0:
                self.pos[1] = 0.0
                terminated = True
            elif abs(self.theta_x) > 1.05 or abs(self.theta_y) > 1.05 or abs(self.theta_z) > 1.05:
                terminated = True
                is_tilt_crash = True
            elif self.pos[1] > self.initial_altitude + 100.0:
                terminated = True
                is_flyaway = True

        # Truncation par temps (20s pour Stage 1, 60s pour Stages >= 2)
        max_time = 20.0 if self.current_episode_stage == 1 else 60.0
        if self.time > max_time:
            truncated = True
        elif self.current_episode_stage != 1:
            if self.pos[1] > 4200.0 or np.any(np.abs(self.pos[[0, 2]]) > 500.0):
                truncated = True

        # 6. Récompense basée sur la différence de potentiel (Ng et al. reward shaping)
        current_potential = self._compute_potential()
        reward = current_potential - self.potential
        self.potential = current_potential
        
        self.prev_action = np.array(action, dtype=np.float32)

        if terminated:
            if self.current_episode_stage == 1:
                if success:
                    terminal_rwd = 500.0
                else:
                    if is_tilt_crash:
                        is_correcting_x = True
                        if self.theta_x > 0.05 and action[1] >= 0.0:
                            is_correcting_x = False
                        elif self.theta_x < -0.05 and action[1] <= 0.0:
                            is_correcting_x = False

                        is_correcting_z = True
                        if self.theta_z > 0.05 and action[2] <= 0.0:
                            is_correcting_z = False
                        elif self.theta_z < -0.05 and action[2] >= 0.0:
                            is_correcting_z = False

                        if not is_correcting_x or not is_correcting_z:
                            terminal_rwd = -1000.0
                        else:
                            terminal_rwd = -500.0
                    else:
                        terminal_rwd = -200.0
            else:
                if is_tilt_crash:
                    is_correcting_x = True
                    if self.theta_x > 0.05 and action[1] >= 0.0:
                        is_correcting_x = False
                    elif self.theta_x < -0.05 and action[1] <= 0.0:
                        is_correcting_x = False

                    is_correcting_z = True
                    if self.theta_z > 0.05 and action[2] <= 0.0:
                        is_correcting_z = False
                    elif self.theta_z < -0.05 and action[2] >= 0.0:
                        is_correcting_z = False

                    if not is_correcting_x or not is_correcting_z:
                        terminal_rwd = -1000.0
                    else:
                        terminal_rwd = -500.0
                elif is_flyaway:
                    terminal_rwd = -500.0
                else:
                    impact_vel_y = self.vel[1]
                    impact_vel_xz = np.linalg.norm(self.vel[[0, 2]])
                    dist_xz = np.sqrt(self.pos[0]**2 + self.pos[2]**2)
                    attitude_err = self.theta_x**2 + self.theta_y**2 + self.theta_z**2
                    
                    terminal_rwd = - 0.05 * dist_xz - 0.1 * (impact_vel_y**2) - 0.1 * (impact_vel_xz**2) - 10.0 * attitude_err
                    
                    if self.action_history:
                        terminal_rwd -= 5.0 * np.mean(np.var(self.action_history, axis=0))
                    
                    inside_pad = True if self.current_episode_stage == 2 else (np.abs(self.pos[0]) < 20.0 and np.abs(self.pos[2]) < 20.0)
                    landed_soft = (impact_vel_y >= -2.5 and impact_vel_xz <= 2.0)
                    landed_upright = (abs(self.theta_x) < 0.1 and abs(self.theta_z) < 0.1)
                    
                    if inside_pad and landed_soft and landed_upright:
                        terminal_rwd += 500.0
                        success = True
                    else:
                        terminal_rwd -= 200.0
                
            reward += terminal_rwd
            
        elif truncated:
            reward += -100.0
            
        return self._get_obs(), float(reward), terminated, truncated, {"success": success}
