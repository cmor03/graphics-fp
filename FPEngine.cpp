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
     : CSCI441::OpenGLEngine(4, 1, WINDOW_WIDTH, WINDOW_HEIGHT, "FP") {

    for(auto& _key : _keys) _key = GL_FALSE;

    _mousePosition = glm::vec2(MOUSE_UNINITIALIZED, MOUSE_UNINITIALIZED );
    _leftMouseButtonState = GLFW_RELEASE;
    _direction = 0;
    _phi = 0;
    _pos = glm::vec2(3,3);
    _playerRadius = 0.5f;
    _lastDir = 0;
    _hitTimer = 0;
    _ghostFreezeTimer = 0.0f;
    const float GHOST_FREEZE_DURATION = 10.0f;  // 10 seconds
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

void FPEngine::handleCursorPositionEvent(GLFWwindow* window, glm::vec2 currMousePosition) {
    // if mouse hasn't moved in the window, prevent camera from flipping out
    if(fabs(_mousePosition.x - MOUSE_UNINITIALIZED) <= 0.000001f) {
        _mousePosition = currMousePosition;
    }

    _direction += (_mousePosition.x - currMousePosition.x) * 0.005;
    _phi += (_mousePosition.y - currMousePosition.y) * 0.005;
    if (_phi > M_PI/2 - 0.001){
        _phi = M_PI/2 - 0.001;
    }
    if (_phi < -M_PI/2 + 0.001){
        _phi = -M_PI/2 + 0.001;
    }
    // update the mouse position
    _mousePosition = currMousePosition;
}

void FPEngine::handleScrollEvent(glm::vec2 offset) {
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
    glfwSetInputMode(mpWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


}

void FPEngine::mSetupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                    // enable depth testing
    glDepthFunc( GL_LESS );							                // use less than depth test

    glEnable(GL_BLEND);									            // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black
    
}

void FPEngine::mSetupShaders() {
    _shaderProgram = new CSCI441::ShaderProgram("shaders/fp.v.glsl", "shaders/fp.f.glsl" );
    _slenderShaderProgram = new CSCI441::ShaderProgram("shaders/slendershader.v.glsl", "shaders/slendershader.f.glsl" );

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
    _shaderUniformLocations.time      = _slenderShaderProgram->getUniformLocation("time");

    _shaderAttributeLocations.vPos         = _shaderProgram->getAttributeLocation("vPos");
    _shaderAttributeLocations.normalVec      = _shaderProgram->getAttributeLocation("normalVec");
    _shaderAttributeLocations.inTexCoord      = _shaderProgram->getAttributeLocation("inTexCoord");

    // query uniform locations for slender shader separately because linux and mac compiler doesnt optimize and store them at the same location like windows
    _slenderShaderUniformLocations.mvpMatrix      = _slenderShaderProgram->getUniformLocation("mvpMatrix");
    _slenderShaderUniformLocations.lightDirection      = _slenderShaderProgram->getUniformLocation("lightDirection");
    _slenderShaderUniformLocations.directionalLightColor      = _slenderShaderProgram->getUniformLocation("directionalLightColor");
    _slenderShaderUniformLocations.pointLightPosition      = _slenderShaderProgram->getUniformLocation("pointLightPosition");
    _slenderShaderUniformLocations.pointLightColor      = _slenderShaderProgram->getUniformLocation("pointLightColor");
    _slenderShaderUniformLocations.materialColor      = _slenderShaderProgram->getUniformLocation("materialColor");
    _slenderShaderUniformLocations.normalMatrix      = _slenderShaderProgram->getUniformLocation("normalMatrix");
    _slenderShaderUniformLocations.viewVector      = _slenderShaderProgram->getUniformLocation("viewVector");
    _slenderShaderUniformLocations.textureMap      = _slenderShaderProgram->getUniformLocation("textureMap");
    _slenderShaderUniformLocations.time      = _slenderShaderProgram->getUniformLocation("time");

    _slenderShaderAttributeLocations.vPos         = _slenderShaderProgram->getAttributeLocation("vPos");
    _slenderShaderAttributeLocations.normalVec      = _slenderShaderProgram->getAttributeLocation("normalVec");
    _slenderShaderAttributeLocations.inTexCoord      = _slenderShaderProgram->getAttributeLocation("inTexCoord");

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

}

void FPEngine::_createPlatform(GLuint vao, GLuint vbo, GLuint ibo, GLsizei &numVAOPoints) const {
    struct VertexNormalTextured {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // create our platform
    VertexNormalTextured platformVertices[4] = {
            { { -1.5, 1.0f, -1.5 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }, // 0 - BL
            { {  WORLD_SIZE_X*3-1.5, 1.0f, -1.5 }, { 0.0f, 1.0f, 0.0f }, { 10.0f, 0.0f } }, // 1 - BR
            { { -1.5, 1.0f,  WORLD_SIZE_Y*3-1.5}, { 0.0f, 1.0f, 0.0f }, { 0.0f, 10.0f } }, // 2 - TL
            { {  WORLD_SIZE_X*3-1.5, 1.0f,  WORLD_SIZE_Y*3-1.5 }, { 0.0f, 1.0f, 0.0f }, { 10.0f, 10.0f } }  // 3 - TR
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

    // Modified UV coordinates to map to entire texture (changed from 3.0 to 1.0)
    VertexNormalTextured quadVertices[4] = {
        { { -2.5f, -2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }, // 0 - BL
        { {  2.5f, -2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } }, // 1 - BR
        { { -2.5f,  2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }, // 2 - TL
        { {  2.5f,  2.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } }  // 3 - TR
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
    _texHandles[TEXTURE_ID::GROUND] = _loadAndRegisterTexture("assets/textures/dirt.png");
    _texHandles[TEXTURE_ID::BUILDING] = _loadAndRegisterTexture("assets/textures/wall.jpg");
    _texHandles[TEXTURE_ID::GHOST] = _loadAndRegisterTexture("assets/textures/ghost.jpeg");
    _texHandles[TEXTURE_ID::LAVA] = _loadAndRegisterTexture("assets/textures/lava.jpg");
    _texHandles[TEXTURE_ID::BLOOD] = _loadAndRegisterTexture("assets/textures/blood.jpg");
    _texHandles[TEXTURE_ID::SKY] = _loadAndRegisterTexture("assets/textures/skybox.png");
    
    fprintf(stdout, "[INFO]: Skybox texture handle: %d\n", _texHandles[TEXTURE_ID::SKY]);
}

void FPEngine::mSetupScene() {
    _objectIndex = 0;
    _objectAngle = 0.0f;

    glm::vec3 directionalLightColor = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDirection = glm::vec3(-1,-1,-1);
    glm::vec3 viewVector = glm::vec3(1,0,0);
    
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
}

void FPEngine::mCleanupTextures() {
    fprintf( stdout, "[INFO]: ...deleting textures\n" );
    // TODO #23 - delete textures
    glDeleteTextures(4, _texHandles);

}

void FPEngine::mCleanupScene() {
    fprintf(stdout, "[INFO]: ...deleting scene...\n");
    
    // Cleanup skybox resources
    delete _skyboxShader;
    glDeleteVertexArrays(1, &_skyboxVAO);
    glDeleteBuffers(1, &_skyboxVBO);
    glDeleteTextures(1, &_texHandles[TEXTURE_ID::SKY]);

    delete _particleSystem;
    for(CarData& carData : _carData) {
        delete carData.car;
    }
    _carData.clear();
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

    world_matrix = read_csv("world.csv");
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
            else if(world_matrix[i][j]==2){
                glm::mat4 transToHeight = glm::translate( glm::mat4(1.0), glm::vec3(0, 1.0f, 0) );
                glm::mat4 modelMatrix = transToSpotMtx * transToHeight;
                glm::vec3 color(1,1,1);
                Ghost ghost = {modelMatrix, color, glm::vec2(i,j),glm::vec2(i,j),glm::vec2(i,j), glm::vec2(i,j), GHOST_SPEED, 1, false};
                _ghosts.emplace_back(ghost);
            }
            else if(world_matrix[i][j]==3){
                glm::mat4 transToHeight = glm::translate( glm::mat4(1.0), glm::vec3(0, 1.0f, 0) );
                glm::mat4 modelMatrix = transToSpotMtx * transToHeight;
                
                Plane* car = new Plane(_shaderProgram->getShaderProgramHandle(),
                                  _shaderUniformLocations.mvpMatrix,
                                  _shaderUniformLocations.normalMatrix,
                                  _shaderUniformLocations.materialColor);
                
                CarData carData = {
                    car,
                    glm::vec2(i*3, j*3),
                    false
                };
                _carData.push_back(carData);
            }
        }
    }

}
//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void FPEngine::_renderScene(glm::mat4 viewMtx, glm::mat4 projMtx) const {
    // shader uniforms and attribues
    CSCI441::ShaderProgram* shader;
    TextureShaderUniformLocations uniforms;
    TextureShaderAttributeLocations attributes;

    //set shader program, uniforms and attributes based on whether we are using the glitched or normal shader
    if(_hitTimer>0 && !_isExploding){
        shader = _slenderShaderProgram;
        uniforms = _slenderShaderUniformLocations;
        attributes = _slenderShaderAttributeLocations;
        glProgramUniform1i(_slenderShaderProgram->getShaderProgramHandle(),_shaderUniformLocations.time,_hitTimer);
    }
    else {
        shader = _shaderProgram;
        uniforms = _shaderUniformLocations;
        attributes = _shaderAttributeLocations;
    }
    shader->useProgram();

    glm::vec3 defaultColor = glm::vec3(-1,-1,-1);
    glProgramUniform3fv(shader->getShaderProgramHandle(), uniforms.materialColor, 1, glm::value_ptr(defaultColor));
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.1f, 0.0f));
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMatrix;
    shader->setProgramUniform(uniforms.mvpMatrix, mvpMtx);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
    shader->setProgramUniform(uniforms.normalMatrix, normalMatrix);


    glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::GROUND]);

    glBindVertexArray( _vaos[VAO_ID::PLATFORM] );
    glDrawElements( GL_TRIANGLE_STRIP, _numVAOPoints[VAO_ID::PLATFORM], GL_UNSIGNED_SHORT, (void*)nullptr );

    for( const BuildingData& currentBuilding : _buildings ) {
        glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::BUILDING]);
        mvpMtx = projMtx * viewMtx * currentBuilding.modelMatrix;
        shader->setProgramUniform(uniforms.mvpMatrix, mvpMtx);
        CSCI441::drawSolidCubeTextured(1.0);
    }
    glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::LAVA]);
    for( const PointsData& currentPoint : _points){

        mvpMtx = projMtx * viewMtx * currentPoint.modelMatrix;
        shader->setProgramUniform(uniforms.mvpMatrix, mvpMtx);
        CSCI441::drawSolidSphere(0.2,8,8);
    }

    for (const Ghost& ghost : _ghosts) {
        // Calculate billboard matrix
        glm::vec3 ghostPos = glm::vec3(ghost.current_pos.x*3, 1.0f, ghost.current_pos.y*3);
        glm::mat4 billboardModel = _createBillboardMatrix(ghostPos, viewMtx);
        
        // Set uniforms
        glm::mat4 mvpMtx = projMtx * viewMtx * billboardModel;
        shader->setProgramUniform(uniforms.mvpMatrix, mvpMtx);
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(billboardModel)));
        shader->setProgramUniform(uniforms.normalMatrix, normalMatrix);
        
        // Bind ghost texture
        glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::GHOST]);
        
        // Draw billboard quad
        glBindVertexArray(_vaos[VAO_ID::QUAD]);
        glDrawElements(GL_TRIANGLE_STRIP, _numVAOPoints[VAO_ID::QUAD], GL_UNSIGNED_SHORT, (void*)nullptr);
    }

    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.1f, 0.0f));
    modelMatrix = glm::rotate( modelMatrix, _objectAngle, CSCI441::Y_AXIS );
    mvpMtx = projMtx * viewMtx * modelMatrix;
    shader->setProgramUniform(uniforms.mvpMatrix, mvpMtx);

    glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::BLOOD]);
    if(_isExploding) {
        _particleSystem->draw(viewMtx, projMtx);
    }
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.f));

    // Draw static car
    for(const CarData& carData : _carData) {
        if(!carData.collected) {
            glm::mat4 carModelMatrix = glm::translate(glm::mat4(1.0f), 
                glm::vec3(carData.position.x, 0.0f, carData.position.y));
            carData.car->drawPlane(carModelMatrix, viewMtx, projMtx);
        }
    }
}

