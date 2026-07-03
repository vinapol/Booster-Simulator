import sys
import time
import numpy as np

try:
    from stable_baselines3 import SAC
except ImportError:
    print("stable-baselines3 non installé.")
    sys.exit(1)

from booster_env import Booster3DEnv

def run():
    # Chercher le meilleur modèle SAC ou le modèle standard
    model_path = None
    for path in ["booster_sac_model_interrupted.zip", "booster_sac_stage_6_completed.zip", "booster_sac_model.zip", "booster_sac_stage_5_completed.zip", "./best_sac/best_model.zip"]:
        import os
        if os.path.exists(path):
            model_path = path
            break

    if model_path is None:
        print("[ERREUR] Aucun modèle SAC trouvé. Lancez d'abord train_sac.py")
        sys.exit(1)

    print(f"Chargement du modèle : {model_path}")
    env = Booster3DEnv()
    env.set_difficulty(1.0) # Toujours évaluer en difficulté max (suprême)
    model = SAC.load(model_path, env=env)

    obs, _ = env.reset()
    done = False
    step_count = 0

    print(f"\n{'Temps':<10} | {'Alt (m)':<9} | {'Vy (m/s)':<11} | {'Pos X':<11} | {'Pos Z':<11} | {'Gaz %':<7} | {'Gimbal X/Z':<13} | Roll Cmd")
    print("-" * 105)

    while not done:
        action, _ = model.predict(obs, deterministic=True)
        obs, reward, terminated, truncated, info = env.step(action)
        done = terminated or truncated

        pos_x = env.pos[0]; pos_y = env.pos[1]; pos_z = env.pos[2]
        vel_y = env.vel[1]; vel_x = env.vel[0]; vel_z = env.vel[2]
        t = step_count * env.dt

        throttle_pct = ((action[0] + 1.0) / 2.0) * 100.0
        gim_x_deg = action[1] * 15.0
        gim_z_deg = action[2] * 15.0
        roll_cmd = action[3]

        if step_count % 15 == 0 or done:
            print(f"{t:<10.2f} | {pos_y:<9.1f} | {vel_y:<11.1f} | {pos_x:<11.1f} | {pos_z:<11.1f} | {throttle_pct:<7.1f} | {gim_x_deg:+.1f}° / {gim_z_deg:+.1f}° | {roll_cmd:+.2f}")

        step_count += 1
        time.sleep(0.01)

    print("-" * 90)
    dist_center = np.sqrt(env.pos[0]**2 + env.pos[2]**2)
    impact_vy = env.vel[1]
    impact_vxz = np.sqrt(env.vel[0]**2 + env.vel[2]**2)
    inside_pad = (abs(env.pos[0]) < 20.0 and abs(env.pos[2]) < 20.0)
    landed_soft = (impact_vy >= -2.5 and impact_vxz <= 2.0)

    print("\n================ RAPPORT FINAL ================")
    print(f"Altitude finale       : {env.pos[1]:.2f} m")
    print(f"Vitesse d'impact Vert : {impact_vy:.2f} m/s")
    print(f"Vitesse d'impact Hrz  : {impact_vxz:.2f} m/s")
    print(f"Distance au centre    : {dist_center:.2f} m")
    print(f"Propergol restant     : {env.fuel_mass:.1f} kg")
    print("-----------------------------------------------")
    if inside_pad and landed_soft:
        print(" RÉSULTAT : ATTERRISSAGE RÉUSSI ! ✅")
    else:
        print(" RÉSULTAT : CRASH DU BOOSTER (ÉCHEC)")
        if not inside_pad: print(f" -> Motif : Le booster a manqué la cible du pad.")
        if not landed_soft: print(f" -> Motif : La vitesse de contact au sol était trop élevée.")
    print("===============================================\n")

if __name__ == "__main__":
    run()
