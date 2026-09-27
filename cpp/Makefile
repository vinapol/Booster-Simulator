# =============================================================================
# Makefile — chaque build efface builds/ puis recompile tout
# =============================================================================

BUILD_DIR = builds
TARGET    = $(BUILD_DIR)/booster_sim

CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LIBS     = -lSDL2 -lGL

INCLUDES = \
	-Imath \
	-Iphysics \
	-Icontrol \
	-Iinput \
	-Iaudio \
	-Igraphics \
	-Ibooster_visual \
	-Iui \
	-Isimulation

CXXFLAGS += $(INCLUDES)

SRCS = \
	main.cpp \
	math/vector3d.cpp \
	math/physique.cpp \
	physics/get_air_density.cpp \
	physics/get_drag_force.cpp \
	physics/booster.cpp \
	control/autopilot.cpp \
	control/neuralNetwork.cpp \
	input/keyboardInput.cpp \
	input/joystickInput.cpp \
	input/inputManager.cpp \
	graphics/setup_projection.cpp \
	graphics/setup_camera.cpp \
	graphics/draw_sky.cpp \
	graphics/draw_stars.cpp \
	graphics/draw_wind_indicator_3d.cpp \
	graphics/draw_ground_and_pad_3d.cpp \
	booster_visual/draw_booster_and_flame_3d.cpp \
	booster_visual/draw_forces_vectors_3d.cpp \
	ui/font8x8.cpp \
	ui/draw_char_opengl.cpp \
	ui/draw_string.cpp \
	ui/draw_telemetry.cpp \
	audio/audio_callback.cpp \
	audio/audio_init.cpp \
	audio/audio_shutdown.cpp \
	simulation/simState.cpp \
	simulation/generate_stars.cpp \
	simulation/reset_common.cpp \
	simulation/reset_hard.cpp \
	simulation/reset_easy.cpp \
	simulation/reset_very_easy.cpp \
	simulation/cycle_control_mode.cpp \
	simulation/update_wind.cpp \
	simulation/update_controls.cpp \
	simulation/step_physics.cpp

OBJS = $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

.PHONY: all clean

# Chaque make : supprimer l'ancien builds/, puis recompiler les nouveaux fichiers
all:
	$(MAKE) clean
	$(MAKE) $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "→ Exécutable : $(TARGET)"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	find . -name '*.o' -delete 2>/dev/null || true
	rm -f booster_sim
