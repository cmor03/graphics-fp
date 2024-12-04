#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <glm/glm.hpp>
#include <vector>

struct CollisionObject {
    glm::vec3 position;
    float radius;  // For simple circle-based collision
    bool isTree;   // To differentiate between trees and buildings
};

class CollisionDetector {
public:
    static bool checkCollision(const glm::vec2& playerPos, const std::vector<CollisionObject>& objects, float playerRadius = 1.0f);
    static void addCollisionObject(const glm::vec3& pos, float radius, bool isTree);
    static void clearCollisionObjects();
    static const std::vector<CollisionObject>& getCollisionObjects() { return _collisionObjects; }

private:
    static std::vector<CollisionObject> _collisionObjects;
};

#endif 