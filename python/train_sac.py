
import os
import sys

try:
    import torch
    import numpy as np
    from stable_baselines3 import SAC
    from stable_baselines3.common.callbacks import CheckpointCallback, EvalCallback, BaseCallback
    from stable_baselines3.common.monitor import Monitor
except ImportError:
    print("\n[ERREUR] Dépendances manquantes !")
    print("pip install gymnasium stable-baselines3 torch numpy\n")
    sys.exit(1)

from booster_env import Booster3DEnv

class StagedCurriculumCallback(BaseCallback):
    """
    Callback pour évaluer périodiquement le modèle sur le stage en cours et
    passer au stage suivant lorsque le taux de réussite atteint 90%.
    """
    def __init__(self, eval_env, eval_freq=15000, n_eval_episodes=100, verbose=1):
        super(StagedCurriculumCallback, self).__init__(verbose)
        self.eval_env = eval_env
        self.eval_freq = eval_freq
        self.n_eval_episodes = n_eval_episodes
        self.current_stage = 1

    def _on_step(self) -> bool:
        # Appliquer le stage courant à l'environnement d'entraînement et d'évaluation
        self.training_env.env_method("set_stage", self.current_stage)
        self.training_env.env_method("set_difficulty", 1.0)
        self.eval_env.unwrapped.set_stage(self.current_stage)
        self.eval_env.unwrapped.set_difficulty(1.0)

        if self.n_calls % self.eval_freq == 0:
            print(f"\n[EVALUATION STAGE {self.current_stage}] Début de l'évaluation ({self.num_timesteps:,} étapes)...")
            successes = 0
            
            for episode in range(self.n_eval_episodes):
                obs, _ = self.eval_env.reset()
                done = False
                while not done:
                    action, _ = self.model.predict(obs, deterministic=True)
                    obs, reward, terminated, truncated, info = self.eval_env.step(action)
                    done = terminated or truncated
                if info.get("success", False):
                    successes += 1
            
            success_rate = successes / self.n_eval_episodes
            print(f"[EVALUATION STAGE {self.current_stage}] Taux de réussite = {successes}/{self.n_eval_episodes} ({success_rate*100:.1f}%)")

            # Sauvegarder systématiquement le meilleur modèle actuel au cas où
            if successes > 0:
                self.model.save(f"best_sac/best_model")

            if success_rate >= 0.90:
                print(f"\n====================================================")
                print(f" STAGE {self.current_stage} ACQUIS ! Sauvegarde du modèle...")
                print(f"====================================================")
                self.model.save(f"booster_sac_stage_{self.current_stage}_completed")
                
                if self.current_stage < 6:
                    self.current_stage += 1
                    print(f">>> Passage au Stage {self.current_stage} !")
                    self.training_env.env_method("set_stage", self.current_stage)
                    self.eval_env.unwrapped.set_stage(self.current_stage)
                else:
                    print(">>> ENTRAÎNEMENT ENTIÈREMENT TERMINÉ ET MAÎTRISÉ ! 🏆")
        return True

def train():
    # Optimisation CPU : évite la contention de verrous sur PyTorch CPU
    torch.set_num_threads(1)

    print("====================================================")
    print("  ENTRAÎNEMENT IA — SAC (Staged Curriculum)         ")
    print("====================================================")

    env = Monitor(Booster3DEnv())
    env.unwrapped.curriculum_training = True
    eval_env = Monitor(Booster3DEnv())

    # Charger le meilleur modèle existant si disponible pour continuer l'entraînement
    resume_path = "./best_sac/best_model.zip"
    if os.path.exists(resume_path):
        print(f"\n[INFO] Reprise de l'apprentissage depuis le modèle existant : {resume_path}")
        model = SAC.load(resume_path, env=env)
    else:
        print("\n[INFO] Aucun modèle existant trouvé. Démarrage d'un nouvel entraînement SAC.")
        # SAC : algorithme off-policy, état de l'art pour le contrôle continu
        model = SAC(
            policy="MlpPolicy",
            env=env,
            learning_rate=3e-4,
            buffer_size=1_000_000,    # Replay buffer large
            learning_starts=10_000,   # Exploration pure avant d'apprendre
            batch_size=256,
            tau=0.005,                # Mise à jour douce du réseau cible
            gamma=0.999,              # Horizon long
            train_freq=4,
            gradient_steps=1,
            policy_kwargs=dict(
                net_arch=[256, 256],  # Réseau large
            ),
            verbose=1,
        )

    # Sauvegarde automatique toutes les 50 000 étapes
    checkpoint_cb = CheckpointCallback(
        save_freq=50_000,
        save_path="./checkpoints_sac/",
        name_prefix="booster_sac"
    )

    # Détecter le stage de démarrage
    start_stage = 1
    for s in range(1, 7):
        if os.path.exists(f"booster_sac_stage_{s}_completed.zip"):
            start_stage = s + 1
    start_stage = min(start_stage, 6)

    staged_curriculum_cb = StagedCurriculumCallback(eval_env=eval_env, eval_freq=15000, n_eval_episodes=100)
    staged_curriculum_cb.current_stage = start_stage
    print(f"[INFO] Démarrage de l'apprentissage au Stage {start_stage}")

    total_timesteps = 1_500_000
    print(f"Démarrage de l'apprentissage pour {total_timesteps:,} étapes...")

    try:
        model.learn(
            total_timesteps=total_timesteps,
            callback=[checkpoint_cb, staged_curriculum_cb],
            log_interval=10
        )
        print("\nEntraînement SAC terminé avec succès !")

        model_name = "booster_sac_model"
        model.save(model_name)
        print(f"Modèle final sauvegardé sous : {model_name}.zip")

    except KeyboardInterrupt:
        print("\nEntraînement interrompu. Sauvegarde...")
        model.save("booster_sac_model_interrupted")

if __name__ == "__main__":
    train()
