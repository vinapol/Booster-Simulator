#pragma once
#include <vector>
#include <string>

class NeuralNetwork {
private:
    // Dimensions dynamiques : supporte PPO (64) et SAC (256)
    int h1_size = 0;
    int h2_size = 0;
    int input_size = 0;
    int output_size = 0;

    std::vector<std::vector<double>> w1;
    std::vector<double> b1;
    std::vector<std::vector<double>> w2;
    std::vector<double> b2;
    std::vector<std::vector<double>> w3;
    std::vector<double> b3;

    bool loaded = false;

    bool load_matrix(const std::string& path, std::vector<std::vector<double>>& mat, int rows, int cols);
    bool load_vector(const std::string& path, std::vector<double>& vec, int size);
    bool detect_dims(const std::string& path, int& rows, int& cols);

public:
    NeuralNetwork() = default;

    // Chargement automatique — détecte PPO (64) ou SAC (256)
    bool load_weights(const std::string& base_dir);
    bool isLoaded() const;

    // Propagation avant (forward pass)
    std::vector<double> forward(const std::vector<double>& input);
};

