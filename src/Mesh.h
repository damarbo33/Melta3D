#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

// GL Includes
#define GLEW_STATIC
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// GL includes
#include "Shader.h"

#include "ogldev_math_3d.h"

#include "common/structs.h"

#define NUM_BONES_PER_VERTEX 4
static const int MAX_BONES = 100;

struct Texture {
    GLuint id;
    GLuint type;
    aiString path;

    Texture(){
    };

    ~Texture(){
    };
};

struct BoneInfo{

//    glm::mat4 BoneOffset;
//    glm::mat4 FinalTransformation;

    Matrix4f BoneOffset;
    Matrix4f FinalTransformation;

    BoneInfo()
    {
//        BoneOffset = glm::mat4();
//        FinalTransformation = glm::mat4();
        BoneOffset.SetZero();
        FinalTransformation.SetZero();
    }
};

struct VertexBoneData
{
    uint32_t IDs[NUM_BONES_PER_VERTEX];
    float Weights[NUM_BONES_PER_VERTEX];

    VertexBoneData()
    {
        Reset();
    };

    ~VertexBoneData(){
        Reset();
    }

    void Reset()
    {
        ZERO_MEM(IDs);
        ZERO_MEM(Weights);
    }

    /**
    *
    */
    void AddBoneData(uint32_t BoneID, float Weight){
        for (uint32_t i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(IDs) ; i++) {
            if (Weights[i] == 0.0) {
                IDs[i]     = BoneID;
                Weights[i] = Weight;
                return;
            }
        }
        // should never get here - more bones than we have space for
        assert(0);
    }

};


    #define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry {
        MeshEntry()
        {
            NumIndices    = 0;
            BaseVertex    = 0;
            BaseIndex     = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };


#define POSITION_LOCATION    0
#define NORMAL_LOCATION      1
#define TEX_COORD_LOCATION   2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4

class Mesh {
public:
    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    vector<VertexBoneData> Bones;


    vector<VertexBoneData> * getBones(){
        return &Bones;
    }

    vector<Vertex> * getVertices(){
        return &vertices;
    }

    vector<GLuint> * getIndices(){
        return &indices;
    }


    /*  Functions  */
    // Constructor
    Mesh(){};

    /**
    *
    */
    Mesh(vector<Vertex> *vertices, vector<GLuint> *indices, vector<Texture> *textures,
         vector<VertexBoneData> *Bones, Shader *shader)
    {
        this->vertices.clear();
        this->indices.clear();
        this->textures.clear();
        this->Bones.clear();

        this->vertices.assign(vertices->begin(),vertices->end());
        this->indices.assign(indices->begin(),indices->end());
        this->textures.assign(textures->begin(),textures->end());
        this->Bones.assign(Bones->begin(),Bones->end());

        this->precomputedTexture.texLocId = NULL;
        // Now that we have all the required data, set the vertex buffers and its attribute pointers.
        this->setupMesh();
        //And assign some array to locations of shader variables. boost in fps
        this->preprocessMesh(shader);
    }

    /**
    *
    */
    ~Mesh(){
        cleanMesh();
    }

    /**
    * Render the mesh
    */
    void Draw(Shader *shader){
        GLuint opaqueNr = 0;
        // Bind appropriate textures
        for(GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
            // Now set the sampler to the correct texture unit
            glUniform1i(this->precomputedTexture.texLocId[i], i);
            // And finally bind the texture
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id);

            if (this->textures[i].type == aiTextureType_OPACITY)
                opaqueNr++; // Transfer GLuint to stream
        }

        // Also set each mesh's shininess property to a default value (if you want you could extend this to another mesh property and possibly change this value)
        glUniform1f(this->precomputedTexture.material_shininess, 16.0f);
        glUniform1f(this->precomputedTexture.isOpaque, opaqueNr > 0);

        // Draw mesh
        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Always good practice to set everything back to defaults once configured.
        for (GLuint i = 0; i < this->textures.size(); i++){
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

private:
    /*  Render data  */
    GLuint VAO, VBO, EBO;

    /* Bones data*/
    GLuint BBO;

    struct TextureShaderInfo{
        //char [25] textureName;
        //GLint  texLocId[25];
        GLint *texLocId;
        GLint isOpaque;
        GLint material_shininess;
    };

    TextureShaderInfo precomputedTexture;

    /**  Functions    */
    // Initializes all the buffer objects/arrays
    void setupMesh()
    {
        // Create buffers/arrays
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);
        glGenBuffers(1, &this->BBO);

        glBindVertexArray(this->VAO);
        // Load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Set the vertex attribute pointers
        // Vertex Positions
        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Vertex Normals
        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(TEX_COORD_LOCATION);
        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
//        cout << "Bones.size() " << Bones.size() << endl;
        if (Bones.size() > 0){
            glBindBuffer(GL_ARRAY_BUFFER, BBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(BONE_ID_LOCATION);
            glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (GLvoid*)offsetof(VertexBoneData, IDs));

            glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
            glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (GLvoid*)offsetof(VertexBoneData, Weights));

            glBindVertexArray(0);
        }
    }

    /**
    *
    */
    void preprocessMesh(Shader *shader){

        //cout << "preprocessMesh" << endl;
        // Bind appropriate textures
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        GLuint opaqueNr = 1;

        if (this->textures.size() > 0){

            if (this->precomputedTexture.texLocId != NULL){
                delete[] this->precomputedTexture.texLocId;
            }

            this->precomputedTexture.texLocId = new GLint[this->textures.size()];

            for(GLuint i = 0; i < this->textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
                // Retrieve texture number (the N in diffuse_textureN)
                stringstream ss;
                string number;
                string name;

                if(this->textures[i].type == aiTextureType_DIFFUSE){
                    name = "texture_diffuse";
                    ss << diffuseNr++; // Transfer GLuint to stream
                } else if(this->textures[i].type == aiTextureType_SPECULAR){
                    name = "texture_specular";
                    ss << specularNr++; // Transfer GLuint to stream
                } else if (this->textures[i].type == aiTextureType_OPACITY){
                    name = "texture_opaque";
                    ss << opaqueNr++; // Transfer GLuint to stream
                }

//
//                number = ss.str();
//                // Now set the sampler to the correct texture unit
//                this->precomputedTexture.texLocId[i] = glGetUniformLocation(shader->Program, (name + number).c_str());
                this->precomputedTexture.texLocId[i] = glGetUniformLocation(shader->Program, name.c_str());
                cout << name << endl;
            }



        }

        this->precomputedTexture.material_shininess = glGetUniformLocation(shader->Program, "material_shininess");
        this->precomputedTexture.isOpaque = glGetUniformLocation(shader->Program, "is_opaque");
    }

    /**
    *
    */
    cleanMesh(){
        //cout << "cleanMesh" << endl;
        // Properly de-allocate all resources once they've outlived their purpose
        glDeleteVertexArrays(1, &this->VAO);
        glDeleteBuffers(1, &this->VBO);
        glDeleteBuffers(1, &this->EBO);
        glDeleteBuffers(1, &this->BBO);

        vertices.clear();
        indices.clear();
        textures.clear();
        Bones.clear();

        if (this->precomputedTexture.texLocId != NULL){
            delete[] this->precomputedTexture.texLocId;
        }
    }
};


