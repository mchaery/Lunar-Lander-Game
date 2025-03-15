/**
* Author: Kristie Lee
* Assignment: Lunar Lander
* Date due: 2025-3-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

// --------------------- Constants --------------------- //

constexpr float WINDOW_SIZE_MULT = 2.0f;

constexpr int WINDOW_WIDTH = 640 * WINDOW_SIZE_MULT,
WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;

constexpr float BG_RED = 0.0f,
BG_GREEN = 0.0f,
BG_BLUE = 0.0f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char PLAYER_FILEPATH[] = "assets/santa.png";
constexpr char GOAL_FILEPATH[] = "assets/chimney.png";
constexpr char TRAP_FILEPATH[] = "assets/flame.png";
constexpr char WIN_MESSAGE_FILEPATH[] = "assets/complete.jpg";
constexpr char LOSE_MESSAGE_FILEPATH[] = "assets/failed.jpg";
constexpr char FUEL_100_FILEPATH[] = "assets/battery_100.png";
constexpr char FUEL_50_FILEPATH[] = "assets/battery_50.png";
constexpr char FUEL_10_FILEPATH[] = "assets/battery_10.png";
constexpr char FUEL_0_FILEPATH[] = "assets/battery_0.png";

GLuint g_fuel_100_texture, g_fuel_50_texture, g_fuel_10_texture, g_fuel_0_texture;



constexpr float ACC_OF_GRAVITY = -0.3f; // gravitational acceleration
int GOAL_PLATFORM_COUNT = 4;
int TRAP_PLATFORM_COUNT = 8;
int FUEL_REMAINING = 6000;
constexpr float FLOOR_Y = -3.4f;

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;

// ————---------------— STRUCTS AND ENUMS —--------------————//
enum AppStatus { RUNNING, TERMINATED };

struct GameState { 
    Entity* player; 
    Entity* goals;
    Entity* traps;
    Entity* messages;
    Entity* fuel;
};

// --------------------- Global Variables --------------------- //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;

bool g_player_win = false;
bool g_player_lose = false;

bool acc_available() {
    return !g_player_win && !g_player_lose &&
        (FUEL_REMAINING > 0);
}

void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
        SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("User-Input and Collisions Exercise",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr) shutdown();

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ————— PLAYER ————— //
    g_game_state.player = new Entity();
    g_game_state.player->set_position(glm::vec3(0.0f, 4.5f, 0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    g_game_state.player->set_speed(1.0f);
    g_game_state.player->m_texture_id = load_texture(PLAYER_FILEPATH);
    g_game_state.player->set_scale(glm::vec3(0.75f, 1.0f, 0.0f));
    g_game_state.player->set_entity_type(PLAYER);

    // ————— WIN PLATFORM (GOAL) ————— //
    float goal_x_pos[9] = { -4.0, -1.5f, 1.0f, 3.0f, -3.0f, -0.75f, 2.0f, 2.5f, 4.0f };
    
    g_game_state.goals = new Entity[GOAL_PLATFORM_COUNT];
    for (int i = 0; i < GOAL_PLATFORM_COUNT; i++)
    {
        g_game_state.goals[i].m_texture_id = load_texture(GOAL_FILEPATH);
        g_game_state.goals[i].set_position(glm::vec3(goal_x_pos[i], FLOOR_Y + (i % 2), 0.0f));
        g_game_state.goals[i].update(0.0f);
        g_game_state.goals[i].set_entity_type(GOAL_PLATFORM);
    }

    // ————— LOSE PLATFORM (TRAP) ————— //
    float trap_x_pos[9] = { -4.5f, -3.0f, -2.3f, -0.5f, 0.2f, 2.0f, 2.5f, 4.0f };

    g_game_state.traps = new Entity[TRAP_PLATFORM_COUNT];
    for (int i = 0; i < TRAP_PLATFORM_COUNT; i++)
    {
        g_game_state.traps[i].m_texture_id = load_texture(TRAP_FILEPATH);
        g_game_state.traps[i].set_position(glm::vec3(trap_x_pos[i], FLOOR_Y -0.2f + (i % 2), 0.0f));
        g_game_state.traps[i].set_scale(glm::vec3(0.85f, 0.85f, 0.0f));
        g_game_state.traps[i].update(0.0f);
        g_game_state.traps[i].set_entity_type(TRAP_PLATFORM);
    }

    // ––––– MESSAGES ––––– //
    g_game_state.messages = new Entity[2];
    g_game_state.messages[0].m_texture_id = load_texture(WIN_MESSAGE_FILEPATH);
    g_game_state.messages[1].m_texture_id = load_texture(LOSE_MESSAGE_FILEPATH);
    for (int i = 0; i < 2; i++){
        g_game_state.messages[i].set_position(glm::vec3(0.0f));
        g_game_state.messages[i].set_scale(glm::vec3(7.5f, 4.0f, 0.0f));
        g_game_state.messages[i].update(0.0f);
        g_game_state.messages[i].set_entity_type(MESSAGE);
        g_game_state.messages[i].deactivate();
    }

    // ––––– FUEL REMAINING ––––– //
    g_fuel_100_texture = load_texture(FUEL_100_FILEPATH);
    g_fuel_50_texture = load_texture(FUEL_50_FILEPATH);
    g_fuel_10_texture = load_texture(FUEL_10_FILEPATH);
    g_fuel_0_texture = load_texture(FUEL_0_FILEPATH);

    g_game_state.fuel = new Entity;
    g_game_state.fuel->m_texture_id = g_fuel_100_texture;
    g_game_state.fuel->set_position(glm::vec3(4.0f, 3.0f, 0.0f));
    g_game_state.fuel->update(0.0f);
    g_game_state.fuel->set_entity_type(FUEL);


    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    }

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q:
                g_app_status = TERMINATED;
                break;

            default: break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT] && acc_available())
    {
        g_game_state.player->move_left();
        FUEL_REMAINING--;
    }
    else if (key_state[SDL_SCANCODE_RIGHT] && acc_available())
    {
        g_game_state.player->move_right();
        FUEL_REMAINING--;
    }
    else {
        g_game_state.player->set_left_accel(false);
        g_game_state.player->set_right_accel(false);
    }

    //normalize
    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    {
        g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
    }
}

void update()
{
    // --- DELTA TIME CALCULATIONS --- //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // --- MOVING TRAPS --- //
    for (int i = 1; i < TRAP_PLATFORM_COUNT; i+=3) {
        g_game_state.traps[i].move_updown(FLOOR_Y + (i % 2) + 0.3f, FLOOR_Y + (i % 2) - 0.3f, delta_time);
    }

    // --- FUEL REMAINING ---
    if (FUEL_REMAINING < 4000) {
        if (FUEL_REMAINING > 2000) {
            g_game_state.fuel->m_texture_id = g_fuel_50_texture;
        }
        else if (FUEL_REMAINING > 0) {
            g_game_state.fuel->m_texture_id = g_fuel_10_texture;
        }
        else {
            g_game_state.fuel->m_texture_id = g_fuel_0_texture;
        }
    }

    // --- PLAYER  --- //
    g_game_state.player->update(delta_time);

    // --- COLLISION --- //
    for (int i = 0; i < TRAP_PLATFORM_COUNT; i++) {
        if (g_game_state.player->check_collision(&g_game_state.traps[i]) 
            || g_game_state.player->hit_floor(FLOOR_Y)) {
            g_game_state.player->update_collision(&g_game_state.traps[i]);
            g_player_lose = true;
        }
    }
    for (int i = 0; i < GOAL_PLATFORM_COUNT; i++) {
        if (g_game_state.player->check_collision(&g_game_state.goals[i])) {
            g_game_state.player->update_collision(&g_game_state.goals[i]);
            g_player_win = true;
        }
    }

    // --- MESSAGE ---
    if (g_player_win) g_game_state.messages[0].activate();
    if (g_player_lose) g_game_state.messages[1].activate();

}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // ————— PLAYER ————— //
    g_game_state.player->render(&g_shader_program);

    // ————— TRAP PLATFORM ————— //
    for (int i = 0; i < TRAP_PLATFORM_COUNT; i++) g_game_state.traps[i].render(&g_shader_program);

    // ————— GOAL PLATFORM ————— //
    for (int i = 0; i < GOAL_PLATFORM_COUNT; i++) g_game_state.goals[i].render(&g_shader_program);

    // ————— MESSAGE ————— //
    for (int i = 0; i < 2; i++) g_game_state.messages[i].render(&g_shader_program);

    // ————— FUEL REMAINING ————— //
    g_game_state.fuel->render(&g_shader_program);

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}

