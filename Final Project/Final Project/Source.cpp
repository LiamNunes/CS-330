#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"



using namespace std;

/*shader macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

//unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Final Project"; // Macro for window title

    //vars for window width and height
    const int WINDOW_WIDTH = 1600;
    const int WINDOW_HEIGHT = 900;

    //stores GL data relative to a mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    //main GLFW window
    GLFWwindow* gWindow = nullptr;

    //triangle mesh data
    GLMesh gMeshKnifeBlade;
    GLMesh gMeshKnifeHandle;
    GLMesh gLightMesh;
    GLMesh gPlaneMesh;
    GLMesh gCuttingBoardMesh;
    GLMesh gCheeseMesh;
    GLMesh gSalamiBodyMesh;
    GLMesh gSalamiEndsMesh;

    //texture
    GLuint gHandleTexture;
    GLuint gBladeTexture;
    GLuint gCuttingBoardTexture;
    GLuint gCounterTexture;
    GLuint gCheeseTexture;
    GLuint gSalamiBodyTexture;
    GLuint gSalamiEndsTexture;
    glm::vec2 gUVScale(1.0f, 1.0f);

    // Shader programs
    GLuint gProgramId;
    GLuint gLampProgramId;

    //camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    //timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    //cube and light color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);

    //light position and scale
    glm::vec3 gLightPosition(4.0f, 8.5f, -3.0f);
    glm::vec3 gLightScale(0.5f);

    //bool to track to see if using perspective mode
    bool perspectiveMode = true;

    //tracks the button event for p so that is doesnt execute the code rapidly
    bool click = false;

    //tracks mouse down event for better control over scean
    bool mouseClick = false;

}

//function prototypes
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UCreateKnifeBladeMesh(GLMesh& mesh);
void UCreateKnifeHandleMesh(GLMesh& mesh);
void UCreateCubeMesh(GLMesh& mesh);
void UCreateCheeseMesh(GLMesh& mesh);
void UCreatePlaneMesh(GLMesh& mesh);
void UCreateCuttingBoardMesh(GLMesh& mesh);
void UCreateSalamiBodyMesh(GLMesh& mesh);
void UCreateSalamiEndsMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId, int location);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
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
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);



//flip image
void flipImageVertically(unsigned char* image, int width, int height, int channels) {
    for (int j = 0; j < height / 2; ++j) {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i) {
            unsigned char temp = image[index1];
            image[index1] = image[index2];
            image[index2] = temp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //create the meshes
    UCreateKnifeHandleMesh(gMeshKnifeHandle);
    UCreateKnifeBladeMesh(gMeshKnifeBlade);
    UCreateCubeMesh(gLightMesh);
    UCreateCheeseMesh(gCheeseMesh);
    UCreatePlaneMesh(gPlaneMesh);
    UCreateCuttingBoardMesh(gCuttingBoardMesh);
    UCreateSalamiBodyMesh(gSalamiBodyMesh);
    UCreateSalamiEndsMesh(gSalamiEndsMesh);

    //create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;



    //load knife handle textur
    const char* handleTexFile = "../resources/textures/wood.jpg";
    if (!UCreateTexture(handleTexFile, gHandleTexture, 0))
    {
        cout << "Failed to load handle texture " << handleTexFile << endl;
        return EXIT_FAILURE;
    }

    //load knife blade texture
    const char* bladeTexFile = "../resources/textures/metal.jpg";
    if (!UCreateTexture(bladeTexFile, gBladeTexture, 1))
    {
        cout << "Failed to load blade texture " << bladeTexFile << endl;
        return EXIT_FAILURE;
    }

    //load Cutting Board texture
    const char* CuttingBoardTexFile = "../resources/textures/CuttingBoard.jpg";
    if (!UCreateTexture(CuttingBoardTexFile, gCuttingBoardTexture, 2))
    {
        cout << "Failed to load blade texture " << CuttingBoardTexFile << endl;
        return EXIT_FAILURE;
    }

    //load counbter texture
    const char* CounterTexFile = "../resources/textures/Counter.jpg";
    if (!UCreateTexture(CounterTexFile, gCounterTexture, 3))
    {
        cout << "Failed to load blade texture " << CounterTexFile << endl;
        return EXIT_FAILURE;
    }

    //load cheese texture
    const char* CheeseTexFile = "../resources/textures/Cheese.jpg";
    if (!UCreateTexture(CheeseTexFile, gCheeseTexture, 4))
    {
        cout << "Failed to load blade texture " << CheeseTexFile << endl;
        return EXIT_FAILURE;
    }

    //load salami body texture
    const char* SalamiBodyTexFile = "../resources/textures/SalamiSkin.jpg";
    if (!UCreateTexture(SalamiBodyTexFile, gSalamiBodyTexture, 5))
    {
        cout << "Failed to load blade texture " << SalamiBodyTexFile << endl;
        return EXIT_FAILURE;
    }

    //load salami ends texture
    const char* SalamiEndsTexFile = "../resources/textures/SalamiInside.jpg";
    if (!UCreateTexture(SalamiEndsTexFile, gSalamiEndsTexture, 6))
    {
        cout << "Failed to load blade texture " << SalamiEndsTexFile << endl;
        return EXIT_FAILURE;
    }

    //tell opengl which texture unit it belongs to
    glUseProgram(gProgramId);
    glUniform1i(glGetUniformLocation(gProgramId, "gHandleTexture"), 0);
    glUniform1i(glGetUniformLocation(gProgramId, "gBladeTexture"), 1);
    glUniform1i(glGetUniformLocation(gProgramId, "gCuttingBoardTexture"), 2);
    glUniform1i(glGetUniformLocation(gProgramId, "gCounterTexture"), 3);
    glUniform1i(glGetUniformLocation(gProgramId, "gCheeseTexture"), 4);


    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMeshKnifeHandle);
    UDestroyMesh(gMeshKnifeBlade);
    UDestroyMesh(gLightMesh);
    UDestroyMesh(gPlaneMesh);
    UDestroyMesh(gCuttingBoardMesh);
    UDestroyMesh(gCheeseMesh);
    UDestroyMesh(gSalamiBodyMesh);
    UDestroyMesh(gSalamiEndsMesh);

    // Release texture
    UDestroyTexture(gHandleTexture);
    UDestroyTexture(gBladeTexture);
    UDestroyTexture(gCuttingBoardTexture);
    UDestroyTexture(gCounterTexture);
    UDestroyTexture(gCheeseTexture);
    UDestroyTexture(gSalamiBodyTexture);
    UDestroyTexture(gSalamiEndsTexture);

    // Release shader programs
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


//initialize glfw, glew, and creates window
bool UInitialize(int argc, char* argv[], GLFWwindow** window) {
    //glfw initialize and config
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //window creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);

    //glew initialize
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


//process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(TURNL, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(TURNR, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(TURNU, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(TURND, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        click = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && click) {

        if (perspectiveMode) {
            perspectiveMode = false;
        }
        else {
            perspectiveMode = true;
        }
        click = false;
    }
}


//glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


//glfw: whenever the mouse moves, this callback is called
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
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
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}



// Functioned called to render a frame
void URender()
{
    //enable z-depth
    glEnable(GL_DEPTH_TEST);

    //clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //knife handle--------------------------------------------
    glBindVertexArray(gMeshKnifeHandle.vao);

    glUseProgram(gProgramId);

    // bind textures being used
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gHandleTexture);

    //set scale rotation and translation for object
    glm::mat4 scale = glm::scale(glm::vec3(1.1f, 1.1f, 1.1f));
    glm::mat4 rotation = glm::rotate(2.4f, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 translation = glm::translate(glm::vec3(3.5f, 0.94f, -0.9f));

    //add the changes to the model data
    glm::mat4 model = translation * rotation * scale;

    //camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();


    GLint projLoc;
    glm::mat4 projection;
    if (perspectiveMode) {
        projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        GLint projLoc = glGetUniformLocation(gProgramId, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }
    else {
        projection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, -1000.0f, 1000.0f);
        GLint projLoc = glGetUniformLocation(gProgramId, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }

    //retrieve and passe matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    //pass data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);

    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshKnifeHandle.nVertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    //knife blade-----------------------------------

    // bind textures being used
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBladeTexture);

    //tell program which mesh is being worked on
    glBindVertexArray(gMeshKnifeBlade.vao);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshKnifeBlade.nVertices);

    glBindTexture(GL_TEXTURE_2D, 0);


    //Cheese-----------------------------------------------

    // bind textures being used
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCheeseTexture);

    //tell program which mesh is being worked on
    glBindVertexArray(gCheeseMesh.vao);

    //set scale rotation and translation for object
    scale = glm::scale(glm::vec3(1.3f, 1.5f, 1.5f));
    translation = glm::translate(glm::vec3(-1.5f, 1.3f, 2.5f));
    rotation = glm::rotate(0.1f, glm::vec3(0.0f, 1.0f, 0.0f));

    //transform the smaller cube used as a visual que for the light source
    model = translation * scale * rotation;

    //reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    //pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gCheeseMesh.nVertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    //plane-------------------------------------------


    // bind textures being used
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCounterTexture);

    //tell program which mesh is being worked on
    glBindVertexArray(gPlaneMesh.vao);

    //set scale rotation and translation for object
    scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

    //transform the smaller cube used as a visual que for the light source
    model = translation * scale;

    //reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    //pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gPlaneMesh.nVertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    //Cutting Board-----------------------------------------

    // bind textures being used
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCuttingBoardTexture);

    //tell program which mesh is being worked on
    glBindVertexArray(gCuttingBoardMesh.vao);

    //set scale rotation and translation for object
    scale = glm::scale(glm::vec3(1.3f, 1.0f, 1.3f));
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    rotation = glm::rotate(0.1f, glm::vec3(0.0f, 1.0f, 0.0f));

    //transform the smaller cube used as a visual que for the light source
    model = translation * scale * rotation;

    //reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    //pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gCuttingBoardMesh.nVertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    //Salami----------------------------------------------------------
    
    // bind textures being used for body
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gSalamiBodyTexture);

    //tell program which mesh is being worked on
    glBindVertexArray(gSalamiBodyMesh.vao);

    //set scale rotation and translation for object
    scale = glm::scale(glm::vec3(1.6f, 0.7f, 0.7f));
    translation = glm::translate(glm::vec3(1.7f, 1.6f, 0.0f));
    rotation = glm::rotate(1.57f, glm::vec3(0.0f, 0.0f, 1.0f));

    //transform the smaller cube used as a visual que for the light source
    model = scale * rotation * translation;

    //reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    //pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draws the trianglesfor body
    glDrawArrays(GL_TRIANGLES, 0, gSalamiBodyMesh.nVertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    // bind textures being used for ends
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gSalamiEndsTexture);

    //tell program which mesh is being worked on
    glBindVertexArray(gSalamiEndsMesh.vao);

    // Draws the trianglesfor body
    glDrawArrays(GL_TRIANGLES, 0, gSalamiEndsMesh.nVertices);

    glBindTexture(GL_TEXTURE_2D, 0);



    //light1-------------------------------------------=


    glBindVertexArray(gLightMesh.vao);
    glUseProgram(gLampProgramId);

    //transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    //reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    //pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gLightMesh.nVertices);


    //deactivate the vao and shader
    glBindVertexArray(0);
    glUseProgram(0);

    glfwSwapBuffers(gWindow);
}


//creates the mesh for the light
void UCreateCubeMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

       //Front Face         //Positive Z Normal
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

      //Left Face          //Negative X Normal
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Right Face         //Positive X Normal
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Bottom Face        //Negative Y Normal
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    //create 2 buffers
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    //stride between vertexs
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //create atribute pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateKnifeHandleMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
       -4.0f, -0.4f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 0.0f,
        0.0f, -0.4f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, -0.4f, -0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 1.0f,
        0.0f, -0.4f, -0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 1.0f,
       -4.0f, -0.4f, -0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 1.0f,
       -4.0f, -0.4f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 0.0f,

       //Front Face         //Positive Z Normal
      -4.0f,  0.4f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
       0.0f,  0.4f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
       0.0f,  0.4f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
       0.0f,  0.4f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
      -4.0f,  0.4f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
      -4.0f,  0.4f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,

      //Left Face          //Negative X Normal
      -4.0f, -0.4f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -4.0f,  0.4f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -4.0f,  0.4f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -4.0f,  0.4f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      -4.0f, -0.4f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -4.0f, -0.4f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Right Face         //Positive X Normal
    0.0f, -0.4f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.0f,  0.4f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.0f,  0.4f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.0f,  0.4f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.0f, -0.4f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.0f, -0.4f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Bottom Face        //Negative Y Normal
    -4.0f, -0.4f, -0.5f,  0.0f, 0.0f,  -1.0f,  0.0f, 1.0f,
     0.0f, -0.4f, -0.5f,  0.0f, 0.0f,  -1.0f,  1.0f, 1.0f,
     0.0f,  0.4f, -0.5f,  0.0f, 0.0f,  -1.0f,  1.0f, 0.0f,
     0.0f,  0.4f, -0.5f,  0.0f, 0.0f,  -1.0f,  1.0f, 0.0f,
    -4.0f,  0.4f, -0.5f,  0.0f, 0.0f,  -1.0f,  0.0f, 0.0f,
    -4.0f, -0.4f, -0.5f,  0.0f, 0.0f,  -1.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
   -4.0f, -0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    0.0f, -0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    0.0f,  0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
    0.0f,  0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
   -4.0f,  0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
   -4.0f, -0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    //create 2 buffers
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    //stride between vertexs
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //create atribute pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateKnifeBladeMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions         //Normals
        // ------------------------------------------------------
        //Back Face        //Negative Z Normal  Texture Coords.
      6.0f, -0.05f, -1.1f, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
      0.0f, -0.05f, -1.1f, 0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
      0.0f,  0.05f, -1.1f, 0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
      0.0f,  0.05f, -1.1f, 0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
      6.0f,  0.05f, -1.1f, 0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
      6.0f, -0.05f, -1.1f, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

      //Front Face         //Positive Z Normal
     6.0f, -0.05f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     0.0f, -0.05f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     0.0f,  0.05f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     0.0f,  0.05f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     6.0f,  0.05f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
     6.0f, -0.05f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

     //Left Face          //Negative X Normal
    0.0f, -0.05f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    0.0f,  0.05f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    0.0f,  0.05f, -1.1f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    0.0f,  0.05f, -1.1f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    0.0f, -0.05f, -1.1f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    0.0f, -0.05f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    //Bottom Face        //Negative Y Normal
    6.0f, -0.05f, 0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    0.0f, -0.05f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
    0.0f, -0.05f, -1.1f, 0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    0.0f, -0.05f, -1.1f, 0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    6.0f, -0.05f, -1.1f, 0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    6.0f, -0.05f, 0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
     6.0f,  0.05f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.0f,  0.05f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.0f,  0.05f, -1.1f, 0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.0f,  0.05f, -1.1f, 0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     6.0f,  0.05f, -1.1f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
     6.0f,  0.05f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,

     //pyr side 1 top
     6.0f,  0.05f, -1.1f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f,//4
     6.0f,  0.05f,  0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f,//8
     8.0f, 0.0f ,  0.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.5f,//x

     //pyr side 2 back
     6.0f, -0.05f, -1.1f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,//1
     6.0f,  0.05f, -1.1f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f,//4
     8.0f, 0.0f ,  0.0f, 0.0f,  0.0f, -1.0f, 1.0f, 0.5f,//x

     //pyr side 3 front
     6.0f,  0.05f, 0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,//8
     6.0f, -0.05f, 0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,//5
     8.0f, 0.0f , 0.0f, 0.0f,  0.0f, 1.0f, 1.0f, 0.5f,//x

     //pyr side 4 bottom
     6.0f, -0.05f, 0.5f, 0.0f,  -1.0f,  0.0f, 0.0f, 0.0f,//5
     6.0f, -0.05f, -1.1f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,//1
     8.0f, 0.0f ,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.5f//x
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    //create 2 buffers
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    //stride between vertexs
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //create atribute pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateCuttingBoardMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
       -4.0f,  0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  0.15f, 0.2f,
        4.0f,  0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  0.15f, 0.9f,
        4.0f,  0.5f, -3.0f,  0.0f,  0.0f, -1.0f,  0.18f, 0.9f,
        4.0f,  0.5f, -3.0f,  0.0f,  0.0f, -1.0f,  0.18f, 0.9f,
       -4.0f,  0.5f, -3.0f,  0.0f,  0.0f, -1.0f,  0.18f, 0.2f,
       -4.0f,  0.0f, -3.0f,  0.0f,  0.0f, -1.0f,  0.15f, 0.2f,

       //Front Face         //Positive Z Normal
      -4.0f,  0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.15f, 0.2f,
       4.0f,  0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.15f, 0.9f,
       4.0f,  0.5f,  3.0f,  0.0f,  0.0f,  1.0f,  0.18f, 0.9f,
       4.0f,  0.5f,  3.0f,  0.0f,  0.0f,  1.0f,  0.18f, 0.9f,
      -4.0f,  0.5f,  3.0f,  0.0f,  0.0f,  1.0f,  0.18f, 0.2f,
      -4.0f,  0.0f,  3.0f,  0.0f,  0.0f,  1.0f,  0.15f, 0.2f,

      //Left Face          //Negative X Normal
     -4.0f,  0.5f,  3.0f, -1.0f,  0.0f,  0.0f,  0.15f, 0.2f,
     -4.0f,  0.5f, -3.0f, -1.0f,  0.0f,  0.0f,  0.15f, 0.9f,
     -4.0f,  0.0f, -3.0f, -1.0f,  0.0f,  0.0f,  0.18f, 0.9f,
     -4.0f,  0.0f, -3.0f, -1.0f,  0.0f,  0.0f,  0.18f, 0.9f,
     -4.0f,  0.0f,  3.0f, -1.0f,  0.0f,  0.0f,  0.18f, 0.2f,
     -4.0f,  0.5f,  3.0f, -1.0f,  0.0f,  0.0f,  0.15f, 0.2f,

     //Right Face         //Positive X Normal
     4.0f,  0.5f,  3.0f,  1.0f,  0.0f,  0.0f,  0.15f, 0.2f,
     4.0f,  0.5f, -3.0f,  1.0f,  0.0f,  0.0f,  0.15f, 0.9f,
     4.0f,  0.0f, -3.0f,  1.0f,  0.0f,  0.0f,  0.18f, 0.9f,
     4.0f,  0.0f, -3.0f,  1.0f,  0.0f,  0.0f,  0.18f, 0.9f,
     4.0f,  0.0f,  3.0f,  1.0f,  0.0f,  0.0f,  0.18f, 0.2f,
     4.0f,  0.5f,  3.0f,  1.0f,  0.0f,  0.0f,  0.15f, 0.2f,

     //Bottom Face        //Negative Y Normal
    -4.0f,  0.0f, -3.0f,  0.0f,  1.0f,  0.0f,  0.15f, 0.1f,
     4.0f,  0.0f, -3.0f,  0.0f,  1.0f,  0.0f,  0.15f, 0.9f,
     4.0f,  0.0f,  3.0f,  0.0f,  1.0f,  0.0f,  0.85f, 0.9f,
     4.0f,  0.0f,  3.0f,  0.0f,  1.0f,  0.0f,  0.85f, 0.9f,
    -4.0f,  0.0f,  3.0f,  0.0f,  1.0f,  0.0f,  0.85f, 0.1f,
    -4.0f,  0.0f, -3.0f,  0.0f,  1.0f,  0.0f,  0.15f, 0.1f,

    //Top Face           //Positive Y Normal
   -4.0f,  0.5f, -3.0f,  0.0f,  1.0f,  0.0f,  0.15f, 0.1f,
    4.0f,  0.5f, -3.0f,  0.0f,  1.0f,  0.0f,  0.15f, 0.9f,
    4.0f,  0.5f,  3.0f,  0.0f,  1.0f,  0.0f,  0.85f, 0.9f,
    4.0f,  0.5f,  3.0f,  0.0f,  1.0f,  0.0f,  0.85f, 0.9f,
   -4.0f,  0.5f,  3.0f,  0.0f,  1.0f,  0.0f,  0.85f, 0.1f,
   -4.0f,  0.5f, -3.0f,  0.0f,  1.0f,  0.0f,  0.15f, 0.1f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    //create 2 buffers
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    //stride between vertexs
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //create atribute pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreatePlaneMesh(GLMesh& mesh)
{
    GLuint repeatUp = 2.0f;
    GLuint RepeatHori = 3.0f;
    // Position and Color data
    GLfloat verts[] = {
        //Positions            //Normals
        // ------------------------------------------------------
        //Top Face             //Positive Y Normal
       -15.0f,  0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  0.0f,     0.0f,
        15.0f,  0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  0.0f,     RepeatHori,
        15.0f,  0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  repeatUp, RepeatHori,
        15.0f,  0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  repeatUp, RepeatHori,
       -15.0f,  0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  repeatUp, 0.0f,
       -15.0f,  0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  0.0f,     0.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    //create 2 buffers
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    //stride between vertexs
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //create atribute pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//creates the mesh for the salami ends
void UCreateSalamiEndsMesh(GLMesh& mesh) {
    GLfloat verts[] = {
        //Positions          //Normals
        // ------------------------------------------------------

        //Bottom Face        //Negative Y Normal
       -0.4f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.33f, 1.0f,
        0.4f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.66f, 1.0f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
        0.4f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.66f, 1.0f,
        1.0f, -1.0f, -0.4f,  0.0f, -1.0f,  0.0f,  1.0f, 0.66f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
        1.0f, -1.0f,  0.4f,  0.0f, -1.0f,  0.0f,  1.00f, 0.33f,
        1.0f, -1.0f, -0.4f,  0.0f, -1.0f,  0.0f,  1.0f, 0.66f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
        0.4f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.66f, 0.0f,
        1.0f, -1.0f,  0.4f,  0.0f, -1.0f,  0.0f,  1.00f, 0.33f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
        0.4f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.66f, 0.0f,
        1.0f, -1.0f,  0.4f,  0.0f, -1.0f,  0.0f,  1.00f, 0.33f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
        0.4f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.66f, 0.0f,
       -0.4f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.33f, 0.00f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
       -1.0f, -1.0f,  0.4f,  0.0f, -1.0f,  0.0f,  0.0f, 0.33f,
       -0.4f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.33f, 0.0f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
       -1.0f, -1.0f,  0.4f,  0.0f, -1.0f,  0.0f,  0.0f, 0.33f,
       -1.0f, -1.0f, -0.4f,  0.0f, -1.0f,  0.0f,  0.0f, 0.66f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,
       -0.4f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.33f, 1.0f,
       -1.0f, -1.0f, -0.4f,  0.0f, -1.0f,  0.0f,  0.0f, 0.66f,
        0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.5f, 0.5f,

        //Top Face        //Positive Y Normal
       -0.4f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.33f, 1.0f,
        0.4f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.66f, 1.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
        0.4f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.66f, 1.0f,
        1.0f,  1.0f, -0.4f,  0.0f,  1.0f,  0.0f,  1.0f, 0.66f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
        1.0f,  1.0f,  0.4f,  0.0f,  1.0f,  0.0f,  1.00f, 0.33f,
        1.0f,  1.0f, -0.4f,  0.0f,  1.0f,  0.0f,  1.0f, 0.66f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
        0.4f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.66f, 0.0f,
        1.0f,  1.0f,  0.4f,  0.0f,  1.0f,  0.0f,  1.00f, 0.33f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
        0.4f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.66f, 0.0f,
        1.0f,  1.0f,  0.4f,  0.0f,  1.0f,  0.0f,  1.00f, 0.33f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
        0.4f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.66f, 0.0f,
       -0.4f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.33f, 0.00f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
       -1.0f,  1.0f,  0.4f,  0.0f,  1.0f,  0.0f,  0.0f, 0.33f,
       -0.4f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.33f, 0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
       -1.0f,  1.0f,  0.4f,  0.0f,  1.0f,  0.0f,  0.0f, 0.33f,
       -1.0f,  1.0f, -0.4f,  0.0f,  1.0f,  0.0f,  0.0f, 0.66f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f,
       -0.4f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.33f, 1.0f,
       -1.0f,  1.0f, -0.4f,  0.0f,  1.0f,  0.0f,  0.0f, 0.66f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f, 0.5f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//creates the mesh for the salami body
void UCreateSalamiBodyMesh(GLMesh& mesh) {
    // Position and Color data
    GLfloat verts[] = {
        //South Face         //Positive Z Normal
       -0.4f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.4f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.4f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.4f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
       -0.4f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       -0.4f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

       //North Face         //Negative Z Normal
      -0.4f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       0.4f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
       0.4f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
       0.4f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
      -0.4f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
      -0.4f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

      //East Face         //Positive X Normal
      1.0f, 1.0f, 0.4f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 1.0f, -0.4f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      1.0f, -1.0f, -0.4f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, -0.4f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.4f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      1.0f, 1.0f, 0.4f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

      //West Face         //Negative X Normal
    -1.0f, 1.0f, 0.4f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, -0.4f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, -0.4f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, -0.4f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.4f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 0.4f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

    //NorthEst Face       
    1.0f, -1.0f, -0.4f, 0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
    0.4f, -1.0f, -1.0f, 0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
    0.4f, 1.0f, -1.0f, 0.5f, 0.0f, -0.5f, 1.0f, 1.0f,
    0.4f, 1.0f, -1.0f, 0.5f, 0.0f, -0.5f, 1.0f, 1.0f,
    1.0f, 1.0f, -0.4f, 0.5f, 0.0f, -0.5f, 0.0f, 1.0f,
    1.0f, -1.0f, -0.4f, 0.5f, 0.0f, -0.5f, 0.0f, 0.0f,

    //Southwest Face       
    -0.4f, -1.0f, 1.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f,
    -1.0f, -1.0f, 0.4f, -0.5f, 0.0f, 0.5f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.4f, -0.5f, 0.0f, 0.5f, 1.0f, 1.0f,
    -1.0f, 1.0f, 0.4f, -0.5f, 0.0f, 0.5f, 1.0f, 1.0f,
    -0.4f, 1.0f, 1.0f, -0.5f, 0.0f, 0.5f, 0.0f, 1.0f,
    -0.4f, -1.0f, 1.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f,

    //Northwest Face
    -1.0f, -1.0f, -0.4f, -0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
    -0.4f, -1.0f, -1.0f, -0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
    -0.4f, 1.0f, -1.0f, -0.5f, 0.0f, -0.5f, 1.0f, 1.0f,
    -0.4f, 1.0f, -1.0f, -0.5f, 0.0f, -0.5f, 1.0f, 1.0f,
    -1.0f, 1.0f, -0.4f, -0.5f, 0.0f, -0.5f, 0.0f, 1.0f,
    -1.0f, -1.0f, -0.4f, -0.5f, 0.0f, -0.5f, 0.0f, 0.0f,

    //Southeast Face
    1.0f, -1.0f, 0.4f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f,
    0.4f, -1.0f, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 0.0f,
    0.4f, 1.0f, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f,
    0.4f, 1.0f, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.4f, 0.5f, 0.0f, 0.5f, 0.0f, 1.0f,
    1.0f, -1.0f, 0.4f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//creates the mesh for the light
void UCreateCheeseMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

       //Front Face         //Positive Z Normal
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

      //Left Face          //Negative X Normal
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Right Face         //Positive X Normal
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Bottom Face        //Negative Y Normal
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    //create 2 buffers
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    //stride between vertexs
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //create atribute pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//load the texture
bool UCreateTexture(const char* filename, GLuint& textureId, int position) {
    if (position == 0) {
        glActiveTexture(GL_TEXTURE0);
    }
    else if (position == 1) {
        glActiveTexture(GL_TEXTURE1);
    }
    else if (position == 2) {
        glActiveTexture(GL_TEXTURE2);
    }
    else if (position == 3) {
        glActiveTexture(GL_TEXTURE3);
    }
    else if (position == 4) {
        glActiveTexture(GL_TEXTURE4);
    }
    else if (position == 5) {
        glActiveTexture(GL_TEXTURE5);
    }
    else if (position == 6) {
        glActiveTexture(GL_TEXTURE6);
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    //set tex wrapping param
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //set tex filtering param
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load image, create tex, and generate mipmaps
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);

        //unbinds the texture
        glBindTexture(GL_TEXTURE_2D, position);

        return true;
    }
    else {
        return false;
    }
}

//destroys texture 
void UDestroyTexture(GLuint textureId) {
    glGenTextures(1, &textureId);
}


void UDestroyMesh(GLMesh& mesh) {
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

//implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    //compilation and error reporting
    int success = 0;
    char infoLog[512];

    //creates shader object.
    programId = glCreateProgram();

    //create the vertex and shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    //retrive shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    //compile vertex shader and print compilation errors 
    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    glCompileShader(fragmentShaderId);

    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    //attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);
    //check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }


    glUseProgram(programId);

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}