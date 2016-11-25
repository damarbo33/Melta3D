#include "objectutils.h"

ObjectUtils::ObjectUtils(){
    textWrapFactor = 1.0f;
}

ObjectUtils::~ObjectUtils(){

}

void ObjectUtils::makeGroundVAO(GLuint &planeVAO, GLuint &planeVBO){
    // Setup plane VAO
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

    int nelems = sizeof(planeVertices) / sizeof(planeVertices[0]);

    GLfloat planeVerticesDim[nelems];
    memcpy(planeVerticesDim, planeVertices, sizeof(planeVerticesDim));

    for (int i=0; i < nelems / 5; i++){
        planeVerticesDim[i*5 + 3] *= textWrapFactor;
        planeVerticesDim[i*5 + 4] *= textWrapFactor;
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerticesDim), &planeVerticesDim, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);
}

void ObjectUtils::makeSquareVao(GLuint &VBO, GLuint &lightVAO){
    // First, set the VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // Set the vertex attributes (only position data for the lamp))
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// This function loads a texture from file. Note: texture loading functions like these are usually
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio).
// For learning purposes we'll just define it as a utility function.
GLuint ObjectUtils::loadTexture(GLchar* path)
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
//     Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;

}

void ObjectUtils::init(){
    makeGroundVAO(planeVAO, planeVBO);
    floorShader = new Shader("shaders/animation/floor.vertexshader", "shaders/animation/floor.fragmentshader");
    //floorTexture = loadTexture("res/wall.jpg");
    floorTexture = loadTexture("res/sueloMadera.jpg");
}

/**
*
*/
void ObjectUtils::generatePlaneGround(Model *ourWorld2, object3D *obj){

    textWrapFactor = max(max(obj->dimension.x(), obj->dimension.y()), obj->dimension.z());
    if (textWrapFactor < 1.0f) textWrapFactor = 1.0f;

    init();

    int tam = (sizeof(planeVertices) / sizeof(planeVertices[0])) / 5;
    int i=0;
    Mesh *mesh = new Mesh();
    while (i < tam){
        Vertex *vert= new Vertex();
        vert->Position = glm::vec3(planeVertices[i*5], planeVertices[i*5 + 1], planeVertices[i*5 + 2]);
        mesh->vertices.push_back(*vert);
        i++;
    }
    ourWorld2->meshes.push_back(mesh);

    obj->friction = 10.0f;
    obj->convex = false;
    obj->rollingFriction = 10.0f;
    obj->spinningFriction = 1.0f;
    obj->restitution = 0.0f;
    obj->mass = 0.0f;
//    obj->dimension = btVector3(100.0f, 0.0f, 0.0f);
    obj->position = btVector3(0,0,0);
    obj->rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f));
    obj->meshModel = ourWorld2;
    obj->tag = "ground";
}

void ObjectUtils::drawPlane(SceneObjects &sceneObjects, glm::mat4 &projection, glm::mat4 &view ){
    /**Un piso de ejemplo*/
    glm::mat4 model = glm::mat4();
    object3D *userPointer = sceneObjects.getObjPointer(0);
    if (sceneObjects.getObjectModel(0,
                                   glm::vec3(userPointer->scaling.x(), userPointer->scaling.y(), userPointer->scaling.z()),
                                   glm::vec3(0.0f, 0.0f, 0.0f), model)){
        glFrontFace(GL_CCW);
        floorShader->Use();
        glUniformMatrix4fv(glGetUniformLocation(floorShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(floorShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(planeVAO);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glUniformMatrix4fv(glGetUniformLocation(floorShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    /**Fin piso de ejemplo*/
}
