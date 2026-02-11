#ifndef POINT3D_H
#define POINT3D_H

#include <math.h>
#include "../debug_api.h"

struct Point3D
{

    float x, y, z;
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

inline static float dotProduct(const Point3D &a, const Point3D &b);

inline static Point3D crossProduct(const Point3D &a, const Point3D &b);

struct alignas(16) Point3DChunk
{
    float x[4];
    float y[4];
    float z[4];
};

struct Point3DCloud
{
    Point3DChunk* chunks;
    int vertex_count;
    int chunk_count;
    int max_vertices;
    
    Point3D get(int index) const {
        int chunkIndex = index / 4;
        int laneIndex = index % 4;
        return {
            chunks[chunkIndex].x[laneIndex],
            chunks[chunkIndex].y[laneIndex],
            chunks[chunkIndex].z[laneIndex]
        };
    }
    
    void set(int index, const Point3D& point) {
        int chunkIndex = index / 4;
        int laneIndex = index % 4;
        chunks[chunkIndex].x[laneIndex] = point.x;
        chunks[chunkIndex].y[laneIndex] = point.y;
        chunks[chunkIndex].z[laneIndex] = point.z;
    }

    void addVertex(const Point3D& vertex) {
        if (vertex_count > max_vertices) {
            #if HANDMADE_FAST
            #else
            *(int * )0 = 0; // crash if we exceed the maximum number of vertices
            #endif
            return;
        }
        int chunkIndex = vertex_count / 4;
        int laneIndex = vertex_count % 4;
        chunks[chunkIndex].x[laneIndex] = vertex.x;
        chunks[chunkIndex].y[laneIndex] = vertex.y;
        chunks[chunkIndex].z[laneIndex] = vertex.z;
        vertex_count++;
    }

    void translate(const Point3D &translation) {
        for (int i = 0; i < chunk_count; ++i)
        {      
            //TODO : SIMD this
            for (int laneIndex = 0; laneIndex < 4; ++laneIndex)
            {
                chunks[i].x[laneIndex] += translation.x;
                chunks[i].y[laneIndex] += translation.y;
                chunks[i].z[laneIndex] += translation.z;
            }
            //on déborde potentiellement dans le dernier chunk... mais osef, il est déjà réservé.
        }
    }

    void BasicProjectionInPlace() {
        for (int i = 0; i < chunk_count; ++i)
        {      
            //TODO : SIMD this
            for (int laneIndex = 0; laneIndex < 4; ++laneIndex)
            {
                float z = chunks[i].z[laneIndex];
                if (z != 0.0f) // avoid division by zero
                {
                    chunks[i].x[laneIndex] /= z;
                    chunks[i].y[laneIndex] /= z;
                }
            }
        }
    }

};
#endif // POINT3D_H