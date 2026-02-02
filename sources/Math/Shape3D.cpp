#include "Shape3D.h"
#include "Point3D.cpp"

Shape3D::Shape3D(const Shape3D &other)
    : center(other.center), vertex_count(other.vertex_count), max_vertices(other.max_vertices)
{
    if (max_vertices > 0)
    {
        vertices = new Point3D[max_vertices];
        for (int i = 0; i < vertex_count; ++i)
        {
            vertices[i] = other.vertices[i];
        }
    }
    else
    {
        vertices = nullptr;
    }
}
Shape3D &Shape3D::operator=(const Shape3D &other)
{
    if (this != &other)
    {
        center = other.center;
        vertex_count = other.vertex_count;
        max_vertices = other.max_vertices;

        if (vertices)
        {
            delete[] vertices;
        }

        if (max_vertices > 0)
        {
            vertices = new Point3D[max_vertices];
            for (int i = 0; i < vertex_count; ++i)
            {
                vertices[i] = other.vertices[i];
            }
        }
        else
        {
            vertices = nullptr;
        }
    }
    return *this;
}
void Shape3D::addVertex(const Point3D vertex)
{
    if (vertex_count < max_vertices)
    {
        vertices[vertex_count++] = vertex;
    }
    else
    {
        // TODO: Handle the case where max_vertices is exceeded
    }
}

void Shape3D::rotate_degree(const Point3D &axis, float angleDegrees)
{
    float angleRadians = angleDegrees * (3.14159265f / 180.0f);
    rotate(axis, angleRadians);
}

void Shape3D::translate(const Point3D &translation)
{
    center = center + translation;
    for (int i = 0; i < vertex_count; ++i)
    {
        vertices[i] = vertices[i] + translation;
    }
}

void Shape3D::scale(float scaleFactor)
{
    for (int i = 0; i < vertex_count; ++i)
    {
        Point3D direction = vertices[i] - center;
        vertices[i] = center + direction * scaleFactor;
    }
}

void Shape3D::rotate(const Point3D &axis, float angleRadians)
{
    float cosTheta = cosf(angleRadians);
    float sinTheta = sinf(angleRadians);

    for (int i = 0; i < vertex_count; ++i)
    {
        Point3D &v = vertices[i];
        Point3D translated = v - center;

        Point3D rotated;
        rotated.x = (cosTheta + (1 - cosTheta) * axis.x * axis.x) * translated.x +
                    ((1 - cosTheta) * axis.x * axis.y - axis.z * sinTheta) * translated.y +
                    ((1 - cosTheta) * axis.x * axis.z + axis.y * sinTheta) * translated.z;

        rotated.y = ((1 - cosTheta) * axis.y * axis.x + axis.z * sinTheta) * translated.x +
                    (cosTheta + (1 - cosTheta) * axis.y * axis.y) * translated.y +
                    ((1 - cosTheta) * axis.y * axis.z - axis.x * sinTheta) * translated.z;

        rotated.z = ((1 - cosTheta) * axis.z * axis.x - axis.y * sinTheta) * translated.x +
                    ((1 - cosTheta) * axis.z * axis.y + axis.x * sinTheta) * translated.y +
                    (cosTheta + (1 - cosTheta) * axis.z * axis.z) * translated.z;

        vertices[i] = rotated + center;
    }
}