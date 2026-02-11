#include "visual_func.h"
#include <cstdlib>
#include <string>
#include "../font8x8_basic.h"

// visual engine functions
void renderCharacter(HandmadeScreenBuffer *buffer, char character, int x, int y)
{
    uint8_t *row = (uint8_t *)buffer->Memory + y * buffer->Pitch + x * 4;
    for (int glyph_y = 0; glyph_y < 8; ++glyph_y)
    {
        uint8_t glyph_row = font8x8_basic[(uint8_t)character][glyph_y];
        uint32_t *pixel = (uint32_t *)row;
        for (int glyph_x = 0; glyph_x < 8; ++glyph_x)
        {
            uint8_t mask = 1 << glyph_x;
            if (glyph_row & mask)
            {
                *pixel++ = (uint32_t)(0x00FFFFFF); // White
            }
            else
            {
                *pixel++ = (uint32_t)(0x00000000); // Black
            }
        }
        row += buffer->Pitch;
    }
}

void renderString(HandmadeScreenBuffer *buffer, const std::string &str, int x, int y)
{
    int cursor_x = x;
    for (char c : str)
    {
        renderCharacter(buffer, c, cursor_x, y);
        cursor_x += 8; // Advance cursor by character width
        if (cursor_x + 8 > buffer->Width)
        {
            break; // Stop rendering if we exceed buffer width
        }
    }
}

void RenderGradient(HandmadeScreenBuffer *Buffer, int XOffset, int YOffset)
{

    int small_white_square_width = 1;
    int small_white_square_height = 50;
    int centerX = Buffer->Width / 2;
    int centerY = Buffer->Height / 2;
    uint8_t *row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *pixel = (uint32_t *)row;
        for (int X = 0; X < Buffer->Width; ++X)
        {

            if ((X - centerX + XOffset) >= 0 && (X - centerX + XOffset) < small_white_square_width &&
                (Y - centerY + YOffset) >= 0 && (Y - centerY + YOffset) < small_white_square_height)
            {
                *pixel++ = 0x00FFFFFF;
            }
            else
            {
                uint8_t blue = (X + XOffset) % 256;
                uint8_t green = (Y + YOffset) % 256;
                uint8_t red = 0;
                *pixel++ = ((red << 16) | (green << 8) | blue);
            }
        }
        row += Buffer->Pitch;
    }
}

void renderArrayPattern(HandmadeScreenBuffer *Buffer, RenderingArray array,
                        int x_offset = 0, int y_offset = 0, float zoom_level = 1.0f)
{

    float StepX, StepY;
    if (array.CellSize > 0)
    {
        StepX = 1 / ((float)array.CellSize * zoom_level);
        StepY = 1 / ((float)array.CellSize * zoom_level);
    }
    else
    {
        StepX = (float)(array.Width) / ((float)Buffer->Width * zoom_level);
        StepY = (float)(array.Height) / ((float)Buffer->Height * zoom_level);
    }

    uint8_t *row = (uint8_t *)Buffer->Memory;

    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *pixel = (uint32_t *)row;

        float current_y = (Y + y_offset) * StepY;
        int array_index_y = (int)current_y;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            float current_x = (X + x_offset) * StepX;
            int array_index_x = (int)current_x;

            if (array_index_x >= array.Width || array_index_x < 0 || array_index_y >= array.Height || array_index_y < 0)
            {
                // out of bounds
                *pixel++ = 0x000F0F0F; // very mid gray
            }
            else
            {
                *pixel++ = array.Array[array_index_y * array.Width + array_index_x];
            }
        }
        row += Buffer->Pitch;
    }
}

void RenderPoints(HandmadeScreenBuffer *Buffer, PointCloud &cube)
{

    Point3D screenCenter = {};
    screenCenter.x = Buffer->Width / 2;
    screenCenter.y = Buffer->Height / 2;

    float quarter_size = (std::min)(Buffer->Width, Buffer->Height) / 4.0f;

    for (int i = 0; i < cube.vertex_count; i++)
    {
        Point3D screenPoint = (cube.vertices[i] + cube.center).BasicProjection();
        // now x and y are "clamped" between -1 and 1
        screenPoint.x *= quarter_size; // scale to screen space
        screenPoint.y *= quarter_size;
        // point on screen as if it was centered at 0,0
        screenPoint += screenCenter;
        SAFE_COLOR_PIXEL(Buffer, (int)screenPoint.x, (int)screenPoint.y, 0x00FFFF00)
    }
}

