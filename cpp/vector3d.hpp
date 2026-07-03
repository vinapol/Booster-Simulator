#ifndef VECTOR3D_HPP
#define VECTOR3D_HPP

#include <cmath>

struct Vector3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vector3D() = default;
    Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

    Vector3D operator+(const Vector3D& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vector3D operator-(const Vector3D& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vector3D operator*(double scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    Vector3D operator/(double scalar) const {
        return {x / scalar, y / scalar, z / scalar};
    }

    Vector3D& operator+=(const Vector3D& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    double dot(const Vector3D& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3D cross(const Vector3D& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3D normalized() const {
        double len = length();
        if (len > 1e-9) {
            return *this / len;
        }
        return {0.0, 0.0, 0.0};
    }
};

inline Vector3D operator*(double scalar, const Vector3D& vec) {
    return vec * scalar;
}

#endif
