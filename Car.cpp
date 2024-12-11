#include "Car.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <CSCI441/objects.hpp>
#include <CSCI441/OpenGLUtils.hpp>

Car::Car( GLuint shaderProgramHandle, GLint mvpMtxUniformLocation, GLint normalMtxUniformLocation, GLint materialColorUniformLocation ) {
    _propAngle = 0.0f;
    _propAngleRotationSpeed = 0.0f;

    _shaderProgramHandle                            = shaderProgramHandle;
    _shaderProgramUniformLocations.mvpMtx           = mvpMtxUniformLocation;
    _shaderProgramUniformLocations.normalMtx        = normalMtxUniformLocation;
    _shaderProgramUniformLocations.materialColor    = materialColorUniformLocation;

    _rotateCarAngle = _PI / 2.0f;

    _colorBody = glm::vec3(0.8f, 0.1f, 0.1f);
    _scaleBody = glm::vec3(2.0f, 0.5f, 1.0f);

    _colorWing = glm::vec3(0.2f, 0.2f, 0.2f);
    _scaleWing = glm::vec3(1.5f, 0.5f, 1.0f);
    _rotateWingAngle = _PI / 2.0f;

    _colorNose = glm::vec3(0.8f, 0.1f, 0.1f);
    _rotateNoseAngle = _PI / 2.0f;

    _colorProp = glm::vec3(0.2f, 0.2f, 0.2f);
    _scaleProp = glm::vec3(1.1f, 1.0f, 0.025f);
    _transProp = glm::vec3(0.1f, 0.0f, 0.0f);

    _colorTail = glm::vec3(0.2f, 0.2f, 0.2f);
}

void Car::drawCar( glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) {
    modelMtx = glm::rotate( modelMtx, -_rotateCarAngle, CSCI441::Y_AXIS );
    modelMtx = glm::rotate( modelMtx, _rotateCarAngle, CSCI441::Z_AXIS );
    //car body
    _drawCarBody(modelMtx, viewMtx, projMtx);
    _drawCarTop(modelMtx, viewMtx, projMtx);

    //wheels
    glm::mat4 wheelMtx = glm::translate(modelMtx, glm::vec3(0,-glm::length(_transProp)/2, 0));
    _drawCarWheel(wheelMtx, viewMtx, projMtx);
    wheelMtx = glm::translate( wheelMtx, -_transProp-_transProp );
    _drawCarWheel(wheelMtx, viewMtx, projMtx);
}

void Car::_drawCarBody(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) const {
    modelMtx = glm::translate(modelMtx, glm::vec3(0,-glm::length(_transProp),0));
    modelMtx = glm::scale( modelMtx, _scaleBody );

    _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

    glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(_colorBody));

    CSCI441::drawSolidCube( 0.1f );
}

void Car::_drawCarTop(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) const {
    modelMtx = glm::translate(modelMtx, glm::vec3(0.05,-glm::length(_transProp)*1.25,0));
    modelMtx = glm::rotate( modelMtx, _rotateNoseAngle, CSCI441::Z_AXIS );
    modelMtx = glm::scale( modelMtx, glm::vec3(.5, .5, .5) );

    _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

    glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(_colorNose));

    CSCI441::drawSolidCone( 0.025f, 0.3f, 16, 16 );
}

void Car::_drawCarWheel(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx ) const {
    glm::mat4 modelMtx1 = glm::translate( modelMtx, _transProp );
    modelMtx1 = glm::rotate(modelMtx1, (GLfloat)M_PI/2.0f, CSCI441::Y_AXIS);
    modelMtx1 = glm::rotate( modelMtx1, _propAngle, CSCI441::X_AXIS );
    modelMtx1 = glm::scale( modelMtx1, _scaleProp );

    _computeAndSendMatrixUniforms(modelMtx1, viewMtx, projMtx);

    glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(_colorProp));

    CSCI441::drawSolidCube( 0.1f );

    glm::mat4 modelMtx2 = glm::translate( modelMtx, _transProp );
    modelMtx2 = glm::rotate(modelMtx2, (GLfloat)M_PI/2.0f, CSCI441::Y_AXIS);
    modelMtx2 = glm::rotate( modelMtx2, _PI_OVER_2 + _propAngle, CSCI441::X_AXIS );
    modelMtx2 = glm::scale( modelMtx2, _scaleProp );

    _computeAndSendMatrixUniforms(modelMtx2, viewMtx, projMtx);

    CSCI441::drawSolidCube( 0.1f );
}

void Car::_computeAndSendMatrixUniforms(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx) const {

    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv( _shaderProgramHandle, _shaderProgramUniformLocations.mvpMtx, 1, GL_FALSE, glm::value_ptr(mvpMtx) );

    glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx )));
    glProgramUniformMatrix3fv( _shaderProgramHandle, _shaderProgramUniformLocations.normalMtx, 1, GL_FALSE, glm::value_ptr(normalMtx) );
}
