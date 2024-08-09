#include <stdio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "Projection.h"

/*void DrawFilledCube(Vector2 render_points[8])
{
    //DrawFilledQuad((Vector2[]){render_points[0], render_points[1], render_points[2], render_points[3]},C_WHITE);
    DrawFilledQuad((Vector2[]){render_points[4], render_points[5], render_points[6], render_points[7]},C_WHITE);
    if (render_points[0].y < render_points[4].y)
        DrawFilledQuad((Vector2[]){render_points[0], render_points[1], render_points[5], render_points[4]},C_BLUE);
    if (render_points[5].x < render_points[1].x)
        DrawFilledQuad((Vector2[]){render_points[5], render_points[1], render_points[2], render_points[6]},C_GREEN);
    if (render_points[6].y < render_points[2].y)
        DrawFilledQuad((Vector2[]){render_points[3], render_points[2], render_points[6], render_points[7]},C_BLUE);
    if (render_points[3].x < render_points[7].x)
        DrawFilledQuad((Vector2[]){render_points[0], render_points[4], render_points[7], render_points[3]},C_GREEN);
}*/

RenderQuad quads[512];
size_t quad_count = 0;

void AddQuad(Vector2 points[4], int color, int z)
{
    if (quad_count >= 512)
        return;
    //quads[quad_count] = (RenderQuad){(Vector2[4]){points[0], points[1], points[2], points[3]}, z, color};
    quads[quad_count].points[0] = points[0];
    quads[quad_count].points[1] = points[1];
    quads[quad_count].points[2] = points[2];
    quads[quad_count].points[3] = points[3];
    quads[quad_count].z = z;
    quads[quad_count].color = color;
    quad_count++;
}

void SortQuads()
{
    for (size_t i = 0; i < quad_count; i++)
    {
        for (size_t j = 0; j < quad_count - i - 1; j++)
        {
            if (quads[j].z < quads[j + 1].z)
            {
                RenderQuad temp = quads[j];
                quads[j] = quads[j + 1];
                quads[j + 1] = temp;
            }
        }
    }
}

void DrawQuads()
{
    for (size_t i = 0; i < quad_count; i++)
    {
        DrawFilledQuad(quads[i].points, quads[i].color);
    }
    quad_count = 0;
}






int DetectInRange(Camera* camera, Vector3 position)
{
    int x = position.x - camera->transform.position.x;
    int y = position.y - camera->transform.position.y;
    int z = position.z - camera->transform.position.z;

    const float cos_theta = cos(camera->transform.rotation.x);
    const float sin_theta = sin(camera->transform.rotation.x);
    const float new_y = y * cos_theta - z * sin_theta;
    const float new_z = y * sin_theta + z * cos_theta;
    y = new_y;
    z = new_z;

    float f = 300 / (float)z;
    x = (x * f);
    y = (y * f);
    if (x > (SCREEN_WIDTH / 2) + 80 || x < -(SCREEN_WIDTH / 2) - 80)
        return 0;
    if (y > (SCREEN_HEIGHT / 2) + 80 || y < -(SCREEN_HEIGHT / 2) - 80)
        return 0;
    return 1;
}

void DrawLinesCube(Vertex vertices[8])
{
    for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            if (vertices[i].projected.z <= 1 || vertices[j].projected.z <= 1)
                continue;
            if (i != j)
                dline(vertices[i].projected.x,vertices[i].projected.y,
                        vertices[j].projected.x,vertices[j].projected.y,
                        C_RED);
        }
    }
}