void RenderCubeSides(HandmadeScreenBuffer *Buffer, WireFrame3D &cube)
{
    // TODO : draw edges instead of vertices
    Point3D screenCenter = {};
    screenCenter.x = Buffer->Width / 2;
    screenCenter.y = Buffer->Height / 2;

    float quarter_size = (std::min)(Buffer->Width, Buffer->Height) / 4.0f;
    Point3D *vertices = (Point3D *)PushSize(&GlobalMemory.Transient, sizeof(Point3D) * cube.vertex_count);
    // copies the points, while sending them in screenspace.
    for (int i = 0; i < cube.vertex_count; i++)
    {
        vertices[i] = (cube.vertices[i] + cube.center).BasicProjection();
        vertices[i].x *= quarter_size; // scale to screen space
        vertices[i].y *= quarter_size;
        vertices[i] += screenCenter;
    }

    for (int i = 0; i < cube.edge_count; i++)
    {
        std::pair<uint16_t, uint16_t> Edge = cube.edges[i];

        Point3D p1 = vertices[Edge.first];
        Point3D p2 = vertices[Edge.second];

        int x0 = (int)p1.x;
        int y0 = (int)p1.y;
        int x1 = (int)p2.x;
        int y1 = (int)p2.y;

        int dx = abs(x1 - x0);
        int dy = -abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx + dy;
        int e2;

        float alpha = 0;
        float alpha_factor = 1.0f / (float)(dx + abs(dy) + 1);
        while (true)
        {
            alpha += alpha_factor;
            if (alpha > 1.0f)
            {
                alpha = 1.0f;
            }
            else if (alpha < 0.0f)
            {
                alpha = 0.0f;
            }

            float sample_z = p1.z * (1.0f - alpha) + p2.z * alpha;

            float color_intensity = (1.1f - (sample_z / 750.0f));
            if (color_intensity < 0.0f)
            {
                color_intensity = 0.0f;
            }
            else if (color_intensity > 1.0f)
            {
                color_intensity = 1.0f;
            }

            uint8_t red = (uint8_t)((1.0f - color_intensity) * 0xFF);
            uint8_t green = (uint8_t)(color_intensity * 0xFF);
            uint8_t blue = 0xA0;
            uint32_t color = (red << 16) | (green << 8) | blue;

            SAFE_COLOR_PIXEL(Buffer, x0, y0, color);
            if (x0 == x1 && y0 == y1)
            {
                break;
            }
            e2 = 2 * err;
            if (e2 >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    GlobalMemory.Transient.used -= sizeof(Point3D) * cube.vertex_count; // we can free the memory we allocated for the vertices, since we won't be using it anymore

}

void RenderMesh3D(HandmadeScreenBuffer *Buffer, Mesh3D &mesh, float *depthBuffer)
{
    // TODO : draw edges instead of vertices
    Point3D screenCenter = {};
    screenCenter.x = Buffer->Width / 2;
    screenCenter.y = Buffer->Height / 2;

    float quarter_size = (std::min)(Buffer->Width, Buffer->Height) / 4.0f;

    Point3D *vertices = (Point3D *)PushSize(&GlobalMemory.Transient, sizeof(Point3D) * mesh.vertex_count);
    // copies the points, while sending them in screenspace.
    for (int i = 0; i < mesh.vertex_count; i++)
    {
        vertices[i] = (mesh.vertices[i] + mesh.center).BasicProjection();
        vertices[i].x *= quarter_size; // scale to screen space
        vertices[i].y *= quarter_size;
        vertices[i] += screenCenter;
    }

    for (int i = 0; i < mesh.face_count; i++)
    {
        Face *face = &mesh.faces[i];

        Point3D p1 = vertices[face->v[0]];
        Point3D p2 = vertices[face->v[1]];
        Point3D p3 = vertices[face->v[2]];

        float min_x = (std::max)(0.0f, (std::min)({p1.x, p2.x, p3.x}));
        float max_x = (std::min)((float)(Buffer->Width), (std::max)({p1.x, p2.x, p3.x}));
        if (max_x <= min_x)
            continue;
        float min_y = (std::max)(0.0f, (std::min)({p1.y, p2.y, p3.y}));
        float max_y = (std::min)((float)(Buffer->Height), (std::max)({p1.y, p2.y, p3.y}));
        if (max_y <= min_y)
            continue;


        float alpha_divide = ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));
        if (alpha_divide == 0.0f) [[unlikely]]
        {
            continue;
        }
        else
            alpha_divide = 1.0f / alpha_divide;
        float beta_divide = ((p3.y - p1.y) * (p2.x - p3.x) + (p1.x - p3.x) * (p2.y - p3.y));
        // likely false test :
        if (beta_divide == 0.0f) [[unlikely]]
        {
            continue;
        }
        else
            beta_divide = 1.0f / beta_divide;

        for (int y = (int)min_y; y <= (int)max_y; ++y)
        {
            for (int x = (int)min_x; x <= (int)max_x; ++x)
            {
                // barycentric coordinates
                float alpha = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) * alpha_divide;
                float beta = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) * beta_divide;
                float gamma = 1.0f - alpha - beta;

                if (alpha >= 0 && beta >= 0 && gamma >= 0)
                {
                    int pixelBufferIndex = y * Buffer->Width + x;
                    // inside the triangle
                    float sample_z = p1.z * alpha + p2.z * beta + p3.z * gamma;

                    if (sample_z < depthBuffer[pixelBufferIndex])
                    {
                        depthBuffer[pixelBufferIndex] = sample_z;
                        COLOR_PIXEL(Buffer, x, y, face->color);
                    }
                }
            }
        }
    }
    GlobalMemory.Transient.used -= sizeof(Point3D) * mesh.vertex_count; // we can free the memory we allocated for the vertices, since we won't be using it anymore
}