void FPEngine::_updateScene() {

    //set shader program, uniforms and attributes based on whether we are using the glitched or normal shader
    CSCI441::ShaderProgram* shader;
    TextureShaderUniformLocations uniforms;
    TextureShaderAttributeLocations attributes;
    if(_hitTimer>0){
        shader = _slenderShaderProgram;
        uniforms = _slenderShaderUniformLocations;
        attributes = _slenderShaderAttributeLocations;
        glProgramUniform1f(_slenderShaderProgram->getShaderProgramHandle(),_shaderUniformLocations.time,_hitTimer);
        _hitTimer--;
    }
    else {
        shader = _shaderProgram;
        uniforms = _shaderUniformLocations;
        attributes = _shaderAttributeLocations;
    }
    // Handle ghost collision explosion
    if(NUM_LIVES<=0 && !_isExploding) {
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
            NUM_LIVES = 5;
            generate_points();
        }
        return;
    }



    if(_pos.x > WORLD_SIZE_X*3-3.1){
        _pos.x = WORLD_SIZE_X*3-3;
    }
    if(_pos.y > WORLD_SIZE_Y*3-3.1){
        _pos.y = WORLD_SIZE_Y*3-3;
    }
    if(_pos.x < 0){
        _pos.x = 0;
    }
    if(_pos.y < 0){
        _pos.y = 0;
    }
    //get player position in grid
    glm::vec2 player_aligned_pos = glm::vec2(std::round(_pos.x/3.0f), std::round(_pos.y/3.0f));
    //fprintf(stdout, "player aligned position: (%f,%f)\n", player_aligned_pos.x, player_aligned_pos.y);
    // move ghosts
    for( Ghost& ghost : _ghosts){
        // Skip ghost movement if frozen
        if (_ghostFreezeTimer > 0) {
            continue;  // Skip the movement code while frozen
        }

        if (!ghost.is_moving) {
            ghost.start_pos = ghost.current_pos;
            ghost.target_pos = findBestMove(world_matrix, ghost.current_pos,player_aligned_pos);

            // Only start moving if target is different
            if (ghost.target_pos != ghost.start_pos) {
                ghost.is_moving = true;
                ghost.progress = 0.0f;
            }
        }
        // Update movement progress
        if (ghost.is_moving) {
            ghost.progress += GHOST_SPEED;

            // Interpolate position
            ghost.current_pos = glm::mix(ghost.start_pos, ghost.target_pos, glm::clamp(ghost.progress, 0.0f, 1.0f));

            // Check if movement is complete
            if (ghost.progress >= 1.0f) {
                ghost.current_pos = ghost.target_pos;
                ghost.is_moving = false;
            }
        }
    }
    //check collisions with ghosts
    for(Ghost& ghost : _ghosts){
        float distance = glm::distance(_pos,glm::vec2(ghost.current_pos.x*3, ghost.current_pos.y*3));
        if(distance<1.2){
            NUM_LIVES-=1;
            ghost.current_pos=ghost.spawn_pos;
            ghost.start_pos=ghost.spawn_pos;
            ghost.target_pos = ghost.spawn_pos;

            fprintf(stdout,"You have been hit by a ghost, %d lives remaining\n",NUM_LIVES);
            _hitTimer = 120;
        }
    }

    if(NUM_LIVES<=0){
        fprintf(stdout,"You have lost all of your lives, you lose.\n");
        //_isExploding = true;
    }
    // Check collisions with points
    for(PointsData& point : _points){
        float distance = glm::distance(_pos, point.position);
        if(distance<1){
            point.toBeDeleted = true;
        }
    }

    // delete points that have been collected
    _points.erase(
            std::remove_if(_points.begin(), _points.end(),
                           [](const PointsData& point) { return point.toBeDeleted; }),
            _points.end()
    );

    //check to see if all points have been collected
    if(_points.size()==0){
        fprintf(stdout,"You have collected all of the points, congratulations!");
        generate_points();
        GHOST_SPEED += 0.005;
    }


    float speed = 0.15f;

    // Rest of update code (movement, ghosts, etc.)
    if(_keys[GLFW_KEY_W]) {
        // Calculate new position
        glm::vec2 newPos = _pos;
        newPos.x += speed * glm::sin(_direction);
        newPos.y += speed * glm::cos(_direction);
        
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
                _pos.x += speed * glm::sin(_direction);
                _lastDir = 1;
            }
            if(z && (_lastDir == 0 || _lastDir == 2)){
                _pos.y += speed * glm::cos(_direction);
                _lastDir = 2;
            }
            if(_checkCollisions(_pos)){
                _pos = _lastValidPosition;
                _lastDir = 0;
            }else{
                _lastValidPosition = _pos;
            }
        }


    }
    if(_keys[GLFW_KEY_S]){
        glm::vec2 newPos = _pos;
        newPos.x -= speed * glm::sin(_direction);
        newPos.y -= speed * glm::cos(_direction);
        
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
                _pos.x -= speed * glm::sin(_direction);
                _lastDir = 1;
            }
            if(z && (_lastDir == 0 || _lastDir == 2)){
                _pos.y -= speed * glm::cos(_direction);
                _lastDir = 2;
            }
            if(_checkCollisions(_pos)){
                _pos = _lastValidPosition;
                _lastDir = 0;
            }else{
                _lastValidPosition = _pos;
            }
        }
    }

    //turn when A or D pressed
    if(_keys[GLFW_KEY_A]){
        // Calculate new position
        glm::vec2 newPos = _pos;
        newPos.x += speed * glm::sin(_direction + M_PI/2);
        newPos.y += speed * glm::cos(_direction + M_PI/2);
        
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
                _pos.x += speed * glm::sin(_direction + M_PI/2);
                _lastDir = 1;
            }
            if(z && (_lastDir == 0 || _lastDir == 2)){
                _pos.y += speed * glm::cos(_direction + M_PI/2);
                _lastDir = 2;
            }
            if(_checkCollisions(_pos)){
                _pos = _lastValidPosition;
                _lastDir = 0;
            }else{
                _lastValidPosition = _pos;
            }
        }


    }
    if(_keys[GLFW_KEY_D]){
        // Calculate new position
        glm::vec2 newPos = _pos;
        newPos.x += speed * glm::sin(_direction - M_PI/2);
        newPos.y += speed * glm::cos(_direction - M_PI/2);
        
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
                _pos.x += speed * glm::sin(_direction - M_PI/2);
                _lastDir = 1;
            }
            if(z && (_lastDir == 0 || _lastDir == 2)){
                _pos.y += speed * glm::cos(_direction - M_PI/2);
                _lastDir = 2;
            }
            if(_checkCollisions(_pos)){
                _pos = _lastValidPosition;
                _lastDir = 0;
            }else{
                _lastValidPosition = _pos;
            }
        }


    }
    

    // Update point light position to be in front of and above the plane
    glm::vec3 planePos = glm::vec3(_pos.x+1.5, 0, _pos.y+3);
    glm::vec3 lightOffset = glm::vec3(
        -3.0f * glm::sin(_direction), // 3 units in front
        2.0f,                                          // 2 units above
        -3.0f * glm::cos(_direction)  // 3 units in front
    );
    
    // Update light position uniform
    glm::vec3 pointLightPosition = planePos + lightOffset;
    glProgramUniform3fv(shader->getShaderProgramHandle(),
        uniforms.pointLightPosition,
        1,
        glm::value_ptr(pointLightPosition));

    // Make light brighter when moving forward
    glm::vec3 pointLightColor;
    if(_keys[GLFW_KEY_W]) {
        pointLightColor = glm::vec3(0.4f, 0.4f, 0.4f); // Brighter white when moving
    } else {
        pointLightColor = glm::vec3(0.2f, 0.2f, 0.2f); // Dimmer white when stationary
    }
    glProgramUniform3fv(shader->getShaderProgramHandle(),
        uniforms.pointLightColor,
        1,
        glm::value_ptr(pointLightColor));

    // Update ghosts only if we're not falling or exploding
    if(!_isFalling && !_isExploding) {
    }

    // Add car collection logic
    for(CarData& carData : _carData) {
        if(!carData.collected) {
            float distance = glm::distance(_pos, carData.position);
            if(distance < 2.0f) {  // Adjust collision radius as needed
                carData.collected = true;
                _ghostFreezeTimer = GHOST_FREEZE_DURATION;  // Start freeze timer
                fprintf(stdout, "Ghosts frozen for 10 seconds!\n");
            }
        }
    }

    if (_ghostFreezeTimer > 0) {
        _ghostFreezeTimer -= 0.016f;  // Decrease timer (assuming 60 FPS)
        if (_ghostFreezeTimer <= 0) {
            _ghostFreezeTimer = 0;
            fprintf(stdout, "Ghosts unfrozen!\n");
        }
    }
}

