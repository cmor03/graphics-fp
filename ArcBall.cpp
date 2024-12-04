#include "ArcBall.h"

void ArcBall::recomputeOrientation() {
    GLfloat theta = getTheta();
    GLfloat phi = getPhi();
    GLfloat radius = getRadius();


    mCameraPosition = glm::vec3(radius * glm::sin(theta) * glm::sin(phi),
        -radius * glm::cos(phi),
        -radius * glm::cos(theta) * glm::sin(phi)) + getLookAtPoint();

    _updateArcBallCameraViewMatrix();
}
void ArcBall::moveForward(GLfloat movementFactor) {
    mCameraRadius -= movementFactor;
    recomputeOrientation();
}

void ArcBall::moveBackward(GLfloat movementFactor) {
    mCameraRadius += movementFactor;
    recomputeOrientation();
}

void ArcBall::_updateArcBallCameraViewMatrix() {

    glm::vec3 newLookAt = mCameraDirection + getPosition();
    // setLookAtPoint(newLookAt);

    computeViewMatrix();
}