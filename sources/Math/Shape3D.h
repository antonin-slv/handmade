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

/// !!! TRIANGLE ONLY
struct Face {

    uint16_t v[4];//we can store the normal in the 4th index if needed, anyway the alignment will be the same
    uint32_t color;
};

struct Mesh3D : PointCloud {
    Face* faces = nullptr;
    int face_count = 0;
    int max_faces = 0;

    void addFace(uint16_t v1, uint16_t v2, uint16_t v3, uint32_t color) {
        if (face_count < max_faces) {
            faces[face_count++] = Face{{v1, v2, v3, 0}, color};
        } else {
            //TODO
        }
    }
};

/**
 * Shpère de base :
 * - centre en center
 * - rayon radius
 * 
 * - 2 vertex de référence (pour "avant" et "haut")
 * - TODO : méthode propre pour stocker le maillage sphérique (possible de n'utiliser que les 2 vecteurs susmentionnés ?)
 */
struct Sphere : PointCloud
{
    float radius = 1.0f;

};

#endif