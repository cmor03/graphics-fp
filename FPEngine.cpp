#include "FPEngine.h"

#include <CSCI441/objects.hpp>

#include <cmath>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

//*************************************************************************************
//
// Helper Functions

static const GLfloat GLM_PI = glm::pi<float>();
static const GLfloat GLM_2PI = glm::two_pi<float>();

//*************************************************************************************
//
// Public Interface

FPEngine::FPEngine()
     : CSCI441::OpenGLEngine(4, 1, 720, 720, "A4") {

    for(auto& _key : _keys) _key = GL_FALSE;

    _mousePosition = glm::vec2(MOUSE_UNINITIALIZED, MOUSE_UNINITIALIZED );
    _leftMouseButtonState = GLFW_RELEASE;
    _direction = 0;
    _pos = glm::vec2(0,0);
    _playerRadius = 0.5f;
    _lastDir = 0;
}
GLfloat getRand() {
    return (GLfloat)rand() / (GLfloat)RAND_MAX;
}

void FPEngine::handleKeyEvent(GLint key, GLint action) {
    if(key != GLFW_KEY_UNKNOWN)
        _keys[key] = ((action == GLFW_PRESS) || (action == GLFW_REPEAT));

    if(action == GLFW_PRESS) {
        switch( key ) {
            // quit!
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                setWindowShouldClose();
                break;
            default: break;
        }
    }
}

void FPEngine::handleMouseButtonEvent(GLint button, GLint action) {

    if( button == GLFW_MOUSE_BUTTON_LEFT ) {
        _leftMouseButtonState = action;
    }
}

void FPEngine::handleCursorPositionEvent(glm::vec2 currMousePosition) {
    // if mouse hasn't moved in the window, prevent camera from flipping out
    if(fabs(_mousePosition.x - MOUSE_UNINITIALIZED) <= 0.000001f) {
        _mousePosition = currMousePosition;
    }

    // active motion - if the left mouse button is being held down while the mouse is moving
    if(_leftMouseButtonState == GLFW_PRESS) {
        glm::vec2 lastMousePosition = _mousePosition;
        GLfloat dTheta = (currMousePosition.x - lastMousePosition.x) * 0.005;
        GLfloat dPhi = (lastMousePosition.y - currMousePosition.y) * 0.005;
        // rotate the camera by the distance the mouse moved
        
        _pArcBall->rotate( dTheta,dPhi );

        // update the last mouse position
        _mousePosition = currMousePosition;
    }
    // passive motion
    else {

    }

    // update the mouse position
    _mousePosition = currMousePosition;
}

void FPEngine::handleScrollEvent(glm::vec2 offset) {
    _pArcBall->moveForward(offset.y);
    glm::vec3 pos = _pArcBall->getPosition();
}

//*************************************************************************************
//
// Engine Setup

void FPEngine::mSetupGLFW() {
    CSCI441::OpenGLEngine::mSetupGLFW();

    // Update callback references from mp_ to a4_
    glfwSetKeyCallback(mpWindow, fp_keyboard_callback);
    glfwSetMouseButtonCallback(mpWindow, fp_mouse_button_callback);
    glfwSetCursorPosCallback(mpWindow, fp_cursor_callback);
    glfwSetScrollCallback(mpWindow, fp_scroll_callback);
}

void FPEngine::mSetupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                    // enable depth testing
    glDepthFunc( GL_LESS );							                // use less than depth test

    glEnable(GL_BLEND);									            // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black
    
}

void FPEngine::mSetupShaders() {
    _shaderProgram = new CSCI441::ShaderProgram("shaders/mp.v.glsl", "shaders/mp.f.glsl" );
    // query uniform locations
    _shaderUniformLocations.mvpMatrix      = _shaderProgram->getUniformLocation("mvpMatrix");
    _shaderUniformLocations.lightDirection      = _shaderProgram->getUniformLocation("lightDirection");
    _shaderUniformLocations.directionalLightColor      = _shaderProgram->getUniformLocation("directionalLightColor");
    _shaderUniformLocations.pointLightPosition      = _shaderProgram->getUniformLocation("pointLightPosition");
    _shaderUniformLocations.pointLightColor      = _shaderProgram->getUniformLocation("pointLightColor");
    _shaderUniformLocations.materialColor      = _shaderProgram->getUniformLocation("materialColor");
    _shaderUniformLocations.normalMatrix      = _shaderProgram->getUniformLocation("normalMatrix");
    _shaderUniformLocations.viewVector      = _shaderProgram->getUniformLocation("viewVector");
    _shaderUniformLocations.textureMap      = _shaderProgram->getUniformLocation("textureMap");
    _shaderAttributeLocations.vPos         = _shaderProgram->getAttributeLocation("vPos");
    _shaderAttributeLocations.normalVec      = _shaderProgram->getAttributeLocation("normalVec");
    _shaderAttributeLocations.inTexCoord      = _shaderProgram->getAttributeLocation("inTexCoord");

    _shaderProgram->setProgramUniform("textureMap", 0);


    CSCI441::setVertexAttributeLocations(_shaderAttributeLocations.vPos,
                                         _shaderAttributeLocations.normalVec,
                                         _shaderAttributeLocations.inTexCoord);
}

