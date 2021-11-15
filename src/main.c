#include <engine.h>

int main() {
    Body bodies[] = {
        new_body(50, 50, calloc(2, sizeof(unsigned int)), calloc(2, sizeof(unsigned int))),
    };
    bodies[0].pos[0] = 50;
    bodies[0].pos[1] = 50;
    Body player = new_body(20, 150, calloc(2, sizeof(unsigned int)), calloc(2, sizeof(unsigned int)));
    player.pos[0] = 150;
    player.pos[1] = 150;
    Simulation *sim = init_sim(player, bodies, 1);
    Instance *instance = init_instance(sim, 5, 1280, 720, "test", 0, "image.bmp", NULL);
    return run_instance(instance);
}