#include <simulation.h>
#include <stdio.h>

Body new_body(unsigned int mass, unsigned int radius, unsigned int pos[2], unsigned int texture[2]) {
    Body out = { mass, radius, pos, texture };
    return out;
}

Simulation *init_sim(Body player, Body *bodies, size_t bodies_len) {
    Simulation *sim = malloc(sizeof(Simulation));
    sim->player = player;
    sim->velocity = calloc(2, sizeof(u32));
    sim->bodies = bodies;
    sim->bodies_len = bodies_len;
    return sim;
}

void add_bodies(Simulation *sim, Body *bodies, size_t len) {
    sim->bodies = realloc(sim->bodies, sim->bodies_len + len);
    for (size_t i = sim->bodies_len; i < len; ++i)
        sim->bodies[i] = bodies[i - sim->bodies_len];
    sim->bodies_len += len;
}

void add_body(Simulation *sim, Body body) {
    add_bodies(sim, &body, 1);
}

Body delete_body(Simulation *sim, size_t index) {
    Body temp;
    for (size_t i = 0; i < index; ++i) {
        temp = sim->bodies[i + 1];
        sim->bodies[i + 1] = sim->bodies[0];
        sim->bodies[0] = temp;
    }
    sim->bodies += 1;
    return temp;
}

void calculate_player(Simulation *sim, int interval) { // interval = milliseconds
    size_t i, j;
    unsigned int sides[2], distance, amount;
    int force[2];
    double before, after, acceleration[2]; // acceleration = pixels / second / second
    int angle;
    for (i = 0; i < sim->bodies_len; ++i) {
        for (j = 0; j < 2; ++j)
            sides[j] = abs(sim->bodies[i].pos[j] - sim->player.pos[j]);
        distance = hypot(sides[0], sides[1]);
        // A = acos(b/2c + c/2b - a^2/2bc)
        angle = (int) acos(
            sides[1] / (2 * distance) +
            distance / (2 * sides[1]) -
            (sides[0] * sides[0]) / (2 * sides[1] * distance)
        ) % 360;
        // F = (m[1]m[2]) / distance
        amount = (sim->bodies[i].mass * sim->player.mass) / distance;
        before = amount * ((angle % 90) / (f32) 90);
        after = amount - before;
        switch (angle / 90) {
            case NORTHEAST:
                force[0] = after;
                force[1] = -before;
                break;
            case SOUTHEAST:
                force[0] = before;
                force[1] = after;
                break;
            case SOUTHWEST:
                force[0] = -after;
                force[1] = before;
                break;
            case NORTHWEST:
                force[0] = -before;
                force[1] = -after;
                break;
        }
        // a = F / m
        for (j = 0; j < 2; ++j)
            acceleration[j] += force[j] / sim->player.mass;
    }
    int temp[2], distances[2];
    double change;
    sim->velocity[i] += (unsigned int) acceleration[i];
    for (j = 0; j < 2; ++j)
        temp[j] = sim->player.pos[j] + (unsigned int) (sim->velocity[j] * (interval / 1000.0));
    for (i = 0; i < sim->bodies_len; ++i) {
        for (j = 0; j < 2; ++j)
            distances[j] = (temp[j] + sim->player.radius) - (sim->bodies[i].pos[j] + sim->bodies[i].radius);
        distance = sqrt((distances[0] * distances[0]) + (distances[1] * distances[1]));
        if (distance < sim->player.radius + sim->bodies[i].radius) { // MY MATH MAY BE WRONG HERE!!!!
            change = (double) distance / (sim->player.radius + sim->bodies[i].radius);
            for (j = 0; j < 2; ++j)
                distances[j] = (int) (distances[j] * change);
        }
    }
    for (j = 0; j < 2; ++j) {
        if (sim->player.pos[j] + distances[j] < 0)
            sim->player.pos[j] = 0;
        else
            sim->player.pos[j] += distances[j];
    }
    // if ( < 0) // DO THIS
    printf("ACCEL: %d, %d\n", acceleration[0], acceleration[1]);
    printf("VELOCITY: %d, %d\n", sim->velocity[0], sim->velocity[1]);
    printf("CHANGES: %d, %d\n", distances[0], distances[1]);
}
