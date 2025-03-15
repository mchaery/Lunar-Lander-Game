enum EntityType { GOAL_PLATFORM, TRAP_PLATFORM, PLAYER, MESSAGE, FUEL };

class Entity
{
private:
    bool m_is_active = true;

    bool m_left_accel = false;
    bool m_right_accel = false;
    bool m_moving_up = true;

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    // ————— TRANSFORMATIONS ————— //
    float     m_speed;
    glm::vec3 m_movement;
    glm::vec3 init_scale = glm::vec3(1.0f, 1.0f, 0.0f);
    glm::mat4 m_model_matrix;
    EntityType m_entity_type;

    float m_width = init_scale.x;
    float m_height = init_scale.y;


public:
    GLuint    m_texture_id;

    // ————— METHODS ————— //
    Entity();

    void update(float delta_time);
    void render(ShaderProgram* program);

    bool const check_collision(Entity* other) const;
    void const update_collision(Entity* collidable_entitiy);

    void move_left() {
        m_acceleration.x = -0.3f;
        m_left_accel = true;
    };
    void move_right() {
        m_acceleration.x = 0.3f;
        m_right_accel = true;
    };
    void move_updown(float upper_limit, float lower_limit, float delta_time);


    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type; };
    glm::vec3 const get_position()     const { return m_position; };
    glm::vec3 const get_velocity()     const { return m_velocity; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    glm::vec3 const get_movement()     const { return m_movement; };
    float     const get_speed()        const { return m_speed; };
    int       const get_width()        const { return m_width; };
    int       const get_height()       const { return m_height; };
    bool      const hit_floor(float floor_y) const { return m_position.y < floor_y; }

    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type)  { m_entity_type = new_entity_type; };
    void const set_position(glm::vec3 new_position)         { m_position = new_position; };
    void const set_velocity(glm::vec3 new_velocity)         { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_position)     { m_acceleration = new_position; };
    void const set_movement(glm::vec3 new_movement)         { m_movement = new_movement; };
    void const set_speed(float new_speed)                   { m_speed = new_speed; };
    void const set_width(float new_width)                   { m_width = new_width; };
    void const set_height(float new_height)                 { m_height = new_height; };
    void const set_left_accel(bool val)                     { m_left_accel = val; };
    void const set_right_accel(bool val)                    { m_right_accel = val; };
    void const set_scale(glm::vec3 scale)                   { 
        init_scale = scale; 
        m_width = scale.x;
        m_height = scale.y;
    };
};