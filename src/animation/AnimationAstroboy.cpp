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
#include "objectutils.h"
#include "physics/mydebug.h"



// Properties
GLuint screenWidth = 640, screenHeight = 480;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
void processLights(vector<Light *> &luces, Shader &shader);
void initLights(vector<Light *> &luces, Shader &shader);
void activateObjectOutlining();
int initGround(btVector3 initialPosition, Model *ourModel, btVector3 dimension);
GLuint loadTexture(GLchar* path);
bool getOMWorld(int i, glm::vec3 scale, glm::vec3 offset, glm::mat4 &model);

// Camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
//Camera camera(glm::vec3(0.0f, 20.0f, 25.0f));

bool keys[1024];
GLfloat lastX = 640, lastY = 480;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

SceneObjects sceneObjects;

glm::vec2 escenaAtras = glm::vec2(2.85,2.85);
glm::vec2 escenaAdelante = glm::vec2(0.0,3.0);
glm::vec2 escenaParado = glm::vec2(21.25,10.0);
glm::vec2 estadoPersonaje = escenaParado;

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
    Shader debugShader("shaders/animation/debug.vertexshader", "shaders/animation/debug.fragmentshader");

    ObjectUtils objUtil;
    GLuint VBO, lightVAO;
    objUtil.makeSquareVao(VBO, lightVAO);

    shader.Use();   // <-- Don't forget this one!
    // Load models
    Model *ourModel2 = new Model("models/Bikini_Girl/Bikini_Girl.dae", &shader, 1, true);
    Model *ourWorld = new Model("models/cs_assault/cs_assault.obj", &shader);
    Model *ourModel = new Model("models/ArmyPilot/ArmyPilot.ms3d", &shader, 1, true);

    // Draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    //Por temas de rendimiento. Solo pinta las caras visibles
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);


    //To generate a plane ground
//    Model *ourWorld = new Model();
//    object3D *obj = new object3D();
//    obj->dimension = btVector3(10.0f, 0.0f, 0.0f);
//    objUtil.generatePlaneGround(ourWorld, obj);
//    sceneObjects.initShape(obj);

    object3D *obj = new object3D();
    obj->friction = 10.0f;
    obj->convex = false;
    obj->rollingFriction = 10.0f;
    obj->spinningFriction = 1.0f;
    obj->restitution = 0.0f;
    obj->mass = 0.0f;
    obj->dimension = btVector3(0.0f, 70.0f, 0.0f);
    obj->position = btVector3(30,-50,0);
    obj->rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f)); //Rotacion de 90 grados en el eje y
    obj->meshModel = ourWorld;
    obj->tag = "ground";
    sceneObjects.initShape(obj);


    object3D *objM = new object3D();
    objM->aproxHullShape = APROXCYCLINDER;
    objM->spinningFriction = 10.0f;
    objM->dimension = btVector3(0.0, 1.8, 0.0);
    //obj->position = btVector3(i*2 % 20+1, 0, i*2 / 20+3);
    objM->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.f, 1.f, 0.f)); //Rotacion de 90 grados en el eje y
    objM->meshModel = ourModel;
    objM->tag = "model_pilot";

    //Creating the object pilot
    for (int i=0; i < 100; i++){
        objM->position = btVector3(2, 1.8 * i, 2);
        sceneObjects.initShape(objM);
    }

    //Another model
    object3D *obj2 = new object3D();
//    obj2->aproxHullShape = APROXCYCLINDER;
    obj2->spinningFriction = 10.0f;
    obj2->dimension = btVector3(0.0, 0.0, 1.8); // El modelo viene tumbado
    obj2->position = btVector3(5,0,5);
    obj2->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)); //Rotacion 90 grados en eje x
    obj2->rotation = obj2->rotation * glm::angleAxis(glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f)); //Rotacion 90 grados en eje z
    obj2->meshModel = ourModel2;
    obj2->tag = "model_bikini";
    sceneObjects.initShape(obj2);

    double lastTime = 0;
    int nbFrames = 0;

    vector<Light *> luces;
    initLights(luces, shader);

    GLint personLoc = glGetUniformLocation(shader.Program, "model");
    GLfloat initTime = glfwGetTime();

    glm::vec3 cielo = glm::vec3(119.0f, 181.0f, 254.0f)/255.0f;

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
        glClearColor(cielo.x, cielo.y, cielo.z, 1.0f);
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
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // Draw the loaded model
        glm::mat4 model;
        glm::mat4 transInversMatrix;

        /** para la escena del modelo*/
        //Calculate the physics
        sceneObjects.getPhysics()->getDynamicsWorld()->stepSimulation(deltaTime); //suppose you have 60 frames per second
        for (int i = 0; i< sceneObjects.getPhysics()->getCollisionObjectCount(); i++) {
            object3D *userPointer = sceneObjects.getObjPointer(i);
            if (userPointer != NULL) {
                model = glm::mat4();
                if (sceneObjects.getObjectModel(i,
                               glm::vec3(userPointer->scaling.x(), userPointer->scaling.y(), userPointer->scaling.z()),
                               glm::vec3(0.0f, 0.0f, 0.0f), model))
                {
                    //Informing data of model
                    glUniformMatrix4fv(personLoc, 1, GL_FALSE, glm::value_ptr(model));
                    //Calculamos la inversa de la matriz por temas de iluminacion y rendimiento
                    transInversMatrix = transpose(inverse(model));
                    glUniformMatrix4fv(transInversLoc, 1, GL_FALSE, glm::value_ptr(transInversMatrix));
                    //Drawing the model
                    userPointer->meshModel->Draw(&shader, estadoPersonaje.x + fmod(currentFrame * 2.0f, estadoPersonaje.y));
                }
            }
		}
        /**Fin modelo*/

