#include "Projection.h"
#include <math.h>

int uv_map[40*40];

void set_uv_map(int x, int y, int color)
{
    uv_map[x + y * 40] = color;
}

int get_uv_map(int x, int y)
{
    return uv_map[x + y * 40];
}

void init_uv_map()
{
    for (int i = 0; i < 40; i++)
    {
        for (int j = 0; j < 40; j++)
        {
            float u = i / 40.0;
            float v = j / 40.0;
            set_uv_map(i, j, rgb((int)(u * 255), (int)(v * 255), 0));
        }
    }
}



inline float sin_approx(float i)
{
    const float B = 4 / M_PI;
    const float C = -4 / (M_PI * M_PI);

    return B * i + C * i * fabs(i);
}

inline float cos_approx(float i)
{
    return sin_approx(i + M_PI_2);
}

/* Approximation de sin en virgule fixe */
inline fixed_t fsin_approx(fixed_t i)
{
    const fixed_t B = fix(4 / M_PI);
    const fixed_t C = fix(-4 / (M_PI * M_PI));

    return fmul(B, i) + fmul(C, fmul(i, abs(i)));
}

/* Approximation de cos en virgule fixe */
inline fixed_t fcos_approx(fixed_t i)
{
    return fsin_approx(i + fix(M_PI_2));
}

/* Appliquer la rotation autour de l'axe X */
inline static void ApplyRotationX(fixed_t *y, fixed_t *z, fixed_t cos_theta, fixed_t sin_theta)
{
    const fixed_t new_y = fmul(*y, cos_theta) - fmul(*z, sin_theta);
    const fixed_t new_z = fmul(*y, sin_theta) + fmul(*z, cos_theta);
    *y = new_y;
    *z = new_z;
}

/* Appliquer la rotation autour de l'axe Y */
inline static void ApplyRotationY(fixed_t *x, fixed_t *z, fixed_t cos_theta, fixed_t sin_theta)
{
    const fixed_t new_x = fmul(*x, cos_theta) + fmul(*z, sin_theta);
    const fixed_t new_z = fmul(-*x, sin_theta) + fmul(*z, cos_theta);
    *x = new_x;
    *z = new_z;
}

/* Appliquer la rotation autour de l'axe Z */
inline static void ApplyRotationZ(fixed_t *x, fixed_t *y, fixed_t cos_theta, fixed_t sin_theta)
{
    const fixed_t new_x = fmul(*x, cos_theta) - fmul(*y, sin_theta);
    const fixed_t new_y = fmul(*x, sin_theta) + fmul(*y, cos_theta);
    *x = new_x;
    *y = new_y;
}

