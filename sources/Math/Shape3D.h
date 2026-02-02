#ifndef SHAPE3D_H
#define SHAPE3D_H

#include "Point3D.h"
#include <tuple>
#include <Utility>

//no constructor no destructor so that everything is controllable
struct PointCloud
{
    Point3D center; // rotation center
    Point3D *vertices = nullptr; //coord of the vertices (local space)
    int vertex_count = 0;
    int max_vertices = 0;

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

struct WireFrame3D : PointCloud
{
    std::pair<uint16_t, uint16_t> * edges = nullptr; // pairs of vertex indices defining edges
    int edge_count = 0;
    int max_edges = 0;
        void addEdge(uint16_t vertexIndex1, uint16_t vertexIndex2)
    {
        if (edge_count < max_edges)
        {
            edges[edge_count++] = std::make_pair(vertexIndex1, vertexIndex2);
        }
        else
        {//TODO
        }
    }
};

#endif