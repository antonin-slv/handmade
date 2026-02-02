#ifndef POINT3D_H
#define POINT3D_H
#include "math.h"

class Point3D
{
public:
    float x;
    float y;
    float z;
    // redefine operators
    Point3D operator+(const Point3D &other) const;

    Point3D operator-(const Point3D &other) const;

    Point3D operator*(float scalar) const;

    Point3D operator/(float scalar) const;

    Point3D &operator+=(const Point3D &other);

    Point3D &operator-=(const Point3D &other);

    Point3D normalized() const;

    float length() const;

    float lengthSquared() const;

    Point3D BasicProjection() const;
};

static float dotProduct(const Point3D &a, const Point3D &b);

static Point3D crossProduct(const Point3D &a, const Point3D &b);

#endif // POINT3D_H