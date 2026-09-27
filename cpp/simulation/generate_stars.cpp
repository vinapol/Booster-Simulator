#include "generate_stars.hpp"
#include <cstdlib>

std::vector<Star> generate_stars(int count) {
    std::vector<Star> stars;
    stars.reserve(count);
    for (int i = 0; i < count; ++i) {
        stars.push_back({
            (double)(rand() % 2000 - 1000),
            (double)(rand() % 8000 + 100),
            (double)(rand() % 2000 - 1000),
            rand() % 155 + 100
        });
    }
    return stars;
}
