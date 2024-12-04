#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glm/glm.hpp>
#include <vector>
#include <CSCI441/ShaderProgram.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    float size;
    glm::vec3 color;
};

class ParticleSystem {
public:
    ParticleSystem(GLuint shaderProgramHandle, GLint mvpMatrixUniform, GLint colorUniform);
    
    void spawn(const glm::vec3& position);
    void update(float deltaTime);
    void draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
    bool isAlive() const { return _timeSinceSpawn < _totalLifetime; }
    
private:
    std::vector<Particle> _particles;
    float _timeSinceSpawn;
    const float _totalLifetime = 5.0f;
    
    GLuint _shaderProgramHandle;
    GLint _mvpMatrixUniform;
    GLint _colorUniform;
    
    static constexpr int NUM_PARTICLES = 100;
    static constexpr float PARTICLE_SPEED = 5.0f;
    static constexpr float PARTICLE_SIZE = 0.2f;
    static constexpr float GRAVITY = -9.81f;
};

#endif 