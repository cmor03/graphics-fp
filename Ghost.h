#ifndef GHOST_H
#define GHOST_H

#include <glm/glm.hpp>
#include <CSCI441/ShaderProgram.hpp>
#include <vector>
#include "ParticleSystem.h"

class Ghost {
public:
    static constexpr float WORLD_SIZE = 60.0f;
    static constexpr float GHOST_AVOIDANCE_RADIUS = 3.0f;
    static constexpr float AVOIDANCE_WEIGHT = 1.5f;
    static constexpr float MERGE_DISTANCE = 1.0f;
    static constexpr float MAX_GHOST_SIZE = 4.0f;
    
    Ghost(const glm::vec3& startPos, GLuint shaderProgramHandle, 
          GLint mvpMatrixUniform, GLint normalMatrixUniform, GLint materialColorUniform,
          float size = 1.0f);
    
    void update(float deltaTime, const glm::vec3& targetPos);
    void draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
    glm::vec3 getPosition() const { return _position; }
    bool isAlive() const { return !_isFalling || _fallTime < MAX_FALL_TIME; }
    float getSize() const { return _size; }
    
    static void setMovementSpeed(float speed) { MOVEMENT_SPEED = speed; }
    static void setGhostSize(float size) { BASE_GHOST_SIZE = size; }
    static void setGhostColor(const glm::vec3& color) { GHOST_COLOR = color; }
    static void setFadeDistance(float dist) { FADE_DISTANCE = dist; }
    
    static Ghost merge(const Ghost& g1, const Ghost& g2, GLuint shaderProgramHandle, 
                      GLint mvpMatrixUniform, GLint normalMatrixUniform, GLint materialColorUniform);
    
private:
    glm::vec3 _position;
    glm::vec3 _currentDirection;
    float _verticalOffset;
    float _bobTimer;
    bool _isFalling;
    float _fallTime;
    float _size;
    
    GLuint _shaderProgramHandle;
    GLint _mvpMatrixUniform;
    GLint _normalMatrixUniform;
    GLint _materialColorUniform;
    
    inline static float MOVEMENT_SPEED = 3.0f;
    inline static float BASE_GHOST_SIZE = 1.0f;
    inline static glm::vec3 GHOST_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
    inline static float FADE_DISTANCE = 30.0f;
    
    static constexpr float BOB_SPEED = 2.0f;
    static constexpr float BOB_AMOUNT = 0.5f;
    static constexpr float COLLISION_RADIUS = 2.0f;
    static constexpr float MAX_FALL_TIME = 3.0f;
    static constexpr float FALL_SPEED = 15.0f;
    static constexpr float TURN_RATE = 2.0f;
};

class GhostManager {
public:
    GhostManager(GLuint shaderProgramHandle, GLint mvpMatrixUniform, 
                GLint normalMatrixUniform, GLint materialColorUniform);
    
    void update(float deltaTime, const glm::vec3& playerPos);
    void draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
    void reset();
    
    void setMaxGhosts(size_t max) { MAX_GHOSTS = max; }
    void setSpawnRadius(float radius) { SPAWN_RADIUS = radius; }
    void setSpawnInterval(float interval) { SPAWN_INTERVAL = interval; }
    void setDensity(float density) { GHOST_DENSITY = density; }
    
    static const std::vector<glm::vec3>& getAllGhostPositions() {
        return _allGhostPositions;
    }
    
    static bool hasCollided() { return _hasCollided; }
    static void setCollision(bool state) { _hasCollided = state; }
    
private:
    void spawnGhost(const glm::vec3& position);
    void checkGhostMerges();
    
    std::vector<Ghost> _ghosts;
    float _spawnTimer = 0.0f;
    
    GLuint _shaderProgramHandle;
    GLint _mvpMatrixUniform;
    GLint _normalMatrixUniform;
    GLint _materialColorUniform;
    
    size_t MAX_GHOSTS = 50;
    float SPAWN_RADIUS = 40.0f;
    float SPAWN_INTERVAL = 2.0f;
    float GHOST_DENSITY = 0.5f;
    
    static std::vector<glm::vec3> _allGhostPositions;
    static bool _hasCollided;
    
    float _timeSinceStart = 0.0f;
    float _initialSpawnInterval = 3.0f;
    float _initialDensity = 0.3f;
    size_t _initialMaxGhosts = 30;
    
    static constexpr float DIFFICULTY_SCALE_RATE = 0.1f;
    static constexpr float MIN_SPAWN_INTERVAL = 0.5f;
    static constexpr float MAX_DENSITY = 0.9f;
    static constexpr size_t ABSOLUTE_MAX_GHOSTS = 200;
};

#endif