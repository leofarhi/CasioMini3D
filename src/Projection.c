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
static inline fixed_t fsin_approx(fixed_t i)
{
    const fixed_t B = fix(4 / M_PI);
    const fixed_t C = fix(-4 / (M_PI * M_PI));

    return fmul(B, i) + fmul(C, fmul(i, abs(i)));
}

/* Approximation de cos en virgule fixe */
static inline fixed_t fcos_approx(fixed_t i)
{
    return fsin_approx(i + fix(M_PI_2));
}

/*inline static void ApplyRotationX(float *y, float *z, float cos_theta, float sin_theta)
{
    const float new_y = *y * cos_theta - *z * sin_theta;
    const float new_z = *y * sin_theta + *z * cos_theta;
    *y = new_y;
    *z = new_z;
}

inline static void ApplyRotationY(float *x, float *z, float cos_theta, float sin_theta)
{
    const float new_x = *x * cos_theta + *z * sin_theta;
    const float new_z = -*x * sin_theta + *z * cos_theta;
    *x = new_x;
    *z = new_z;
}

inline static void ApplyRotationZ(float *x, float *y, float cos_theta, float sin_theta)
{
    const float new_x = *x * cos_theta - *y * sin_theta;
    const float new_y = *x * sin_theta + *y * cos_theta;
    *x = new_x;
    *y = new_y;
}

void CalculateProjection(Camera *camera, Projection* obj)
{
    const Vector2 center = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    const Transform *_transform = &obj->transform;
    const Transform* cameraTransform = &camera->transform;


    // Precompute cos and sin for object rotation
    / *const Vector3f cos_transform = (Vector3f){cos_approx(_transform->rotation.x), cos_approx(_transform->rotation.y), cos_approx(_transform->rotation.z)};
    const Vector3f sin_transform = (Vector3f){sin_approx(_transform->rotation.x), sin_approx(_transform->rotation.y), sin_approx(_transform->rotation.z)};* /
    const Vector3f cos_transform = (Vector3f){1,1,1};
    const Vector3f sin_transform = (Vector3f){0,0,0};
    (void)cos_transform;
    (void)sin_transform;

    // Precompute cos and sin for camera rotation
    const Vector3f cos_camera = (Vector3f){cos_approx(cameraTransform->rotation.x), cos_approx(cameraTransform->rotation.y), cos_approx(cameraTransform->rotation.z)};
    const Vector3f sin_camera = (Vector3f){sin_approx(cameraTransform->rotation.x), sin_approx(cameraTransform->rotation.y), sin_approx(cameraTransform->rotation.z)};
    

    const float min_z = 1;  // Define a minimum z value to avoid division by zero or too close projections

    for (size_t i = 0; i < obj->mesh->verticesCount; i++)
    {
        //Vector3f m = obj->mesh->vertices[i].position * _transform->scale;
        Vector3f m = (Vector3f){obj->mesh->vertices[i].position.x * _transform->scale.x,
                    obj->mesh->vertices[i].position.y * _transform->scale.y,
                    obj->mesh->vertices[i].position.z * _transform->scale.z};


        // Apply local rotation
        / *ApplyRotationX(&m.y, &m.z, cos_transform.x, sin_transform.x);
        ApplyRotationY(&m.x, &m.z, cos_transform.y, sin_transform.y);
        ApplyRotationZ(&m.x, &m.y, cos_transform.z, sin_transform.z);* /

        // Apply translation (including inverse camera translation)
        m.x += _transform->position.x - cameraTransform->position.x;
        m.y += _transform->position.y - cameraTransform->position.y;
        m.z += _transform->position.z - cameraTransform->position.z;

        // Apply camera rotation
        ApplyRotationX(&m.y, &m.z, cos_camera.x, sin_camera.x);
        ApplyRotationY(&m.x, &m.z, cos_camera.y, sin_camera.y);
        ApplyRotationZ(&m.x, &m.y, cos_camera.z, sin_camera.z);

        // Projection
        float m_z = (m.z < min_z) ? min_z : m.z;  // Ensure m.z is not less than the minimum z value
        float f = 300 / (float)m_z;

        m.x = (m.x * f) + center.x;
        m.y = (-m.y * f) + center.y;

        obj->mesh->vertices[i].projected = (Vector3){m.x, m.y, m.z};
    }
}*/





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
    const Vector2 center = {INT_TO_FIXED(SCREEN_WIDTH / 2), INT_TO_FIXED(SCREEN_HEIGHT / 2)};
    const Transform *_transform = &obj->transform;
    const Transform* cameraTransform = &camera->transform;

    /* Pré-calcul des cos et sin pour la rotation de la caméra */
    const fVector3 cos_camera = {
        fcos_approx(FLOAT_TO_FIXED(cameraTransform->rotation.x)),
        fcos_approx(FLOAT_TO_FIXED(cameraTransform->rotation.y)),
        fcos_approx(FLOAT_TO_FIXED(cameraTransform->rotation.z))
    };
    const fVector3 sin_camera = {
        fsin_approx(FLOAT_TO_FIXED(cameraTransform->rotation.x)),
        fsin_approx(FLOAT_TO_FIXED(cameraTransform->rotation.y)),
        fsin_approx(FLOAT_TO_FIXED(cameraTransform->rotation.z))
    };

    const fVector3 scale = {
        FLOAT_TO_FIXED(_transform->scale.x),
        FLOAT_TO_FIXED(_transform->scale.y),
        FLOAT_TO_FIXED(_transform->scale.z)
    };

    const fVector3 position = {
        INT_TO_FIXED(_transform->position.x),
        INT_TO_FIXED(_transform->position.y),
        INT_TO_FIXED(_transform->position.z)
    };

    const fVector3 camera_position = {
        INT_TO_FIXED(cameraTransform->position.x),
        INT_TO_FIXED(cameraTransform->position.y),
        INT_TO_FIXED(cameraTransform->position.z)
    };

    const fixed_t min_z = INT_TO_FIXED(1);

    for (size_t i = 0; i < obj->mesh->verticesCount; i++)
    {
        /* Application de l'échelle */
        fVector3 m = {
            fmul(INT_TO_FIXED(obj->mesh->vertices[i].position.x), scale.x),
            fmul(INT_TO_FIXED(obj->mesh->vertices[i].position.y), scale.y),
            fmul(INT_TO_FIXED(obj->mesh->vertices[i].position.z), scale.z)
        };

        /* Application de la translation (incluant l'inverse de la translation de la caméra) */
        m.x += position.x - camera_position.x;
        m.y += position.y - camera_position.y;
        m.z += position.z - camera_position.z;

        /* Application de la rotation de la caméra */
        ApplyRotationX(&m.y, &m.z, cos_camera.x, sin_camera.x);
        ApplyRotationY(&m.x, &m.z, cos_camera.y, sin_camera.y);
        ApplyRotationZ(&m.x, &m.y, cos_camera.z, sin_camera.z);

        /* Projection */
        fixed_t m_z = (m.z < min_z) ? min_z : m.z;
        fixed_t f = fdiv(INT_TO_FIXED(300), m_z);

        m.x = fmul(m.x, f) + center.x;
        m.y = fmul(-m.y, f) + center.y;

        obj->mesh->vertices[i].projected = (Vector3){TO_INT(m.x), TO_INT(m.y), TO_INT(m.z)};
    }
}




