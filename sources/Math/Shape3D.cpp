#include "Shape3D.h"
#include "Point3D.cpp"

void PointCloud::addVertex(const Point3D vertex)
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

void PointCloud::rotate_degree(const Point3D &axis, float angleDegrees)
{
    float angleRadians = angleDegrees * (3.14159265f / 180.0f);
    rotate(axis, angleRadians);
}

void PointCloud::translate(const Point3D &translation)
{
    center = center + translation;
}

void PointCloud::scale(float scaleFactor)
{
    for (int i = 0; i < vertex_count; ++i)
    {
        vertices[i] = vertices[i] * scaleFactor;
    }
}

void PointCloud::rotate(const Point3D &axis, float angleRadians)
{
    float cosTheta = cosf(angleRadians);
    float sinTheta = sinf(angleRadians);

    float rotationMatrix[9] = {
        {cosTheta + (1 - cosTheta) * axis.x * axis.x},
        {(1 - cosTheta) * axis.x * axis.y - axis.z * sinTheta},
        {(1 - cosTheta) * axis.x * axis.z + axis.y * sinTheta},
        {(1 - cosTheta) * axis.y * axis.x + axis.z * sinTheta},
        {cosTheta + (1 - cosTheta) * axis.y * axis.y},
        {(1 - cosTheta) * axis.y * axis.z - axis.x * sinTheta},
        {(1 - cosTheta) * axis.z * axis.x - axis.y * sinTheta},
        {(1 - cosTheta) * axis.z * axis.y + axis.x * sinTheta},
        {cosTheta + (1 - cosTheta) * axis.z * axis.z}};

    for (int i = 0; i < vertex_count; ++i)
    {
        Point3D &v = vertices[i];
        Point3D temp = v;

        v.x = rotationMatrix[0] * temp.x + rotationMatrix[1] * temp.y + rotationMatrix[2] * temp.z;

        v.y = rotationMatrix[3] * temp.x + rotationMatrix[4] * temp.y + rotationMatrix[5] * temp.z;

        v.z = rotationMatrix[6] * temp.x + rotationMatrix[7] * temp.y + rotationMatrix[8] * temp.z;

    }
}



WireFrame3D GetSimpleCube(ScratchArena *arena) {
    WireFrame3D cube_shape = {};

    cube_shape.max_vertices = 8;
    cube_shape.vertices = (Point3D *)PushSize(arena, sizeof(Point3D) * cube_shape.max_vertices);

    cube_shape.addVertex(Point3D{-1.0f, -1.0f, -1.0f});
    cube_shape.addVertex(Point3D{1.0f, -1.0f, -1.0f});
    cube_shape.addVertex(Point3D{1.0f, 1.0f, -1.0f});
    cube_shape.addVertex(Point3D{-1.0f, 1.0f, -1.0f});
    cube_shape.addVertex(Point3D{-1.0f, -1.0f, 1.0f});
    cube_shape.addVertex(Point3D{1.0f, -1.0f, 1.0f});
    cube_shape.addVertex(Point3D{1.0f, 1.0f, 1.0f});
    cube_shape.addVertex(Point3D{-1.0f, 1.0f, 1.0f});

    cube_shape.center = Point3D{0.0f, 0.0f, 0.0f};

    cube_shape.max_edges = 12;
    
    cube_shape.edges = (std::pair<uint16_t, uint16_t> *)PushSize(arena, sizeof(std::pair<uint16_t, uint16_t>) * cube_shape.max_edges);

    // front face
    cube_shape.addEdge(0, 1);
    cube_shape.addEdge(1, 2);
    cube_shape.addEdge(2, 3);
    cube_shape.addEdge(3, 0);
    // back face
    cube_shape.addEdge(4, 5);
    cube_shape.addEdge(5, 6);
    cube_shape.addEdge(6, 7);
    cube_shape.addEdge(7, 4);
    // sides
    cube_shape.addEdge(0, 4);
    cube_shape.addEdge(1, 5);
    cube_shape.addEdge(2, 6);
    cube_shape.addEdge(3, 7);

    return cube_shape;
}

/* 
    summons Cube mesh into TRANSIANT memory.
    do what you can with it (like copy it to permanent)
*/
Mesh3D GetCubeMesh(ScratchArena *arena) {
    Mesh3D cube_mesh = {};

    cube_mesh.max_vertices = 8;
    cube_mesh.vertices = (Point3D *)PushSize(arena, sizeof(Point3D) * cube_mesh.max_vertices);

    cube_mesh.addVertex(Point3D{-1.0f, -1.0f, -1.0f});//0
    cube_mesh.addVertex(Point3D{1.0f, -1.0f, -1.0f});//1
    cube_mesh.addVertex(Point3D{1.0f, 1.0f, -1.0f});//2
    cube_mesh.addVertex(Point3D{-1.0f, 1.0f, -1.0f});//3
    cube_mesh.addVertex(Point3D{-1.0f, -1.0f, 1.0f});//4
    cube_mesh.addVertex(Point3D{1.0f, -1.0f, 1.0f});//5
    cube_mesh.addVertex(Point3D{1.0f, 1.0f, 1.0f});//6
    cube_mesh.addVertex(Point3D{-1.0f, 1.0f, 1.0f});//7

    cube_mesh.center = Point3D{0.0f, 0.0f, 0.0f};

    cube_mesh.max_faces = 12; // 2 triangles per face * 6 faces
    cube_mesh.faces = (Face *)PushSize(arena, sizeof(Face) * cube_mesh.max_faces);

    // front face
    cube_mesh.addFace(0, 1, 2, 0xFF0000); // red
    cube_mesh.addFace(0, 2, 3, 0xFF0000); // red
    // back face
    cube_mesh.addFace(4, 6, 5, 0x00FF00); // green
    cube_mesh.addFace(4, 7, 6, 0x00FF00); // green
    // left face
    cube_mesh.addFace(4, 5, 1, 0x0000FF); // blue
    cube_mesh.addFace(4, 1, 0, 0x0000FF); // blue
    // right face
    cube_mesh.addFace(3, 2, 6, 0xFFFF00); // yellow
    cube_mesh.addFace(3, 6, 7, 0xFFFF00); // yellow
    // top face
    cube_mesh.addFace(1, 5, 6, 0xFF00FF); // magenta
    cube_mesh.addFace(1, 6, 2, 0xFF00FF); // magenta
    // bottom face
    cube_mesh.addFace(4, 0, 3, 0x00FFFF); // cyan
    cube_mesh.addFace(4, 3, 7, 0x00FFFF); // cyan
    return cube_mesh;
}