#include "vector3d.hpp"
#include <cmath>

Vector3D::Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

Vector3D Vector3D::operator+(const Vector3D& other) const {
    return {x + other.x, y + other.y, z + other.z};
}

Vector3D Vector3D::operator-(const Vector3D& other) const {
    return {x - other.x, y - other.y, z - other.z};
}

Vector3D Vector3D::operator*(double scalar) const {
    return {x * scalar, y * scalar, z * scalar};
}

Vector3D Vector3D::operator/(double scalar) const {
    return {x / scalar, y / scalar, z / scalar};
}

Vector3D& Vector3D::operator+=(const Vector3D& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

double Vector3D::dot(const Vector3D& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vector3D Vector3D::cross(const Vector3D& other) const {
    return {
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    };
}

double Vector3D::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3D Vector3D::normalized() const {
    double len = length();
    if (len > 1e-9) {
        return *this / len;
    }
    return {0.0, 0.0, 0.0};
}

Vector3D operator*(double scalar, const Vector3D& vec) {
    return vec * scalar;
}