void FPEngine::mSetupBuffers() {
    glGenVertexArrays( NUM_VAOS, _vaos );
    glGenBuffers( NUM_VAOS, _vbos );
    glGenBuffers( NUM_VAOS, _ibos );

    _createPlatform(_vaos[VAO_ID::PLATFORM], _vbos[VAO_ID::PLATFORM], _ibos[VAO_ID::PLATFORM], _numVAOPoints[VAO_ID::PLATFORM]);
    _generateEnvironment();
    _createQuad(_vaos[VAO_ID::QUAD], _vbos[VAO_ID::QUAD], _ibos[VAO_ID::QUAD], _numVAOPoints[VAO_ID::QUAD]);

    _pColtonPlane = new Plane(_shaderProgram->getShaderProgramHandle(),
                            _shaderUniformLocations.mvpMatrix,
                            _shaderUniformLocations.normalMatrix,
                            _shaderUniformLocations.materialColor);
}

void FPEngine::_createPlatform(GLuint vao, GLuint vbo, GLuint ibo, GLsizei &numVAOPoints) const {
    struct VertexNormalTextured {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // create our platform
    VertexNormalTextured platformVertices[4] = {
            { { 0, 1.0f, 0 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }, // 0 - BL
            { {  WORLD_SIZE_X, 1.0f, 0 }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } }, // 1 - BR
            { { 0, 1.0f,  WORLD_SIZE_Y}, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }, // 2 - TL
            { {  WORLD_SIZE_X, 1.0f,  WORLD_SIZE_Y }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } }  // 3 - TR
    };

    GLushort platformIndices[4] = { 0, 1, 2, 3 };
    numVAOPoints = 4;

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData( GL_ARRAY_BUFFER, sizeof( platformVertices ), platformVertices, GL_STATIC_DRAW );

    glEnableVertexAttribArray( _shaderAttributeLocations.vPos );
    glVertexAttribPointer( _shaderAttributeLocations.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured), (void*)nullptr );

    glEnableVertexAttribArray( _shaderAttributeLocations.normalVec );
    glVertexAttribPointer( _shaderAttributeLocations.normalVec, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured), (void*)(sizeof(glm::vec3)) );

    glEnableVertexAttribArray(_shaderAttributeLocations.inTexCoord);
    glVertexAttribPointer( _shaderAttributeLocations.inTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured), (void*)(sizeof(glm::vec3)*2) );



    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    fprintf( stdout, "[INFO]: platform read in with VAO/VBO/IBO %d/%d/%d & %d points\n", vao, vbo, ibo, numVAOPoints );
}

void FPEngine::_createQuad(GLuint vao, GLuint vbo, GLuint ibo, GLsizei &numVAOPoints) const {

   struct VertexNormalTextured {
       glm::vec3 position;
       glm::vec3 normal;
       glm::vec2 texCoord;
   };

   VertexNormalTextured quadVertices[4] = {
           { { -2.5f, -2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }, // 0 - BL
           { {  2.5f, -2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 3.0f, 0.0f } }, // 1 - BR
           { { -2.5f,  2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 3.0f } }, // 2 - TL
           { {  2.5f,  2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 3.0f, 3.0f } }  // 3 - TR
   };

   GLushort quadIndices[4] = { 0, 1, 2, 3 };
   numVAOPoints = 4;

   glBindVertexArray( vao );

   glBindBuffer( GL_ARRAY_BUFFER, vbo );
   glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );

   glEnableVertexAttribArray( _shaderAttributeLocations.vPos );
   glVertexAttribPointer( _shaderAttributeLocations.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured), (void*)nullptr );

   glEnableVertexAttribArray( _shaderAttributeLocations.normalVec);
   glVertexAttribPointer( _shaderAttributeLocations.normalVec, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured), (void*)(sizeof(glm::vec3)) );

   glEnableVertexAttribArray( _shaderAttributeLocations.inTexCoord );
   glVertexAttribPointer( _shaderAttributeLocations.inTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextured), (void*)(2*sizeof(glm::vec3)) );

   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( quadIndices ), quadIndices, GL_STATIC_DRAW );

   fprintf( stdout, "[INFO]: quad read in with VAO/VBO/IBO %d/%d/%d & %d points\n", vao, vbo, ibo, numVAOPoints );
}

