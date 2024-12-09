#include "CollisionDetector.h"

std::vector<CollisionObject> CollisionDetector::_collisionObjects;

bool CollisionDetector::checkCollision(const glm::vec2& playerPos, float playerRadius) {
    return getCollidedObject(playerPos, playerRadius) != nullptr;
}
CollisionObject* CollisionDetector::getCollidedObject(const glm::vec2& playerPos, float playerRadius){
    for(int i = 0; i < _collisionObjects.size(); i++) {
        CollisionObject& obj = _collisionObjects[i];
        // Calculate 2D distance (ignoring height)
        float distance = glm::length(
            glm::vec2(obj.position.x, obj.position.z) - playerPos
        );
        
        // If distance is less than combined radii, collision occurred
        if(distance < (playerRadius + obj.radius)) {
            return &obj;
        }
    }
    return nullptr;
}

void CollisionDetector::addCollisionObject(const glm::vec3& pos, float radius, bool isTree) {
    _collisionObjects.push_back({pos, radius, isTree});
}

void CollisionDetector::clearCollisionObjects() {
    _collisionObjects.clear();
} 