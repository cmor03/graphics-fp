#include "Ghost.h"
#include <CSCI441/objects.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

std::vector<glm::vec3> GhostManager::_allGhostPositions;
bool GhostManager::_hasCollided = false;

Ghost::Ghost(const glm::vec3& startPos, GLuint shaderProgramHandle, 
            GLint mvpMatrixUniform, GLint normalMatrixUniform, GLint materialColorUniform,
            float size)
    : _position(startPos), 
      _currentDirection(glm::vec3(0.0f)),
      _verticalOffset(0.0f), 
      _bobTimer(0.0f),
      _isFalling(false),
      _fallTime(0.0f),
      _size(size),
      _shaderProgramHandle(shaderProgramHandle), 
      _mvpMatrixUniform(mvpMatrixUniform),
      _normalMatrixUniform(normalMatrixUniform),
      _materialColorUniform(materialColorUniform) {}

Ghost Ghost::merge(const Ghost& g1, const Ghost& g2, GLuint shaderProgramHandle, 
                  GLint mvpMatrixUniform, GLint normalMatrixUniform, GLint materialColorUniform) {
    glm::vec3 newPos = (g1._position + g2._position) * 0.5f;
    
    float newSize = std::min(g1._size + g2._size, MAX_GHOST_SIZE);
    
    return Ghost(newPos, shaderProgramHandle, mvpMatrixUniform, normalMatrixUniform, 
                materialColorUniform, newSize);
}

void Ghost::update(float deltaTime, const glm::vec3& targetPos) {
    if(_isFalling) {
        _fallTime += deltaTime;
        _position.y -= FALL_SPEED * deltaTime;
        return;
    }
    
    _bobTimer += deltaTime * BOB_SPEED;
    _verticalOffset = sin(_bobTimer) * BOB_AMOUNT;
    
    // Check if ghost should fall
    if(_position.x > WORLD_SIZE || _position.x < -WORLD_SIZE || 
       _position.z > WORLD_SIZE || _position.z < -WORLD_SIZE) {
        _isFalling = true;
        _fallTime = 0.0f;
        return;
    }
    
    // Calculate direction to target
    glm::vec3 toTarget = targetPos - _position;
    toTarget.y = 0; 
    
    if(glm::length(toTarget) > 0.1f) {
        glm::vec3 desiredDirection = glm::normalize(toTarget);
        
        if(glm::length(_currentDirection) < 0.1f) {
            _currentDirection = desiredDirection;
        }
        
        _currentDirection = glm::normalize(
            _currentDirection + (desiredDirection - _currentDirection) * TURN_RATE * deltaTime
        );
        
        glm::vec3 avoidance(0.0f);
        for(const auto& other : GhostManager::getAllGhostPositions()) {
            if(other != _position) {
                glm::vec3 diff = _position - other;
                float dist = glm::length(diff);
                if(dist < GHOST_AVOIDANCE_RADIUS) {
                    avoidance += glm::normalize(diff) * (GHOST_AVOIDANCE_RADIUS - dist) / GHOST_AVOIDANCE_RADIUS;
                }
            }
        }
        
        // Combine movement direction with avoidance
        glm::vec3 finalDirection = glm::normalize(_currentDirection + avoidance * AVOIDANCE_WEIGHT);
        
        // Always move forward along the current heading
        _position += finalDirection * MOVEMENT_SPEED * deltaTime;
        
        // Check for collision with player
        if(glm::length(toTarget) < COLLISION_RADIUS) {
            GhostManager::setCollision(true);
        }
    }
}

void Ghost::draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    glm::mat4 modelMtx = glm::mat4(1.0f);
    modelMtx = glm::translate(modelMtx, _position + glm::vec3(0, 1.5f + _verticalOffset, 0));
    modelMtx = glm::scale(modelMtx, glm::vec3(_size));
    
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
    
    glm::vec3 cameraPos = glm::vec3(glm::inverse(viewMtx)[3]);
    float distance = glm::length(_position - cameraPos);
    float alpha = glm::clamp(1.0f - (distance / FADE_DISTANCE), 0.2f, 0.8f);
    
    glm::vec4 ghostColor = glm::vec4(GHOST_COLOR, alpha);
    
    glProgramUniformMatrix4fv(_shaderProgramHandle, _mvpMatrixUniform, 1, GL_FALSE, &mvpMtx[0][0]);
    glProgramUniformMatrix3fv(_shaderProgramHandle, _normalMatrixUniform, 1, GL_FALSE, &normalMtx[0][0]);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glProgramUniform4fv(_shaderProgramHandle, _materialColorUniform, 1, &ghostColor[0]);
    
    // Draw main body
    modelMtx = glm::scale(modelMtx, glm::vec3(1.0f, 1.2f, 1.0f));
    mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv(_shaderProgramHandle, _mvpMatrixUniform, 1, GL_FALSE, &mvpMtx[0][0]);
    CSCI441::drawSolidSphere(1.0f, 32, 32);
    
    modelMtx = glm::translate(modelMtx, glm::vec3(0, -1.0f, 0));
    modelMtx = glm::scale(modelMtx, glm::vec3(1.0f, 0.3f, 1.0f));
    mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv(_shaderProgramHandle, _mvpMatrixUniform, 1, GL_FALSE, &mvpMtx[0][0]);
    
    for(int i = 0; i < 6; i++) {
        float xOffset = sin(i * M_PI / 3) * 0.8f;
        float zOffset = cos(i * M_PI / 3) * 0.8f;
        glm::mat4 waveMtx = glm::translate(modelMtx, glm::vec3(xOffset, sin(_bobTimer + i) * 0.2f, zOffset));
        mvpMtx = projMtx * viewMtx * waveMtx;
        glProgramUniformMatrix4fv(_shaderProgramHandle, _mvpMatrixUniform, 1, GL_FALSE, &mvpMtx[0][0]);
        CSCI441::drawSolidSphere(0.5f, 16, 16);
    }
    
    glDisable(GL_BLEND);
}

