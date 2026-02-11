#include "Quaternions.h"

Quaternion Quaternion::operator*(const Quaternion &q) const
{
    return {
        w * q.w - x * q.x - y * q.y - z * q.z,
        w * q.x + x * q.w + y * q.z - z * q.y,
        w * q.y - x * q.z + y * q.w + z * q.x,
        w * q.z + x * q.y - y * q.x + z * q.w};
}
// il y a une traduction implicite de Point3D en Quaternion, avec w = 0
Quaternion Quaternion::operator*(const Point3D &v) const
{
    return {
        -x * v.x - y * v.y - z * v.z,
        w * v.x + y * v.z - z * v.y,
        w * v.y - x * v.z + z * v.x,
        w * v.z + x * v.y - y * v.x};
}

Quaternion Quaternion::operator+(const Quaternion &q) const
{
    return {w + q.w, x + q.x, y + q.y, z + q.z};
}

Quaternion Quaternion::operator-(const Quaternion &q) const
{
    return {w - q.w, x - q.x, y - q.y, z - q.z};
}

Quaternion Quaternion::operator*(float scalar) const
{
    return {w * scalar, x * scalar, y * scalar, z * scalar};
}

Quaternion Quaternion::conjugate() const
{
    return {w, -x, -y, -z};
}

Quaternion Quaternion::normalized() const
{
    float n = norm();
    if (n == 0.0f)
        return {1.0f, 0.0f, 0.0f, 0.0f}; // return identity quaternion
    float invN = 1.0f / n;
    return {w * invN, x * invN, y * invN, z * invN};
}

float Quaternion::norm() const
{
    return sqrtf(w * w + x * x + y * y + z * z);
}

float Quaternion::normSquared() const
{
    return w * w + x * x + y * y + z * z;
}

Quaternion Quaternion::inverse() const
{
    float n2 = normSquared();
    if (n2 == 0.0f)
        return {1.0f, 0.0f, 0.0f, 0.0f}; // return identity quaternion
    float invN2 = 1.0f / n2;
    return conjugate() * invN2;
}

Point3D Quaternion::rotate(const Point3D &v) const
{
    Quaternion result = ((*this) * v) * this->inverse(); // parenthèses car Point3D * Quaternion non défini ()
    return {result.x, result.y, result.z};
}

Quaternion Quaternion::power(float exponent) const
{
    float theta = std::acosf(w / norm());
    if (std::abs(theta) < 0.0001f)
    {
        return {1.0f, 0.0f, 0.0f, 0.0f};
    }
    float newTheta = theta * exponent;
    float sinTheta = sinf(theta);

    float coeff = sinf(newTheta) / sinTheta;
    return {
        cosf(newTheta),
        x * coeff,
        y * coeff,
        z * coeff};
}

/**
 * Rotates an array of Point3D by this quaternion.
 * result memory must be allocated and have the same size as points.
 */
void Quaternion::rotateArrayOf3DPoint(Point3DCloud *points, Point3DCloud *result)
{
    // Formula: v' = v + 2 * cross(q_vec, cross(q_vec, v) + q_w * v)
    // This is structurally more efficient than q * v * q^-1
    // We use the normalized quaternion as the rotation operator
    Quaternion q = this->normalized();

    int i = 0;

    // SIMD Loop (processes 4 points at a time)
    __m128 qw = _mm_set1_ps(q.w);
    __m128 qx = _mm_set1_ps(q.x);
    __m128 qy = _mm_set1_ps(q.y);
    __m128 qz = _mm_set1_ps(q.z);
    __m128 two = _mm_set1_ps(2.0f);

    for (; i <= points->vertex_count - 4; i += 4)
    {
        // Load 4 points into SoA
        // We us set_ps to load variables in reverse order (3, 2, 1, 0)
        int chunkIndex = i / 4;

        __m128 vx = _mm_load_ps(points->chunks[chunkIndex].x);
        __m128 vy = _mm_load_ps(points->chunks[chunkIndex].y);
        __m128 vz = _mm_load_ps(points->chunks[chunkIndex].z);

        // t = 2 * cross(q_vec, v)
        // cross_x = qy * vz - qz * vy
        __m128 tx = _mm_mul_ps(two, _mm_sub_ps(_mm_mul_ps(qy, vz), _mm_mul_ps(qz, vy)));
        // cross_y = qz * vx - qx * vz
        __m128 ty = _mm_mul_ps(two, _mm_sub_ps(_mm_mul_ps(qz, vx), _mm_mul_ps(qx, vz)));
        // cross_z = qx * vy - qy * vx
        __m128 tz = _mm_mul_ps(two, _mm_sub_ps(_mm_mul_ps(qx, vy), _mm_mul_ps(qy, vx)));

        // v' = v + q_w * t + cross(q_vec, t)
        // term2 = q_w * t
        __m128 t2x = _mm_mul_ps(qw, tx);
        __m128 t2y = _mm_mul_ps(qw, ty);
        __m128 t2z = _mm_mul_ps(qw, tz);

        // term3 = cross(q_vec, t)
        __m128 t3x = _mm_sub_ps(_mm_mul_ps(qy, tz), _mm_mul_ps(qz, ty));
        __m128 t3y = _mm_sub_ps(_mm_mul_ps(qz, tx), _mm_mul_ps(qx, tz));
        __m128 t3z = _mm_sub_ps(_mm_mul_ps(qx, ty), _mm_mul_ps(qy, tx));

        // result = v + term2 + term3
        vx = _mm_add_ps(vx, _mm_add_ps(t2x, t3x));
        vy = _mm_add_ps(vy, _mm_add_ps(t2y, t3y));
        vz = _mm_add_ps(vz, _mm_add_ps(t2z, t3z));

        // Store results
        _mm_store_ps(result->chunks[chunkIndex].x, vx);
        _mm_store_ps(result->chunks[chunkIndex].y, vy);
        _mm_store_ps(result->chunks[chunkIndex].z, vz);
    }

    // Tail Loop (Scalar)
    for (; i < points->vertex_count; ++i)
    {
        

        int chunkIndex = i / 4;
        int laneIndex = i % 4;
        Point3D v;
        v.x = points->chunks[chunkIndex].x[laneIndex];
        v.y = points->chunks[chunkIndex].y[laneIndex];
        v.z = points->chunks[chunkIndex].z[laneIndex];

        // t = 2 * cross(q_vec, v)
        float tx = 2.0f * (q.y * v.z - q.z * v.y);
        float ty = 2.0f * (q.z * v.x - q.x * v.z);
        float tz = 2.0f * (q.x * v.y - q.y * v.x);

        // v' = v + q.w * t + cross(q_vec, t)
        result->chunks[chunkIndex].x[laneIndex] = v.x + q.w * tx + (q.y * tz - q.z * ty);
        result->chunks[chunkIndex].y[laneIndex] = v.y + q.w * ty + (q.z * tx - q.x * tz);
        result->chunks[chunkIndex].z[laneIndex] = v.z + q.w * tz + (q.x * ty - q.y * tx);
    }
}

Quaternion Pow(const Quaternion &q, float exponent)
{
    return q.power(exponent);
}
