#include "ParticleSystem.h"
#include <CSCI441/objects.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>

ParticleSystem::ParticleSystem(GLuint shaderProgramHandle, GLint mvpMatrixUniform, GLint colorUniform)
    : _timeSinceSpawn(0.0f),
      _shaderProgramHandle(shaderProgramHandle),
      _mvpMatrixUniform(mvpMatrixUniform),
      _colorUniform(colorUniform) {}

void ParticleSystem::spawn(const glm::vec3& position) {
    _particles.clear();
    _timeSinceSpawn = 0.0f;
    
    for(int i = 0; i < NUM_PARTICLES; i++) {
        float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float phi = static_cast<float>(rand()) / RAND_MAX * M_PI;
        float speed = PARTICLE_SPEED * (0.5f + static_cast<float>(rand()) / RAND_MAX);
        
        glm::vec3 direction(
            sin(phi) * cos(theta),
            cos(phi),
            sin(phi) * sin(theta)
        );
        
        Particle p;
        p.position = position;
        p.velocity = direction * speed;
        p.life = 1.0f;
        p.size = PARTICLE_SIZE * (0.5f + static_cast<float>(rand()) / RAND_MAX);
        
        p.color = glm::vec3(
            1.0f,
            static_cast<float>(rand()) / RAND_MAX * 0.5f,
            0.0f
        );
        
        _particles.push_back(p);
    }
}

void ParticleSystem::update(float deltaTime) {
    _timeSinceSpawn += deltaTime;
    
    for(auto& p : _particles) {
        p.velocity.y += GRAVITY * deltaTime;
        p.position += p.velocity * deltaTime;
        p.life = 1.0f - (_timeSinceSpawn / _totalLifetime);
    }
}

void ParticleSystem::draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for(const auto& p : _particles) {
        glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), p.position);
        modelMtx = glm::scale(modelMtx, glm::vec3(p.size));
        
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glProgramUniformMatrix4fv(_shaderProgramHandle, _mvpMatrixUniform, 1, GL_FALSE, &mvpMtx[0][0]);
        
        glm::vec4 colorWithAlpha = glm::vec4(p.color, p.life);
        glProgramUniform4fv(_shaderProgramHandle, _colorUniform, 1, &colorWithAlpha[0]);
        
        CSCI441::drawSolidSphere(0.5f, 8, 8);
    }
    
    glDisable(GL_BLEND);
} 