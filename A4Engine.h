#ifndef A4_ENGINE_H
#define A4_ENGINE_H

#include <CSCI441/FixedCam.hpp>
#include <CSCI441/FreeCam.hpp>
#include <CSCI441/MD5Model.hpp>
#include <CSCI441/ModelLoader.hpp>
#include <CSCI441/OpenGLEngine.hpp>
#include <CSCI441/ShaderProgram.hpp>
#include "ArcBall.h"
#include "Plane.h"
#include "CollisionDetector.h"
#include "Ghost.h"
#include "ParticleSystem.h"


class A4Engine final : public CSCI441::OpenGLEngine {
public:
    //***************************************************************************
    // Engine Interface
    A4Engine();

    void run() final;

    //***************************************************************************
    // Event Handlers

    /// \desc handle any key events inside the engine
    /// \param key key as represented by GLFW_KEY_ macros
    /// \param action key event action as represented by GLFW_ macros
    void handleKeyEvent(GLint key, GLint action);

    /// \desc handle any mouse button events inside the engine
    /// \param button mouse button as represented by GLFW_MOUSE_BUTTON_ macros
    /// \param action mouse event as represented by GLFW_ macros
    void handleMouseButtonEvent(GLint button, GLint action);

    /// \desc handle any cursor movement events inside the engine
    /// \param currMousePosition the current cursor position
    void handleCursorPositionEvent(glm::vec2 currMousePosition);

    /// \desc handle any scroll events inside the engine
    /// \param offset the current scroll offset
    void handleScrollEvent(glm::vec2 offset);

    /// \desc value off-screen to represent mouse has not begun interacting with window yet
    static constexpr GLfloat MOUSE_UNINITIALIZED = -9999.0f;

private:
    //***************************************************************************
    // Engine Setup
    void mSetupGLFW() final;
    void mSetupOpenGL() final;
    void mSetupShaders() final;
    void mSetupBuffers() final;
    void mSetupTextures() final;
    void mSetupScene() final;

    //***************************************************************************
    // Engine Cleanup
    void mCleanupScene() final;
    void mCleanupTextures() final;
    void mCleanupBuffers() final;
    void mCleanupShaders() final;

    //***************************************************************************
    // Engine Rendering & Updating

    /// \desc draws everything to the scene from a particular point of view
    /// \param viewMtx the current view matrix for our camera
    /// \param projMtx the current projection matrix for our camera
    void _renderScene(glm::mat4 viewMtx, glm::mat4 projMtx) const;
    /// \desc handles moving our FreeCam as determined by keyboard input
    void _updateScene();

    //***************************************************************************
    // Input Tracking (Keyboard & Mouse)

    /// \desc tracks the number of different keys that can be present as determined by GLFW
    static constexpr GLuint NUM_KEYS = GLFW_KEY_LAST;
    /// \desc boolean array tracking each key state.  if true, then the key is in a pressed or held
    /// down state.  if false, then the key is in a released state and not being interacted with
    GLboolean _keys[NUM_KEYS];

    /// \desc last location of the mouse in window coordinates
    glm::vec2 _mousePosition;
    /// \desc current state of the left mouse button
    GLint _leftMouseButtonState;

    //***************************************************************************
    // Camera Information

    /// \desc the arcball camera in our world
    ArcBall* _pArcBall;
    GLfloat _direction[3];
    glm::vec2 _pos[3];
    GLint _currentVehicle;

    /// \desc the freecam in our world
    CSCI441::FreeCam* _pFreeCam;
    glm::vec2 _cameraSpeed;
    bool freeCam;

    //vehicles
    Plane* _pColtonPlane;

    GLfloat WORLD_SIZE_X = 60.0f;
    GLfloat WORLD_SIZE_Y = 60.0f;



    //***************************************************************************
    // VAO & Object Information

    /// \desc total number of VAOs in our scene
    static constexpr GLuint NUM_VAOS = 2;
    /// \desc used to index through our VAO/VBO/IBO array to give named access
    enum VAO_ID {
        /// \desc the platform that represents our ground for everything to appear on
        PLATFORM = 0,
        /// \desc the quad that we'll create to apply a texture to
        QUAD = 1
    };
    /// \desc VAO for our objects
    GLuint _vaos[NUM_VAOS];
    /// \desc VBO for our objects
    GLuint _vbos[NUM_VAOS];
    /// \desc IBO for our objects
    GLuint _ibos[NUM_VAOS];
    /// \desc the number of points that make up our VAO
    GLsizei _numVAOPoints[NUM_VAOS];

