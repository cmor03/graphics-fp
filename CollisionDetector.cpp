#include "CollisionDetector.h"
#include <iostream>
std::vector<CollisionObject> CollisionDetector::_collisionObjects;

bool CollisionDetector::checkCollision(const glm::vec2& playerPos, float playerRadius) {
    return getCollidedObject(playerPos, playerRadius) != nullptr;
}
CollisionObject* CollisionDetector::getCollidedObject(const glm::vec2& playerPos, float playerRadius){
    for(int i = 0; i < _collisionObjects.size(); i++) {
        CollisionObject obj = _collisionObjects[i];
        if(playerPos.x < obj.position.x + obj.radius && 
        playerPos.x > obj.position.x - obj.radius &&
        playerPos.y < obj.position.z + obj.radius && 
        playerPos.y > obj.position.z - obj.radius){
            return &_collisionObjects[i];
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