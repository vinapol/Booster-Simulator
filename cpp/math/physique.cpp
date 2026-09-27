#include "physique.hpp"
#include <cmath>

Vector2D::Vector2D(double x, double y) : x(x), y(y) {}

Vector2D Vector2D::operator+(const Vector2D& other) const {
    return {x + other.x, y + other.y};
}

Vector2D Vector2D::operator-(const Vector2D& other) const {
    return {x - other.x, y - other.y};
}

Vector2D Vector2D::operator*(double scalar) const {
    return {x * scalar, y * scalar};
}

double Vector2D::length() const {
    return std::sqrt(x * x + y * y);
}
