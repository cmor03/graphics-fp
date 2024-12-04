#include "CollisionDetector.h"

std::vector<CollisionObject> CollisionDetector::_collisionObjects;

bool CollisionDetector::checkCollision(const glm::vec2& playerPos, const std::vector<CollisionObject>& objects, float playerRadius) {
    for(const auto& obj : objects) {
        // Calculate 2D distance (ignoring height)
        float distance = glm::length(
            glm::vec2(obj.position.x, obj.position.z) - playerPos
        );
        
        // If distance is less than combined radii, collision occurred
        if(distance < (playerRadius + obj.radius)) {
            return true;
        }
    }
    return false;
}

void CollisionDetector::addCollisionObject(const glm::vec3& pos, float radius, bool isTree) {
    _collisionObjects.push_back({pos, radius, isTree});
}

void CollisionDetector::clearCollisionObjects() {
    _collisionObjects.clear();
} 