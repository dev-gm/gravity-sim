#include <number/number.h>
#include <math.h>
#include <stdlib.h>

typedef enum {
    NORTHEAST,
    SOUTHEAST,
    NORTHWEST,
    SOUTHWEST
} Direction;

typedef struct {
    u32 mass;
    u32 radius;
    u32 *pos; // [x, y]
    u32 *texture; // [x, y] of top left
} Body;

Body new_body(unsigned int mass, unsigned int radius, unsigned int pos[2], unsigned int texture[2]);

typedef struct {
    Body *bodies;
    size_t bodies_len;
} Map;

typedef struct {
    Body player;
    u32 *velocity; // pixels / second
    Map *maps;
    size_t map_len;
} Simulation;

Simulation *init_sim(Body player, Body *bodies, size_t bodies_len);

void add_bodies(Simulation *sim, Body *bodies, size_t len);

void add_body(Simulation *sim, Body body);

Body delete_body(Simulation *sim, size_t index);

void calculate_player(Simulation *sim, int interval);
