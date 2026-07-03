# Nom de l'exécutable
TARGET = booster_sim

# Compilateur
CXX = g++

# Flags de compilation
CXXFLAGS = -Wall -std=c++17
LIBS = -lSDL2 -lGL

# Règle de compilation
all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp graphics.cpp gui.cpp $(LIBS)

# Règle pour nettoyer
clean:
	rm -f $(TARGET)