void FPEngine::mSetupTextures() {
    _texHandles[TEXTURE_ID::GROUND] = _loadAndRegisterTexture("assets/textures/ground.png");
    _texHandles[TEXTURE_ID::TREE] = _loadAndRegisterTexture("assets/textures/tree.png");
    _texHandles[TEXTURE_ID::BUILDING] = _loadAndRegisterTexture("assets/textures/building.png");
    _texHandles[TEXTURE_ID::LEAVES] = _loadAndRegisterTexture("assets/textures/leaves.png");

    _skyTexture = _loadAndRegisterTexture("assets/textures/skybox.png");
    fprintf(stdout, "[INFO]: Skybox texture handle: %d\n", _skyTexture);
}

void FPEngine::mSetupScene() {
    _pArcBall = new ArcBall();
    _pArcBall->setRadius(20.0f);
    _pArcBall->setTheta(0.0f);
    _pArcBall->setPhi(M_PI/1.5f);
    _pArcBall->recomputeOrientation();

    _objectIndex = 0;
    _objectAngle = 0.0f;

    glm::vec3 directionalLightColor = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDirection = glm::vec3(-1,-1,-1);
    glm::vec3 viewVector = _pArcBall->getPosition();
    
    glProgramUniform3fv(_shaderProgram->getShaderProgramHandle(),
        _shaderUniformLocations.directionalLightColor,
        1,
        glm::value_ptr(directionalLightColor));

    glProgramUniform3fv(_shaderProgram->getShaderProgramHandle(),
        _shaderUniformLocations.lightDirection,
        1,
        glm::value_ptr(lightDirection));
        
    glProgramUniform3fv(_shaderProgram->getShaderProgramHandle(),
        _shaderUniformLocations.viewVector,
        1,
        glm::value_ptr(viewVector));

    _setupSkybox();

    _ghostManager = new GhostManager(_shaderProgram->getShaderProgramHandle(),
                                   _shaderUniformLocations.mvpMatrix,
                                   _shaderUniformLocations.normalMatrix,
                                   _shaderUniformLocations.materialColor);
    
    _ghostManager->setMaxGhosts(30);
    _ghostManager->setSpawnRadius(40.0f);
    _ghostManager->setSpawnInterval(3.0f);
    _ghostManager->setDensity(0.3f); 
    
    Ghost::setMovementSpeed(3.0f);
    Ghost::setGhostSize(1.0f); 
    Ghost::setGhostColor(glm::vec3(1.0f, 1.0f, 1.0f));
    Ghost::setFadeDistance(30.0f);

    _particleSystem = new ParticleSystem(_shaderProgram->getShaderProgramHandle(),
                                       _shaderUniformLocations.mvpMatrix,
                                       _shaderUniformLocations.materialColor);
}

//*************************************************************************************
//
// Engine Cleanup

void FPEngine::mCleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting Shaders.\n" );
    delete _shaderProgram;
}

void FPEngine::mCleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );
    CSCI441::deleteObjectVAOs();
    glDeleteVertexArrays( NUM_VAOS, _vaos );

    fprintf( stdout, "[INFO]: ...deleting VBOs....\n" );
    CSCI441::deleteObjectVBOs();
    glDeleteBuffers( NUM_VAOS, _vbos );

    fprintf( stdout, "[INFO]: ...deleting IBOs....\n" );
    glDeleteBuffers( NUM_VAOS, _ibos );

    fprintf( stdout, "[INFO]: ...deleting models..\n" );
    delete _pObjModel;
    delete _pHellknight;
}

void FPEngine::mCleanupTextures() {
    fprintf( stdout, "[INFO]: ...deleting textures\n" );
    // TODO #23 - delete textures
    glDeleteTextures(4, _texHandles);

}

void FPEngine::mCleanupScene() {
    fprintf(stdout, "[INFO]: ...deleting scene...\n");
    delete _pArcBall;
    delete _pColtonPlane;
    
    // Cleanup skybox resources
    delete _skyboxShader;
    glDeleteVertexArrays(1, &_skyboxVAO);
    glDeleteBuffers(1, &_skyboxVBO);
    glDeleteTextures(1, &_skyTexture);

    delete _ghostManager;
    delete _particleSystem;
}


