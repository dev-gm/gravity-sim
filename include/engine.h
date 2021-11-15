#include <SDL2/SDL.h>
#include <simulation.h>
#include <number/number.h>

typedef struct {
    int event_type;
    int (*callback)(struct Instance *instance, SDL_Event *event);
} Callback;

typedef struct {
    char *label;
    // Body *body;
    Callback *changes; // changes/additions to callbacks. if returns nonzero value then resets callback to default value, or removes it if not present in default callback array.
    size_t changes_len;
} Option;

typedef struct Instance {
    // char ***hints; // [hints_len, 2, string]
    // size_t hints_len;
    int framerate;
    int w, h;
    char *name;
    u32 flags;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    SDL_Texture *spritesheet;
    Callback *default_callbacks;
    Callback *callbacks; // [callbacks_len] - funcs to callback to on event completion; returns nonzero value to end program
    size_t callbacks_len;
    Simulation *simulation;
    Option *options; // options for planets
    size_t options_len;
    bool running: 1;
} Instance;

Instance *init_instance(Simulation *sim, int framerate, int w, int h, char *name, u32 flags, char *spritesheet, char *background);

int run_instance(Instance *instance);

void render_instance(Instance *instance);

void render_body(Instance *instance, Body *body);

int exit_instance(Instance *instance, char *type, char *error, int errno);

int destroy_instance(Instance *instance);
