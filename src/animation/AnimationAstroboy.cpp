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

#include "../lights/light.h"

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
void processLights(vector<Light *> &luces, Shader &shader);
void initLights(vector<Light *> &luces, Shader &shader);

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
    Shader shader("shaders/animation/model.vertexshader", "shaders/animation/model.fragmentshader");

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

    shader.Use();   // <-- Don't forget this one!

    // Load models
    //Model ourModel("models/JillValentine/pl1010.obj");
    //Model ourModel("models/JillLow/JillOBJ.obj");
//    Model ourModel("models/Nanosuit/nanosuit.obj", &shader);
    //Model ourModel("models/Juliet_Striped_Bikini/Juliet_Striped_Bikini.dae");
    //Model ourModel("models/houseplant4/house plant.obj");
    //Model ourModel("models/MM/MM.obj");
    //Model ourModel("models/Elexis/elexis.obj");
    //Model ourModel("models/Juliet_Purple_Pride/Juliet_Purple_Pride.dae");
    //Model ourModel("models/DefenderLingerie/DefenderLingerie00.blend");
    //Model ourModel("models/Bikini_Girl/Bikini_Girl.dae", &shader);
    //Model ourModel("models/Castle/Castle OBJ.obj");
    //Model ourModel("models/castle2/castle/castle.obj");
    //Model ourModel("models/earth/earth.obj");
    //Model ourModel("models/Moon_3D_Model/moon.obj");
    //Model ourModel("models/OldHouse2/Old House 2 3D Models.obj");
    //
//    Model ourModel("models/guard/boblampclean.md5mesh",&shader, 15);
    //Model ourModel("models/Sonic/Sonic.obj");
    //Model ourModel("models/Alien_Warrior/Alien_Warrior.dae");
    //Model ourModel("models/GirlGame/Girl game N240416.obj");
    //Model ourModel("models/cs_italy/cs_italy.obj", &shader);
    Model ourWorld("models/cs_assault/cs_assault.obj", &shader);
    //Model ourModel("models/Small Tropical Island/Small Tropical Island.obj", &shader);
    //Model ourModel("models/lux/luxury house interior.obj", &shader);
    Model ourModel("models/ArmyPilot/ArmyPilot.ms3d", &shader, 30, false);
    //Model ourModel("models/xna/dude.dae", &shader, 60);
    //Model ourModel("models/Police2/Police.obj", &shader, 60);
