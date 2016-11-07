// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL); // Windowed
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight);

    // Setup and compile our shaders
    Shader shader("shaders/model/model.vertexshader", "shaders/model/model.fragmentshader");

    Shader lampShader("shaders/multiplelights/lamp.vertexshader", "shaders/multiplelights/lamp.fragmentshader");

    GLfloat vertices[] = {
    // Back face
    -0.5f, -0.5f, -0.5f, // Bottom-left
     0.5f, -0.5f, -0.5f, // bottom-right
     0.5f,  0.5f, -0.5f, // top-right
     0.5f,  0.5f, -0.5f, // top-right
    -0.5f,  0.5f, -0.5f, // top-left
    -0.5f, -0.5f, -0.5f, // bottom-left
    // Front face
    -0.5f, -0.5f,  0.5f, // bottom-left
     0.5f,  0.5f,  0.5f, // top-right
     0.5f, -0.5f,  0.5f, // bottom-right
     0.5f,  0.5f,  0.5f, // top-right
    -0.5f, -0.5f,  0.5f, // bottom-left
    -0.5f,  0.5f,  0.5f, // top-left
    // Left face
    -0.5f,  0.5f,  0.5f, // top-right
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f,  0.5f, -0.5f, // top-left
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f,  0.5f,  0.5f, // top-right
    -0.5f, -0.5f,  0.5f, // bottom-right
    // Right face
     0.5f,  0.5f,  0.5f, // top-left
     0.5f,  0.5f, -0.5f, // top-right
     0.5f, -0.5f, -0.5f, // bottom-right
     0.5f, -0.5f, -0.5f, // bottom-right
     0.5f, -0.5f,  0.5f, // bottom-left
     0.5f,  0.5f,  0.5f, // top-left
    // Bottom face
    -0.5f, -0.5f, -0.5f, // top-right
     0.5f, -0.5f,  0.5f, // bottom-left
     0.5f, -0.5f, -0.5f, // top-left
     0.5f, -0.5f,  0.5f, // bottom-left
    -0.5f, -0.5f, -0.5f, // top-right
    -0.5f, -0.5f,  0.5f, // bottom-right
    // Top face
    -0.5f,  0.5f, -0.5f, // top-left
     0.5f,  0.5f, -0.5f, // top-right
     0.5f,  0.5f,  0.5f, // bottom-right
     0.5f,  0.5f,  0.5f, // bottom-right
    -0.5f,  0.5f,  0.5f, // bottom-left
    -0.5f,  0.5f, -0.5f // top-left
};


    // First, set the VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // Set the vertex attributes (only position data for the lamp))
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    // Positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.0f, 1.0f, 1.0f),
        glm::vec3( 0.0f, 0.0f, -1.0f)
    };


    glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
    };

    // Load models
    //Model ourModel("models/JillValentine/pl1010.obj");
    //Model ourModel("models/JillLow/JillOBJ.obj");
    //Model ourModel("models/Nanosuit/nanosuit.obj");
    //Model ourModel("models/Juliet_Striped_Bikini/Juliet_Striped_Bikini.dae");
    //Model ourModel("models/houseplant4/house plant.obj");
    //Model ourModel("models/MM/MM.obj");
    //Model ourModel("models/Elexis/elexis.obj");
    //Model ourModel("models/Juliet_Purple_Pride/Juliet_Purple_Pride.dae");
    //Model ourModel("models/DefenderLingerie/DefenderLingerie00.blend");
    //Model ourModel("models/Bikini_Girl/Bikini_Girl.dae");
    //Model ourModel("models/Castle/Castle OBJ.obj");
    //Model ourModel("models/castle2/castle/castle.obj");
    //Model ourModel("models/earth/earth.obj");
    //Model ourModel("models/Moon_3D_Model/moon.obj");
    //Model ourModel("models/OldHouse2/Old House 2 3D Models.obj");
    Model ourModel("models/AstroBoy_Walk/astroBoy_walk_Max.dae");

    // Draw in wireframe
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    //Por temas de rendimiento. Solo pinta las caras visibles
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);


    // Game loop
    while(!glfwWindowShouldClose(window)){
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        Do_Movement();

        // Clear the colorbuffer
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();   // <-- Don't forget this one!
        glFrontFace(GL_CW);


        GLint viewPosLoc = glGetUniformLocation(shader.Program, "viewPos");
        // Set material properties
        glUniform1f(glGetUniformLocation(shader.Program, "material_shininess"), 32.0f);
        glUniform1i(glGetUniformLocation(shader.Program, "texture_diffuse"), 0);
        glUniform1i(glGetUniformLocation(shader.Program, "texture_specular"), 1);

        glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

        // Directional light
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.2f, 0.2f, 0.2);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"), 0.7f, 0.7f, 0.7f);

        // Point light 1
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), pointLightColors[0].x * 0.1,  pointLightColors[0].y * 0.1,  pointLightColors[0].z * 0.1);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.09);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.032);
        // Point light 2
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), pointLightColors[1].x * 0.1,  pointLightColors[1].y * 0.1,  pointLightColors[1].z * 0.1);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.09);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.032);

        // SpotLight
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.position"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.direction"), camera.Front.x, camera.Front.y, camera.Front.z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.linear"), 0.009);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.quadratic"), 0.0032);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.cutOff"), glm::cos(glm::radians(8.0f)));
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(12.5f)));

        GLint transInversLoc  = glGetUniformLocation(shader.Program,  "transInversMatrix");

        // Transformation matrices
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Draw the loaded model
        glm::mat4 model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //Calculamos la inversa de la matriz por temas de iluminacion y rendimiento
        glm::mat4 transInversMatrix = transpose(inverse(model));
        glUniformMatrix4fv(transInversLoc, 1, GL_FALSE, glm::value_ptr(transInversMatrix));
        ourModel.Draw(shader);

        // Also draw the lamp object, again binding the appropriate shader
        lampShader.Use();
        glFrontFace(GL_CW);

        // Set matrices
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // We now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightVAO);
        for (GLuint i = 0; i < 2; i++){
            model = glm::mat4();
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glm::mat4 transInversMatrix = transpose(inverse(model));
            glUniformMatrix4fv(glGetUniformLocation(lampShader.Program,  "transInversMatrix"), 1, GL_FALSE, glm::value_ptr(transInversMatrix));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

#pragma endregion
