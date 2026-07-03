import sys
import numpy as np
from stable_baselines3 import SAC
sys.path.append("/home/vinapol/Documents/Simulation booster/python")
from booster_env import Booster3DEnv

def check_stages():
    env = Booster3DEnv()
    import glob
    import os
    
    # Chercher le modèle complété le plus récent ou les checkpoints
    model_path = None
    for path in [
        "/home/vinapol/Documents/Simulation booster/python/booster_sac_model_interrupted.zip",
        "/home/vinapol/Documents/Simulation booster/python/booster_sac_stage_6_completed.zip",
        "/home/vinapol/Documents/Simulation booster/python/booster_sac_stage_5_completed.zip",
        "/home/vinapol/Documents/Simulation booster/python/booster_sac_model.zip"
    ]:
        if os.path.exists(path):
            model_path = path
            break

    if model_path is None:
        checkpoints = glob.glob("/home/vinapol/Documents/Simulation booster/python/checkpoints_sac/booster_sac_*_steps.zip")
        if checkpoints:
            checkpoints.sort(key=os.path.getmtime)
            model_path = checkpoints[-1]
        else:
            model_path = "/home/vinapol/Documents/Simulation booster/python/best_sac/best_model.zip"
        
    # S'assurer d'importer time
    import time
    
    print(f"Chargement du modèle de checkpoint : {model_path} (mis à jour le {time.strftime('%H:%M:%S', time.localtime(os.path.getmtime(model_path))) if os.path.exists(model_path) else 'N/A'})")
    
    model = SAC.load(model_path, env=env)
    
    for stage in [1, 2, 3, 4, 5, 6]:
        env.set_stage(stage)
        env.set_difficulty(1.0)
        
        successes = 0
        n_episodes = 100
        rewards = []
        
        for ep in range(n_episodes):
            obs, _ = env.reset()
            done = False
            ep_reward = 0
            while not done:
                action, _ = model.predict(obs, deterministic=True)
                obs, reward, terminated, truncated, info = env.step(action)
                done = terminated or truncated
                ep_reward += reward
            rewards.append(ep_reward)
            if info.get("success", False):
                successes += 1
                
        success_rate = successes / n_episodes
        mean_reward = np.mean(rewards)
        print(f"Stage {stage} : Taux de réussite = {successes}/{n_episodes} ({success_rate*100:.1f}%), Récompense moyenne = {mean_reward:.2f}")

if __name__ == "__main__":
    check_stages()

