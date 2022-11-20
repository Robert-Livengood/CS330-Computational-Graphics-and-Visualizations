#include <iostream>         
#include "cstdlib"          
#include "GL/glew.h"        
#include "GLFW/glfw3.h"     
#include "ShapeGenerator.h" 
#include "ShapeData.h"
// Image loading Utility functions
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM Math Header inclusions
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// Camera class
#include "camera.h"

// Math imports
#define _USE_MATH_DEFINES
#include "GL/glu.h"
#include "vector"
#include "cmath"
#include "algorithm"
#include "iterator"
#include "Vertex.h"

using namespace std;

// Shader program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace {
    const char* const WINDOW_TITLE = "Robert Livengood - Final Project CS330"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1600;
    const int WINDOW_HEIGHT = 900;

    // Stores the GL data relative to a given mesh layer. This program only uses one mesh layer with multiple VAOs and VBOs
    struct GLMesh {
        // Variables for rectangular objects - each pbject needs a Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
        GLuint planeVAO, centerpieceVAO, hedgeVAO, roadVAO, hedgeInnerVAO, centerRoadVAO, skyboxVAO;         // Handle for the vertex array object
        GLuint planeVBO, centerpieceVBO, hedgeVBO, roadVBO, hedgeInnerVBO, centerRoadVBO, skyboxVBO;         // Handle for the vertex buffer object
        GLuint nVerticesPlane, nVerticesCenterpiece, nVerticesHedge, nVerticesRoad, nVerticesHedgeInner, nVerticesCenterRoad, nVerticesSkybox;    // Number of indices of the objects this mesh

        // Variables for sphere objects - each pbject needs a Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
        GLuint sphereNumIndices;
        GLuint sphereVertexArrayObjectID;
        GLuint sphereIndexByteOffset;
        GLuint sphereVBO, sphereVAO;

        // offset variables for plane, sphere vertices - used by outside function call
        GLuint NUM_VERTICES_PER_TRI = 3;
        GLuint NUM_FLOATS_PER_VERTICE = 9;
        GLuint VERTEX_BYTE_SIZE = NUM_FLOATS_PER_VERTICE * sizeof(float);
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture variables - scales and initializations
    GLuint gTextureIdGrass, gTextureIdOrnate, gTextureIdHedge, gTextureIdFlower, gTextureIdBrick, gTextureIdDirt, gTextureIdSky;
    glm::vec2 gUVScale(1.0f, 1.0f);
    glm::vec2 gUVScale2(10.0f, 10.0f);
    glm::vec2 gUVScale3(0.5f, 0.5f);
    glm::vec2 gUVScale4(0.25f, 0.25f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader programs - one for the light and one for the rest of the objects
    GLuint gShaderProgramId;
    GLuint gLampProgramId;

    // Camera initialization 
    Camera gCamera(glm::vec3(0.0f, 9.0f, 28.0f));  // this is the initial camera location
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Setting up the shaders
    // 
    // Object position and scale
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gCubeScale(2.0f);
    glm::vec3 gCubeScale2(1.0f);

    // Object and light color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(0.9f, 0.9f, 1.0f);
    glm::vec3 gLightColor2(0.9f, 0.9f, 1.0f);

    // Light position and scale
    glm::vec3 gLightPosition(0.0f, 50.0f, 0.0f);
    glm::vec3 gLightPosition2(0.0f, 1.0f, 1.0f);
    glm::vec3 gLightScale(2.0f);

    // Lamp animation boolean
    bool gIsLampOrbiting = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 * Functions allow for reusability.
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);

/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;    // Sun
uniform vec3 lightPos;
uniform vec3 lightColor2;   // Ambient light source for fill
uniform vec3 lightPos2;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

//Phong lighting model calculations to generate ambient, diffuse, and specular components
void main()
{
    //Calculate Ambient lighting*/
    float ambientStrength = 0.1f;
    float ambientStrength2 = 0.4f;
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color
    vec3 ambient2 = ambientStrength2 * lightColor2;

    //Calculate Diffuse lighting
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos);
    float impact = max(dot(norm, lightDirection), 0.0);
    float impact2 = max(dot(norm, lightDirection2), 0.0);
    vec3 diffuse = impact * lightColor; // Generate diffuse light color
    vec3 diffuse2 = impact2 * lightColor2;

    //Calculate Specular lighting
    float specularIntensity = 1.8f;
    float highlightSize = 16.0f;
    float specularIntensity2 = 0.0f;
    float highlightSize2 = 0.0f;
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    vec3 reflectDir2 = reflect(-lightDirection2, norm);

    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
    vec3 specular = specularIntensity * specularComponent * lightColor;
    vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
    vec3 phong2 = (ambient2 + diffuse2 + specular2) * textureColor.xyz;

    // Send lighting results to GPU
    fragmentColor = vec4(phong + phong2, 1.0);
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    // VAP position 0 for vertex position data
    layout(location = 0) in vec3 position;

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main() {
    fragmentColor = vec4(1.0f, 1.0f, 0.5f, 1.0f); // Set color to yellow for the Sun with alpha 1.0 (brightness)
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels) {
    for (int j = 0; j < height / 2; ++j) {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i) {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

// Main program starts

int main(int argc, char* argv[])
{
    // set initial camera speed
    gCamera.MovementSpeed = 7.0f;

    if (!UInitialize(argc, argv, &gWindow)) {
        return EXIT_FAILURE;
    }
    else {
    }

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gShaderProgramId)) {
        return EXIT_FAILURE;
    }
    else {
    }
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId)) {
        return EXIT_FAILURE;
    }
    else {
    }

    // Load texture
    const char* texFilename = "grass.jpg";
    if (!UCreateTexture(texFilename, gTextureIdGrass)) {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    else {
    }
    texFilename = "ornate.jpg";
    if (!UCreateTexture(texFilename, gTextureIdOrnate)) {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    else {
    }
    texFilename = "hedge.jpg";
    if (!UCreateTexture(texFilename, gTextureIdHedge)) {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    else {
    }
    texFilename = "purpFlowers.jpg";
    if (!UCreateTexture(texFilename, gTextureIdFlower)) {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    else {
    }
    texFilename = "brick.jpg";
    if (!UCreateTexture(texFilename, gTextureIdBrick)) {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    else {
    }
    texFilename = "dirt.jpg";
    if (!UCreateTexture(texFilename, gTextureIdDirt)) {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    else {
    }
    texFilename = "sky.jpg";
    if (!UCreateTexture(texFilename, gTextureIdSky)) {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    else {
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gShaderProgramId);

    // Set the textures in GPU matrix
    glUniform1i(glGetUniformLocation(gShaderProgramId, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(gShaderProgramId, "uTexture1"), 1);
    glUniform1i(glGetUniformLocation(gShaderProgramId, "uTexture2"), 2);
    glUniform1i(glGetUniformLocation(gShaderProgramId, "uTexture3"), 3);
    glUniform1i(glGetUniformLocation(gShaderProgramId, "uTexture4"), 4);
    glUniform1i(glGetUniformLocation(gShaderProgramId, "uTexture5"), 5);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Render loop
    // -----------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(gWindow)) {
        // per-frame timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        UProcessInput(gWindow);

        // Render this frame
        URender();
        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureIdGrass);
    UDestroyTexture(gTextureIdOrnate);
    UDestroyTexture(gTextureIdHedge);
    UDestroyTexture(gTextureIdFlower);
    UDestroyTexture(gTextureIdDirt);
    UDestroyTexture(gTextureIdSky);
    UDestroyTexture(gTextureIdBrick);

    // Release shader programs
    UDestroyShaderProgram(gShaderProgramId);
    UDestroyShaderProgram(gLampProgramId);

    // Terminate program
    exit(EXIT_SUCCESS);
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window) {
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    else {
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult) {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }
    else {
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released during current frame and react accordingly
void UProcessInput(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else {
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    }
    else {
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    }
    else {
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    }
    else {
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    }
    else {
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    }
    else {
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    }
    else {
    }

    // Pause and unpause lamp orbit
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting) {
        gIsLampOrbiting = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting) {
        gIsLampOrbiting = false;
    }
    else {
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (gFirstMouse) {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: {
            if (action == GLFW_PRESS) {
                cout << "Left mouse button pressed" << endl;
            }
            else {
                cout << "Left mouse button released" << endl;
            }
        }
        break;

        case GLFW_MOUSE_BUTTON_MIDDLE: {
            if (action == GLFW_PRESS) {
                cout << "Middle mouse button pressed" << endl;
            }
            else {
                cout << "Middle mouse button released" << endl;
            }
        }
        break;

        case GLFW_MOUSE_BUTTON_RIGHT: {
            if (action == GLFW_PRESS) {
                cout << "Right mouse button pressed" << endl;
            }
            else {
                cout << "Right mouse button released" << endl;
            }
        }
        break;

        default: {
            cout << "Unhandled mouse button event" << endl;
        }
        break;
    }
}


// Functioned called to render a frame
void URender() {
    // Lamp orbits aroung the origin
    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting) {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 0.0f, 5.0f)) * glm::vec4(gLightPosition, 1.0);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //DRAW Plane
    // 
    // Activate the Plane VAO
    glBindVertexArray(gMesh.planeVAO);

    // Set the shader to be used
    glUseProgram(gShaderProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gCubePosition) * glm::scale(gCubeScale);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gShaderProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gShaderProgramId, "view");
    GLint projLoc = glGetUniformLocation(gShaderProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gShaderProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gShaderProgramId, "lightColor");
    GLint lightColorLoc2 = glGetUniformLocation(gShaderProgramId, "lightColor2");
    GLint lightPositionLoc = glGetUniformLocation(gShaderProgramId, "lightPos");
    GLint lightPositionLoc2 = glGetUniformLocation(gShaderProgramId, "lightPos2");
    GLint viewPositionLoc = glGetUniformLocation(gShaderProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    GLint UVScaleLoc = glGetUniformLocation(gShaderProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale2));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdGrass);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesPlane);

    //-------------------------------------------------------------------------------------------------------------------------------

    //DRAW centerpiece
    // 
    // Activate centerpiece VAO
    glBindVertexArray(gMesh.centerpieceVAO);

    // Set the shader to be used
    glUseProgram(gShaderProgramId);

    // Model matrix: transformations are applied right-to-left order
    model = glm::translate(gCubePosition) * glm::scale(gCubeScale2);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdOrnate);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesCenterpiece);


    //-------------------------------------------------------------------------------------------------------------------------------
    
    //DRAW skybox
    // 
    // Activate skybox VAO
    glBindVertexArray(gMesh.skyboxVAO);

    // Set the shader to be used
    glUseProgram(gShaderProgramId);

    // Model matrix: transformations are applied right-to-left order
    model = glm::translate(gCubePosition) * glm::scale(gCubeScale2);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdSky);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesSkybox);

    //-------------------------------------------------------------------------------------------------------------------------------

    //DRAW Hedge 1
    // 
    // Activate the Hedge VAO
    glBindVertexArray(gMesh.hedgeVAO);

    // Set the shader to be used
    glUseProgram(gShaderProgramId);

    // Set initial model settings
    // Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    // Rotates shape by 0 degrees in the x axis
    glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // Translate object to the origin
    glm::mat4 translation = glm::translate(glm::vec3(3.0f, 0.0f, 14.0f));

    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdHedge);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge #2
    //
    // uses the same hedgeVAO
    translation = glm::translate(glm::vec3(3.0f, 0.0f, -14.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge #3
    translation = glm::translate(glm::vec3(-3.0f, 0.0f, 14.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);


    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge #4
    translation = glm::translate(glm::vec3(-3.0f, 0.0f, -14.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge #5
    //
    // Uses the same hedgeVAO - rotates hedge placement for all succeeding hedgeVAOs
    rotation = glm::rotate(1.571f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(14.0f, 0.0f, 3.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge #6
    translation = glm::translate(glm::vec3(14.0f, 0.0f, -3.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge #7
    translation = glm::translate(glm::vec3(-14.0f, 0.0f, -3.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge #8
    translation = glm::translate(glm::vec3(-14.0f, 0.0f, 3.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Background Hedge #1
    translation = glm::translate(glm::vec3(-11.5f, 0.0f, -19.5f));
    scale = glm::scale(glm::vec3(1.0f, 6.0f, 2.5f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Background Hedge #2
    translation = glm::translate(glm::vec3(11.5f, 0.0f, -19.5f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Background Hedge #3
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    translation = glm::translate(glm::vec3(-19.5f, 0.0f, -11.5f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Background Hedge #4
    translation = glm::translate(glm::vec3(19.5f, 0.0f, -11.5f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Inner Hedge 1
    // 
    // Activate Inner Hedge VAO - different vertices, so a different VAO was used
    glBindVertexArray(gMesh.hedgeInnerVAO);

    // Set initial model settings - (redundant variables left for ease of adding future objects)
    scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    rotation = glm::rotate(0.77f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(5.7f, 0.0f, -5.7f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedgeInner);

    //-------------------------------------------------------------------------------------------------------------------------------
    
    // DRAW Inner hedge 2
    translation = glm::translate(glm::vec3(-5.7f, 0.0f, 5.7f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Inner hedge 3
    rotation = glm::rotate(0.77f, glm::vec3(0.0, -1.0f, 0.0f));
    translation = glm::translate(glm::vec3(5.7f, 0.0f, 5.7f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Inner hedge 4
    translation = glm::translate(glm::vec3(-5.7f, 0.0f, -5.7f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesHedge);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Road 1
    // 
    // Activate Road VAO
    glBindVertexArray(gMesh.roadVAO);

    // Set initial model settings - (redundant variables left for ease of adding future objects)
    scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 14.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBrick);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesRoad);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Road 2
    translation = glm::translate(glm::vec3(0.0f, 0.0f, -14.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesRoad);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Road 3
    rotation = glm::rotate(1.571f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(14.0f, 0.0f, 0.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesRoad);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Road 4
    translation = glm::translate(glm::vec3(-14.0f, 0.0f, 0.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesRoad);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Dirt Plane
    //
    // Activate Plane VAO
    glBindVertexArray(gMesh.planeVAO);

    // Set initial model settings - (redundant variables may be left for ease of adding future objects)
    scale = glm::scale(glm::vec3(0.15f, 0.15f, 0.15f));
    rotation = glm::rotate(0.0f, glm::vec3(0.0, 0.0035f, 0.0f));
    translation = glm::translate(glm::vec3(0.0f, 0.011f, 0.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdDirt);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesPlane);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Road Center
    // 
    // Activate plane VAO - same vao as the plane, but with a new texture and transform.
    glBindVertexArray(gMesh.centerRoadVAO);

    // Set initial model settings - (redundant variables may be left for ease of adding future objects)
    scale = glm::scale(glm::vec3(3.5f, 3.5f, 3.5f));
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBrick);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVerticesCenterRoad);

    //-------------------------------------------------------------------------------------------------------------------------------
    
    // DRAW Hedge Sphere #1
    //
    // Activate Sphere VAO
    glBindVertexArray(gMesh.sphereVAO);

    // Set initial model settings - (redundant variables may be left for ease of adding future objects)
    scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
    rotation = glm::rotate(1.571f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::translate(glm::vec3(2.0f, -0.5f, 0.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale3));
    glBindTexture(GL_TEXTURE_2D, gTextureIdHedge);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------
    
    // DRAW Hedge Sphere #2
    translation = glm::translate(glm::vec3(-2.0f, -0.5f, 0.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge Sphere #3
    translation = glm::translate(glm::vec3(0.0f, -0.5f, 2.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge Sphere #4
    translation = glm::translate(glm::vec3(0.0f, -0.5f, -2.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------
    // 
    // DRAW Hedge Sphere #5
    translation = glm::translate(glm::vec3(1.35f, -0.5f, 1.35f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------
    // 
    // DRAW Hedge Sphere #6
    translation = glm::translate(glm::vec3(1.35f, -0.5f, -1.35f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge Sphere #7
    translation = glm::translate(glm::vec3(-1.35f, -0.5f, 1.35f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Hedge Sphere #8
    translation = glm::translate(glm::vec3(-1.35f, -0.5f, -1.35f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Flower CenterPiece Sphere
    //
    // Activate sphere VAO
    glBindVertexArray(gMesh.sphereVAO);

    // Set initial model settings - (redundant variables may be left for ease of adding future objects)
    scale = glm::scale(glm::vec3(1.25f, 1.25f, 1.25f));
    rotation = glm::rotate(1.571f, glm::vec3(0.0f, 0.0f, 1.0f));
    translation = glm::translate(glm::vec3(0.0f, 2.5f, 0.0f));
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind texture coordinates on corresponding texture units
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale3));
    glBindTexture(GL_TEXTURE_2D, gTextureIdFlower);

    // Draws the trinangles
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------

    // DRAW Sun
    //
    // Activate Sphere VAO
    glBindVertexArray(gMesh.sphereVAO);

    // Activate Lamp Shader Program to utilize as light source
    glUseProgram(gLampProgramId);

    // Set model settings
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh.sphereNumIndices, GL_UNSIGNED_SHORT, (void*)gMesh.sphereIndexByteOffset);

    //-------------------------------------------------------------------------------------------------------------------------------
    
    // TEARDOWN
    // 
    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh) {
    // Set the vertices for each object.
    // Each object is made from triangles
    // 3 vertices form a triangle
    // 1 vertcice has x,y,z position, x,y,z normal vector, and x,y texture coordinates
    // 
    // Plane Vertex data
    GLfloat planeVerts[] = {
        //Positions            Normals             Texture Coordinates
        //Plane Triangle 1
       -10.0f,  0.0f, -10.0f,  0.0f,  1.0f, 0.0f,  0.0f, 0.0f,
       -10.0f,  0.0f,  10.0f,  0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        10.0f,  0.0f, -10.0f,  0.0f,  1.0f, 0.0f,  1.0f, 0.0f,
        //Plane Triangle 2
        10.0f,  0.0f,  10.0f,  0.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -10.0f,  0.0f, 10.0f,  0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        10.0f,  0.0f, -10.0f,  0.0f,  1.0f, 0.0f,  1.0f, 0.0f

    };

    // CenterPiece Flowerbox Vertex data
    GLfloat centerpieceVerts[] = {
        //Positions          Normals             Texture Coordinates
        //Base Triangle 1
       -0.5f,  0.0f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.5f,  0.0f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,    //Top Left Corner
        0.5f,  0.0f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        //Base Triangle 2
        0.5f,  0.0f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.5f,  0.0f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,    //Top Left Corner
        0.5f,  0.0f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,    //Bottom Left Corner
        //Side 1 Triangle 1
        0.5f,  0.0f,  0.5f,  1.0f,  0.0f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.5f,  0.0f, -0.5f,  1.0f,  0.0f, 0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.5f,  1.0f,  0.5f,  1.0f,  0.0f, 0.0f,  1.0f, 1.0f,    //Top Right corner
        //Side 1 Triangle 2
        0.5f,  1.0f,  0.5f,  1.0f,  0.0f, 0.0f,  1.0f, 1.0f,    //Top Right Corner
        0.5f,  1.0f, -0.5f,  1.0f,  0.0f, 0.0f,  0.0f, 1.0f,    //Top Left Corner
        0.5f,  0.0f, -0.5f,  1.0f,  0.0f, 0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        //Side 2 Triangle 1
        0.5f,  0.0f, -0.5f,  0.0f,  0.0f,-1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.5f,  0.0f, -0.5f,  0.0f,  0.0f,-1.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.5f,  1.0f, -0.5f,  0.0f,  0.0f,-1.0f,  0.0f, 1.0f,    //Top Left Corner
        //Side 2 Triangle 2
       -0.5f,  1.0f, -0.5f,  0.0f,  0.0f,-1.0f,  0.0f, 1.0f,    //Top Left Corner
        0.5f,  1.0f, -0.5f,  0.0f,  0.0f,-1.0f,  1.0f, 1.0f,    //Top Right Corner
        0.5f,  0.0f, -0.5f,  0.0f,  0.0f,-1.0f,  1.0f, 0.0f,    //Bottom Right Corner
        //Side 3 Triangle 1
       -0.5f,  0.0f, -0.5f, -1.0f,  0.0f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.5f,  0.0f,  0.5f, -1.0f,  0.0f, 0.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.5f,  1.0f,  0.5f, -1.0f,  0.0f, 0.0f,  0.0f, 1.0f,    //Top Left Corner
        //Side 3 Triangle 2
       -0.5f,  1.0f,  0.5f, -1.0f,  0.0f, 0.0f,  0.0f, 1.0f,    //Top Left Corner
       -0.5f,  1.0f, -0.5f, -1.0f,  0.0f, 0.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.5f,  0.0f, -0.5f, -1.0f,  0.0f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       //Side 4 Triangle 1
       -0.5f,  0.0f,  0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.5f,  0.0f,  0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.5f,  1.0f,  0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,    //Top Left Corner
        //Side 4 Triangle 2
        0.5f,  1.0f,  0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,    //Top Left Corner
       -0.5f,  1.0f,  0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.5f,  0.0f,  0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       //Top Triangle 1
        0.5f,  1.0f, -0.5f,  0.0f,  1.0f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.5f,  1.0f, -0.5f,  0.0f,  1.0f, 0.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.5f,  1.0f,  0.5f,  0.0f,  1.0f, 0.0f,  0.0f, 1.0f,    //Top Left Corner
        //Top Triangle 2
       -0.5f,  1.0f,  0.5f,  0.0f,  1.0f, 0.0f,  0.0f, 1.0f,    //Top Left Corner
        0.5f,  1.0f,  0.5f,  0.0f,  1.0f, 0.0f,  1.0f, 1.0f,    //Top Right Corner
        0.5f,  1.0f, -0.5f,  0.0f,  1.0f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        //Pyramid Triangle 1
        0.5f,  1.0f,  0.5f,  0.5f,  0.5f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.5f,  1.0f, -0.5f,  0.5f,  0.5f, 0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.0f,  1.5f,  0.0f,  0.5f,  0.5f, 0.0f,  0.5f, 1.0f,    //Top Centerpoint
        //Pyramid Triangle 2
        0.5f,  1.0f, -0.5f,  0.0f,  0.5f,-0.5f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.5f,  1.0f, -0.5f,  0.0f,  0.5f,-0.5f,  0.0f, 0.0f,    //Bottom Left Corner
        0.0f,  1.5f,  0.0f,  0.0f,  0.5f,-0.5f,  0.5f, 1.0f,    //Top Centerpoint
        //Pyramid Triangle 3
       -0.5f,  1.0f, -0.5f, -0.5f,  0.5f, 0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.5f,  1.0f,  0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.0f,  1.5f,  0.0f, -0.5f,  0.5f, 0.0f,  0.5f, 1.0f,    //Top Centerpoint
        //Pyramid Triangle 4
       -0.5f,  1.0f,  0.5f,  0.0f,  0.5f, 0.5f,  1.0f, 0.0f,    //Bottom Right Corner
        0.5f,  1.0f,  0.5f,  0.0f,  0.5f, 0.5f,  0.0f, 0.0f,    //Bottom Left Corner
        0.0f,  1.5f,  0.0f,  0.0f,  0.5f, 0.5f,  0.5f, 1.0f     //Top Centerpoint

    };

    // Outer Hedge Row
    GLfloat hedgeVerts[] = {
        //Positions          Normals              //Texture Coordinates
        //Hedge Bottom Triangle 1
        0.50f, 0.0f,  3.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.50f, 0.0f,  3.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
        0.50f, 0.0f, -3.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        //Hedge Bottom Triangle 2
       -0.50f, 0.0f,  3.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
       -0.50f, 0.0f, -3.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 0.0f, -3.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       //Hedge Top Triangle 1
        0.50f, 1.0f,  3.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.50f, 1.0f,  3.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
        0.50f, 1.0f, -3.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       //Hedge Top Triangle 2
       -0.50f, 1.0f,  3.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
       -0.50f, 1.0f, -3.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,    //Bottome Left Corner
        0.50f, 1.0f, -3.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       //Hedge Front Triangle 1
        0.50f, 0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 1.0f,  3.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,    //Top Right Corner
       //Hedge Front Triangle 2
       -0.50f, 0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 1.0f,  3.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.50f, 1.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,    //Top Left Corner
        //Hedge left Triangle 1 
       -0.50f, 0.0f,  3.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 0.0f, -3.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.50f, 1.0f, -3.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
       //Hedge left Triangle 2
       -0.50f, 0.0f,  3.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 1.0f,  3.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.50f, 1.0f, -3.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
        //Hedge Right Triangle 1
        0.50f, 0.0f, -3.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.50f, 0.0f,  3.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 1.0f,  3.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner 
       //Hedge Right Triangle 2
        0.50f, 0.0f, -3.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.50f, 1.0f, -3.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
        0.50f, 1.0f,  3.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
        //Hedge Back Triangle 1
        0.50f, 0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.50f, 1.0f, -3.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,    //Top Left
       //Hedge Back Triangle 2
        0.50f, 0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.50f, 1.0f, -3.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.50f, 1.0f, -3.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f     //Top Left Corner
    };
    GLfloat hedgeInnerVerts[] = {
        //Positions          Normals              //Texture Coordinates
        //Hedge Bottom Triangle 1
        0.50f, 0.0f,  2.1f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
       -0.50f, 0.0f,  2.1f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
        0.50f, 0.0f, -2.1f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,    //Bottome Right Corner
        //Hedge Bottom Triangle 2
       -0.50f, 0.0f,  2.1f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
       -0.50f, 0.0f, -2.1f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 0.0f, -2.1f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       //Hedge Top Triangle 1
        0.50f, 1.02f,  2.1f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,   //Top Right Corner
       -0.50f, 1.02f,  2.1f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
        0.50f, 1.02f, -2.1f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   //Bottom Right Corner
       //Hedge Top Triangle 2
       -0.50f, 1.02f,  2.1f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
       -0.50f, 1.02f, -2.1f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,   //Bottom Left Corner
        0.50f, 1.02f, -2.1f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   //Bottom Right Corner
       //Hedge Front Triangle 1
        0.50f, 0.0f,  2.1f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 0.0f,  2.1f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 1.02f,  2.1f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   //Top Right Corner
       //Hedge Front Triangle 2
       -0.50f, 0.0f,  2.1f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 1.02f,  2.1f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   //Top Right Corner
       -0.50f, 1.02f,  2.1f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,   //Top Left Corner
        //Hedge left Triangle 1
       -0.50f, 0.0f,  2.1f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 0.0f, -2.1f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.50f, 1.02f, -2.1f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
       //Hedge left Triangle 2
       -0.50f, 0.0f,  2.1f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 1.02f,  2.1f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   //Top Right Corner
       -0.50f, 1.02f, -2.1f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
        //Hedge Right Triangle 1
        0.50f, 0.0f, -2.1f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.50f, 0.0f,  2.1f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        0.50f, 1.02f,  2.1f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
       //Hedge Right Triangle 2
        0.50f, 0.0f, -2.1f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.50f, 1.02f, -2.1f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   //Top Right Corner
        0.50f, 1.02f,  2.1f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
        //Hedge Back Triangle 1
        0.50f, 0.0f, -2.1f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -0.50f, 0.0f, -2.1f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -0.50f, 1.02f, -2.1f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,   //Top Left Corner
       //Hedge Back Triangle 2
        0.50f, 0.0f, -2.1f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,    //Bottom Right Corner
        0.50f, 1.02f, -2.1f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   //Top Right Corner
       -0.50f, 1.02f, -2.1f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f    //Top Left Corner
    };

    GLfloat roadVerts[] = {
        //Positions         //Normals            //Texture Coordinates
        //Bottom Triangle 1
        1.0f, 0.0f,  3.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
       -1.0f, 0.0f,  3.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
        1.0f, 0.0f, -3.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        //Bottom Triangle 2
       -1.0f, 0.0f,  3.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
       -1.0f, 0.0f, -3.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        1.0f, 0.0f, -3.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       //Top Triangle 1
        1.0f, 0.01f,  3.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,   //Top Right Corner
       -1.0f, 0.01f,  3.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
        1.0f, 0.01f, -3.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   //Bottom Right Corner
       //Top Triangle 2
       -1.0f, 0.01f,  3.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
       -1.0f, 0.01f, -3.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,   //Bottom Left Corner
        1.0f, 0.01f, -3.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   //Bottom Right Corner
       //Front Triangle 1
        1.0f, 0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -1.0f, 0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,    //Bottom Left Corner
        1.0f, 0.01f,  3.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   //Top Right Corner
       //Front Triangle 2
       -1.0f, 0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,    //Bottom Left Corner
        1.0f, 0.01f,  3.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   //Top Right Corner
       -1.0f, 0.01f,  3.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,   //Top Left Corner
        //left Triangle 1
       -1.0f, 0.0f,  3.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -1.0f, 0.0f, -3.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -1.0f, 0.01f, -3.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
       //left Triangle 2
       -1.0f, 0.0f,  3.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -1.0f, 0.01f,  3.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   //Top Right Corner
       -1.0f, 0.01f, -3.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
        //Right Triangle 1
        1.0f, 0.0f, -3.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        1.0f, 0.0f,  3.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,    //Bottom Left Corner
        1.0f, 0.01f,  3.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
       //Right Triangle 2
        1.0f, 0.0f, -3.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    //Bottom Right Corner
        1.0f, 0.01f, -3.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   //Top Right Corner
        1.0f, 0.01f,  3.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   //Top Left Corner
        //Back Triangle 1
        1.0f, 0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,    //Bottom Right Corner
       -1.0f, 0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,    //Bottom Left Corner
       -1.0f, 0.01f, -3.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,   //Top Left Corner
       //Back Triangle 2
        1.0f, 0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,    //Bottom Right Corner
        1.0f, 0.01f, -3.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   //Top Right Corner
       -1.0f, 0.01f, -3.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f    //Top Left Corner
    };

    GLfloat centerRoadVerts[] = {

        //Positions         //Normals              //Texture Coordinates
        //Top Triangle 1
        0.0f, 0.001f,  3.0f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,    //Top Corner
        3.0f, 0.001f,  0.0f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,    //Right Corner
       -3.0f, 0.001f,  0.0f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,    //Left Corner
        //Top Triangle 2
        0.0f, 0.001f, -3.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,   //Bottom Corner
        3.0f, 0.001f,  0.0f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,    //Right Corner
       -3.0f, 0.001f,  0.0f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,    //Left Corner
        
    };

    float skyScale = 50.0f;
    GLfloat skyboxVerts[] = {

      //Positions         //Normals              //Texture Coordinates
      //Top Triangle 1
     -skyScale, skyScale,  skyScale,  0.0f, 1.0f,  0.0f,  -1.0f, 1.0f,    //Top Left Corner
      skyScale, skyScale,  skyScale,  0.0f, 1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale, skyScale, -skyScale,  0.0f, 1.0f,  0.0f,  -1.0f, -1.0f,    //Bottom Left Corner
      //Top Triangle 2
      skyScale, skyScale, -skyScale,  0.0f, 1.0f,  0.0f,  1.0f, -1.0f,   //Bottom Right Corner
      skyScale, skyScale,  skyScale,  0.0f, 1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale, skyScale, -skyScale,  0.0f, 1.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Bottom Triangle 1
     -skyScale, -skyScale,  skyScale,  0.0f, -1.0f,  0.0f,  -1.0f, 1.0f,    //Top Left Corner
      skyScale, -skyScale,  skyScale,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale, -skyScale, -skyScale,  0.0f, -1.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Bottom Triangle 2
      skyScale, -skyScale, -skyScale,  0.0f, -1.0f,  0.0f,  1.0f, -1.0f,   //Bottom Right Corner
      skyScale, -skyScale,  skyScale,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale, -skyScale, -skyScale,  0.0f, -1.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Right Triangle 1
     skyScale, skyScale, -skyScale,  1.0f, 0.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
     skyScale, skyScale,  skyScale,  1.0f, 0.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     skyScale,-skyScale, -skyScale,  1.0f, 0.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Right Triangle 2
     skyScale,-skyScale,  skyScale,  1.0f, 0.0f,  0.0f,  1.0f, -1.0f,   //Bottom Right Corner
     skyScale, skyScale,  skyScale,  1.0f, 0.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     skyScale,-skyScale, -skyScale,  1.0f, 0.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Left Triangle 1
     -skyScale, skyScale,  skyScale,  -1.0f, 0.0f,  0.0f,  0.0f, 1.0f,    //Top Left Corner
     -skyScale, skyScale,  -skyScale, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale,-skyScale,  skyScale,  -1.0f, 0.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Left Triangle 2
     -skyScale,-skyScale, -skyScale,  -1.0f, 0.0f,  0.0f,  1.0f, -1.0f,   //Bottom Right Corner
     -skyScale, skyScale, -skyScale,  -1.0f, 0.0f,  0.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale,-skyScale,  skyScale,  -1.0f, 0.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Front Triangle 1
     -skyScale, skyScale,  skyScale,  0.0f, 0.0f,  1.0f,  0.0f, 1.0f,    //Top Left Corner
     skyScale,  skyScale,  skyScale,  0.0f, 0.0f,  1.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale,-skyScale,  skyScale,  0.0f, 0.0f,  0.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Front Triangle 2
      skyScale,-skyScale,  skyScale,  0.0f, 0.0f,  1.0f,  1.0f, -1.0f,   //Bottom Right Corner
      skyScale, skyScale,  skyScale,  0.0f, 0.0f,  1.0f,  1.0f, 1.0f,    //Top Right Corner
     -skyScale, -skyScale,  skyScale,  0.0f, 0.0f,  1.0f, -1.0f, 1.0f,    //Bottom Left Corner
     //Back Triangle 1
     skyScale, skyScale, -skyScale,  0.0f, 0.0f,  -1.0f,  0.0f, 1.0f,    //Top Left Corner
     -skyScale,skyScale, -skyScale,  0.0f, 0.0f,  -1.0f,  1.0f, 1.0f,    //Top Right Corner
     skyScale,-skyScale, -skyScale,  0.0f, 0.0f,  -1.0f, -1.0f, -1.0f,    //Bottom Left Corner
     //Back Triangle 2
     -skyScale,-skyScale, -skyScale,  0.0f, 0.0f,  -1.0f,  1.0f, -1.0f,   //Bottom Right Corner
     -skyScale, skyScale, -skyScale,  0.0f, 0.0f,  -1.0f,  1.0f, 1.0f,    //Top Right Corner
     skyScale, -skyScale, -skyScale,  0.0f, 0.0f,  -1.0f, -1.0f, -1.0f,    //Bottom Left Corner
    };

    // set variables for stride to parse vertice data and send to mesh
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    // Strides between vertex coordinates is 8
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    //-------------------------------------------------------------------------------------------------------------------------------
    
    // planeVAO and planeVBO bindings
    // 
    // set vertice data in mesh layer
    mesh.nVerticesPlane = sizeof(planeVerts) / (sizeof(planeVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.planeVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.planeVAO);

    // Create buffer
    glGenBuffers(1, &mesh.planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.planeVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);


    //-------------------------------------------------------------------------------------------------------------------------------

    // centerpieceVAO and centerpieceVBO bindings
    //
    // set vertice data in mesh layer
    mesh.nVerticesCenterpiece = sizeof(centerpieceVerts) / (sizeof(centerpieceVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.centerpieceVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.centerpieceVAO);

    // Create buffer
    glGenBuffers(1, &mesh.centerpieceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.centerpieceVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(centerpieceVerts), centerpieceVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //-------------------------------------------------------------------------------------------------------------------------------

    // hedgeVAO and hedgeVBO bindings
    //
    // set vertice data in mesh layer
    mesh.nVerticesHedge = sizeof(hedgeVerts) / (sizeof(hedgeVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.hedgeVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.hedgeVAO);

    // Create buffer
    glGenBuffers(1, &mesh.hedgeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.hedgeVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(hedgeVerts), hedgeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //-------------------------------------------------------------------------------------------------------------------------------
   
    // roadVAO and roadVBO bindings
    //
    // set the vertice data in mesh layer
    mesh.nVerticesRoad = sizeof(roadVerts) / (sizeof(roadVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.roadVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.roadVAO);

    // Create buffer
    glGenBuffers(1, &mesh.roadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.roadVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(roadVerts), roadVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //-------------------------------------------------------------------------------------------------------------------------------

    // CenterRoadVAO and CenterRoadVBO bindings
    //
    // set the vertice data in mesh layer
    mesh.nVerticesCenterRoad = sizeof(centerRoadVerts) / (sizeof(centerRoadVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.centerRoadVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.centerRoadVAO);

    // Create buffer
    glGenBuffers(1, &mesh.centerRoadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.centerRoadVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(centerRoadVerts), centerRoadVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //-------------------------------------------------------------------------------------------------------------------------------

    // hedgeInnerVAO and hedgeInnerVBO bindings
    //
    // set the vertice data in mesh layer
    mesh.nVerticesHedgeInner = sizeof(hedgeInnerVerts) / (sizeof(hedgeInnerVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.hedgeInnerVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.hedgeInnerVAO);

    // Create buffer
    glGenBuffers(1, &mesh.hedgeInnerVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.hedgeInnerVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(hedgeInnerVerts), hedgeInnerVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //-------------------------------------------------------------------------------------------------------------------------------
    
    // CREATE sphere object and bindings
    // 
    // generate shape
    int currentOffset = 0;
    ShapeData sphere = ShapeGenerator::makeSphere();

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.sphereVAO);
    glBindVertexArray(mesh.sphereVAO);

    // Create buffer
    glGenBuffers(1, &mesh.sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.sphereVBO);  // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sphere.vertexBufferSize() + sphere.indexBufferSize(), 0, GL_STATIC_DRAW);  // Sends vertex or coordinate data to the GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.sphereVBO);
    currentOffset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, currentOffset, sphere.vertexBufferSize(), sphere.vertices);  // Send sphere vertex data(vertices) to the GPU
    currentOffset += sphere.vertexBufferSize();
    mesh.sphereIndexByteOffset = currentOffset;
    glBufferSubData(GL_ARRAY_BUFFER, currentOffset, sphere.indexBufferSize(), sphere.indices);  // Send sphere vertex data(indices) to the GPU
    mesh.sphereNumIndices = sphere.numIndices;

    // Create Vertex Attribut Pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, mesh.VERTEX_BYTE_SIZE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, mesh.VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, mesh.VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    //-------------------------------------------------------------------------------------------------------------------------------

    // skyboxVAO and skyboxVAO bindings
    //
    // set the vertice data in mesh layer
    mesh.nVerticesSkybox = sizeof(skyboxVerts) / (sizeof(skyboxVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // set data to corresponding VAO
    glGenVertexArrays(1, &mesh.skyboxVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.skyboxVAO);

    // Create buffer
    glGenBuffers(1, &mesh.skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.skyboxVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVerts), skyboxVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //-------------------------------------------------------------------------------------------------------------------------------
}

// Teardown mesh layer
void UDestroyMesh(GLMesh& mesh) {
    glDeleteVertexArrays(1, &mesh.planeVAO);
    glDeleteBuffers(1, &mesh.planeVBO);
    glDeleteVertexArrays(1, &mesh.centerpieceVAO);
    glDeleteBuffers(1, &mesh.centerpieceVBO);
    glDeleteVertexArrays(1, &mesh.hedgeVAO);
    glDeleteBuffers(1, &mesh.hedgeVBO);
    glDeleteVertexArrays(1, &mesh.roadVAO);
    glDeleteBuffers(1, &mesh.roadVBO);
    glDeleteVertexArrays(1, &mesh.hedgeInnerVAO);
    glDeleteBuffers(1, &mesh.hedgeInnerVBO);
    glDeleteVertexArrays(1, &mesh.sphereVAO);
    glDeleteBuffers(1, &mesh.sphereVBO);
    glDeleteVertexArrays(1, &mesh.centerRoadVAO);
    glDeleteBuffers(1, &mesh.centerRoadVBO);
    glDeleteVertexArrays(1, &mesh.skyboxVAO);
    glDeleteBuffers(1, &mesh.skyboxVBO);
}


// Generate and load the texture
bool UCreateTexture(const char* filename, GLuint& textureId) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image) {
        flipImageVertically(image, width, height, channels);
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        }
        else if (channels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        }
        else {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }
    else {
        // Error loading the image
        return false;
    }
    // Error loading the image
    return false;
}

// Destroy textures
void UDestroyTexture(GLuint textureId) {
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId) {

    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader
    glCompileShader(vertexShaderId);

    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }
    else {
    }

    // compile the fragment shader
    glCompileShader(fragmentShaderId);

    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }
    else {
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);   // links the shader program

    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }
    else {
    }

    // Use the shader program
    glUseProgram(programId);

    return true;
}

// Implement teardown of shader program
void UDestroyShaderProgram(GLuint programId) {
    glDeleteProgram(programId);
}
