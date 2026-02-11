#ifndef QUATERNIONS_H
#define QUATERNIONS_H


#include <cmath>
#include <immintrin.h> // Required for SSE
#include "Point3D.h"
#include "Matrix.cpp"

struct Quaternion
{

    float w, x, y, z; // we don't need a double
    //  w partie réelle, x,y,z partie imaginaire
    //  avec : x*x = -1 ; y*y = -1 ; z*z = -1 ;
    //  et xy = z ; yz = x ; zx = y ; yx = -z ; zy = -x ; xz = -y

    Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {} // Identity quaternion

    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}


    Quaternion static fromAxisAngle(const Point3D &axis, float angleRadians)
    {
        float halfAngle = angleRadians * 0.5f;
        float sinHalfAngle = sinf(halfAngle);
        return {
            cosf(halfAngle),
            axis.x * sinHalfAngle,
            axis.y * sinHalfAngle,
            axis.z * sinHalfAngle};
    }

    Quaternion operator*(const Quaternion &q) const;
    // il y a une traduction implicite de Point3D en Quaternion, avec w = 0
    Quaternion operator*(const Point3D &v) const;
    Quaternion operator+(const Quaternion &q) const;

    Quaternion operator-(const Quaternion &q) const;

    Quaternion operator*(float scalar) const;

    Quaternion conjugate() const;

    Quaternion normalized() const;

    float norm() const;

    float normSquared() const;

    Quaternion inverse() const;

    Point3D rotate(const Point3D &v) const;

    Quaternion power(float exponent) const;

    /**
     * Rotates an array of Point3D by this quaternion.
     * result memory must be allocated and have the same size as points.
     */
    void rotateArrayOf3DPoint(Point3DCloud *points, Point3DCloud *result);
};

Quaternion Pow(const Quaternion &q, float exponent);

#endif // QUATERNIONS_H