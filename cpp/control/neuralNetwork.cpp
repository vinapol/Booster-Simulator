#include "neuralNetwork.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

bool NeuralNetwork::load_matrix(const std::string& path, std::vector<std::vector<double>>& mat, int rows, int cols) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    mat.assign(rows, std::vector<double>(cols, 0.0));
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (!(file >> mat[r][c])) return false;
    return true;
}

bool NeuralNetwork::load_vector(const std::string& path, std::vector<double>& vec, int size) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    vec.assign(size, 0.0);
    for (int i = 0; i < size; ++i)
        if (!(file >> vec[i])) return false;
    return true;
}

bool NeuralNetwork::detect_dims(const std::string& path, int& rows, int& cols) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    rows = 0;
    cols = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (rows == 0) {
            std::istringstream iss(line);
            double val;
            while (iss >> val) cols++;
        }
        rows++;
    }
    return rows > 0 && cols > 0;
}

bool NeuralNetwork::load_weights(const std::string& base_dir) {
    std::ifstream f1(base_dir + "/w1.txt");
    if (!f1.is_open()) {
        std::cerr << "[NN] w1.txt introuvable dans " << base_dir << std::endl;
        return false;
    }

    h1_size = 0;
    input_size = 0;
    std::string line;
    while (std::getline(f1, line)) {
        if (line.empty()) continue;
        if (input_size == 0) {
            std::istringstream iss(line);
            double v;
            while (iss >> v) input_size++;
        }
        h1_size++;
    }
    f1.close();

    std::ifstream f2(base_dir + "/w2.txt");
    if (!f2.is_open()) return false;
    h2_size = 0;
    while (std::getline(f2, line)) if (!line.empty()) h2_size++;
    f2.close();

    if (!load_matrix(base_dir + "/w1.txt", w1, h1_size, input_size)) return false;
    if (!load_vector(base_dir + "/b1.txt", b1, h1_size)) return false;
    if (!load_matrix(base_dir + "/w2.txt", w2, h2_size, h1_size)) return false;
    if (!load_vector(base_dir + "/b2.txt", b2, h2_size)) return false;

    std::ifstream f3(base_dir + "/w3.txt");
    if (!f3.is_open()) return false;
    output_size = 0;
    while (std::getline(f3, line)) if (!line.empty()) output_size++;
    f3.close();

    if (!load_matrix(base_dir + "/w3.txt", w3, output_size, h2_size)) return false;
    if (!load_vector(base_dir + "/b3.txt", b3, output_size)) return false;

    loaded = true;
    std::cout << "[NN] Réseau chargé : " << input_size << " -> " << h1_size
              << " -> " << h2_size << " -> " << output_size << std::endl;
    return true;
}

bool NeuralNetwork::isLoaded() const {
    return loaded;
}

std::vector<double> NeuralNetwork::forward(const std::vector<double>& input) {
    if (!loaded) return std::vector<double>(output_size > 0 ? output_size : 3, 0.0);

    std::vector<double> h1(h1_size, 0.0);
    for (int r = 0; r < h1_size; ++r) {
        double val = b1[r];
        for (int c = 0; c < input_size; ++c) val += w1[r][c] * input[c];
        h1[r] = val > 0.0 ? val : 0.0;
    }

    std::vector<double> h2(h2_size, 0.0);
    for (int r = 0; r < h2_size; ++r) {
        double val = b2[r];
        for (int c = 0; c < h1_size; ++c) val += w2[r][c] * h1[c];
        h2[r] = val > 0.0 ? val : 0.0;
    }

    std::vector<double> out(output_size, 0.0);
    for (int r = 0; r < output_size; ++r) {
        double val = b3[r];
        for (int c = 0; c < h2_size; ++c) val += w3[r][c] * h2[c];

        // SAC (h1=256) : tanh en sortie ; PPO : linéaire
        if (h1_size == 256) {
            out[r] = std::tanh(val);
        } else {
            out[r] = val;
        }
    }
    return out;
}