std::vector<std::vector<int>> read_csv(const std::string& filename) {
    std::vector<std::vector<int>> data;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Could not open the file" << std::endl;
        return data;
    }

    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            row.push_back(std::stoi(cell));
        }

        data.push_back(row);
    }

    file.close();
    return data;
}

void FPEngine::_generateEnvironment() {
    // Clear any existing collision objects
    CollisionDetector::clearCollisionObjects();
    
    //******************************************************************
    // parameters to make up our grid size and spacing, feel free to
    // play around with this

    std::vector<std::vector<int>> world_matrix = read_csv("world.csv");
    WORLD_SIZE_X = world_matrix[0].size();
    WORLD_SIZE_Y = world_matrix.size();


    const GLfloat GRID_WIDTH = WORLD_SIZE_X * 1.8f;
    const GLfloat GRID_LENGTH = WORLD_SIZE_Y * 1.8f;
    const GLfloat GRID_SPACING_WIDTH = 1.0f;
    const GLfloat GRID_SPACING_LENGTH = 1.0f;
    // precomputed parameters based on above
    const GLfloat LEFT_END_POINT = -GRID_WIDTH / 2.0f - 5.0f;
    const GLfloat RIGHT_END_POINT = GRID_WIDTH / 2.0f + 5.0f;
    const GLfloat BOTTOM_END_POINT = -GRID_LENGTH / 2.0f - 5.0f;
    const GLfloat TOP_END_POINT = GRID_LENGTH / 2.0f + 5.0f;
    //******************************************************************

    srand( time(0) );                                                   // seed our RNG

    for(int i = 0; i < WORLD_SIZE_X; i++){
        for(int j=0; j < WORLD_SIZE_Y; j++){
            glm::mat4 transToSpotMtx = glm::translate( glm::mat4(1.0), glm::vec3(i*3, 0.0f, j*3));
            if(world_matrix[i][j]==1){
                glm::mat4 scaleToHeightMtx = glm::scale( glm::mat4(1.0), glm::vec3(3, 3, 3) );
                glm::mat4 transToHeight = glm::translate( glm::mat4(1.0), glm::vec3(0, 3/2.0f, 0) );
                glm::mat4 modelMatrix = transToSpotMtx * transToHeight * scaleToHeightMtx;
                // compute random color
                glm::vec3 color( getRand(), getRand(), getRand() );
                // store building properties
                BuildingData currentBuilding = {modelMatrix, color};
                _buildings.emplace_back( currentBuilding );
                CollisionDetector::addCollisionObject(glm::vec3(i*3, 0, j*3),2.1, false);
            }
            else if(world_matrix[i][j]==0){
                glm::mat4 transToHeight = glm::translate( glm::mat4(1.0), glm::vec3(0, 1.0f, 0) );
                glm::mat4 modelMatrix = transToSpotMtx * transToHeight;
                glm::vec3 color(1,1,1);
                PointsData currentPoint  ={modelMatrix, color, glm::vec2(i*3, j*3), false};
                _points.emplace_back(currentPoint);
            }
        }
    }

}
//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void FPEngine::_renderScene(glm::mat4 viewMtx, glm::mat4 projMtx) const {
    // use our lighting shader program
    _shaderProgram->useProgram();
    glm::vec3 defaultColor = glm::vec3(-1,-1,-1);
    glProgramUniform3fv(_shaderProgram->getShaderProgramHandle(), _shaderUniformLocations.materialColor, 1, glm::value_ptr(defaultColor));
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.1f, 0.0f));
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMatrix;
    _shaderProgram->setProgramUniform(_shaderUniformLocations.mvpMatrix, mvpMtx);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
    _shaderProgram->setProgramUniform(_shaderUniformLocations.normalMatrix, normalMatrix);

    // TODO #20 - bind texture
    glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::GROUND]);

    glBindVertexArray( _vaos[VAO_ID::PLATFORM] );
    glDrawElements( GL_TRIANGLE_STRIP, _numVAOPoints[VAO_ID::PLATFORM], GL_UNSIGNED_SHORT, (void*)nullptr );


    for( const BuildingData& currentBuilding : _buildings ) {
        glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::BUILDING]);
        mvpMtx = projMtx * viewMtx * currentBuilding.modelMatrix;
        _shaderProgram->setProgramUniform(_shaderUniformLocations.mvpMatrix, mvpMtx);
        CSCI441::drawSolidCubeTextured(1.0);
    }
    for( const PointsData& currentPoint : _points){

        mvpMtx = projMtx * viewMtx * currentPoint.modelMatrix;
        _shaderProgram->setProgramUniform(_shaderUniformLocations.mvpMatrix, mvpMtx);
        CSCI441::drawSolidSphere(0.2,8,8);
    }

    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.1f, 0.0f));
    modelMatrix = glm::rotate( modelMatrix, _objectAngle, CSCI441::Y_AXIS );
    mvpMtx = projMtx * viewMtx * modelMatrix;
    _shaderProgram->setProgramUniform(_shaderUniformLocations.mvpMatrix, mvpMtx);

    // TODO #21 - bind texture

    glBindTexture(GL_TEXTURE_2D, 0);
    if(_isExploding) {
        _particleSystem->draw(viewMtx, projMtx);
    } else {
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(_pos.x, _currentHeight, _pos.y));
        modelMatrix = glm::rotate(modelMatrix, _direction, CSCI441::Y_AXIS);
        _pColtonPlane->drawPlane(modelMatrix, viewMtx, projMtx);
    }

    _ghostManager->draw(viewMtx, projMtx);
}

