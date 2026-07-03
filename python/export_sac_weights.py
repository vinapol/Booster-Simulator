import os
import sys
import numpy as np

try:
    from stable_baselines3 import SAC
except ImportError:
    print("stable-baselines3 non installé.")
    sys.exit(1)

def export():
    # Chercher le meilleur modèle SAC
    model_path = None
    for path in ["booster_sac_model_interrupted.zip", "booster_sac_stage_6_completed.zip", "booster_sac_model.zip", "booster_sac_stage_5_completed.zip", "./best_sac/best_model.zip"]:
        if os.path.exists(path):
            model_path = path
            break

    if model_path is None:
        print("[ERREUR] Aucun modèle SAC trouvé.")
        sys.exit(1)

    print(f"Chargement du modèle : {model_path}")
    model = SAC.load(model_path)
    
    target_dir = "../cpp/weights"
    os.makedirs(target_dir, exist_ok=True)

    def save_matrix(filename, tensor):
        arr = tensor.cpu().detach().numpy()
        np.savetxt(os.path.join(target_dir, filename), arr, fmt="%.10f")

    # Architecture SAC actor : MlpPolicy avec net_arch=[256, 256]
    # Les couches de l'acteur dans SB3 SAC sont dans actor.latent_pi
    sd = model.policy.state_dict()
    
    print("Clés disponibles dans state_dict :")
    for k in sd.keys():
        print(f"  {k}: {sd[k].shape}")

    # Extraire les poids de l'acteur (policy network)
    # Dans SAC SB3, l'acteur utilise : actor.latent_pi.0, actor.latent_pi.2 (réseau caché)
    # puis actor.mu (couche de sortie action mean)
    save_matrix("w1.txt", sd["actor.latent_pi.0.weight"])
    save_matrix("b1.txt", sd["actor.latent_pi.0.bias"])
    save_matrix("w2.txt", sd["actor.latent_pi.2.weight"])
    save_matrix("b2.txt", sd["actor.latent_pi.2.bias"])
    save_matrix("w3.txt", sd["actor.mu.weight"])
    save_matrix("b3.txt", sd["actor.mu.bias"])

    print(f"\nPoids SAC exportés avec succès dans : {target_dir}")
    print(f"Architecture : {sd['actor.latent_pi.0.weight'].shape[1]} → {sd['actor.latent_pi.0.weight'].shape[0]} → {sd['actor.latent_pi.2.weight'].shape[0]} → {sd['actor.mu.weight'].shape[0]}")

if __name__ == "__main__":
    export()
