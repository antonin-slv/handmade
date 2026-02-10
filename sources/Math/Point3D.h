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
   inline Point3D operator+(const Point3D &other) const;

    inline Point3D operator-(const Point3D &other) const;

    inline Point3D operator*(float scalar) const;

    inline Point3D operator/(float scalar) const;

    inline Point3D &operator+=(const Point3D &other);

    inline Point3D &operator-=(const Point3D &other);

    inline Point3D normalized() const;

    inline float length() const;

    inline float lengthSquared() const;

    inline Point3D BasicProjection() const;
};

inline static float dotProduct(const Point3D &a, const Point3D &b) ;

inline static Point3D crossProduct(const Point3D &a, const Point3D &b);

#endif // POINT3D_H