void FPEngine::_updateScene() {
    // Handle ghost collision explosion
    if(GhostManager::hasCollided() && !_isExploding) {
        _isExploding = true;
        _particleSystem->spawn(glm::vec3(_pos.x, 
                                        _currentHeight, 
                                        _pos.y));
        return;
    }
    
    // Handle explosion animation and reset
    if(_isExploding) {
        _particleSystem->update(0.016f);
        
        if(!_particleSystem->isAlive()) {
            // Reset everything after explosion
            _isExploding = false;
            _pos = glm::vec2(_spawnPosition.x, _spawnPosition.z);
            _direction = 0.0f;
            _currentHeight = _spawnPosition.y;
            _ghostManager->reset();
        }
        return;
    }

    // Handle falling state
    if(_isFalling) {
        _currentHeight -= FALL_SPEED * 0.016f;
        _fallTime += 0.016f;
        
        if(_fallTime >= MAX_FALL_TIME) {
            // Reset everything
            _isFalling = false;
            _fallTime = 0.0f;
            _pos = glm::vec2(_spawnPosition.x, _spawnPosition.z);
            _direction = 0.0f;
            _currentHeight = _spawnPosition.y;
            _ghostManager->reset();
            return;
        }
        return;  // Skip other updates while falling
    }


    if(_pos.x > WORLD_SIZE_X*3){
        _pos.x = WORLD_SIZE_X*3;
    }
    if(_pos.y > WORLD_SIZE_Y*3){
        _pos.y = WORLD_SIZE_Y*3;
    }
    if(_pos.x < 0){
        _pos.x = 0;
    }
    if(_pos.y < 0){
        _pos.y = 0;
    }

    // Rest of update code (movement, ghosts, etc.)
    if(_keys[GLFW_KEY_W]) {
        // Calculate new position
        glm::vec2 newPos = _pos;
        newPos.x += 0.15f * glm::sin(_direction);
        newPos.y += 0.15f * glm::cos(_direction);
        
        // Check for collisions
        if(!_checkCollisions(newPos)) {
            // No collision, update position
            _pos = newPos;
            _lastValidPosition = newPos;
            _lastDir = 0;
        } else {
            // Collision detected, revert to last valid position
            CollisionObject* col = CollisionDetector::getCollidedObject(newPos);
            bool x = false;
            bool z = false;
            if(_lastValidPosition.x > col->position.x + col->radius){
                z = true;
            }else if(_lastValidPosition.x < col->position.x - col->radius){
                z = true;
            } 
            
            if(_lastValidPosition.y > col->position.z + col->radius){
                x = true;
            }else if(_lastValidPosition.y < col->position.z - col->radius){
                x = true;
            }

            if(x && (_lastDir == 0 || _lastDir == 1)){
                _pos.x += 0.15f * glm::sin(_direction);
                _lastDir = 1;
            }
            if(z && (_lastDir == 0 || _lastDir == 2)){
                _pos.y += 0.15f * glm::cos(_direction);
                _lastDir = 2;
            }
            if(_checkCollisions(_pos)){
                _pos = _lastValidPosition;
                _lastDir = 0;
            }else{
                _lastValidPosition = _pos;
            }
        }

        // Check collisions with points
        for(PointsData& point : _points){
            float distance = glm::distance(_pos, point.position);
            if(distance<0.5){
                point.toBeDeleted = true;
            }
        }
        // delete points that have been collected
        _points.erase(
                std::remove_if(_points.begin(), _points.end(),
                                [](const PointsData& enemy) { return enemy.toBeDeleted; }),
                _points.end()
        );
        
        // Update camera position to follow behind plane
        glm::vec3 planePos = glm::vec3(_pos.x, 0, _pos.y);
        // Calculate camera offset based on plane's direction - now offset is in FRONT of plane
        glm::vec3 offset = glm::vec3(
            10.0f * glm::sin(_direction), // Removed negative
            5.0f,                                          // Height stays same
            10.0f * glm::cos(_direction)  // Removed negative
        );
        
        // Set camera position and look at point
        _pArcBall->setPosition(planePos + offset);
        _pArcBall->setLookAtPoint(planePos);
        _pArcBall->setTheta(-_direction); // Removed + pi
        _pArcBall->recomputeOrientation();
    }
    if(_keys[GLFW_KEY_S]){
        glm::vec2 newPos = _pos;
        newPos.x -= 0.15f * glm::sin(_direction);
        newPos.y -= 0.15f * glm::cos(_direction);
        
        if(!_checkCollisions(newPos)) {
            _pos = newPos;
            _lastValidPosition = newPos;
            _pColtonPlane->moveBackward();
        } else {
            _pos = _lastValidPosition;
        }
        
        // Update camera position to follow behind plane
        glm::vec3 planePos = glm::vec3(_pos.x, 0, _pos.y);
        glm::vec3 offset = glm::vec3(
            10.0f * glm::sin(_direction),
            5.0f,
            10.0f * glm::cos(_direction)
        );
        
        _pArcBall->setPosition(planePos + offset);
        _pArcBall->setLookAtPoint(planePos);
        _pArcBall->setTheta(-_direction);
        _pArcBall->recomputeOrientation();
    }

    //turn when A or D pressed
    if(_keys[GLFW_KEY_A]){
        _direction += 0.03f;
        
        // Update camera when turning
        glm::vec3 planePos = glm::vec3(_pos.x, 0, _pos.y);
        glm::vec3 offset = glm::vec3(
            10.0f * glm::sin(_direction),
            5.0f,
            10.0f * glm::cos(_direction)
        );
        
        _pArcBall->setPosition(planePos + offset);
        _pArcBall->setLookAtPoint(planePos);
        _pArcBall->setTheta(-_direction);
        _pArcBall->recomputeOrientation();
    }
    if(_keys[GLFW_KEY_D]){
        _direction -= 0.03f;
        
        // Update camera when turning
        glm::vec3 planePos = glm::vec3(_pos.x, 0, _pos.y);
        glm::vec3 offset = glm::vec3(
            10.0f * glm::sin(_direction),
            5.0f,
            10.0f * glm::cos(_direction)
        );
        
        _pArcBall->setPosition(planePos + offset);
        _pArcBall->setLookAtPoint(planePos);
        _pArcBall->setTheta(-_direction);
        _pArcBall->recomputeOrientation();
    }
    

    // Update point light position to be in front of and above the plane
    glm::vec3 planePos = glm::vec3(_pos.x, 0, _pos.y);
    glm::vec3 lightOffset = glm::vec3(
        -3.0f * glm::sin(_direction), // 3 units in front
        2.0f,                                          // 2 units above
        -3.0f * glm::cos(_direction)  // 3 units in front
    );
    
    // Update light position uniform
    glm::vec3 pointLightPosition = planePos + lightOffset;
    glProgramUniform3fv(_shaderProgram->getShaderProgramHandle(),
        _shaderUniformLocations.pointLightPosition,
        1,
        glm::value_ptr(pointLightPosition));

    // Make light brighter when moving forward
    glm::vec3 pointLightColor;
    if(_keys[GLFW_KEY_W]) {
        pointLightColor = glm::vec3(0.4f, 0.4f, 0.4f); // Brighter white when moving
    } else {
        pointLightColor = glm::vec3(0.2f, 0.2f, 0.2f); // Dimmer white when stationary
    }
    glProgramUniform3fv(_shaderProgram->getShaderProgramHandle(),
        _shaderUniformLocations.pointLightColor,
        1,
        glm::value_ptr(pointLightColor));

    // Update ghosts only if we're not falling or exploding
    if(!_isFalling && !_isExploding) {
        // _ghostManager->update(0.016f, glm::vec3(_pos.x, 
                                            //    _currentHeight, 
                                            //    _pos.y));
    }
}

