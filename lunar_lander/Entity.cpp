#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include <vector>

Entity::Entity()
{
    // ––––– PHYSICS ––––– //
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);

    // ––––– TRANSLATION ––––– //
    m_movement = glm::vec3(0.0f);
    m_speed = 0;
    m_model_matrix = glm::mat4(1.0f);
}


void Entity::move_updown(float upper_limit, float lower_limit, float delta_time) {
    float move_speed = 1.0f;

    m_velocity.y = m_moving_up ? move_speed : -move_speed;

    m_position.y += m_velocity.y * delta_time;

    if (m_position.y >= upper_limit) {
        m_position.y = upper_limit;
        m_moving_up = false;
    }
    else if (m_position.y <= lower_limit) {
        m_position.y = lower_limit;
        m_moving_up = true;
    }

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, init_scale);
}

void Entity::update(float delta_time)
{
    if (!m_is_active) return;

    // ––––– GRAVITY ––––– //
    m_velocity += m_acceleration * delta_time;
    m_position.y += m_velocity.y * delta_time;
    m_position.x += m_velocity.x * delta_time;

    if (!m_left_accel && !m_right_accel) { // friction handling
        if (m_velocity.x > 0) {
            m_acceleration.x = -0.2;
        }
        else if (m_velocity.x < 0) {
            m_acceleration.x = 0.2;
        }
        else {
            m_acceleration.x = 0;
        }
    }

    // ––––– TRANSFORMATIONS ––––– //
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, init_scale);
}

bool const Entity::check_collision(Entity* other) const
{
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::update_collision(Entity* collidable_entity)
{
    // Calculate the distance between its centre and our centre
    // and use that to calculate the amount of overlap between
    // both bodies.
    float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
    float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));

    // "Unclip" ourselves from the other entity, and zero our
    // vertical velocity.
    if (m_velocity.y > 0) {
        m_position.y -= y_overlap;
        m_velocity.y = 0;
    }
    else if (m_velocity.y < 0) {
        m_position.y += y_overlap;
        m_velocity.y = 0;
    }
    m_velocity.x = 0;
    m_velocity.x = 0;
}

void Entity::render(ShaderProgram* program)
{
    if (!m_is_active) return;

    program->set_model_matrix(m_model_matrix);

    
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}