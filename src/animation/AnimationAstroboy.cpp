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
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Other Libs
#include <SOIL.h>

//Scene
#include "sceneobjects.h"

#include "../lights/light.h"

#include "../physics/Physics.h"



// Properties
GLuint screenWidth = 1280, screenHeight = 720;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
void processLights(vector<Light *> &luces, Shader &shader);
void initLights(vector<Light *> &luces, Shader &shader);
void activateObjectOutlining();
int initShape(btVector3 initialPosition);
int initGround();
GLuint loadTexture(GLchar* path);
bool getOMWorld(int i, glm::vec3 scale, glm::vec3 offset, glm::mat4 &model);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//Camera camera(glm::vec3(0.0f, 20.0f, 25.0f));

bool keys[1024];
GLfloat lastX = 640, lastY = 480;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Physics *physicsEngine = new Physics();

glm::vec2 escenaAtras = glm::vec2(2.85,2.85);
glm::vec2 escenaAdelante = glm::vec2(0.0,3.0);
glm::vec2 escenaParado = glm::vec2(21.25,10.0);
glm::vec2 estadoPersonaje = escenaParado;



//struct ContactSensorCallback : public btCollisionWorld::ContactResultCallback {
//
//    //! Constructor, pass whatever context you want to have available when processing contacts
//    /*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
//     *  (supplied by the superclass) for needsCollision() */
//    ContactSensorCallback(btRigidBody& tgtBody /*, YourContext& context*/ /*, ... */)
//        : btCollisionWorld::ContactResultCallback(), body(tgtBody)/*, ctxt(context)*/ { }
//
//    btRigidBody& body; //!< The body the sensor is monitoring
//    //YourContext& ctxt; //!< External information for contact processing
//
//    //! If you don't want to consider collisions where the bodies are joined by a constraint, override needsCollision:
//    /*! However, if you use a btCollisionObject for #body instead of a btRigidBody,
//     *  then this is unnecessary—checkCollideWithOverride isn't available */
//    virtual bool needsCollision(btBroadphaseProxy* proxy) const {
//        // superclass will check m_collisionFilterGroup and m_collisionFilterMask
//        if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
//            return false;
//        // if passed filters, may also want to avoid contacts between constraints
//        return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
//    }
//
//    //! Called with each contact for your own processing (e.g. test if contacts fall in within sensor parameters)
//    virtual btScalar addSingleResult(btManifoldPoint& cp,
//        const btCollisionObjectWrapper* colObj0,int partId0,int index0,
//        const btCollisionObjectWrapper* colObj1,int partId1,int index1)
//    {
//        btVector3 pt; // will be set to point of collision relative to body
//        if(colObj0->m_collisionObject==&body) {
//            pt = cp.m_localPointA;
//        } else {
//            assert(colObj1->m_collisionObject==&body && "body does not match either collision object");
//            pt = cp.m_localPointB;
//        }
//        // do stuff with the collision point
//        return 0; // There was a planned purpose for the return value of addSingleResult, but it is not used so you can ignore it.
//    }
//};
// The MAIN function, from here we start our application and run our Game loop
int main(int argc, char *argv[]){

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL); // Windowed
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. OpenGL2.1 needed\n" );
        glfwTerminate();
        return -1;
    }

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

    Shader floorShader("shaders/animation/floor.vertexshader", "shaders/animation/floor.fragmentshader");

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

    GLfloat planeVertices[] = {
        // Positions            // Texture Coords (note we set these higher than 1 that together with GL_REPEAT as texture wrapping mode will cause the floor texture to repeat)
        5.0f,  -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

        5.0f,  -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
        5.0f,  -0.5f, -5.0f,  2.0f, 2.0f
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

    // Setup plane VAO
    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    GLuint floorTexture = loadTexture("res/wall.jpg");

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
    //Model *ourModel = new Model("models/guard/boblampclean.md5mesh",&shader, 3, true);
    //Model ourModel("models/Sonic/Sonic.obj");
    //Model ourModel("models/Alien_Warrior/Alien_Warrior.dae");
    //Model ourModel("models/GirlGame/Girl game N240416.obj");
    //Model ourModel("models/cs_italy/cs_italy.obj", &shader);
    Model *ourWorld = new Model("models/cs_assault/cs_assault.obj", &shader);
    //Model ourModel("models/Small Tropical Island/Small Tropical Island.obj", &shader);
    //Model ourModel("models/lux/luxury house interior.obj", &shader);
    Model *ourModel = new Model("models/ArmyPilot/ArmyPilot.ms3d", &shader, 1, true);
//    Model *ourModel = new Model("models/xna/dude.dae", &shader, 30, true);
    //Model ourModel("models/Police2/Police.obj", &shader, 60);
//    Model ourModel("models/ninja/ninja.ms3d", &shader, 60);

    // Draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    //Por temas de rendimiento. Solo pinta las caras visibles
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);


    physicsEngine->getDynamicsWorld()->setGravity(btVector3(0, -9.8f, 0));
    initGround();
    int inicio = 0;

    SceneObjects sceneObjects;
    //sceneObjects.activateStencil(true);

    for (int i=0; i < 2; i++){
        initShape(btVector3(i*2 % 20, 0.0f, i*2 / 20));
//        initShape(btVector3(0, i*3+50, 0));
    }

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
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.Use();   // <-- Don't forget this one!

        glFrontFace(GL_CW);
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
        //Calculate the physics
        physicsEngine->getDynamicsWorld()->stepSimulation(deltaTime); //suppose you have 60 frames per second

        btCollisionObject* obj = physicsEngine->getDynamicsWorld()->getCollisionObjectArray()[1];
        btRigidBody* capsuleBody = btRigidBody::upcast(obj);
        btTransform newTrans = capsuleBody->getWorldTransform();