void DrawFilledCube(Vector2 render_points[8],int depth[8])
{
    //AddQuad((Vector2[]){render_points[0], render_points[1], render_points[2], render_points[3]}, C_WHITE, (render_points[0].z + render_points[1].z + render_points[2].z + render_points[3].z) / 4);
    AddQuad((Vector2[]){render_points[4], render_points[5], render_points[6], render_points[7]}, C_WHITE, (depth[4] + depth[5] + depth[6] + depth[7]) / 4);
    if (render_points[0].y < render_points[4].y)
        AddQuad((Vector2[]){render_points[0], render_points[1], render_points[5], render_points[4]}, C_BLUE, (depth[0] + depth[1] + depth[5] + depth[4]) / 4);
    if (render_points[5].x < render_points[1].x)
        AddQuad((Vector2[]){render_points[5], render_points[1], render_points[2], render_points[6]}, C_GREEN, (depth[5] + depth[1] + depth[2] + depth[6]) / 4);
    if (render_points[6].y < render_points[2].y)
        AddQuad((Vector2[]){render_points[3], render_points[2], render_points[6], render_points[7]}, C_BLUE, (depth[3] + depth[2] + depth[6] + depth[7]) / 4);
    if (render_points[3].x < render_points[7].x)
        AddQuad((Vector2[]){render_points[0], render_points[4], render_points[7], render_points[3]}, C_GREEN, (depth[0] + depth[4] + depth[7] + depth[3]) / 4);
}

void DrawBlock(Camera* camera, Vector3 position)
{
    if (!DetectInRange(camera, position))
        return;
    int size = 40;
    Vertex vertices[8] = {
        {{-size, size, size}, {0, 0, 0}},
        {{ size, size, size}, {0, 0, 0}},
        {{ size,-size, size}, {0, 0, 0}},
        {{-size,-size, size}, {0, 0, 0}},

        {{-size, size,-size}, {0, 0, 0}},
        {{ size, size,-size}, {0, 0, 0}},
        {{ size,-size,-size}, {0, 0, 0}},
        {{-size,-size,-size}, {0, 0, 0}},
    };
    Mesh mesh = (Mesh){vertices,8};
    Projection obj = (Projection){&mesh,{position,(Vector3f){0,0,0},(Vector3f){1,1,1}}};
    CalculateProjection(camera, &obj);
    Vector2 render_points[8] = {
        {vertices[0].projected.x, vertices[0].projected.y},
        {vertices[1].projected.x, vertices[1].projected.y},
        {vertices[2].projected.x, vertices[2].projected.y},
        {vertices[3].projected.x, vertices[3].projected.y},
        {vertices[4].projected.x, vertices[4].projected.y},
        {vertices[5].projected.x, vertices[5].projected.y},
        {vertices[6].projected.x, vertices[6].projected.y},
        {vertices[7].projected.x, vertices[7].projected.y},
    };
    int depth[8] = {
        vertices[0].projected.z,
        vertices[1].projected.z,
        vertices[2].projected.z,
        vertices[3].projected.z,
        vertices[4].projected.z,
        vertices[5].projected.z,
        vertices[6].projected.z,
        vertices[7].projected.z,
    };
    //DrawLinesCube(vertices);
    DrawFilledCube(render_points, depth);
    
    
}



int main(){
    init_uv_map();
    Camera camera = (Camera){(Vector3){0,0,-500},(Vector3f){0,0,0},(Vector3f){1,1,1}};
    while (!keydown(KEY_MENU))
    {
        dclear(C_BLACK);
        clearevents();
        DrawBlock(&camera, (Vector3){0,0,100});
        DrawBlock(&camera, (Vector3){100, 100, 200});
        for (size_t i = 0; i < 100; i++)
        {
            DrawBlock(&camera, (Vector3){i * 100, 0, 300});
        }

        if(keydown(KEY_LEFT))
            camera.transform.position.x -= 10;
        if(keydown(KEY_RIGHT))
            camera.transform.position.x += 10;
        if(keydown(KEY_UP))
            camera.transform.position.z += 10;
        if(keydown(KEY_DOWN))
            camera.transform.position.z -= 10;

        if(keydown(KEY_8))
            camera.transform.position.y += 10;
        if(keydown(KEY_2))
            camera.transform.position.y -= 10;

        if(keydown(KEY_7))
            camera.transform.rotation.x += 0.1;
        if(keydown(KEY_1))
            camera.transform.rotation.x -= 0.1;

        if(keydown(KEY_4))
            camera.transform.rotation.y += 0.1;
        if(keydown(KEY_6))
            camera.transform.rotation.y -= 0.1;

        camera.transform.rotation.x = clamp(camera.transform.rotation.x, -M_PI_2/3, M_PI_2/3);
        camera.transform.rotation.y = clamp(camera.transform.rotation.y, -M_PI_2/3, M_PI_2/3);

        SortQuads();
        DrawQuads();

        dupdate();
    }
    return 0;
}