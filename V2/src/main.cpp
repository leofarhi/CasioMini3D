#include "world.hpp"
#include "display.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
    srand(time(NULL));
    init_uv_map();
    //clock_set_speed(CLOCK_SPEED_F5);
    Camera *camera = new Camera(Vector3<int>(0, 100, -300), 0);
    World *world = new World(16, 16, 16, new Block*[2]{nullptr,new Cube()});
    world->set_id_unsafe(0, 0, 0, 1);
    world->set_id_unsafe(1, 0, 0, 1);
    for (size_t i = 0; i < 200; i++)
    {
        world->set_id_unsafe(rand() % 16, rand() % 16, rand() % 16, 1);
    }
    /*for (size_t i = 0; i < 16*16; i++)
    {
        world->set_id_unsafe(i/16, 0, i%16, 1);
    }*/
    while (!keydown(KEY_MENU))
    {
        dclear(C_BLACK);
        clearevents();

        if(keydown(KEY_LEFT))
            camera->position.x -= 10;
        if(keydown(KEY_RIGHT))
            camera->position.x += 10;
        if(keydown(KEY_UP))
            camera->position.z += 10;
        if(keydown(KEY_DOWN))
            camera->position.z -= 10;


        if(keydown(KEY_8))
            camera->position.y += 10;
        if(keydown(KEY_2))
            camera->position.y -= 10;

        if(keydown(KEY_7))
            camera->angle += 1;
        if(keydown(KEY_1))
            camera->angle -= 1;

        if(keydown(KEY_9))
            camera->offset.y -= 10;
        if(keydown(KEY_3))
            camera->offset.y += 10;

        world->Render();
        camera->Render();
        dupdate();
    }
    return 0;
}