std::vector<glm::vec2> getPossibleMoves(
        const std::vector<std::vector<int>> world_matrix,
        const glm::vec2 current_pos
) {
    // Potential adjacent moves (right, left, down, up)
    std::vector<glm::vec2> possible_moves = {
            current_pos + glm::vec2(1, 0),   // right
            current_pos + glm::vec2(-1, 0),  // left
            current_pos + glm::vec2(0, 1),   // down
            current_pos + glm::vec2(0, -1)   // up
    };

    std::vector<glm::vec2> actually_possible_moves;
    // Filter out invalid moves
    for(glm::vec2 move : possible_moves){
        //fprintf(stdout,"looking at move (%f,%f)\n",move.x,move.y);
        if(!(move.x<0 || move.y<0 || move.x>world_matrix[0].size()*3 || move.y>world_matrix.size()*3)&&world_matrix[move.x][move.y]!=1){
            actually_possible_moves.emplace_back(move);
        }
    }

    return actually_possible_moves;
}

void FPEngine::generate_points() {
    _points.clear();
    for(int i=0;i<WORLD_SIZE_X;i++){
        fprintf(stdout,"\n");
        for(int j=0;j<WORLD_SIZE_Y;j++){
            fprintf(stdout,"%d",world_matrix[i][j]);
            glm::mat4 transToSpotMtx = glm::translate( glm::mat4(1.0), glm::vec3(i*3, 0.0f, j*3));
            if(world_matrix[i][j]==0) {
                glm::mat4 transToHeight = glm::translate( glm::mat4(1.0), glm::vec3(0, 1.0f, 0) );
                glm::mat4 modelMatrix = transToSpotMtx * transToHeight;
                glm::vec3 color(1,1,1);
                PointsData currentPoint  ={modelMatrix, color, glm::vec2(i*3, j*3), false};
                _points.emplace_back(currentPoint);
            }
        }

    }
}