void DrawPixel(int x, int y, int color)
{
    gint_vram[DWIDTH * y + x] = color;
}

float slope(Vector2 p1, Vector2 p2)
{
    if (p2.x == p1.x)
        return 0;
    return (p2.y - p1.y) / (float)(p2.x - p1.x);
}

void DrawFilledQuad(Vector2 points[4], int color)
{
    Vector2 top_right = points[0];
    Vector2 top_left = points[1];
    Vector2 bottom_left = points[2];
    Vector2 bottom_right = points[3];
    if (top_left.y > bottom_left.y)
    {
        top_left = points[2];
        bottom_left = points[3];
        bottom_right = points[0];
        top_right = points[1];
    }
    float slope_top = slope(top_left, top_right);
    float slope_left = slope(top_left, bottom_left);
    float slope_right = slope(top_right, bottom_right);
    float slope_bottom = slope(bottom_left, bottom_right);

    int y_start = max(min(top_left.y, top_right.y), 0);
    int y_end = min(max(bottom_left.y, bottom_right.y), SCREEN_HEIGHT - 1);

    int x_start, x_end;
    for (int y = y_start; y <= y_end; y++)
    {
        if (y < top_left.y)
        {
            x_start = top_left.x;
            if (slope_top)
                x_start += (y - top_left.y) / slope_top;
        }
        else if (y <= bottom_left.y)
        {
            x_start = top_left.x;
            if (slope_left)
                x_start += (y - top_left.y) / slope_left;
        }
        else
        {
            x_start = bottom_left.x;
            if (slope_bottom)
                x_start += (y - bottom_left.y) / slope_bottom;
        }

        if (y < top_right.y)
        {
            x_end = top_right.x;
            if (slope_top)
                x_end += (y - top_right.y) / slope_top;
        }
        else if (y <= bottom_right.y)
        {
            x_end = top_right.x;
            if (slope_right)
                x_end += (y - top_right.y) / slope_right;
        }
        else
        {
            x_end = bottom_right.x;
            if (slope_bottom)
                x_end += (y - bottom_right.y) / slope_bottom;
        }
        if (x_start > x_end)
        {
            int temp = x_start;
            x_start = x_end;
            x_end = temp;
        }
        //dline((int)x_start, y, (int)x_end, y, color);

        x_start = max(min(x_start, SCREEN_WIDTH - 1), 0);
        x_end = max(min(x_end, SCREEN_WIDTH - 1), 0);
        int dx = (x_end - x_start);
        int dy = (y_end - y_start);
        (void)dx;
        (void)dy;
        (void)color;
        for (int x = x_start; x < x_end; x++)
        {
            /*long long int u = 0;
            long long int v = 0;

            if (dx != 0)
                u = ((x - x_start) * FIXED_SCALE) / dx;
            if (dy != 0)
                v = ((y - y_start) * FIXED_SCALE) / dy;

            DrawPixel(x, y, get_uv_map((u*40 >> FIXED_SHIFT) % 40, (v*40 >> FIXED_SHIFT) % 40));*/

            /*int r = min(max((u * 255) >> FIXED_SHIFT, 0), 255);
            int g = min(max((v * 255) >> FIXED_SHIFT, 0), 255);
            int b = 0;
            DrawPixel(x, y, rgb(r, g, b));*/
            
            
            DrawPixel(x, y, color);
            
        }
    }
}