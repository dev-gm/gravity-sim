#include <engine.h>

static int each_turn(Instance *instance) {
    // calculate_player(instance->simulation, instance->delay);
    render_instance(instance);
}

static int on_quit(Instance *instance, SDL_Event *event) { return 1; }

static int on_up(Instance *instance, SDL_Event *event) {
    instance->simulation->player.pos[1] -= 100;
    return 0;
}
static int on_left(Instance *instance, SDL_Event *event) {
    instance->simulation->player.pos[0] -= 100;
    return 0;
}
static int on_down(Instance *instance, SDL_Event *event) {
    instance->simulation->player.pos[1] += 100;
    return 0;
}
static int on_right(Instance *instance, SDL_Event *event) {
    instance->simulation->player.pos[0] += 100;
    return 0;
}

int get_callbacks(Instance *instance) { // puts callbacks into instance; returns len
    instance->callbacks = calloc(5, sizeof(Callback));
    // QUIT
    instance->callbacks[0].event_type = SDL_QUIT;
    instance->callbacks[0].callback = &on_quit;
    // UP
    instance->callbacks[1].event_type = SDL_KEYDOWN;
    instance->callbacks[1].event_info.key = SDLK_UP;
    instance->callbacks[1].callback = &on_up;
    // LEFT
    instance->callbacks[2].event_type = SDL_KEYDOWN;
    instance->callbacks[2].event_info.key = SDLK_LEFT;
    instance->callbacks[2].callback = &on_left;
    // DOWN
    instance->callbacks[3].event_type = SDL_KEYDOWN;
    instance->callbacks[3].event_info.key = SDLK_DOWN;
    instance->callbacks[3].callback = &on_down;
    // RIGHT
    instance->callbacks[4].event_type = SDL_KEYDOWN;
    instance->callbacks[4].event_info.key = SDLK_RIGHT;
    instance->callbacks[4].callback = &on_right;
    return 5;
}

Instance *init_instance(Simulation *sim, int delay, int w, int h, char *name, u32 flags, char *spritesheet, char *background) {
    Instance *instance = malloc(sizeof(Instance));
    instance->delay = delay;
    instance->w = w;
    instance->h = h;
    instance->name = name;
    instance->each_turn = &each_turn;
    instance->callbacks_len = get_callbacks(instance);
    instance->simulation = sim;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return NULL;
    }
    instance->window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    if (!instance->window) {
        exit_instance(instance, "create window", 1);
        return NULL;
    }
    instance->renderer = SDL_CreateRenderer(instance->window, -1, SDL_RENDERER_ACCELERATED);
    if (!instance->renderer) {
        exit_instance(instance, "create renderer", 1);
        return NULL;
    }
    instance->background = SDL_CreateTexture(instance->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h - h / 8);
    if (background) {
        SDL_Surface *bg_surface = SDL_LoadBMP(background);
        if (!bg_surface) {
            exit_instance(instance, "get background", 1);
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
        exit_instance(instance, "get spritesheet", 1);
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
    int errno;
    while (instance->running) {
        while (SDL_PollEvent(&event)) {
            for (size_t i = 0; i < instance->callbacks_len; ++i) {
                if (instance->callbacks[i].event_type == event.type &&
                    ((event.type == SDL_KEYDOWN && instance->callbacks[i].event_info.key == event.key.keysym.sym) ||
                    (event.type == SDL_MOUSEBUTTONDOWN && instance->callbacks[i].event_info.button == event.button.button) ||
                    (event.type == SDL_QUIT))
                ) {
                    if (errno = instance->callbacks[i].callback((struct Instance *) instance, &event) != 0) {
                        char error[24];
                        snprintf(error, 24, "parse input event '%d'", event.type);
                        return exit_instance(instance, error, errno);
                    }
                }
            }
        }
        if (errno = instance->each_turn(instance) != 0)
            return exit_instance(instance, "complete 'each_turn' func", errno);
        SDL_Delay(instance->delay);
    }
    return exit_instance(instance, NULL, 0);
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

int exit_instance(Instance *instance, const char *type, int errno) { // if errno == 0, func returns success. otherwise, attempts to print error message and returns errno
    if (errno != 0) {
        fprintf(stderr, "Failed to %s: %s\n", type, SDL_GetError());
    }
    destroy_instance(instance);
    return errno;
}

int destroy_instance(Instance *instance) {
    instance->running = false;
    SDL_DestroyWindow(instance->window);
    SDL_DestroyRenderer(instance->renderer);
    SDL_DestroyTexture(instance->background);
    SDL_Quit();
}