GhostManager::GhostManager(GLuint shaderProgramHandle, GLint mvpMatrixUniform, 
                         GLint normalMatrixUniform, GLint materialColorUniform)
    : _shaderProgramHandle(shaderProgramHandle),
      _mvpMatrixUniform(mvpMatrixUniform),
      _normalMatrixUniform(normalMatrixUniform),
      _materialColorUniform(materialColorUniform),
      SPAWN_INTERVAL(_initialSpawnInterval),
      GHOST_DENSITY(_initialDensity),
      MAX_GHOSTS(_initialMaxGhosts) {}

void GhostManager::update(float deltaTime, const glm::vec3& playerPos) {
    _timeSinceStart += deltaTime;
    
    // Scale difficulty based on time
    float difficultyMultiplier = 1.0f + (_timeSinceStart * DIFFICULTY_SCALE_RATE);
    
    // Update spawn parameters based on difficulty
    SPAWN_INTERVAL = std::max(MIN_SPAWN_INTERVAL, 
                            _initialSpawnInterval / difficultyMultiplier);
    
    GHOST_DENSITY = std::min(MAX_DENSITY, 
                            _initialDensity * difficultyMultiplier);
    
    MAX_GHOSTS = std::min(ABSOLUTE_MAX_GHOSTS, 
                         static_cast<size_t>(_initialMaxGhosts * difficultyMultiplier));
    
    _allGhostPositions.clear();
    for(auto& ghost : _ghosts) {
        ghost.update(deltaTime, playerPos);
        _allGhostPositions.push_back(ghost.getPosition());
    }
    
    _ghosts.erase(
        std::remove_if(_ghosts.begin(), _ghosts.end(),
            [](const Ghost& ghost) { return !ghost.isAlive(); }),
        _ghosts.end()
    );
    
    _spawnTimer += deltaTime;
    if(_spawnTimer >= SPAWN_INTERVAL && _ghosts.size() < MAX_GHOSTS) {
        _spawnTimer = 0.0f;
        
        if(static_cast<float>(rand()) / RAND_MAX < GHOST_DENSITY) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            glm::vec3 spawnPos = playerPos + glm::vec3(
                cos(angle) * SPAWN_RADIUS,
                0.0f,
                sin(angle) * SPAWN_RADIUS
            );
            spawnGhost(spawnPos);
        }
    }
    
    checkGhostMerges();
}

void GhostManager::draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    for(const auto& ghost : _ghosts) {
        ghost.draw(viewMtx, projMtx);
    }
}

void GhostManager::spawnGhost(const glm::vec3& position) {
    if(_ghosts.size() < MAX_GHOSTS) {
        _ghosts.emplace_back(position, _shaderProgramHandle, 
                           _mvpMatrixUniform, _normalMatrixUniform, 
                           _materialColorUniform, 1.0f);
    }
}

void GhostManager::reset() {
    _ghosts.clear();
    _allGhostPositions.clear();
    _hasCollided = false;
    _spawnTimer = 0.0f;
    _timeSinceStart = 0.0f; 
    
    SPAWN_INTERVAL = _initialSpawnInterval;
    GHOST_DENSITY = _initialDensity;
    MAX_GHOSTS = _initialMaxGhosts;
}

void GhostManager::checkGhostMerges() {
    bool mergeOccurred;
    do {
        mergeOccurred = false;
        for(size_t i = 0; i < _ghosts.size(); i++) {
            for(size_t j = i + 1; j < _ghosts.size(); j++) {
                glm::vec3 diff = _ghosts[i].getPosition() - _ghosts[j].getPosition();
                float dist = glm::length(diff);
                
                if(dist < Ghost::MERGE_DISTANCE * (_ghosts[i].getSize() + _ghosts[j].getSize())) {
                    Ghost mergedGhost = Ghost::merge(_ghosts[i], _ghosts[j], 
                                                   _shaderProgramHandle, _mvpMatrixUniform,
                                                   _normalMatrixUniform, _materialColorUniform);
                    
                    _ghosts.erase(_ghosts.begin() + j);
                    _ghosts.erase(_ghosts.begin() + i);
                    
                    _ghosts.push_back(mergedGhost);
                    
                    mergeOccurred = true;
                    break;
                }
            }
            if(mergeOccurred) break;
        }
    } while(mergeOccurred);
}