void CalculateProjection(Camera *camera, Projection* obj)
{
    const fixed_t half_screen_width = INT_TO_FIXED(SCREEN_WIDTH / 2);
    const fixed_t half_screen_height = INT_TO_FIXED(SCREEN_HEIGHT / 2);

    /* Pré-calcul des cos et sin pour la rotation de l'obj */
    const fVector3 cos_transform = {
        fcos_approx(FLOAT_TO_FIXED(obj->transform.rotation.x)),
        fcos_approx(FLOAT_TO_FIXED(obj->transform.rotation.y)),
        fcos_approx(FLOAT_TO_FIXED(obj->transform.rotation.z))
    };

    const fVector3 sin_transform = {
        fsin_approx(FLOAT_TO_FIXED(obj->transform.rotation.x)),
        fsin_approx(FLOAT_TO_FIXED(obj->transform.rotation.y)),
        fsin_approx(FLOAT_TO_FIXED(obj->transform.rotation.z))
    };

    const fVector3 scale = {
        FLOAT_TO_FIXED(obj->transform.scale.x),
        FLOAT_TO_FIXED(obj->transform.scale.y),
        FLOAT_TO_FIXED(obj->transform.scale.z)
    };

    const fVector3 position = {
        INT_TO_FIXED(obj->transform.position.x),
        INT_TO_FIXED(obj->transform.position.y),
        INT_TO_FIXED(obj->transform.position.z)
    };

    /* Pré-calcul des cos et sin pour la rotation de la caméra */
    const fVector3 cos_camera = {
        fcos_approx(FLOAT_TO_FIXED(camera->transform.rotation.x)),
        fcos_approx(FLOAT_TO_FIXED(camera->transform.rotation.y)),
        fcos_approx(FLOAT_TO_FIXED(camera->transform.rotation.z))
    };

    const fVector3 sin_camera = {
        fsin_approx(FLOAT_TO_FIXED(camera->transform.rotation.x)),
        fsin_approx(FLOAT_TO_FIXED(camera->transform.rotation.y)),
        fsin_approx(FLOAT_TO_FIXED(camera->transform.rotation.z))
    };

    const fVector3 camera_position = {
        INT_TO_FIXED(camera->transform.position.x),
        INT_TO_FIXED(camera->transform.position.y),
        INT_TO_FIXED(camera->transform.position.z)
    };

    const fixed_t min_z = INT_TO_FIXED(1);

    for (size_t i = 0; i < obj->mesh->verticesCount; i++)
    {
        /* Accès direct à la position du sommet, en évitant de répéter les conversions */
        const fVector3 vertex_pos = {
            INT_TO_FIXED(obj->mesh->vertices[i].position.x),
            INT_TO_FIXED(obj->mesh->vertices[i].position.y),
            INT_TO_FIXED(obj->mesh->vertices[i].position.z)
        };

        /* Application de l'échelle */
        fVector3 m = {
            fmul(vertex_pos.x, scale.x),
            fmul(vertex_pos.y, scale.y),
            fmul(vertex_pos.z, scale.z)
        };

        // Apply local rotation
        ApplyRotationX(&m.y, &m.z, cos_transform.x, sin_transform.x);
        ApplyRotationY(&m.x, &m.z, cos_transform.y, sin_transform.y);
        ApplyRotationZ(&m.x, &m.y, cos_transform.z, sin_transform.z);

        /* Application de la translation (incluant l'inverse de la translation de la caméra) */
        m.x += position.x - camera_position.x;
        m.y += position.y - camera_position.y;
        m.z += position.z - camera_position.z;

        /* Application de la rotation de la caméra */
        ApplyRotationX(&m.y, &m.z, cos_camera.x, sin_camera.x);
        ApplyRotationY(&m.x, &m.z, cos_camera.y, sin_camera.y);
        ApplyRotationZ(&m.x, &m.y, cos_camera.z, sin_camera.z);

        /* Projection */
        const fixed_t m_z = (m.z < min_z) ? min_z : m.z;
        const fixed_t f = fdiv(INT_TO_FIXED(300), m_z);

        /* Calcul final de la position projetée */
        obj->mesh->vertices[i].projected.x = fmul(m.x, f) + half_screen_width;
        obj->mesh->vertices[i].projected.y = fmul(-m.y, f) + half_screen_height;
        obj->mesh->vertices[i].projected.z = m.z;
    }
}





void DrawPixel(int x, int y, int color)
{
    gint_vram[DWIDTH * y + x] = color;
}

fixed_t slope(fVector2 p1, fVector2 p2)
{
    if (p2.x == p1.x)
        return INT_TO_FIXED(0);
    return fdiv(p2.y - p1.y, p2.x - p1.x);
}

