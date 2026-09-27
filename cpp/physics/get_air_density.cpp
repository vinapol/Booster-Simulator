#include "get_air_density.hpp"
#include <cmath>

double get_air_density(double altitude) {
    const double rho0 = 1.225;
    const double H = 8500.0;
    if (altitude < 0) return rho0;
    return rho0 * std::exp(-altitude / H);
}