//    Model ourModel("models/ninja/ninja.ms3d", &shader, 60);

    // Draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    //Por temas de rendimiento. Solo pinta las caras visibles
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    double lastTime = 0;
    int nbFrames = 0;

    vector<Light *> luces;
    initLights(luces, shader);

    GLint personLoc = glGetUniformLocation(shader.Program, "model");

    GLfloat initTime = glfwGetTime();
    // Game loop
    while(!glfwWindowShouldClose(window)){
        // Set frame time
        GLfloat currentFrame = glfwGetTime() - initTime;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        nbFrames++;
        if ( currentFrame - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
            // printf and reset timer
            printf("%d frames/s\n", nbFrames);
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Check and call events
        glfwPollEvents();
        Do_Movement();

        // Clear the colorbuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.Use();   // <-- Don't forget this one!

        //Camera positions
        GLint viewPosLoc = glGetUniformLocation(shader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

        //Light processing
        processLights(luces, shader);

        // Transformation matrices
        GLint transInversLoc  = glGetUniformLocation(shader.Program,  "transInversMatrix");
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Draw the loaded model
        glm::mat4 model;
        glm::mat4 transInversMatrix;

        /** para la escena del modelo*/
        for (int j=0; j < 70; j++){
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(j*2 % 20, -1.75f, j*2 / 20)); // Translate it down a bit so it's at the center of the scene
//            model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));	// Para astroboy
//            model = glm::scale(model, glm::vec3(0.035f, 0.035f, 0.035f));	// Para el de la lampara
//            model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));	// Para nanosuit
            //model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));	// Para la bikinigirl
//            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Para la bikinigirl
            model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.005f));	// Para el piloto
//            model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));	// Para el xna model
//            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Para el xna model
            //model = glm::scale(model, glm::vec3(0.035f, 0.035f, 0.035f));	// Para el poli de half life 2
//            model = glm::scale(model, glm::vec3(0.20f, 0.20f, 0.20f));	// Para el ninja

            glUniformMatrix4fv(personLoc, 1, GL_FALSE, glm::value_ptr(model));
            //Calculamos la inversa de la matriz por temas de iluminacion y rendimiento
            transInversMatrix = transpose(inverse(model));
            glUniformMatrix4fv(transInversLoc, 1, GL_FALSE, glm::value_ptr(transInversMatrix));
            //Drawing the model
            ourModel.Draw(&shader, currentFrame);
        }
        /**Fin modelo*/


        /**Para el escenario*/
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, -100.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));	// It's a bit too big for our scene, so scale it down
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //Calculamos la inversa de la matriz por temas de iluminacion y rendimiento
        transInversMatrix = transpose(inverse(model));
        glUniformMatrix4fv(transInversLoc, 1, GL_FALSE, glm::value_ptr(transInversMatrix));
        //Drawing the model
        ourWorld.Draw(&shader, currentFrame);
        /** Fin escenario*/

        // Also draw the lamp object, again binding the appropriate shader
        lampShader.Use();
        //glFrontFace(GL_CW);

        // Set matrices
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // We now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightVAO);
        for (GLuint i = 0; i < 2; i++){
            model = glm::mat4();
            model = glm::translate(model, luces.at(i+1)->vPosition);
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

/**
* Moves/alters the camera positions based on user input
*/
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

/**
* Is called whenever a key is pressed/released via GLFW
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

/**
* Callback para el raton
*/
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

/**
* Callback para la rueda del mouse
*/
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

/**
* Inicia los valores de las luces
*/
void initLights(vector<Light *> &luces, Shader &shader){

    // Positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.0f, 1.0f, 1.0f),
        glm::vec3( 0.0f, 0.0f, -1.0f)
    };

    glm::vec3 pointLightColors[] = {
        glm::vec3(1,1,1),
        glm::vec3(1,1,1)
    };

    Light *luz = new Light();
    luz->locDirection = glGetUniformLocation(shader.Program, "dirLight.direction");
    luz->locAmbient = glGetUniformLocation(shader.Program, "dirLight.ambient");
    luz->locDiffuse = glGetUniformLocation(shader.Program, "dirLight.diffuse");
    luz->locSpecular = glGetUniformLocation(shader.Program, "dirLight.specular");
    luz->vAmbient = glm::vec3(0.6f, 0.6f, 0.6f);
    luz->vDiffuse = glm::vec3(0.1f, 0.1f, 0.1f);
    luz->vSpecular = glm::vec3(0.1f, 0.1f, 0.1f);
    luz->vDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
    luces.push_back(luz);

    Pointlight *luz2 = new Pointlight();
    luz2->locPosition = glGetUniformLocation(shader.Program,  "pointLights[0].position");
    luz2->locAmbient = glGetUniformLocation(shader.Program,  "pointLights[0].ambient");
    luz2->locDiffuse = glGetUniformLocation(shader.Program,  "pointLights[0].diffuse");
    luz2->locSpecular = glGetUniformLocation(shader.Program,  "pointLights[0].specular");
    luz2->locConstant = glGetUniformLocation(shader.Program, "pointLights[0].constant");
    luz2->locLinear = glGetUniformLocation(shader.Program,   "pointLights[0].linear");
    luz2->locQuadratic = glGetUniformLocation(shader.Program, "pointLights[0].quadratic");

    luz2->vPosition = glm::vec3(pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
    luz2->vAmbient = glm::vec3(pointLightColors[0].x * 0.3f,  pointLightColors[0].y * 0.3f,  pointLightColors[0].z * 0.3f);
    luz2->vDiffuse = glm::vec3(pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
    luz2->vSpecular = glm::vec3(pointLightColors[0].x,  pointLightColors[0].y,  pointLightColors[0].z);
    luz2->vConstant = 1.0f;
    luz2->vLinear = 0.09f;
    luz2->vQuadratic = 0.032f;
    luces.push_back(luz2);

    Pointlight *luz3 = new Pointlight();
    luz3->locPosition = glGetUniformLocation(shader.Program,  "pointLights[1].position");
    luz3->locAmbient = glGetUniformLocation(shader.Program, "pointLights[1].ambient");
    luz3->locDiffuse = glGetUniformLocation(shader.Program, "pointLights[1].diffuse");
    luz3->locSpecular = glGetUniformLocation(shader.Program, "pointLights[1].specular");
    luz3->locConstant = glGetUniformLocation(shader.Program, "pointLights[1].constant");
    luz3->locLinear = glGetUniformLocation(shader.Program, "pointLights[1].linear");
    luz3->locQuadratic = glGetUniformLocation(shader.Program, "pointLights[1].quadratic");

    luz3->vPosition = glm::vec3(pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
    luz3->vAmbient = glm::vec3(pointLightColors[1].x * 0.3f,  pointLightColors[1].y * 0.3f,  pointLightColors[1].z * 0.3f);
    luz3->vDiffuse = glm::vec3(pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
    luz3->vSpecular = glm::vec3(pointLightColors[1].x,  pointLightColors[1].y,  pointLightColors[1].z);
    luz3->vConstant = 1.0f;
    luz3->vLinear = 0.09f;
    luz3->vQuadratic = 0.032f;
    luces.push_back(luz3);

    SpotLight *luz4 = new SpotLight();
    luz4->locPosition = glGetUniformLocation(shader.Program,  "spotLight.position");
    luz4->locDirection = glGetUniformLocation(shader.Program, "spotLight.direction");
    luz4->locAmbient = glGetUniformLocation(shader.Program, "spotLight.ambient");
    luz4->locDiffuse = glGetUniformLocation(shader.Program, "spotLight.diffuse");
    luz4->locSpecular = glGetUniformLocation(shader.Program, "spotLight.specular");
    luz4->locConstant = glGetUniformLocation(shader.Program, "spotLight.constant");
    luz4->locLinear = glGetUniformLocation(shader.Program, "spotLight.linear");
    luz4->locQuadratic = glGetUniformLocation(shader.Program, "spotLight.quadratic");
    luz4->locCutOff = glGetUniformLocation(shader.Program, "spotLight.cutOff");
    luz4->locOuterCutOff = glGetUniformLocation(shader.Program, "spotLight.outerCutOff");

    luz4->vPosition = glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z);
    luz4->vDirection = glm::vec3(camera.Front.x, camera.Front.y, camera.Front.z);
    luz4->vAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
    luz4->vDiffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    luz4->vSpecular = glm::vec3(0.0f, 0.0f, 0.0f);
    luz4->vConstant = 1.0f;
    luz4->vLinear = 0.009f;
    luz4->vQuadratic = 0.0032f;
    luz4->vCutOff = glm::cos(glm::radians(8.0f));
    luz4->vOuterCutOff = glm::cos(glm::radians(12.5f));
    luces.push_back(luz4);
}

/**
*
*/
void processLights(vector<Light *> &luces, Shader &shader){
    for (int i=0; i < luces.size(); i++){
        Light *luz = luces.at(i);

        //Exceptional case in which light needs to be specified by camera data
        if (luz->lightType == SPOTLIGHT){
            luz->vPosition = glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z);
            luz->vDirection = glm::vec3(camera.Front.x, camera.Front.y, camera.Front.z);
        }

        if (luz->lightType == AMBIENTLIGHT || luz->lightType == POINTLIGHT
            || luz->lightType == SPOTLIGHT){
            // Directional light
            glUniform3fv(luz->locAmbient, 1, &luz->vAmbient[0]);
            glUniform3fv(luz->locDirection, 1, &luz->vDirection[0]);
            glUniform3fv(luz->locDiffuse, 1, &luz->vDiffuse[0]);
            glUniform3fv(luz->locSpecular, 1, &luz->vSpecular[0]);
        }

        if (luz->lightType == POINTLIGHT || luz->lightType == SPOTLIGHT){
            Pointlight *pointluz = (Pointlight *) luces.at(i);
            glUniform3fv(pointluz->locPosition, 1, &pointluz->vPosition[0]);
            glUniform1f(pointluz->locConstant,pointluz->vConstant);
            glUniform1f(pointluz->locLinear,pointluz->vLinear);
            glUniform1f(pointluz->locQuadratic,pointluz->vQuadratic);
        }

        if (luz->lightType == SPOTLIGHT){
            SpotLight *spotluz = (SpotLight *) luces.at(i);
            glUniform1f(spotluz->locCutOff,spotluz->vCutOff);
            glUniform1f(spotluz->locOuterCutOff,spotluz->vOuterCutOff);
        }
    }
}

#pragma endregion
