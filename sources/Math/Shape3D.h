#ifndef SHAPE3D_H
#define SHAPE3D_H

#include "Point3D.h"

class Shape3D
{
public:
    Point3D center;
    Point3D *vertices = nullptr;
    int vertex_count = 0;
    int max_vertices = 0;

    Shape3D() : center{0.0f, 0.0f, 1.0f}, vertices(nullptr), vertex_count(0), max_vertices(0) {}
    Shape3D(int maxVerts) : center{0.0f, 0.0f, 1.0f}, vertex_count(0), max_vertices(maxVerts)
    {
        vertices = new Point3D[maxVerts];
    }

    ~Shape3D()
    {
        if (vertices)
        {
            delete[] vertices;
        }
    }

    Shape3D(const Shape3D &other);

    Shape3D &operator=(const Shape3D &other);

    void addVertex(const Point3D vertex);
    
    /** 
     * rotate the 3D shape around a given axis by angle in degrees
     */
    void rotate_degree(const Point3D &axis, float angleDegrees);
    /** 
     * rotate the 3D shape around a given axis by angle in radians
     * 
     */
    void rotate(const Point3D &axis, float angleRadians);

    void translate(const Point3D &translation);
    
    void scale(float scaleFactor);
};

#endif