void DrawFilledQuadColor(fVector2 points[4], int color)
{
    fVector2 top_right = points[0];
    fVector2 top_left = points[1];
    fVector2 bottom_left = points[2];
    fVector2 bottom_right = points[3];

    if (top_left.y > bottom_left.y)
    {
        top_left = points[2];
        bottom_left = points[3];
        bottom_right = points[0];
        top_right = points[1];
    }

    const fixed_t slope_top = slope(top_left, top_right);
    const fixed_t slope_left = slope(top_left, bottom_left);
    const fixed_t slope_right = slope(top_right, bottom_right);
    const fixed_t slope_bottom = slope(bottom_left, bottom_right);

    const int y_start = max(min(TO_INT(top_left.y), TO_INT(top_right.y)), 0);
    const int y_end = min(max(TO_INT(bottom_left.y), TO_INT(bottom_right.y)), SCREEN_HEIGHT - 1);

    fixed_t x_start, x_end;
    for (int y = y_start; y <= y_end; y++)
    {
        fixed_t fy = INT_TO_FIXED(y);

        // Determine x_start based on the current Y position
        if (fy < top_left.y)
        {
            x_start = top_left.x;
            if (slope_top)
                x_start += fdiv(fy - top_left.y, slope_top);
        }
        else if (fy <= bottom_left.y)
        {
            x_start = top_left.x;
            if (slope_left)
                x_start += fdiv(fy - top_left.y, slope_left);
        }
        else
        {
            x_start = bottom_left.x;
            if (slope_bottom)
                x_start += fdiv(fy - bottom_left.y, slope_bottom);
        }

        // Determine x_end based on the current Y position
        if (fy < top_right.y)
        {
            x_end = top_right.x;
            if (slope_top)
                x_end += fdiv(fy - top_right.y, slope_top);
        }
        else if (fy <= bottom_right.y)
        {
            x_end = top_right.x;
            if (slope_right)
                x_end += fdiv(fy - top_right.y, slope_right);
        }
        else
        {
            x_end = bottom_right.x;
            if (slope_bottom)
                x_end += fdiv(fy - bottom_right.y, slope_bottom);
        }

        // Swap if necessary
        if (x_start > x_end)
            swap(&x_start, &x_end);

        // Draw the line using integer values
        int x_start_int = TO_INT(x_start);
        int x_end_int = TO_INT(x_end);
        if (y < 0 || y >= SCREEN_HEIGHT)
            continue;
        x_start_int = max(min(x_start_int, SCREEN_WIDTH - 1), 0);
        x_end_int = max(min(x_end_int, SCREEN_WIDTH - 1), 0);
        for (int x = x_start_int; x < x_end_int; x++)
        {
            DrawPixel(x, y, color);
        }
    }
}


void DrawFilledQuad2(fVector2 points[4], int color)
{
    fVector2 top_right = points[0];
    fVector2 top_left = points[1];
    fVector2 bottom_left = points[2];
    fVector2 bottom_right = points[3];

    if (top_left.y > bottom_left.y)
    {
        top_left = points[2];
        bottom_left = points[3];
        bottom_right = points[0];
        top_right = points[1];
    }

    const fixed_t slope_top = slope(top_left, top_right);
    const fixed_t slope_left = slope(top_left, bottom_left);
    const fixed_t slope_right = slope(top_right, bottom_right);
    const fixed_t slope_bottom = slope(bottom_left, bottom_right);

    //const int y_start = max(min(TO_INT(top_left.y), TO_INT(top_right.y)), 0);
    //const int y_end = min(max(TO_INT(bottom_left.y), TO_INT(bottom_right.y)), SCREEN_HEIGHT - 1);
    int y_start = min(TO_INT(top_left.y), TO_INT(top_right.y));
    int y_end = max(TO_INT(bottom_left.y), TO_INT(bottom_right.y));
    int y_start_org = y_start;
    int y_end_org = y_end;
    y_start = max(y_start, 0);
    y_end = min(y_end, SCREEN_HEIGHT - 1);

    // Pré-calcul des termes constants
    const fixed_t size = INT_TO_FIXED(40);

    fixed_t u, v;
    fixed_t du, dv;
    if (y_end_org != y_start_org)
        dv = fdiv(size, INT_TO_FIXED(y_end_org) - INT_TO_FIXED(y_start_org));
    else
        dv = 0;
    v = (y_start - y_start_org) * dv;
    for (int y = y_start; y <= y_end; y++)
    {
        fixed_t fy = INT_TO_FIXED(y);

        fixed_t x_start, x_end;
        if (fy < top_left.y)
        {
            x_start = top_left.x;
            if (slope_top)
                x_start += fdiv(fy - top_left.y, slope_top);
        }
        else if (fy <= bottom_left.y)
        {
            x_start = top_left.x;
            if (slope_left)
                x_start += fdiv(fy - top_left.y, slope_left);
        }
        else
        {
            x_start = bottom_left.x;
            if (slope_bottom)
                x_start += fdiv(fy - bottom_left.y, slope_bottom);
        }

        if (fy < top_right.y)
        {
            x_end = top_right.x;
            if (slope_top)
                x_end += fdiv(fy - top_right.y, slope_top);
        }
        else if (fy <= bottom_right.y)
        {
            x_end = top_right.x;
            if (slope_right)
                x_end += fdiv(fy - top_right.y, slope_right);
        }
        else
        {
            x_end = bottom_right.x;
            if (slope_bottom)
                x_end += fdiv(fy - bottom_right.y, slope_bottom);
        }

        if (x_start > x_end)
            swap(&x_start, &x_end);

        int x_start_int = TO_INT(x_start);
        int x_end_int = TO_INT(x_end);
        if (y < 0 || y >= SCREEN_HEIGHT)
            continue;
        const int x_start_int_org = x_start_int;
        x_start_int = max(min(x_start_int, SCREEN_WIDTH - 1), 0);
        x_end_int = max(min(x_end_int, SCREEN_WIDTH - 1), 0);

        
        // Précalculs pour l'interpolation des UV
        if (x_end != x_start)
        {
            du = fdiv(size, x_end - x_start);
            u = (x_start_int - x_start_int_org) * du;
            for (int x = x_start_int; x < x_end_int; x++)
            {
                int tex_color = get_uv_map((u*4 >> PRECISION) % 40, (v*4 >> PRECISION) % 40);
                DrawPixel(x, y, tex_color);
                u += du;
            }
        }
        v += dv;

    }
}


