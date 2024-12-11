#include "Plane.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <CSCI441/objects.hpp>
#include <CSCI441/OpenGLUtils.hpp>

#include <cmath> // for sin function

Plane::Plane( GLuint shaderProgramHandle, GLint mvpMtxUniformLocation, GLint normalMtxUniformLocation, GLint materialColorUniformLocation ) {
    _shaderProgramHandle                            = shaderProgramHandle;
    _shaderProgramUniformLocations.mvpMtx           = mvpMtxUniformLocation;
    _shaderProgramUniformLocations.normalMtx        = normalMtxUniformLocation;
    _shaderProgramUniformLocations.materialColor    = materialColorUniformLocation;

    _colorBody = glm::vec3( 0.8f, 0.1f, 0.1f );
    _colorWheels = glm::vec3( 0.2f, 0.2f, 0.2f );
    _colorWindows = glm::vec3( 0.7f, 0.9f, 1.0f );
    _colorLights = glm::vec3( 1.0f, 0.9f, 0.0f );
    _scaleBody = glm::vec3( 2.2f, 0.5f, 1.2f );

    _internalTimer = 0.0f;
}

void Plane::drawPlane( glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    _internalTimer += 0.016f;
    if (_internalTimer >= _2PI) {
        _internalTimer -= _2PI;
    }

    modelMtx = glm::rotate(modelMtx, _internalTimer * _spinSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

    modelMtx = glm::rotate(modelMtx, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    float carHeight = 0.5f;
    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, carHeight, 0.0f));

    float rumbleOffset = sin(_internalTimer * _rumbleSpeed) * _rumbleAmount;
    modelMtx = glm::translate(modelMtx, glm::vec3(0, rumbleOffset, 0));

    glm::mat4 bodyMtx = glm::scale(modelMtx, _scaleBody);
    _computeAndSendMatrixUniforms(bodyMtx, viewMtx, projMtx);
    glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(_colorBody));
    CSCI441::drawSolidCube(1.0f);

    glm::mat4 roofMtx = glm::translate(modelMtx, glm::vec3(0, 0.35f, 0));
    roofMtx = glm::scale(roofMtx, glm::vec3(1.6f, 0.3f, 1.0f));
    _computeAndSendMatrixUniforms(roofMtx, viewMtx, projMtx);
    CSCI441::drawSolidCube(1.0f);

    glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(_colorWheels));
    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            glm::mat4 wheelMtx = glm::translate(modelMtx, glm::vec3(i * 0.9f, -0.25f, j * 0.7f));
            wheelMtx = glm::rotate(wheelMtx, glm::radians(90.0f), glm::vec3(0, 0, 1));
            wheelMtx = glm::scale(wheelMtx, glm::vec3(0.3f, 0.1f, 0.3f));
            _computeAndSendMatrixUniforms(wheelMtx, viewMtx, projMtx);
            CSCI441::drawSolidCylinder(0.5f, 0.5f, 1.0f, 16, 16);
        }
    }

    glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(_colorWindows));
    for (int i = -1; i <= 1; i += 2) {
        glm::mat4 windowMtx = glm::translate(modelMtx, glm::vec3(i * 0.7f, 0.3f, 0.0f));
        windowMtx = glm::scale(windowMtx, glm::vec3(0.2f, 0.25f, 0.9f));
        windowMtx = glm::rotate(windowMtx, glm::radians(i * 20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        _computeAndSendMatrixUniforms(windowMtx, viewMtx, projMtx);
        CSCI441::drawSolidCube(1.0f);
    }

    glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(_colorLights));
    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            glm::mat4 lightMtx = glm::translate(modelMtx, glm::vec3(i * 1.1f, 0.0f, j * 0.5f));
            lightMtx = glm::scale(lightMtx, glm::vec3(0.1f, 0.1f, 0.1f));
            _computeAndSendMatrixUniforms(lightMtx, viewMtx, projMtx);
            CSCI441::drawSolidSphere(0.5f, 10, 10);
        }
    }
}

void Plane::_computeAndSendMatrixUniforms(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx) const {
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv( _shaderProgramHandle, _shaderProgramUniformLocations.mvpMtx, 1, GL_FALSE, glm::value_ptr(mvpMtx) );

    glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx )));
    glProgramUniformMatrix3fv( _shaderProgramHandle, _shaderProgramUniformLocations.normalMtx, 1, GL_FALSE, glm::value_ptr(normalMtx) );
}