void FPEngine::run() {
    while(!glfwWindowShouldClose(mpWindow)) {
        glDrawBuffer(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(mpWindow, &framebufferWidth, &framebufferHeight);

        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glm::mat4 projMtx = glm::perspective(45.0f, (GLfloat)framebufferWidth / (GLfloat)framebufferHeight, 0.001f, 1000.0f);
        glm::mat4 viewMtx = _pArcBall->getViewMatrix();

        // Render skybox first
        _renderSkybox(viewMtx, projMtx);
        
        // Then render scene
        _renderScene(viewMtx, projMtx);

        _updateScene();

        glfwSwapBuffers(mpWindow);
        glfwPollEvents();
    }
}

//*************************************************************************************
//
// Private Helper FUnctions

void FPEngine::_renderFPV(glm::mat4 projMtx) const {
    glm::vec3 position = glm::vec3(
        _pos.x + 1.0f * glm::sin(_direction),
        1.0f,
        _pos.y + 1.0f * glm::cos(_direction)
    );

    
    glm::vec3 forward = glm::vec3(
        glm::sin(_direction),
        0.0f,
        glm::cos(_direction)
    );
    
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 viewMtx = glm::lookAt(position, position + forward, up);
    projMtx = glm::perspective(glm::radians(65.0f),
                              (GLfloat)mWindowWidth / ((GLfloat)mWindowHeight), 
                              0.1f,
                              1000.0f);
    
    _renderScene(viewMtx, projMtx);
}



GLuint FPEngine::_loadAndRegisterTexture(const char* FILENAME) {
    // our handle to the GPU
    GLuint textureHandle = 0;

    // enable setting to prevent image from being upside down
    stbi_set_flip_vertically_on_load(true);

    // will hold image parameters after load
    GLint imageWidth, imageHeight, imageChannels;
    // load image from file
    GLubyte* data = stbi_load( FILENAME, &imageWidth, &imageHeight, &imageChannels, 0);

    // if data was read from file
    if( data ) {
        const GLint STORAGE_TYPE = (imageChannels == 4 ? GL_RGBA : GL_RGB);

        // TODO #01 - generate a texture handle
        glGenTextures(1, &textureHandle);

        // TODO #02 - bind it to be active
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        // set texture parameters
        // TODO #03 - mag filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // TODO #04 - min filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


        // TODO #05 - wrap s
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

        // TODO #06 - wrap t
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // TODO #07 - transfer image data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, STORAGE_TYPE, imageWidth, imageHeight, 0, STORAGE_TYPE, GL_UNSIGNED_BYTE, data);

        fprintf( stdout, "[INFO]: %s texture map read in with handle %d\n", FILENAME, textureHandle);

        // release image memory from CPU - it now lives on the GPU
        stbi_image_free(data);
    } else {
        // load failed
        fprintf( stderr, "[ERROR]: Could not load texture map \"%s\"\n", FILENAME );
    }

    // return generated texture handle
    return textureHandle;
}

//*************************************************************************************
//
// Callbacks

void fp_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods ) {
    auto engine = (FPEngine*) glfwGetWindowUserPointer(window);

    // pass the key and action through to the engine
    engine->handleKeyEvent(key, action);
}

