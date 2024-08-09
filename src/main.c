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

void AddQuad(fVector3 points[4], int color)
{
    if (quad_count >= 512)
        return;
    int z = (points[0].z + points[1].z + points[2].z + points[3].z) / 4;
    quads[quad_count].points[0] = (fVector2){points[0].x, points[0].y};
    quads[quad_count].points[1] = (fVector2){points[1].x, points[1].y};
    quads[quad_count].points[2] = (fVector2){points[2].x, points[2].y};
    quads[quad_count].points[3] = (fVector2){points[3].x, points[3].y};
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
    /* Conversion des positions en fixed-point */
    fixed_t x = INT_TO_FIXED(position.x - camera->transform.position.x);
    fixed_t y = INT_TO_FIXED(position.y - camera->transform.position.y);
    fixed_t z = INT_TO_FIXED(position.z - camera->transform.position.z);

    /* Calcul du cosinus et sinus de l'angle de rotation de la caméra en fixed-point */
    fixed_t cos_theta = fcos_approx(FLOAT_TO_FIXED(camera->transform.rotation.x));
    fixed_t sin_theta = fsin_approx(FLOAT_TO_FIXED(camera->transform.rotation.x));

    /* Application de la rotation autour de l'axe X */
    fixed_t new_y = fmul(y, cos_theta) - fmul(z, sin_theta);
    fixed_t new_z = fmul(y, sin_theta) + fmul(z, cos_theta);
    y = new_y;
    z = new_z;

    /* Projection en coordonnées écran */
    if (z <= 0)
        return 0;
    fixed_t f = fdiv(INT_TO_FIXED(300), z);
    x = fmul(x, f);
    y = fmul(y, f);

    /* Vérification si l'objet est hors de l'écran */
    if (TO_INT(x) > (SCREEN_WIDTH / 2) + 80 || TO_INT(x) < -(SCREEN_WIDTH / 2) - 80)
        return 0;
    if (TO_INT(y) > (SCREEN_HEIGHT / 2) + 80 || TO_INT(y) < -(SCREEN_HEIGHT / 2) - 80)
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

void DrawFilledCube(fVector3 render_points[8])
{
    //AddQuad((fVector3[]){render_points[0], render_points[1], render_points[2], render_points[3]}, C_WHITE);
    AddQuad((fVector3[]){render_points[4], render_points[5], render_points[6], render_points[7]}, C_WHITE);
    if (render_points[0].y < render_points[4].y)
        AddQuad((fVector3[]){render_points[0], render_points[1], render_points[5], render_points[4]}, C_BLUE);
    if (render_points[5].x < render_points[1].x)
        AddQuad((fVector3[]){render_points[5], render_points[1], render_points[2], render_points[6]}, C_GREEN);
    if (render_points[6].y < render_points[2].y)
        AddQuad((fVector3[]){render_points[3], render_points[2], render_points[6], render_points[7]}, C_BLUE);
    if (render_points[3].x < render_points[7].x)
        AddQuad((fVector3[]){render_points[0], render_points[4], render_points[7], render_points[3]}, C_GREEN);
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
    fVector3 render_points[8] = {
        vertices[0].projected,
        vertices[1].projected,
        vertices[2].projected,
        vertices[3].projected,
        vertices[4].projected,
        vertices[5].projected,
        vertices[6].projected,
        vertices[7].projected
    };
    //DrawLinesCube(vertices);
    DrawFilledCube(render_points);
    
    
}



int main(){
    srand(time(NULL));
    clock_set_speed(CLOCK_SPEED_F5);
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