//        camera.Position.x = newTrans.getOrigin().x() + 0.5;
//        camera.Position.y = newTrans.getOrigin().y() + 0.5;
//        camera.Position.z = newTrans.getOrigin().z();

        for (int i = 0; i< physicsEngine->getCollisionObjectCount(); i++) {
            model = glm::mat4();
            if (getOMWorld(i, glm::vec3(0.005f, 0.005f, 0.005f), glm::vec3(0.0f, -0.5f, 0.0f), model)){
                glUniformMatrix4fv(personLoc, 1, GL_FALSE, glm::value_ptr(model));
                //Calculamos la inversa de la matriz por temas de iluminacion y rendimiento
                transInversMatrix = transpose(inverse(model));
                glUniformMatrix4fv(transInversLoc, 1, GL_FALSE, glm::value_ptr(transInversMatrix));
                //Drawing the model
                ourModel->Draw(&shader, estadoPersonaje.x + fmod(currentFrame * 2.0f, estadoPersonaje.y));
            }
		}
        /**Fin modelo*/

        model = glm::mat4();
        /**Un piso de ejemplo*/
        if (!getOMWorld(0, glm::vec3(1,1,1), glm::vec3(0,0,0), model)){
            glFrontFace(GL_CCW);
            floorShader.Use();
            glUniformMatrix4fv(glGetUniformLocation(floorShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(floorShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(planeVAO);
            glBindTexture(GL_TEXTURE_2D, floorTexture);
            glUniformMatrix4fv(glGetUniformLocation(floorShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
        /**Fin piso de ejemplo*/


        /**Para el escenario*/
//        glFrontFace(GL_CW);
//        model = glm::mat4();
//        model = glm::translate(model, glm::vec3(0.0f, -100.0f, 0.0f));
//        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));	// It's a bit too big for our scene, so scale it down
//        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
//        //Calculamos la inversa de la matriz por temas de iluminacion y rendimiento
//        transInversMatrix = transpose(inverse(model));
//        glUniformMatrix4fv(transInversLoc, 1, GL_FALSE, glm::value_ptr(transInversMatrix));
//        //Drawing the model
//        ourWorld->Draw(&shader, currentFrame);
        /** Fin escenario*/

        // Also draw the lamp object, again binding the appropriate shader
        lampShader.Use();

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

        for (int i = 1; i< physicsEngine->getCollisionObjectCount(); i++) {
            model = glm::mat4();
            if (getOMWorld(i, glm::vec3(0.3,1,0.3), glm::vec3((i-1)*2 % 20, 0, (i-1)*2 / 20), model)){
                glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                transInversMatrix = transpose(inverse(model));
                glUniformMatrix4fv(glGetUniformLocation(lampShader.Program,  "transInversMatrix"), 1, GL_FALSE, glm::value_ptr(transInversMatrix));
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
        }




        // Swap the buffers
        glfwSwapBuffers(window);
    }

    delete ourWorld;
    delete ourModel;
    glfwTerminate();
    return 0;
}

#pragma region "User input"

void activateObjectOutlining(){
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
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

/**
*
*/
int initGround(){
    float w =200.0f, h = 200.0f;
    float groundDeep = 1.0f;
    //create the plane entity to the physics engine, and attach it to the node
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(btScalar(-w/2.0f), btScalar(-groundDeep*2.0f), btScalar(-h/2.0f)));

    float RotationAngle = glm::radians(0.0f);
    groundTransform.setRotation(btQuaternion(0,0,0,cos(RotationAngle / 2.0f)));

    btScalar groundMass(0.); //the mass is 0, because the ground is immovable (static)
    btVector3 localGroundInertia(0, 0, 0);

    btCollisionShape *groundShape = new btBoxShape(btVector3(btScalar(w), btScalar(groundDeep), btScalar(h)));
    //btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 20.0f, 0), 1);

    btDefaultMotionState *groundMotionState = new btDefaultMotionState(groundTransform);

    groundShape->calculateLocalInertia(groundMass, localGroundInertia);

    btRigidBody::btRigidBodyConstructionInfo groundRBInfo(groundMass, groundMotionState, groundShape, localGroundInertia);
    btRigidBody *groundBody = new btRigidBody(groundRBInfo);
    groundBody->setFriction(btScalar(10.0));
    groundBody->setRollingFriction(btScalar(10.0));
    groundBody->setSpinningFriction(btScalar(1.0));
    //add the body to the dynamics world
    physicsEngine->getDynamicsWorld()->addRigidBody(groundBody);

    return 0;
}

/**
*
*/
int initShape(btVector3 initialPosition){
    //create the new shape, and tell the physics that is a Box
    btCollisionShape *newRigidShape = new btCylinderShape(btVector3(0.3f, 1.0f, 0.3f));
    physicsEngine->getCollisionShapes()->push_back(newRigidShape);

    //set the initial position and transform. For this demo, we set the tranform to be none
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setRotation(btQuaternion(0,0,0,1));

    //set the mass of the object. a mass of "0" means that it is an immovable object
    btScalar mass = 0.1f;
    btVector3 localInertia(0,0,0);
    //btVector3 localInertia(1.0f, 1.0f, 1.0f);

    startTransform.setOrigin(initialPosition);
    newRigidShape->calculateLocalInertia(mass, localInertia);

    //actually contruct the body and add it to the dynamics world
    btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, newRigidShape, localInertia);
    btRigidBody *body = new btRigidBody(rbInfo);
    //Set bounce property
    body->setRestitution(0.0);

    object3D2 *obj = new object3D2();
    obj->spinningFriction = 10.0f;
    obj->name = "model";
    obj->instantStop = false;
    body->setUserPointer(obj);

    body->setRollingFriction(0.1f);
    body->setFriction(0.1f);

    //restricting movement in planes
    body->setLinearFactor(btVector3(1,1,1));
    body->setAngularFactor(btVector3(0,0,0));

    body->setActivationState(DISABLE_DEACTIVATION);

    physicsEngine->getDynamicsWorld()->addRigidBody(body);
    //physicsEngine->trackRigidBodyWithName(body, physicsCubeName);
}

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

    btCollisionObject* world = physicsEngine->getDynamicsWorld()->getCollisionObjectArray()[0];
    btCollisionObject* obj = physicsEngine->getDynamicsWorld()->getCollisionObjectArray()[1];

    btRigidBody* capsuleBody = btRigidBody::upcast(obj);
    bool out = false;
    if (capsuleBody && capsuleBody->getMotionState()){
        btTransform trans;
        capsuleBody->getMotionState()->getWorldTransform(trans);

        object3D2 *userPointer = (object3D2 *)capsuleBody->getUserPointer();
        if (userPointer) {
            btVector3 impulseAxis = capsuleBody->getWorldTransform().getBasis().getColumn(0);
            float direction = 1.0;
            float torqueDir = 1.0;

            if (keys[GLFW_KEY_DOWN]){
                estadoPersonaje = escenaAtras;
            } else if (keys[GLFW_KEY_UP]){
                estadoPersonaje = escenaAdelante;
                direction = -1.0;
            } else {
                estadoPersonaje = escenaParado;
                if (userPointer->instantStop){
                    capsuleBody->clearForces();
                    capsuleBody->setLinearVelocity(btVector3(0,0,0));
                    capsuleBody->setAngularVelocity(btVector3(0,0,0));
                }
            }

            if (keys[GLFW_KEY_RIGHT]){
                torqueDir = -1.0;
            }

            if (keys[GLFW_KEY_DOWN] || keys[GLFW_KEY_UP]){
                capsuleBody->setSpinningFriction(userPointer->spinningFriction);
                capsuleBody->setAngularVelocity(btVector3(0,0,0));
                impulseAxis.setX(direction * impulseAxis.x());
                impulseAxis.setZ(direction * impulseAxis.z());
                btScalar projVel = capsuleBody->getLinearVelocity().dot(impulseAxis);
                btScalar desiredVel = userPointer->velocity;
                btScalar diff = desiredVel-projVel;
                capsuleBody->setLinearVelocity(capsuleBody->getLinearVelocity()+(impulseAxis*diff));
//                btTransform newTrans = capsuleBody->getWorldTransform();
//                newTrans.getOrigin() += (btVector3(0.01f, 0, 0.01f));
//                capsuleBody->setWorldTransform(newTrans);
//                btVector3 impulseAxis = ((object3D2 *)userPointer)->impulseAxis;
            }

            if (keys[GLFW_KEY_RIGHT] || keys[GLFW_KEY_LEFT]){
                //Only rotate in y axis
                capsuleBody->setAngularFactor(btVector3(0,1,0));
                //Allow rotation
                capsuleBody->setSpinningFriction(0);
                //Limit linear velocity to not sum too much speed
                btVector3 impulseMove = capsuleBody->getLinearVelocity();
                impulseMove.setY(0.0);
                capsuleBody->setLinearVelocity( capsuleBody->getLinearVelocity() - impulseMove * 0.2 );
                btVector3 objectAxis = capsuleBody->getWorldTransform().getBasis().getColumn(1);
                btVector3 torqueAxis(0, torqueDir * objectAxis.y(), 0);
                btScalar projVel = capsuleBody->getAngularVelocity().dot(torqueAxis);
                btScalar desiredVel = 2.5 * userPointer->velocity;
                btScalar diff = desiredVel-projVel;
                capsuleBody->setAngularVelocity(capsuleBody->getAngularVelocity()+(torqueAxis*diff));
            }
            //cout << capsuleBody->getWorldTransform().getOrigin().getY() << endl;
            //cout << world->getWorldTransform().getOrigin().getY() << endl;

            if (keys[GLFW_KEY_SPACE]){
                SceneObjects::MyContactResultCallback callback;
                physicsEngine->getDynamicsWorld()->contactPairTest(world, obj, callback);
                if (callback.isContact()){
                    btVector3 impulseJump = capsuleBody->getLinearVelocity();
                    impulseJump.setY(3.0f);
                    capsuleBody->setLinearVelocity(impulseJump);
                }
            }
        }
    }
}

// This function loads a texture from file. Note: texture loading functions like these are usually
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio).
// For learning purposes we'll just define it as a utility function.
GLuint loadTexture(GLchar* path)
{
    //Generate texture ID and load texture data
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;

}

/**
*
*/
bool getOMWorld(int i, glm::vec3 scale, glm::vec3 offset,  glm::mat4 &model){
    btCollisionObject* obj = physicsEngine->getDynamicsWorld()->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    bool out = false;
    if (body && body->getMotionState()){
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);

        void *userPointer = body->getUserPointer();
        if (userPointer) {
            btQuaternion orientation = trans.getRotation();
            glm::quat MyQuaternion   = glm::quat(orientation.getW(),orientation.getX(), orientation.getY(), orientation.getZ());
            glm::mat4 RotationMatrix = glm::toMat4(MyQuaternion);
            model = glm::translate(model, glm::vec3(trans.getOrigin().getX()
                    + offset.x, trans.getOrigin().getY() + offset.y, trans.getOrigin().getZ() + offset.z));
            model = glm::scale(model, glm::vec3(scale.x, scale.y, scale.z));	// Para el piloto mesh
            model =  model * RotationMatrix;
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            out = true;
        }
    }

    return out;
}

#pragma endregion
