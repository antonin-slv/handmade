#include "Point3D.h"

// redefine operators
Point3D Point3D::operator+(const Point3D &other) const
{
    Point3D result;
    result.x = this->x + other.x;
    result.y = this->y + other.y;
    result.z = this->z + other.z;
    return result;
};

Point3D Point3D::operator-(const Point3D &other) const
{
    Point3D result;
    result.x = this->x - other.x;
    result.y = this->y - other.y;
    result.z = this->z - other.z;
    return result;
};

Point3D Point3D::operator*(float scalar) const
{
    Point3D result;
    result.x = this->x * scalar;
    result.y = this->y * scalar;
    result.z = this->z * scalar;
    return result;
};

Point3D Point3D::operator/(float scalar) const
{
    Point3D result;
    result.x = this->x / scalar;
    result.y = this->y / scalar;
    result.z = this->z / scalar;
    return result;
};

Point3D &Point3D::operator+=(const Point3D &other)
{
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
    return *this;
};

Point3D &Point3D::operator-=(const Point3D &other)
{
    this->x -= other.x;
    this->y -= other.y;
    this->z -= other.z;
    return *this;
};

Point3D Point3D::normalized() const
{
    float length = sqrtf(x * x + y * y + z * z);
    if (length == 0.0f)
        return Point3D{0.0f, 0.0f, 0.0f};
    length = 1.0f / length;
    return Point3D{x * length, y * length, z * length};
};

float Point3D::length() const
{
    return sqrtf(x * x + y * y + z * z);
};

float Point3D::lengthSquared() const
{
    return x * x + y * y + z * z;
};

static float dotProduct(const Point3D &a, const Point3D &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
};

static Point3D crossProduct(const Point3D &a, const Point3D &b)
{
    Point3D result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}


Point3D Point3D::BasicProjection() const {
    float logic_z = (this->z <=  0.001f) ? 0.001f : this->z;
    return {this->x / logic_z, this->y / logic_z, 0};
}