void RenderMesh3DWithFaceOrientation(HandmadeScreenBuffer *Buffer, Mesh3D &mesh, float *depthBuffer)
{
    // TODO : draw edges instead of vertices
    Point3D screenCenter = {};
    screenCenter.x = Buffer->Width / 2;
    screenCenter.y = Buffer->Height / 2;

    float quarter_size = (std::min)(Buffer->Width, Buffer->Height) / 4.0f;

    Point3D light_direction = Point3D{1.0f, 0, 1.0f}.normalized();

    // vertices in screen space
    Point3D *vertices = (Point3D *)PushSize(&GlobalMemory.Transient, sizeof(Point3D) * mesh.vertex_count);
    // copies the points, while sending them in screenspace.
    for (int i = 0; i < mesh.vertex_count; i++)
    {
        vertices[i] = (mesh.vertices[i] + mesh.center).BasicProjection();
        vertices[i].x *= quarter_size; // scale to screen space
        vertices[i].y *= quarter_size;
        vertices[i] += screenCenter;
    }

    for (int i = 0; i < mesh.face_count; i++)
    {
        Face *face = &mesh.faces[i];

        Point3D p1 = vertices[face->v[0]];
        Point3D p2 = vertices[face->v[1]];
        Point3D p3 = vertices[face->v[2]];

        float min_x = (std::max)(0.0f, (std::min)({p1.x, p2.x, p3.x}));
        float max_x = (std::min)((float)(Buffer->Width), (std::max)({p1.x, p2.x, p3.x}));
        if (max_x <= min_x)
            continue;
        float min_y = (std::max)(0.0f, (std::min)({p1.y, p2.y, p3.y}));
        float max_y = (std::min)((float)(Buffer->Height), (std::max)({p1.y, p2.y, p3.y}));
        if (max_y <= min_y)
            continue;

        // calcul de l'orientation de la face
        Point3D screen_face_normal = crossProduct(p2 - p1, p3 - p1);
        if (screen_face_normal.z <= 0.0f) [[unlikely]]
            continue; // backface culling

        float alpha_divide = ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));
        if (alpha_divide == 0.f) [[unlikely]]
            continue;
        else
            alpha_divide = 1.0f / alpha_divide;
        float beta_divide = ((p3.y - p1.y) * (p2.x - p3.x) + (p1.x - p3.x) * (p2.y - p3.y));
        if (beta_divide == 0.f) [[unlikely]]
            continue;
        else
            beta_divide = 1.0f / beta_divide;

        uint32_t shaded_color = 0;
        {
            Point3D n_original_p1 = mesh.vertices[face->v[0]];
            Point3D n_original_p2 = mesh.vertices[face->v[1]];
            Point3D n_original_p3 = mesh.vertices[face->v[2]];
            Point3D real_normal = crossProduct(n_original_p2 - n_original_p1, n_original_p3 - n_original_p1);
            float light_intensity = dotProduct(screen_face_normal.normalized(), light_direction);
            if (light_intensity < 0.0f)
                light_intensity = 0.0f;
            else if (light_intensity > 1.0f)
                light_intensity = 1.0f;

            uint8_t face_red = (uint8_t)((face->color >> 16) & 0xFF) * light_intensity;
            uint8_t face_green = (uint8_t)((face->color >> 8) & 0xFF) * light_intensity;
            uint8_t face_blue = (uint8_t)(face->color & 0xFF) * light_intensity;

            shaded_color = (face_red << 16) | (face_green << 8) | face_blue;
        }

        for (int y = (int)min_y; y <= (int)max_y; ++y)
        {
            for (int x = (int)min_x; x <= (int)max_x; ++x)
            {
                // barycentric coordinates
                float alpha = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) * alpha_divide;
                float beta = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) * beta_divide;
                float gamma = 1.0f - alpha - beta;

                if (alpha >= 0 && beta >= 0 && gamma >= 0)
                {
                    int pixelBufferIndex = y * Buffer->Width + x;
                    // inside the triangle
                    float sample_z = p1.z * alpha + p2.z * beta + p3.z * gamma;

                    if (sample_z < depthBuffer[pixelBufferIndex] && sample_z > 0.0f)
                    {
                        depthBuffer[pixelBufferIndex] = sample_z;
                        COLOR_PIXEL(Buffer, x, y, shaded_color);
                    }
                }
            }
        }
    }

}


