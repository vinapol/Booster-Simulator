#pragma once

struct Vector3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vector3D() = default;
    Vector3D(double x, double y, double z);

    Vector3D operator+(const Vector3D& other) const;
    Vector3D operator-(const Vector3D& other) const;
    Vector3D operator*(double scalar) const;
    Vector3D operator/(double scalar) const;
    Vector3D& operator+=(const Vector3D& other);

    double dot(const Vector3D& other) const;
    Vector3D cross(const Vector3D& other) const;
    double length() const;
    Vector3D normalized() const;
};

Vector3D operator*(double scalar, const Vector3D& vec);
