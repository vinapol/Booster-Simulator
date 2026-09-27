#include "simState.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

static std::string resolve_weights_dir(const char* argv0) {
    std::vector<fs::path> candidates = {
        "weights",
        "../weights",
    };

    if (argv0 && argv0[0] != '\0') {
        std::error_code ec;
        fs::path exe = fs::absolute(fs::path(argv0), ec);
        if (!ec) {
            // ./builds/booster_sim → ../weights (= cpp/weights)
            candidates.push_back(exe.parent_path() / "weights");
            candidates.push_back(exe.parent_path().parent_path() / "weights");
        }
    }

    for (const auto& dir : candidates) {
        if (fs::exists(dir / "w1.txt")) {
            return dir.lexically_normal().string();
        }
    }

    std::cerr << "[NN] Aucun dossier weights trouvé (cwd ou relatif à l'exécutable)." << std::endl;
    return "weights";
}

SimState::SimState(const char* argv0)
    : rocket(10000.0, 4000.0, 3000.0, 100.0, {50.0, 3000.0, 0.0}, {0.0, -150.0, 0.0})
{
    const std::string weights_dir = resolve_weights_dir(argv0);
    ai_loaded = ai_net.load_weights(weights_dir);
}