void fp_cursor_callback(GLFWwindow *window, double x, double y ) {
    auto engine = (FPEngine*) glfwGetWindowUserPointer(window);

    // pass the cursor position through to the engine
    engine->handleCursorPositionEvent(glm::vec2(x, y));
}

void fp_mouse_button_callback(GLFWwindow *window, int button, int action, int mods ) {
    auto engine = (FPEngine*) glfwGetWindowUserPointer(window);

    // pass the mouse button and action through to the engine
    engine->handleMouseButtonEvent(button, action);
}

void fp_scroll_callback(GLFWwindow *window, double xOffset, double yOffset) {
    auto engine = (FPEngine*) glfwGetWindowUserPointer(window);

    // pass the scroll offset through to the engine
    engine->handleScrollEvent(glm::vec2(xOffset, yOffset));
}



void FPEngine::_setupSkybox() {
    float vertices[] = {
        // positions          // texture coords
        -100.0f,  100.0f, -100.0f,   0.0f, 1.0f,   // back face
        -100.0f, -100.0f, -100.0f,   0.0f, 0.0f,
         100.0f, -100.0f, -100.0f,   1.0f, 0.0f,
         100.0f, -100.0f, -100.0f,   1.0f, 0.0f,
         100.0f,  100.0f, -100.0f,   1.0f, 1.0f,
        -100.0f,  100.0f, -100.0f,   0.0f, 1.0f,

        -100.0f, -100.0f,  100.0f,   0.0f, 0.0f,   // front face
        -100.0f, -100.0f, -100.0f,   1.0f, 0.0f,
        -100.0f,  100.0f, -100.0f,   1.0f, 1.0f,
        -100.0f,  100.0f, -100.0f,   1.0f, 1.0f,
        -100.0f,  100.0f,  100.0f,   0.0f, 1.0f,
        -100.0f, -100.0f,  100.0f,   0.0f, 0.0f,

         100.0f, -100.0f, -100.0f,   1.0f, 0.0f,   // right face
         100.0f, -100.0f,  100.0f,   0.0f, 0.0f,
         100.0f,  100.0f,  100.0f,   0.0f, 1.0f,
         100.0f,  100.0f,  100.0f,   0.0f, 1.0f,
         100.0f,  100.0f, -100.0f,   1.0f, 1.0f,
         100.0f, -100.0f, -100.0f,   1.0f, 0.0f,

        -100.0f, -100.0f,  100.0f,   0.0f, 0.0f,   // left face
        -100.0f,  100.0f,  100.0f,   0.0f, 1.0f,
         100.0f,  100.0f,  100.0f,   1.0f, 1.0f,
         100.0f,  100.0f,  100.0f,   1.0f, 1.0f,
         100.0f, -100.0f,  100.0f,   1.0f, 0.0f,
        -100.0f, -100.0f,  100.0f,   0.0f, 0.0f,

        -100.0f,  100.0f, -100.0f,   0.0f, 1.0f,   // top face
         100.0f,  100.0f, -100.0f,   1.0f, 1.0f,
         100.0f,  100.0f,  100.0f,   1.0f, 0.0f,
         100.0f,  100.0f,  100.0f,   1.0f, 0.0f,
        -100.0f,  100.0f,  100.0f,   0.0f, 0.0f,
        -100.0f,  100.0f, -100.0f,   0.0f, 1.0f,

        -100.0f, -100.0f, -100.0f,   0.0f, 0.0f,   // bottom face
        -100.0f, -100.0f,  100.0f,   0.0f, 1.0f,
         100.0f, -100.0f, -100.0f,   1.0f, 0.0f,
         100.0f, -100.0f, -100.0f,   1.0f, 0.0f,
        -100.0f, -100.0f,  100.0f,   0.0f, 1.0f,
         100.0f, -100.0f,  100.0f,   1.0f, 1.0f
    };

    glGenVertexArrays(1, &_skyboxVAO);
    glBindVertexArray(_skyboxVAO);

    glGenBuffers(1, &_skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    _skyboxShader = new CSCI441::ShaderProgram("shaders/skybox.v.glsl", "shaders/skybox.f.glsl");
    _skyboxUniformLocations.view = _skyboxShader->getUniformLocation("view");
    _skyboxUniformLocations.projection = _skyboxShader->getUniformLocation("projection");
    _skyboxUniformLocations.skyTexture = _skyboxShader->getUniformLocation("skyTexture");

    _skyboxShader->useProgram();
    _skyboxShader->setProgramUniform(_skyboxUniformLocations.skyTexture, 0);
}

void FPEngine::_renderSkybox(const glm::mat4& view, const glm::mat4& projection) const {
    glDepthFunc(GL_LEQUAL);
    _skyboxShader->useProgram();
    
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    
    _skyboxShader->setProgramUniform(_skyboxUniformLocations.view, skyboxView);
    _skyboxShader->setProgramUniform(_skyboxUniformLocations.projection, projection);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _skyTexture);
    
    glBindVertexArray(_skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glDepthFunc(GL_LESS);
}

bool FPEngine::_checkCollisions(const glm::vec2& newPosition) {
    bool collided = CollisionDetector::checkCollision(
        newPosition, 
        _playerRadius
    );
    
    if(collided) {
        fprintf(stdout, "[COLLISION] Collision detected at position (%.2f, %.2f)\n", 
            newPosition.x, newPosition.y);
            
        // Print nearby objects for debugging
        for(const auto& obj : CollisionDetector::getCollisionObjects()) {
            float distance = glm::length(
                glm::vec2(obj.position.x, obj.position.z) - newPosition
            );
            if(distance < 5.0f) { // Only show nearby objects
                fprintf(stdout, "  - Nearby %s at (%.2f, %.2f) distance: %.2f\n", 
                    obj.isTree ? "tree" : "building",
                    obj.position.x, 
                    obj.position.z,
                    distance);
            }
        }
    }
    
    return collided;
}