void renderMesh2(HandmadeScreenBuffer *Buffer, Mesh3D2 & mesh, float *depthBuffer)
{
    // TODO : draw edges instead of vertices
    Point3D screenCenter = {};
    screenCenter.x = Buffer->Width / 2;
    screenCenter.y = Buffer->Height / 2;

    float quarter_size = (std::min)(Buffer->Width, Buffer->Height) / 4.0f;

    Point3D light_direction = Point3D{1.0f, 0, 1.0f}.normalized();

    Point3DCloud transformed_vertices = {};
    transformed_vertices.vertex_count = mesh.vertex_count;
    transformed_vertices.max_vertices = mesh.max_vertices;
    transformed_vertices.chunk_count = mesh.chunk_count;
    transformed_vertices.chunks = (Point3DChunk *)PushSize(&GlobalMemory.Transient, sizeof(Point3DChunk) * transformed_vertices.chunk_count);
    // copies the points, while sending them in screenspace.

    Quaternion rotation = mesh.Rotation.normalized(); // ensure the rotation is normalized
    rotation.rotateArrayOf3DPoint(&mesh, &transformed_vertices); // rotate the vertices of the mesh and store the result in transformed_vertices
    transformed_vertices.translate(mesh.Location); // translate the vertices of the mesh to their world position
    transformed_vertices.BasicProjectionInPlace(); // project the vertices to screen space
    for (int i = 0; i < transformed_vertices.chunk_count; i++)
    {
        //ça ça se SIMD aussi.
        for (int lane = 0; lane < 4; lane++)
        {
            transformed_vertices.chunks[i].x[lane] *= quarter_size;
            transformed_vertices.chunks[i].y[lane] *= quarter_size;
            transformed_vertices.chunks[i].x[lane] += screenCenter.x;
            transformed_vertices.chunks[i].y[lane] += screenCenter.y;
        }
    }

    for (int i = 0; i < mesh.face_count; i++)
    {
        Face *face = &mesh.faces[i];

        Point3D p1 = transformed_vertices.get(face->v[0]);//this part is very much NOT optimised
        Point3D p2 = transformed_vertices.get(face->v[1]);
        Point3D p3 = transformed_vertices.get(face->v[2]);

        float min_x = (std::max)(0.0f, (std::min)({p1.x, p2.x, p3.x}));
        float max_x = (std::min)((float)(Buffer->Width), (std::max)({p1.x, p2.x, p3.x}));
        if (max_x <= min_x)
            continue;
        float min_y = (std::max)(0.0f, (std::min)({p1.y, p2.y, p3.y}));
        float max_y = (std::min)((float)(Buffer->Height), (std::max)({p1.y, p2.y, p3.y}));
        if (max_y <= min_y)
            continue;

        // calcul de l'orientation de la face
        Point3D screen_face_normal = crossProduct(p2 - p1, p3 - p1);
        if (screen_face_normal.z <= 0.0f) [[unlikely]]
            continue; // backface culling

        float alpha_divide = ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));
        if (alpha_divide == 0.f) [[unlikely]]
            continue;
        else
            alpha_divide = 1.0f / alpha_divide;
        float beta_divide = ((p3.y - p1.y) * (p2.x - p3.x) + (p1.x - p3.x) * (p2.y - p3.y));
        if (beta_divide == 0.f) [[unlikely]]
            continue;
        else
            beta_divide = 1.0f / beta_divide;

        uint32_t shaded_color = 0;
        {
            Point3D n_original_p1 = mesh.get(face->v[0]);//not optimized at all
            Point3D n_original_p2 = mesh.get(face->v[1]);
            Point3D n_original_p3 = mesh.get(face->v[2]);
            Point3D real_normal = crossProduct(n_original_p2 - n_original_p1, n_original_p3 - n_original_p1);
            float light_intensity = dotProduct(screen_face_normal.normalized(), light_direction);
            if (light_intensity < 0.0f)
                light_intensity = 0.0f;
            else if (light_intensity > 1.0f)
                light_intensity = 1.0f;

            uint8_t face_red = (uint8_t)((face->color >> 16) & 0xFF) * light_intensity;
            uint8_t face_green = (uint8_t)((face->color >> 8) & 0xFF) * light_intensity;
            uint8_t face_blue = (uint8_t)(face->color & 0xFF) * light_intensity;

            shaded_color = (face_red << 16) | (face_green << 8) | face_blue;
        }

        for (int y = (int)min_y; y <= (int)max_y; ++y)
        {
            for (int x = (int)min_x; x <= (int)max_x; ++x)
            {
                // barycentric coordinates
                float alpha = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) * alpha_divide;
                float beta = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) * beta_divide;
                float gamma = 1.0f - alpha - beta;

                if (alpha >= 0 && beta >= 0 && gamma >= 0)
                {
                    int pixelBufferIndex = y * Buffer->Width + x;
                    // inside the triangle
                    float sample_z = p1.z * alpha + p2.z * beta + p3.z * gamma;

                    if (sample_z < depthBuffer[pixelBufferIndex] && sample_z > 0.0f)
                    {
                        depthBuffer[pixelBufferIndex] = sample_z;
                        COLOR_PIXEL(Buffer, x, y, shaded_color);
                    }
                }
            }
        }
    }
}


