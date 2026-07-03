# SpaceX Falcon 3D Booster Simulation & Deep RL Autopilot

A 3D 6-DOF (Degrees of Freedom) simulation and Deep Reinforcement Learning (DRL) control system for the vertical landing of a SpaceX Falcon-like booster. The project uses a hybrid control architecture combining a classical cascade PID autopilot with a **Soft Actor-Critic (SAC)** neural network to achieve precise landings under extreme conditions, such as 60 m/s (216 km/h) hurricane-force winds.

The agent is trained in Python using **PyTorch** and **Stable-Baselines3**, and the trained actor's weights are exported to a lightweight, custom C++ inference engine. The C++ simulator runs a Runge-Kutta 4 (RK4) physics solver and renders the flight in real-time at 60 FPS using **OpenGL** and **SDL2**.

---

## 🚀 Key Features

* **6-DOF Physical Model**: Realistic flight dynamics including variable mass (fuel consumption), variable gravity, atmospheric drag (axial and lateral), active grid fin lift/damping, and actuator lag (gimbal speed limit of 30°/s).
* **Hybrid Control (Residual RL)**: 
  * The classical **PID controller** provides basic attitude stabilization (Pitch/Yaw) and vertical speed profiles.
  * The **SAC Neural Network** acts as a residual controller, correcting the PID throttle ($\pm 20\%$) and gimbal angles ($\pm 5^\circ$) while maintaining full, direct control over the **Roll** axis to handle wind torque.
* **Staged Curriculum Learning**: A 6-stage training curriculum that progressively introduces challenges (hovering, free fall, targeting the landing pad, wind, mass variations, and extreme wind conditions up to 35 m/s).
* **High-Performance C++ Simulation**: Python is used for training, but the real-time simulation is written in C++ for maximum performance, using SDL2 for window/input management, OpenGL for 3D rendering, and SDL Audio for procedural engine sound synthesis.

---

## 📁 Repository Structure

```
├── cpp/
│   ├── weights/            # Extracted weights from the trained PyTorch actor (.txt files)
│   ├── main.cpp            # Main entry point, SDL2 loop, and rendering pipeline
│   ├── booster.hpp         # Rocket state and RK4 physics solver
│   ├── autopilot.hpp       # Cascade PID controller implementation
│   ├── neuralNetwork.hpp   # Lightweight C++ feedforward neural network (no external dependencies)
│   ├── aerodynamique.hpp   # Atmospheric density and drag force models
│   ├── graphics.cpp/hpp    # OpenGL drawing routines for the booster, flames, and vectors
│   ├── gui.cpp/hpp         # Telemetry overlay rendering
│   └── Makefile            # C++ build configuration
│
└── python/
    ├── booster_env.py      # Gymnasium environment wrapping the 6-DOF physics equations
    ├── train_sac.py        # Python curriculum training script using Stable-Baselines3
    ├── check_stages.py     # Evaluation script to test success rates across all 6 stages
    ├── export_sac_weights.py # Weights exporter script (PyTorch StateDict -> C++ text files)
    ├── run_sac.py          # Command-line Python simulation runner
    └── venv/               # Local virtual environment
```

---

## 🎓 Curriculum Learning Stages

The environment uses a custom curriculum callback. When the agent reaches a $\ge 90\%$ success rate on a stage during periodic evaluation, it automatically unlocks the next stage:

1. **Stage 1 (Hovering)**: Hover at 200m altitude with no wind and zero initial offset.
2. **Stage 2 (Free Fall)**: Drop from 300m and land safely anywhere on the ground (no target pad constraint).
3. **Stage 3 (Pad Landing)**: Drop from 300m with a lateral offset and land precisely on the 40m wide pad.
4. **Stage 4 (High Drop)**: Drop from 1000m with initial attitude tilts and high descent speeds.
5. **Stage 5 (Wind & Variations)**: Drop from 1000m with lateral wind (up to 20 m/s) and randomized dry mass/thrust factors.
6. **Stage 6 (Full Mission)**: Drop from 3000m with severe wind gusts (up to 35 m/s) and heavy domain randomization.

---

## 🛠️ Installation & Setup

### C++ Prerequisites (Linux)
You need a C++17 compiler, SDL2 (with audio support), and OpenGL libraries.
On Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev libgl1-mesa-dev
```

### Python Prerequisites
Create a virtual environment and install the required RL dependencies:
```bash
cd python
python3 -m venv venv
source venv/bin/activate
pip install gymnasium stable-baselines3 torch numpy
```

---

## 💻 Usage

### 1. Training the AI
Run the reinforcement learning training script. It will automatically save checkpoints and stage-completion models:
```bash
cd python
source venv/bin/activate
python train_sac.py
```

### 2. Evaluating the Model
To check the success rate of the trained model across all 6 stages:
```bash
python check_stages.py
```

### 3. Exporting Weights to C++
Extract the actor network weights into text files format for the C++ engine:
```bash
python export_sac_weights.py
```
This updates the weights in the `cpp/weights/` directory.

### 4. Compiling and Running the C++ 3D Simulator
Compile the C++ source code and run the simulation:
```bash
cd ../cpp
make clean && make
./booster_sim
```

**Controls inside the Simulator:**
* **R**: Reset the simulation at **Stage 6** (Extreme Difficulty, 3000m drop).
* **T**: Reset the simulation at **Stage 4** (Medium Difficulty, 1000m drop).
* **Y**: Reset the simulation at **Stage 2** (Easy Difficulty, 500m drop).
* **Space/Keyboard**: Cycles control modes (Manual $\leftrightarrow$ PID Autopilot $\leftrightarrow$ Neural AI). The AI mode activates by default upon resetting.
