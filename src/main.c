// src/main.c
#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include "cpu_state.h"
#include "tetris_game.h"
#include "ppu.h"
#include "apu.h"

#define NES_WIDTH 256
#define NES_HEIGHT 240
#define FRAME_TIME_NS 16639267 // ~60.0988 Hz in nanoseconds
#define ANALOG_DEADZONE 16000

uint8_t ram[0x800] = {0};
CPU6502 cpu = {0};

static SDL_Gamepad* active_gamepad = NULL;
static bool is_fullscreen = false;

static void toggle_fullscreen(SDL_Window* window) {
    is_fullscreen = !is_fullscreen;
    SDL_SetWindowFullscreen(window, is_fullscreen);
}

static void load_mappings(void) {
    // Try current working directory first, then parent directory
    int added = SDL_AddGamepadMappingsFromFile("gamecontrollerdb.txt");
    if (added < 0) {
        added = SDL_AddGamepadMappingsFromFile("../gamecontrollerdb.txt");
    }

    if (added > 0) {
        printf("Loaded %d gamepad mapping(s) from gamecontrollerdb.txt\n", added);
    } else {
        printf("Notice: gamecontrollerdb.txt not found or empty; using SDL3 defaults.\n");
    }
}

static void open_first_available_gamepad(void) {
    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);
    if (joysticks && count > 0) {
        active_gamepad = SDL_OpenGamepad(joysticks[0]);
        if (active_gamepad) {
            printf("Gamepad connected: %s\n", SDL_GetGamepadName(active_gamepad));
        }
        SDL_free(joysticks);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "Failed to initialize SDL3: %s\n", SDL_GetError());
        return 1;
    }

    // Load custom/community controller mappings
    load_mappings();

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (!SDL_CreateWindowAndRenderer(
            "Monktris (SDL3 Native)",
            NES_WIDTH * 3, NES_HEIGHT * 3,
            SDL_WINDOW_RESIZABLE,
            &window, &renderer)) {
        fprintf(stderr, "Failed to create window/renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderer, NES_WIDTH, NES_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_Texture* screen_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        NES_WIDTH, NES_HEIGHT
    );

    if (!screen_texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    open_first_available_gamepad();

    ppu_init();
    apu_init();
    tetris_init();

    bool running = true;
    SDL_Event event;
    uint64_t last_time = SDL_GetTicksNS();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_F11) {
                    toggle_fullscreen(window);
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                if (!active_gamepad) {
                    active_gamepad = SDL_OpenGamepad(event.gdevice.which);
                    if (active_gamepad) {
                        printf("Gamepad hotplugged: %s\n", SDL_GetGamepadName(active_gamepad));
                    }
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                if (active_gamepad && SDL_GetGamepadID(active_gamepad) == event.gdevice.which) {
                    printf("Gamepad disconnected.\n");
                    SDL_CloseGamepad(active_gamepad);
                    active_gamepad = NULL;
                    // Attempt fallback to another plugged pad if one exists
                    open_first_available_gamepad();
                }
            }
        }

        const bool* keystate = SDL_GetKeyboardState(NULL);
        uint8_t prev_buttons = buttonsDown;
        uint8_t cur_buttons = 0;

        // Keyboard bindings
        if (keystate[SDL_SCANCODE_RIGHT])  cur_buttons |= BTN_RIGHT;
        if (keystate[SDL_SCANCODE_LEFT])   cur_buttons |= BTN_LEFT;
        if (keystate[SDL_SCANCODE_DOWN])   cur_buttons |= BTN_DOWN;
        if (keystate[SDL_SCANCODE_UP])     cur_buttons |= BTN_UP;
        if (keystate[SDL_SCANCODE_RETURN]) cur_buttons |= BTN_START;
        if (keystate[SDL_SCANCODE_RSHIFT]) cur_buttons |= BTN_SELECT;
        if (keystate[SDL_SCANCODE_Z])      cur_buttons |= BTN_B;
        if (keystate[SDL_SCANCODE_X])      cur_buttons |= BTN_A;

        // Gamepad bindings
        if (active_gamepad) {
            // Digital D-Pad
            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) cur_buttons |= BTN_RIGHT;
            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))  cur_buttons |= BTN_LEFT;
            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))  cur_buttons |= BTN_DOWN;
            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP))    cur_buttons |= BTN_UP;

            // Analog Left Stick fallback with deadzone threshold
            int16_t axis_x = SDL_GetGamepadAxis(active_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
            int16_t axis_y = SDL_GetGamepadAxis(active_gamepad, SDL_GAMEPAD_AXIS_LEFTY);

            if (axis_x > ANALOG_DEADZONE)   cur_buttons |= BTN_RIGHT;
            if (axis_x < -ANALOG_DEADZONE)  cur_buttons |= BTN_LEFT;
            if (axis_y > ANALOG_DEADZONE)   cur_buttons |= BTN_DOWN;
            if (axis_y < -ANALOG_DEADZONE)  cur_buttons |= BTN_UP;

            // Action / System buttons
            // South (A/Cross) & East (B/Circle) mapped to NES A
            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_SOUTH) ||
                SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_EAST)) {
                cur_buttons |= BTN_A;
            }

            // West (X/Square) mapped to NES B
            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_WEST)) {
                cur_buttons |= BTN_B;
            }

            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_START)) cur_buttons |= BTN_START;
            if (SDL_GetGamepadButton(active_gamepad, SDL_GAMEPAD_BUTTON_BACK))  cur_buttons |= BTN_SELECT;
        }

        buttonsDown = cur_buttons;
        buttonsPressed = cur_buttons & ~prev_buttons;

        tetris_step_frame();
        apu_tick_frame();

        uint32_t* pixels = NULL;
        int pitch = 0;
        if (SDL_LockTexture(screen_texture, NULL, (void**)&pixels, &pitch)) {
            ppu_render_screen(pixels);
            SDL_UnlockTexture(screen_texture);
        }

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        uint64_t now = SDL_GetTicksNS();
        uint64_t elapsed = now - last_time;
        if (elapsed < FRAME_TIME_NS) {
            SDL_DelayNS(FRAME_TIME_NS - elapsed);
        }
        last_time = SDL_GetTicksNS();
    }

    if (active_gamepad) {
        SDL_CloseGamepad(active_gamepad);
    }

    apu_shutdown();
    SDL_DestroyTexture(screen_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}