#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "vector3d.hpp"
#include <cmath>

struct Vector2D {
    double x, y;

    Vector2D operator+(const Vector2D& other) const { return {x + other.x, y + other.y}; }
    Vector2D operator-(const Vector2D& other) const { return {x - other.x, y - other.y}; }
    Vector2D operator*(double scalar) const { return {x * scalar, y * scalar}; }
    
    double length() const { return std::sqrt(x * x + y * y); }
};

#endif