//        objUtil.drawPlane(sceneObjects, projection, view);

        /**SALIDA DE DEBUG*/
        glFrontFace(GL_CW);
        model = glm::mat4();
        //Mostramos el resto de elementos segun la escala definida por nuestro mundo
        object3D *userPointer2 = sceneObjects.getObjPointer(0);

        if (sceneObjects.getPhysics()->getDebug() > 0){
             if (sceneObjects.getObjectModel(0, glm::vec3(1,1,1)
                                           , -glm::vec3(userPointer2->position.x(), userPointer2->position.y(),
                                                        userPointer2->position.z()),
                                           model)){

                debugShader.Use();
                glUniformMatrix4fv(glGetUniformLocation(debugShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(glGetUniformLocation(debugShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(glGetUniformLocation(debugShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                sceneObjects.getPhysics()->getDynamicsWorld()->debugDrawWorld();
            }
        }
        /**SALIDA DE DEBUG*/

//        glFrontFace(GL_CCW);        // Also draw the lamp object, again binding the appropriate shader
//        lampShader.Use();
////         Set matrices
//        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
//        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
////         We now draw as many light bulbs as we have point lights.
//        glBindVertexArray(lightVAO);
//        for (GLuint i = 0; i < 1; i++){
//            model = glm::mat4();
//            model = glm::translate(model, luces.at(i+1)->vPosition);
//            model = glm::scale(model, glm::vec3(1.0f)); // Make it a smaller cube
//            glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
//            glm::mat4 transInversMatrix = transpose(inverse(model));
//            glUniformMatrix4fv(glGetUniformLocation(lampShader.Program,  "transInversMatrix"), 1, GL_FALSE, glm::value_ptr(transInversMatrix));
//            glDrawArrays(GL_TRIANGLES, 0, 36);
//        }

//        for (int i = 1; i< physicsEngine->getCollisionObjectCount(); i++) {
//            model = glm::mat4();
//            if (getOMWorld(i, glm::vec3(0.3,1,0.3), glm::vec3((i-1)*2 % 20, 0, (i-1)*2 / 20), model)){
//                glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
//                transInversMatrix = transpose(inverse(model));
//                glUniformMatrix4fv(glGetUniformLocation(lampShader.Program,  "transInversMatrix"), 1, GL_FALSE, glm::value_ptr(transInversMatrix));
//                glDrawArrays(GL_TRIANGLES, 0, 36);
//                glBindVertexArray(0);
//            }
//        }

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    delete ourWorld;
    delete ourModel;
    delete ourModel2;
    glfwTerminate();
    return 0;
}

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

    btCollisionObject* world = sceneObjects.getPhysics()->getDynamicsWorld()->getCollisionObjectArray()[0];
    btCollisionObject* obj = sceneObjects.getPhysics()->getDynamicsWorld()->getCollisionObjectArray()[1];

    btRigidBody* capsuleBody = btRigidBody::upcast(obj);
    bool out = false;
    if (capsuleBody && capsuleBody->getMotionState()){
        btTransform trans;
        capsuleBody->getMotionState()->getWorldTransform(trans);
        object3D *userPointer = (object3D *)capsuleBody->getUserPointer();

        if (userPointer->tag.find("model") != string::npos) {
            btVector3 impulseAxis = capsuleBody->getWorldTransform().getBasis().getColumn(userPointer->impulseAxis);

            float direction = userPointer->impulseSense;
            float torqueDir = userPointer->rotationSense;

            //We query if we have make contact with the ground and store if we want to reset forces
            //when there is no key pressed
            if (userPointer->instantStop){
                Physics::MyContactResultCallback callback;
                sceneObjects.getPhysics()->getDynamicsWorld()->contactPairTest(world, obj, callback);
                userPointer->groundContact = callback.isContact();
            }

            if (keys[GLFW_KEY_UP]){
                estadoPersonaje = escenaAdelante;
            } else if (keys[GLFW_KEY_DOWN]){
                estadoPersonaje = escenaAtras;
                direction = direction * -1.0;
            } else {
                estadoPersonaje = escenaParado;
                //We set to 0 the forces if we touched floor and instantStop is true
                if (userPointer->instantStop && userPointer->groundContact){
                    capsuleBody->clearForces();
                    capsuleBody->setLinearVelocity(btVector3(0,0,0));
                    capsuleBody->setAngularVelocity(btVector3(0,0,0));
                }
            }

            if (keys[GLFW_KEY_RIGHT]){
                torqueDir = torqueDir * -1.0;
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
//                btVector3 impulseAxis = ((object3D *)userPointer)->impulseAxis;
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
                Physics::MyContactResultCallback callback;
                sceneObjects.getPhysics()->getDynamicsWorld()->contactPairTest(world, obj, callback);
                if (callback.isContact()){
                    btVector3 impulseJump = capsuleBody->getLinearVelocity();
                    impulseJump.setY(3.0f);
                    capsuleBody->setLinearVelocity(impulseJump);
                    userPointer->groundContact = false;
                }
            }
        }
    }
}


/**
* Inicia los valores de las luces
*/
void initLights(vector<Light *> &luces, Shader &shader){

    // Positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.0f, 0.5f, 1.0f),
        glm::vec3( 0.0f, 0.0f, 0.0f)
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
    luz4->vAmbient = glm::vec3(0,0,0);
    luz4->vDiffuse = glm::vec3(0,0,0);
    luz4->vSpecular = glm::vec3(0,0,0);
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




