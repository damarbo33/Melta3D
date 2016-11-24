#ifndef OBJECTUTILS_H_INCLUDED
#define OBJECTUTILS_H_INCLUDED

GLfloat planeVertices[] = {
    // Positions      // Texture Coords (note we set these higher than 1 that together with GL_REPEAT as texture wrapping mode will cause the floor texture to repeat)
    //Para el suelo
    0.5f,  0,  0.5f,  3.0f, 0.0f,
    -0.5f, 0,  0.5f,  0.0f, 0.0f,
    -0.5f, 0, -0.5f,  0.0f, 3.0f,

    0.5f,  0,  0.5f,  3.0f, 0.0f,
    -0.5f, 0, -0.5f,  0.0f, 3.0f,
    0.5f,  0, -0.5f,  3.0f, 3.0f,
    //Para la pared izquierda
    -0.5f, 0.5f,  0.5f,  3.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f,  0.0f, 3.0f,

    -0.5f, -0.5f, -0.5f,  3.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,  0.0f, 3.0f,
    -0.5f, -0.5f, 0.5f,  3.0f, 3.0f,

};

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


void makeGroundVAO(GLuint &planeVAO, GLuint &planeVBO){
    // Setup plane VAO
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
}

void makeSquareVao(GLuint &VBO, GLuint &lightVAO){
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

/**
*
*/
void generatePlaneGround(Model *ourWorld2, object3D *obj){
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
    obj->dimension = btVector3(5.0f, 0.0f, 0.0f);
    obj->position = btVector3(0,-5,0);
    obj->rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f));
    obj->meshModel = ourWorld2;
    obj->tag = "ground";
}


#endif // OBJECTUTILS_H_INCLUDED
