#include <engine.h>

static int on_quit(Instance *instance, SDL_Event *event) { return -1; }
static int on_right_click(Instance *instance, SDL_Event *event) { return 0; }

int get_callbacks(Instance *instance) { // puts callbacks into instance; returns len
    instance->callbacks = calloc(2, sizeof(Callback));
    instance->callbacks[0].event_type = SDL_QUIT;
    instance->callbacks[0].callback = &on_quit;
    instance->callbacks[1].event_type = SDL_MOUSEBUTTONDOWN;
    instance->callbacks[1].callback = &on_right_click;
    return 2;
}

Instance *init_instance(Simulation *sim, int framerate, int w, int h, char *name, u32 flags, char *spritesheet, char *background) {
    Instance *instance = malloc(sizeof(Instance));
    instance->framerate = framerate;
    instance->w = w;
    instance->h = h;
    instance->name = name;
    instance->callbacks_len = get_callbacks(instance);
    instance->simulation = sim;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return NULL;
    }
    instance->window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    if (!instance->window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        destroy_instance(instance);
        return NULL;
    }
    instance->renderer = SDL_CreateRenderer(instance->window, -1, SDL_RENDERER_ACCELERATED);
    if (!instance->renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        destroy_instance(instance);
        return NULL;
    }
    instance->background = SDL_CreateTexture(instance->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h - h / 8);
    if (background) {
        SDL_Surface *bg_surface = SDL_LoadBMP(background);
        if (!bg_surface) {
            fprintf(stderr, "Failed to get background: %s\n", SDL_GetError());
            destroy_instance(instance);
            return NULL;
        }
        SDL_Texture *bg_texture = SDL_CreateTextureFromSurface(instance->renderer, bg_surface);
        SDL_FreeSurface(bg_surface);
        SDL_SetRenderTarget(instance->renderer, instance->background);
        SDL_RenderCopy(instance->renderer, bg_texture, NULL, NULL);
    }
    SDL_SetRenderTarget(instance->renderer, NULL);
    SDL_Surface *spritesheet_surface = SDL_LoadBMP(spritesheet);
    if (!spritesheet_surface) {
        fprintf(stderr, "Failed to get spritesheet: %s\n", SDL_GetError());
        destroy_instance(instance);
        return NULL;
    }
    instance->spritesheet = SDL_CreateTextureFromSurface(instance->renderer, spritesheet_surface);
    SDL_FreeSurface(spritesheet_surface);
    return instance;
}

int run_instance(Instance *instance) {
    SDL_SetRenderTarget(instance->renderer, NULL);
    SDL_Event event;
    instance->running = true;
    int errno, delay = 1000 / instance->framerate;
    while (instance->running) {
        SDL_PollEvent(&event);
        for (size_t i = 0; i < instance->callbacks_len; ++i) {
            if (instance->callbacks[i].event_type == event.type) {
                errno = instance->callbacks[i].callback((struct Instance *) instance, &event);
                if (errno != 0)
                    return exit_instance(instance, event.type + "0", SDL_GetError(), errno);
            }
        }
        calculate_player(instance->simulation, delay);
        render_instance(instance);
        SDL_Delay(delay);
    }
    return exit_instance(instance, NULL, NULL, 0);
}

void render_instance(Instance *instance) {
    SDL_RenderClear(instance->renderer);
    SDL_RenderCopy(instance->renderer, instance->background, NULL, NULL);
    Body *body = instance->simulation->bodies;
    for (size_t i = 0; i < instance->simulation->bodies_len; ++i)
        render_body(instance, body++);
    render_body(instance, &instance->simulation->player);
    SDL_RenderPresent(instance->renderer);
}

void render_body(Instance *instance, Body *body) {
    SDL_Rect
        src_rect = { body->texture[0], body->texture[1], body->radius, body->radius },
        dst_rect = { body->pos[0], body->pos[1], body->radius, body->radius };
    SDL_RenderCopy(instance->renderer, instance->spritesheet, &src_rect, &dst_rect);
}

int exit_instance(Instance *instance, char *type, char *error, int errno) { // if errno == 0, func returns success. otherwise, attempts to print error message and returns errno
    destroy_instance(instance);
    if (!errno) {
        fprintf(stderr, "Failed while parsing input event '%s': %s\n", type, error);
        return errno;
    }
    return 0;
}

int destroy_instance(Instance *instance) {
    instance->running = false;
    SDL_DestroyWindow(instance->window);
    SDL_DestroyRenderer(instance->renderer);
    SDL_DestroyTexture(instance->background);
    SDL_Quit();
}
