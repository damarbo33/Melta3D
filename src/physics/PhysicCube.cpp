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

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Other Libs
#include <SOIL.h>
#include "Physics.h"

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
int initShape(btVector3 initialPosition);
int initGround();
// Camera
Camera camera(glm::vec3(0.0f, 20.0f, 25.0f));
bool keys[1024];
GLfloat lastX = screenWidth / 2, lastY = screenHeight / 2;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Physics *physicsEngine = new Physics();

// The MAIN function, from here we start our application and run our Game loop
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup and compile our shaders
    Shader ourShader("shaders/coordinates.4.2.vertexshader", "shaders/coordinates.4.2.fragmentshader");

    // Set up our vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
        -1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,

        -1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 0.0f,

         1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,

        -1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f, 1.0f
    };


    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind our Vertex Array Object first, then bind and set our buffers and pointers.
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind VAO

    // Load and create a texture
    GLuint texture1;
    GLuint texture2;
    // --== TEXTURE 1 == --
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image("res/container.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    // --== TEXTURE 2 == --
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    image = SOIL_load_image("res/awesomeface.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);


    physicsEngine->getDynamicsWorld()->setGravity(btVector3(0, -9.8f, 0));
    initGround();
    int inicio = 0;

    for (int i=0; i < 500; i++){
        initShape(btVector3(0,inicio + i*5,0));
    }

    camera.lookTo(glm::vec3(0,0,0));

    // Game loop
    while(!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        Do_Movement();

        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw our first triangle
        ourShader.Use();

        // Bind Textures using texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

        // Create camera transformation
        glm::mat4 view;
        view = camera.GetViewMatrix();
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth/(float)screenHeight, 0.1f, 1000.0f);
        // Get the uniform locations
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        //Calculate the physics
        physicsEngine->getDynamicsWorld()->stepSimulation(deltaTime); //suppose you have 60 frames per second

        for (int i = 0; i< physicsEngine->getCollisionObjectCount(); i++) {
			btCollisionObject* obj = physicsEngine->getDynamicsWorld()->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);

			if (body && body->getMotionState()){
				btTransform trans;
				body->getMotionState()->getWorldTransform(trans);

				void *userPointer = body->getUserPointer();
				if (userPointer) {
					btQuaternion orientation = trans.getRotation();

                    glm::quat MyQuaternion   = glm::quat(orientation.getW(),orientation.getX(), orientation.getY(), orientation.getZ());
                    glm::mat4 RotationMatrix = glm::toMat4(MyQuaternion);

                    glm::mat4 model = glm::mat4();
                    model = glm::scale(model, glm::vec3(1.0f)); // Make it a smaller cube
                    model = glm::translate(model, glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
                    model =  model * RotationMatrix;
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    glDrawArrays(GL_TRIANGLES, 0, 36);

//					Ogre::SceneNode *sceneNode = static_cast<Ogre::SceneNode *>(userPointer);
//					sceneNode->setPosition(Ogre::Vector3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
//					sceneNode->setOrientation(Ogre::Quaternion(orientation.getW(), orientation.getX(), orientation.getY(), orientation.getZ()));
				}
			}
		}


        glBindVertexArray(0);
        // Swap the buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

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
    //cout << key << endl;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if(action == GLFW_PRESS)
            keys[key] = true;
        else if(action == GLFW_RELEASE)
            keys[key] = false;
    }
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
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

int initGround(){

    float w =200.0f, h = 200.0f;

    //create the plane entity to the physics engine, and attach it to the node
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(-w/2.0f, -5.0/2.0f, -h/2.0f));
    float RotationAngle = glm::radians(0.0f);
    groundTransform.setRotation(btQuaternion(0,0,0,cos(RotationAngle / 2)));

    btScalar groundMass(0.); //the mass is 0, because the ground is immovable (static)
    btVector3 localGroundInertia(0, 0, 0);

    btCollisionShape *groundShape = new btBoxShape(btVector3(btScalar(w), 5.0, btScalar(h)));
    btDefaultMotionState *groundMotionState = new btDefaultMotionState(groundTransform);

    groundShape->calculateLocalInertia(groundMass, localGroundInertia);

    btRigidBody::btRigidBodyConstructionInfo groundRBInfo(groundMass, groundMotionState, groundShape, localGroundInertia);
    btRigidBody *groundBody = new btRigidBody(groundRBInfo);
    groundBody->setFriction(btScalar(10));

    //add the body to the dynamics world
    physicsEngine->getDynamicsWorld()->addRigidBody(groundBody);

    return 0;
}

int initShape(btVector3 initialPosition){
    //create the new shape, and tell the physics that is a Box
    btCollisionShape *newRigidShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
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

    //actually contruvc the body and add it to the dynamics world
    btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, newRigidShape, localInertia);
    btRigidBody *body = new btRigidBody(rbInfo);
    body->setRestitution(1);
    //body->setUserPointer(newNode);
    body->setUserPointer(new char());

    physicsEngine->getDynamicsWorld()->addRigidBody(body);
    //physicsEngine->trackRigidBodyWithName(body, physicsCubeName);
}