void FPEngine::run() {
    while(!glfwWindowShouldClose(mpWindow)) {
        glDrawBuffer(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(mpWindow, &framebufferWidth, &framebufferHeight);

        glViewport(0, 0, framebufferWidth, framebufferHeight);

        glm::mat4 projMtx = glm::perspective(30.0f, (GLfloat)framebufferWidth / (GLfloat)framebufferHeight, 0.001f, 1000.0f);
        glm::vec3 position = glm::vec3(
            _pos.x,
            1.5f,
            _pos.y
        );
        glm::vec3 forward = glm::vec3(
            glm::cos(_phi) * glm::sin(_direction),
            glm::sin(_phi),
            glm::cos(_phi) * glm::cos(_direction)
        );

        glProgramUniform3fv(_shaderProgram->getShaderProgramHandle(),
            _shaderUniformLocations.viewVector,
            1,
            glm::value_ptr(glm::normalize(forward)));

        glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::mat4 viewMtx = glm::lookAt(position, position + forward, up);
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
    GLubyte* data = stbi_load(FILENAME, &imageWidth, &imageHeight, &imageChannels, 0);

    // if data was read from file
    if(data) {
        const GLint STORAGE_TYPE = (imageChannels == 4 ? GL_RGBA : GL_RGB);

        glGenTextures(1, &textureHandle);
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        // Check if this is the ghost texture
        if(std::string(FILENAME).find("ghost.jpeg") != std::string::npos) {
            // Special settings for ghost texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
            // Default settings for other textures
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, STORAGE_TYPE, imageWidth, imageHeight, 0, STORAGE_TYPE, GL_UNSIGNED_BYTE, data);

        fprintf(stdout, "[INFO]: %s texture map read in with handle %d\n", FILENAME, textureHandle);
        stbi_image_free(data);
    } else {
        fprintf(stderr, "[ERROR]: Could not load texture map \"%s\"\n", FILENAME);
    }

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
    engine->handleCursorPositionEvent(window, glm::vec2(x, y));
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
    glBindTexture(GL_TEXTURE_2D, _texHandles[TEXTURE_ID::SKY]);
    
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
            
        // Print nearby objects for debugging
        for(const auto& obj : CollisionDetector::getCollisionObjects()) {
            float distance = glm::length(
                glm::vec2(obj.position.x, obj.position.z) - newPosition
            );
            if(distance < 5.0f) { // Only show nearby objects
            }
        }
    }
    
    return collided;
}

glm::vec2 FPEngine::findBestMove(std::vector<std::vector<int>> world_matrix, glm::vec2 ghost_pos, glm::vec2 player_pos) {
    // Get possible moves
    std::vector<glm::vec2> possible_moves = getPossibleMoves(world_matrix, ghost_pos);
    //fprintf(stdout, "looking for new move for ghost, play pos (%f,%f)\n",player_pos.x,player_pos.y);
    // If no moves possible, stay in place
    if (possible_moves.empty()) {
        //fprintf(stdout,"no possible moves for ghost\n");
        return ghost_pos;
    }
    glm::vec2 target;
    float minimum = 1000000000;
    for(glm::vec2 move : possible_moves){
        float distance = glm::distance(move,player_pos);
        if(distance<minimum){
            minimum=distance;
            target=move;
        }
    }
    return target;
}

glm::mat4 FPEngine::_createBillboardMatrix(const glm::vec3& position, const glm::mat4& viewMatrix) const {
    // Extract camera right and up vectors from view matrix
    glm::vec3 right = glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    glm::vec3 up = glm::vec3(0, 1, 0);
    
    // Scale for the billboard size - reduced from 2.0f to 1.0f
    const float BILLBOARD_SIZE = 1.0f;
    
    // Create billboard model matrix
    glm::mat4 billboardModel = glm::mat4(1.0f);
    billboardModel[0] = glm::vec4(right * BILLBOARD_SIZE, 0.0f);
    billboardModel[1] = glm::vec4(up * BILLBOARD_SIZE, 0.0f);
    billboardModel[2] = glm::vec4(glm::cross(right, up) * BILLBOARD_SIZE, 0.0f);
    billboardModel[3] = glm::vec4(position, 1.0f);
    
    return billboardModel;
}