void DrawFilledQuad(fVector2 points[4], int color)
{
    fVector2 top_right = points[0];
    fVector2 top_left = points[1];
    fVector2 bottom_left = points[2];
    fVector2 bottom_right = points[3];

    if (top_left.y > bottom_left.y)
    {
        top_left = points[2];
        bottom_left = points[3];
        bottom_right = points[0];
        top_right = points[1];
    }

    const fixed_t slope_top = slope(top_left, top_right);
    const fixed_t slope_left = slope(top_left, bottom_left);
    const fixed_t slope_right = slope(top_right, bottom_right);
    const fixed_t slope_bottom = slope(bottom_left, bottom_right);

    const int y_start = max(min(TO_INT(top_left.y), TO_INT(top_right.y)), 0);
    const int y_end = min(max(TO_INT(bottom_left.y), TO_INT(bottom_right.y)), SCREEN_HEIGHT - 1);

    // Pré-calcul des termes constants
    const fixed_t size = INT_TO_FIXED(40);

    for (int y = y_start; y <= y_end; y++)
    {
        fixed_t fy = INT_TO_FIXED(y);

        fixed_t x_start, x_end;
        if (fy < top_left.y)
        {
            x_start = top_left.x;
            if (slope_top)
                x_start += fdiv(fy - top_left.y, slope_top);
        }
        else if (fy <= bottom_left.y)
        {
            x_start = top_left.x;
            if (slope_left)
                x_start += fdiv(fy - top_left.y, slope_left);
        }
        else
        {
            x_start = bottom_left.x;
            if (slope_bottom)
                x_start += fdiv(fy - bottom_left.y, slope_bottom);
        }

        if (fy < top_right.y)
        {
            x_end = top_right.x;
            if (slope_top)
                x_end += fdiv(fy - top_right.y, slope_top);
        }
        else if (fy <= bottom_right.y)
        {
            x_end = top_right.x;
            if (slope_right)
                x_end += fdiv(fy - top_right.y, slope_right);
        }
        else
        {
            x_end = bottom_right.x;
            if (slope_bottom)
                x_end += fdiv(fy - bottom_right.y, slope_bottom);
        }

        if (x_start > x_end)
            swap(&x_start, &x_end);

        int x_start_int = TO_INT(x_start);
        int x_end_int = TO_INT(x_end);
        if (y < 0 || y >= SCREEN_HEIGHT)
            continue;
        x_start_int = max(min(x_start_int, SCREEN_WIDTH - 1), 0);
        x_end_int = max(min(x_end_int, SCREEN_WIDTH - 1), 0);

        
        // Précalculs pour l'interpolation des UV
        if (x_end != x_start)
        {
            for (int x = x_start_int; x < x_end_int; x++)
            {
                int tex_color = get_uv_map((u*4 >> PRECISION) % 40, (v*4 >> PRECISION) % 40);
                DrawPixel(x, y, tex_color);
            }
        }

    }
}