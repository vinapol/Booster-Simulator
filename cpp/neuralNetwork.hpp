#ifndef NEURAL_NETWORK_HPP
#define NEURAL_NETWORK_HPP

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

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

    bool load_matrix(const std::string& path, std::vector<std::vector<double>>& mat, int rows, int cols) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        mat.assign(rows, std::vector<double>(cols, 0.0));
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                if (!(file >> mat[r][c])) return false;
        return true;
    }

    bool load_vector(const std::string& path, std::vector<double>& vec, int size) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        vec.assign(size, 0.0);
        for (int i = 0; i < size; ++i)
            if (!(file >> vec[i])) return false;
        return true;
    }

    // Détecte automatiquement les dimensions d'une matrice dans un fichier texte
    bool detect_dims(const std::string& path, int& rows, int& cols) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        rows = 0; cols = 0;
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

public:
    NeuralNetwork() {}

    // Chargement automatique — détecte PPO (64) ou SAC (256)
    bool load_weights(const std::string& base_dir) {
        // Détection automatique des dimensions de w1 (couche 1)
        // w1 est de forme [H1, input_size]
        std::ifstream f1(base_dir + "/w1.txt");
        if (!f1.is_open()) { std::cerr << "[NN] w1.txt introuvable dans " << base_dir << std::endl; return false; }

        h1_size = 0; input_size = 0;
        std::string line;
        while (std::getline(f1, line)) {
            if (line.empty()) continue;
            if (input_size == 0) {
                std::istringstream iss(line);
                double v; while (iss >> v) input_size++;
            }
            h1_size++;
        }
        f1.close();

        // Détection de h2_size via w2 (forme [H2, H1])
        std::ifstream f2(base_dir + "/w2.txt");
        if (!f2.is_open()) return false;
        h2_size = 0;
        while (std::getline(f2, line)) if (!line.empty()) h2_size++;
        f2.close();

        // Chargement
        if (!load_matrix(base_dir + "/w1.txt", w1, h1_size, input_size)) return false;
        if (!load_vector(base_dir + "/b1.txt", b1, h1_size)) return false;
        if (!load_matrix(base_dir + "/w2.txt", w2, h2_size, h1_size)) return false;
        if (!load_vector(base_dir + "/b2.txt", b2, h2_size)) return false;

        // w3 : [output_size, H2]
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

    bool isLoaded() const { return loaded; }

    // Propagation avant (forward pass) — tanh partout sauf sortie linéaire pour PPO, et tanh en sortie pour SAC
    std::vector<double> forward(const std::vector<double>& input) {
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
            
            // Stable-Baselines3 SAC utilise tanh sur l'acteur pour borner l'action à [-1, 1]
            // On le détecte automatiquement en fonction de la taille cachée (256 pour SAC, 64 pour PPO)
            if (h1_size == 256) {
                out[r] = std::tanh(val);
            } else {
                out[r] = val;
            }
        }
        return out;
    }
};

#endif
