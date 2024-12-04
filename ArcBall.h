#ifndef ARC_BALL_H
#define ARC_BALL_H

#include <CSCI441/Camera.hpp>

class ArcBall final : public CSCI441::Camera {
public:
    void recomputeOrientation() final;

    void moveForward(GLfloat movementFactor) final;
    void moveBackward(GLfloat movementFactor) final;
    
private:
    void _updateArcBallCameraViewMatrix();
};

#endif 