void renderSphere3D(HandmadeScreenBuffer *Buffer, Sphere sphere, float *depthBuffer)
{
    Point3D screenCenter = {};
    screenCenter.x = Buffer->Width / 2;
    screenCenter.y = Buffer->Height / 2;

    float quarter_size = (std::min)(Buffer->Width, Buffer->Height) / 4.0f;

    Point3D lightSource = Point3D{-1.0f, 0.0f, 1.0f}.normalized();

    Point3D projectedCenter = (sphere.center).BasicProjection();
    projectedCenter.x *= quarter_size; // scale to screen space
    projectedCenter.y *= quarter_size;
    projectedCenter += screenCenter;                                        // translate to screen center
    float projectedRadius = sphere.radius * quarter_size / sphere.center.z; // simple perspective projection

    int min_x = (int)(projectedCenter.x - projectedRadius);
    if (min_x < 0)
        min_x = 0;
    int max_x = (int)(projectedCenter.x + projectedRadius);
    if (max_x >= Buffer->Width)
        max_x = Buffer->Width - 1;
    int min_y = (int)(projectedCenter.y - projectedRadius);
    if (min_y < 0)
        min_y = 0;
    int max_y = (int)(projectedCenter.y + projectedRadius);
    if (max_y >= Buffer->Height)
        max_y = Buffer->Height - 1;

    for (int y = min_y; y <= max_y; ++y)
    {
        for (int x = min_x; x <= max_x; ++x)
        {
            //

            // test if inside the projected circle
            float dx = x - projectedCenter.x;
            float dy = y - projectedCenter.y;
            if (dx * dx + dy * dy <= projectedRadius * projectedRadius)
            {
                // normal calculation

                float dist_from_center_2 = dx * dx + dy * dy;
                float relative_depth = sqrtf(sphere.radius * sphere.radius - dist_from_center_2);
                float sample_z = sphere.center.z - relative_depth;

                int pixelBufferIndex = y * Buffer->Width + x;
                if (sample_z < depthBuffer[pixelBufferIndex])
                {
                    depthBuffer[pixelBufferIndex] = sample_z;

                    Point3D normal = Point3D{dx, dy, relative_depth}.normalized();
                    float light_intensity = dotProduct(normal, lightSource);
                    if (light_intensity < 0.0f)
                        light_intensity = 0.0f;
                    else if (light_intensity > 1.0f)
                        light_intensity = 1.0f;

                    uint8_t shaded_red = (uint8_t)(255 * light_intensity);
                    uint8_t shaded_green = (uint8_t)(0 * light_intensity);
                    uint8_t shaded_blue = (uint8_t)(255 * light_intensity);
                    uint32_t shaded_color = (shaded_red << 16) | (shaded_green << 8) | shaded_blue;

                    COLOR_PIXEL(Buffer, x, y, shaded_color);
                }
            }
        }
    }
}