    /// \desc creates the platform object
    /// \param [in] vao VAO descriptor to bind
    /// \param [in] vbo VBO descriptor to bind
    /// \param [in] ibo IBO descriptor to bind
    /// \param [out] numVAOPoints sets the number of vertices that make up the IBO array
    void _createPlatform(GLuint vao, GLuint vbo, GLuint ibo, GLsizei &numVAOPoints) const;

    /// \desc creates the textured quad object
    /// \param [in] vao VAO descriptor to bind
    /// \param [in] vbo VBO descriptor to bind
    /// \param [in] ibo IBO descriptor to bind
    /// \param [out] numVAOPoints sets the number of vertices that make up the IBO array
    void _createQuad(GLuint vao, GLuint vbo, GLuint ibo, GLsizei &numVAOPoints) const;

    /// \desc holds our suzanne model object
    CSCI441::ModelLoader* _pObjModel;

    /// \desc holds our hellknight object
    CSCI441::MD5Model* _pHellknight;

    /// \desc tracks which object we want to be viewing
    GLuint _objectIndex;
    /// \desc the current angle of rotation to display our object at
    GLfloat _objectAngle;
    /// \desc rate at which the objects rotate
    static constexpr GLfloat ROTATION_SPEED = 0.01f;

    /// \desc time last frame was rendered
    GLfloat _lastTime;
    /// \desc time current frame was rendered
    GLfloat _currTime;

    struct BuildingData {
        /// \desc transformations to position and size the building
        glm::mat4 modelMatrix;
        /// \desc color to draw the building
        glm::vec3 color;

    };
    /// \desc information list of all the buildings to draw
    std::vector<BuildingData> _buildings;

    struct PointsData{
        glm::mat4 modelMatrix;

        glm::vec3 color;
        glm::vec2 position;

        bool toBeDeleted;
    };

    std::vector<PointsData> _points;

    void _drawTree();

    //***************************************************************************
    // Texture Information

    // TODO #08-START this step has been done for you, but check out how it is implemented

    /// \desc total number of textures in our scene
    static constexpr GLuint NUM_TEXTURES = 4;
    /// \desc used to index through our texture array to give named access
    enum TEXTURE_ID {
        GROUND = 0,
        TREE = 1,
        BUILDING = 2,
        LEAVES = 3
    };
    /// \desc texture handles for our textures
    GLuint _texHandles[NUM_TEXTURES];

    /// \note sets the texture parameters and sends the data to the GPU
    /// \param FILENAME external image filename to load
    static GLuint _loadAndRegisterTexture(const char* FILENAME);

    /// \desc generates building information to make up our scene
    void _generateEnvironment();
    //***************************************************************************
    // Shader Program Information

    /// \desc shader program that performs texturing
    CSCI441::ShaderProgram* _shaderProgram;
    /// \desc stores the locations of all of our shader uniforms
    struct TextureShaderUniformLocations {
        /// \desc precomputed MVP matrix location
        GLint mvpMatrix;
        GLint textureMap;
        GLint lightDirection;
        GLint directionalLightColor;
        GLint materialColor;
        GLint normalMatrix;
        GLint viewVector;
        GLint pointLightPosition;
        GLint pointLightColor;

    } _shaderUniformLocations;
    /// \desc stores the locations of all of our shader attributes
    struct TextureShaderAttributeLocations {
        /// \desc vertex position location
        GLint vPos;
        /// \desc vertex normal location
        /// \note not used in this lab
        GLint normalVec;
        GLint inTexCoord;

    } _shaderAttributeLocations;

    GLuint _skyboxVAO, _skyboxVBO;
    CSCI441::ShaderProgram* _skyboxShader;
    void _setupSkybox();
    void _renderSkybox(const glm::mat4& view, const glm::mat4& projection) const;

    struct SkyboxShaderUniformLocations {
        GLint view;
        GLint projection;
        GLint skyTexture;
    } _skyboxUniformLocations;

    GLuint _skyTexture;

    float _playerRadius = 2.0f;
    glm::vec2 _lastValidPosition;

    bool _checkCollisions(const glm::vec2& newPosition);

    bool _isFalling = false;
    float _fallTime = 0.0f;
    const float MAX_FALL_TIME = 3.0f;
    const float FALL_SPEED = 15.0f;
    glm::vec3 _spawnPosition = glm::vec3(0.0f, 0.5f, 0.0f);
    float _currentHeight = 0.5f;

    GhostManager* _ghostManager;

    ParticleSystem* _particleSystem;
    bool _isExploding = false;
};

void a4_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods );
void a4_cursor_callback(GLFWwindow *window, double x, double y );
void a4_mouse_button_callback(GLFWwindow *window, int button, int action, int mods );
void a4_scroll_callback(GLFWwindow *window, double xOffset, double yOffset);

#endif