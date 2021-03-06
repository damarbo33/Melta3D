#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED



#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <stdio.h>
#include <sys/stat.h>

#ifdef WIN
    #include <windows.h>
#endif // WIN

using namespace std;
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SOIL.h>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>        // Plain-C interface

#include "Mesh.h"
#include "Animation.h"
#include <common/texture.hpp>

#include "ogldev_math_3d.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "../examples/CommonInterfaces/CommonRigidBodyBase.h"

#include <iostream>
using namespace std;

GLint TextureFromFile(const char* path, string directory, GLboolean alpha);

class Model
{
public:

    Model(){
        this->importer = NULL;
        this->mp_scene = NULL;
        this->bonesTransform = NULL;
        this->triMeshPhis = NULL;
        this->collisionShape = NULL;
        this->physMesh = new btTriangleMesh();
        this->mNumPhysFaces = 0;
    }
    /**
    *Constructor, expects a filepath to a 3D model.
    */
    Model(GLchar* path, Shader *shader, float fpsModelFactor = 1, bool precalculateBonesTransform = false){
        this->importer = NULL;
        this->mp_scene = NULL;
        this->bonesTransform = NULL;
        this->triMeshPhis = NULL;
        this->collisionShape = NULL;
        this->physMesh = new btTriangleMesh();
        this->mNumPhysFaces = 0;
        this->importer = new Assimp::Importer();
        this->fpsModelFactor = fpsModelFactor;
        this->precalculateBonesTransform = precalculateBonesTransform;
        this->loadModel(path, shader);
        this->preprocessBones(shader);
    }

    /**
    * Destructor
    */
    ~Model(){
        cleanBones();
        cleanMeshes();
        cleanTextures();
        cleanScene();
        cleanPhysics();
    }

    /**
    *
    */
    float getAnimationTime(GLfloat currentFrame, int nAnim){
        if (getFpsModelFactor() > 0.0){
            float TicksPerSecond = mp_scene->mAnimations[nAnim]->mTicksPerSecond != 0.0f ?
            mp_scene->mAnimations[nAnim]->mTicksPerSecond : 25.0f;
            float TimeInTicks = currentFrame * TicksPerSecond;
            return fmod(TimeInTicks, mp_scene->mAnimations[nAnim]->mDuration);
        } else {
            return 0;
        }
    }

    /**
    *Draws the model, and thus all its meshes
    */
    void Draw(Shader *shader, GLfloat currentFrame, int nAnim = 0){
        glUniform1i(m_animLoc, this->getNumAnimations());

        if (this->hasAnimations()){
            if (this->precalculateBonesTransform){
                int posAnimation = getAnimationTime(currentFrame, nAnim) * getFpsModelFactor();
                if (bonesTransform != NULL)
                    for (int BoneIndex=0; BoneIndex < m_NumBones; BoneIndex++){
                        SetBoneTransform(BoneIndex, bonesTransform[nAnim][posAnimation][BoneIndex]);
                    }
            } else {
                this->BoneTransform(currentFrame, nAnim);
                for (int i = 0 ; i < m_BoneInfo.size() ; i++) {
                    SetBoneTransform(i, m_BoneInfo[i].FinalTransformation);
                }
            }
        }

        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i]->Draw(shader);
    }

    /**
    *
    */
    bool hasAnimations(){
        return (mp_scene != NULL && mp_scene->mNumAnimations > 0);
    }

    /**
    *
    */
    int getNumAnimations(){
        if (mp_scene != NULL){
            return mp_scene->mNumAnimations;
        } else {
            return 0;
        }
    }

    /**
    *
    */
    int getTotalFramesModel(){
        return totalFramesModel;
    }

    /**
    *
    */
    void setTotalFramesModel(int var){
        totalFramesModel = var;
    }

    /**
    *
    */
    float getFpsModelFactor(){
        return fpsModelFactor;
    }

    /**
    *
    */
    void setFpsModelFactor(float var){
        fpsModelFactor = var;
    }

    /**
    *
    */
    void BoneTransform(float TimeInSeconds, int nAnimation){
        //glm::mat4 Identity = glm::mat4();
        Matrix4f Identity;
        Identity.InitIdentity();
        if (mp_scene != NULL && mp_scene->mNumAnimations > nAnimation){
            if (!precalculateBonesTransform){
                ReadNodeHeirarchy(getAnimationTime(TimeInSeconds, nAnimation), mp_scene->mRootNode, Identity);
            }
        }
    }

    /**
    *
    */
    vector<Mesh *>* getMeshes(){
        return &meshes;
    }

    /*  Model Data  */
    vector<Mesh *> meshes;

    btVector3** getTriMeshPhis(){return triMeshPhis;}

    btVector3** triMeshPhis;
    btTriangleMesh* physMesh;

    /**
    *
    */
    void initTriMeshPhis(int len){
        this->triMeshPhis = new btVector3* [len];
        mNumPhysFaces = len;
    }

    btCollisionShape* getCollisionShape(){return collisionShape;}

private:

    string directory;
    vector<Texture> textures_loaded;
    map <string, uint32_t>m_BoneMapping;
    vector<BoneInfo> m_BoneInfo;
    GLuint m_boneLocation[MAX_BONES];
    GLuint m_animLoc;
    uint32_t m_NumBones;
    int totalFramesModel;
    //This is a factor to multiply the number of frames for each model. There
    //are cases wich assimp doesn't load well the model with the correct fps. So
    //we multiply by this factor
    float fpsModelFactor;

    //glm::mat4 m_GlobalInverseTransform;
    Matrix4f m_GlobalInverseTransform;
    const aiScene* mp_scene;
    Assimp::Importer* importer;
    bool precalculateBonesTransform;

    int mNumPhysFaces;

    btCollisionShape* collisionShape;



    //We define this matrix to minimize the memory used, instead of a Matrix4f
    struct matrix4{
        float aa,ab,ac,ad,
              ba,bb,bc,bd,
              ca,cb,cc,cd;
              //da,db,dc,dd; //0,0,0,1

        matrix4(Matrix4f &mat){
            aa = mat.m[0][0]; ab = mat.m[0][1]; ac = mat.m[0][2]; ad = mat.m[0][3];
            ba = mat.m[1][0]; bb = mat.m[1][1]; bc = mat.m[1][2]; bd = mat.m[1][3];
            ca = mat.m[2][0]; cb = mat.m[2][1]; cc = mat.m[2][2]; cd = mat.m[2][3];
//            da = mat.m[3][0]; db = mat.m[3][1]; dc = mat.m[3][2]; dd = mat.m[3][3];
//            cout << mat.m[0][0] << "," << mat.m[0][1] << "," << mat.m[0][2] << "," << mat.m[0][3] << endl;
//            cout << mat.m[1][0] << "," << mat.m[1][1] << "," << mat.m[1][2] << "," << mat.m[1][3] << endl;
//            cout << mat.m[2][0] << "," << mat.m[2][1] << "," << mat.m[2][2] << "," << mat.m[2][3] << endl;
//            cout << mat.m[3][0] << "," << mat.m[3][1] << "," << mat.m[3][2] << "," << mat.m[3][3] << endl;
//            cout << endl;
        }
    };
    //vector with bones matrix transform, per time and per num of animation
    //mf_transformBonesFinal[nAnim][posAnimation][BoneIndex]
//    vector<vector<vector<matrix4 *> > > mf_transformBonesFinal;

    //Creamos un array dinámico de tres dimensiones [nAnim][posAnimation][BoneIndex]
    matrix4 ****bonesTransform;

    /**
    *
    */
    void cleanBones(){
        if (mp_scene != NULL){
            const int nAnimations = mp_scene->mNumAnimations;
            if (bonesTransform != NULL){
                for (int nAnim = 0; nAnim < nAnimations; nAnim++){
                    const int nFrames = ceil(mp_scene->mAnimations[nAnim]->mDuration * getFpsModelFactor());
                    for (int nFrame = 0; nFrame < nFrames; nFrame++){
                        for (int BoneIndex=0; BoneIndex < m_NumBones; BoneIndex++){
                            delete bonesTransform[nAnim][nFrame][BoneIndex];
                        }
                        delete [] bonesTransform[nAnim][nFrame];
                    }
                    delete [] bonesTransform[nAnim];
                }
                delete [] bonesTransform;
                bonesTransform = NULL;
            }
        }

        m_BoneMapping.clear();
        m_BoneInfo.clear();
    }

    /**
    *
    */
    void cleanMeshes(){
        for (int i = 0; i < meshes.size(); i++){
            delete meshes[i];
        }
        meshes.clear();
    }

    /**
    *
    */
    void cleanTextures(){
        for(GLuint j = 0; j < textures_loaded.size(); j++){
            glDeleteTextures(1, &this->textures_loaded[j].id);
        }
        this->textures_loaded.clear();
    }

    /**
    *
    */
    void cleanScene(){
        // We're done. Release all resources associated with this import
        //aiReleaseImport( mp_scene);
        if (importer != NULL)
            delete importer;
    }

    /**
    *
    */
    void SetBoneTransform(int Index, const Matrix4f& Transform){
        //cout << "Index: " << Index << endl;
        assert(Index < MAX_BONES);
        //Transform.Print();
        //glUniformMatrix4fv(m_boneLocation[Index], 1, GL_TRUE, glm::value_ptr(Transform));
        glUniformMatrix4fv(m_boneLocation[Index], 1, GL_TRUE, (const GLfloat*)Transform);
    }

    void cleanPhysics(){
//        for (int i=0; i < mNumPhysFaces; i++){
//            delete triMeshPhis[i];
//        }
        delete triMeshPhis;
        triMeshPhis = NULL;

        delete physMesh;
    }

    /**
    *
    */
    void SetBoneTransform(int Index, const matrix4 *Transform){
        Matrix4f tmpTransform;
        tmpTransform.m[0][0] =  Transform->aa;
        tmpTransform.m[0][1] =  Transform->ab;
        tmpTransform.m[0][2] =  Transform->ac;
        tmpTransform.m[0][3] =  Transform->ad;
        tmpTransform.m[1][0] =  Transform->ba;
        tmpTransform.m[1][1] =  Transform->bb;
        tmpTransform.m[1][2] =  Transform->bc;
        tmpTransform.m[1][3] =  Transform->bd;
        tmpTransform.m[2][0] =  Transform->ca;
        tmpTransform.m[2][1] =  Transform->cb;
        tmpTransform.m[2][2] =  Transform->cc;
        tmpTransform.m[2][3] =  Transform->cd;
        tmpTransform.m[3][0] =  0;
        tmpTransform.m[3][1] =  0;
        tmpTransform.m[3][2] =  0;
        tmpTransform.m[3][3] =  1;
        SetBoneTransform(Index, tmpTransform);
    }

    /**
    * Process to calculate all the transformation matrices for the object
    */
    void calcTransformationMatrices(){
        float TimeInSeconds = 0.0f;
        Matrix4f Identity;
        Identity.InitIdentity();
        const int nAnimations = mp_scene->mNumAnimations;

        if (mp_scene != NULL && nAnimations > 0){
            //Reserving space for all animations
            bonesTransform = new matrix4*** [nAnimations];
            cout << "NumAnimations: " << nAnimations << endl;

            for (int nAnim = 0; nAnim < nAnimations; nAnim++){

                const float TicksPerSecond = mp_scene->mAnimations[nAnim]->mTicksPerSecond > 0.0f ?
                        mp_scene->mAnimations[nAnim]->mTicksPerSecond : 25.0f;

                cout << "TicksPerSecond: " << TicksPerSecond << endl;
                cout << "Duration of the animation in ticks: " << mp_scene->mAnimations[nAnim]->mDuration << endl;
                cout << "duration in s: " << mp_scene->mAnimations[nAnim]->mDuration / TicksPerSecond << " s" << endl;
                cout << "Model FPS: " << getFpsModelFactor() * TicksPerSecond << endl;

                const float endFrameTime = mp_scene->mAnimations[nAnim]->mDuration;
                int nFrame = 0;
                const int totalFrames = ceil(endFrameTime * getFpsModelFactor());
                //Reserving space for all frames of the scene
                bonesTransform[nAnim] = new matrix4** [totalFrames];

                for (nFrame=0; nFrame < totalFrames; nFrame++){
                    //Reading all the nodes
                    ReadNodeHeirarchy(nFrame / getFpsModelFactor(), mp_scene->mRootNode, Identity);
                    //Reserving space for all transformation matrices for each bone
                    bonesTransform[nAnim][nFrame] = new matrix4* [m_NumBones];
                    for (int BoneIndex=0; BoneIndex < m_NumBones; BoneIndex++){
                        bonesTransform[nAnim][nFrame][BoneIndex] = new matrix4(m_BoneInfo[BoneIndex].FinalTransformation);
                    }
                }

                cout << "Added: " << nFrame << " frames for " << mp_scene->mAnimations[nAnim]->mDuration / TicksPerSecond
                << " seconds for animation " << nAnim << endl;
            }
        } else {
            cout << "NumAnimations: 0" << endl;
        }
    }

    /**
    *
    */
    void preprocessBones(Shader *shader){
        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boneLocation) ; i++) {
            char Name[128];
            memset(Name, 0, sizeof(Name));
            sprintf(Name, "gBones[%d]", i);
            m_boneLocation[i] = glGetUniformLocation(shader->Program,Name);
        }
        m_animLoc = glGetUniformLocation(shader->Program, "nAnim");

        if (precalculateBonesTransform){
            calcTransformationMatrices();
        }
    }

    /**
    * Functions
    */
    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string path, Shader *shader)
    {
        m_NumBones = 0;
        m_BoneMapping.clear();

        cout << "Loading model: " << path << endl;
        // Read file via ASSIMP

        // default pp steps
        unsigned int ppsteps =
        aiProcess_JoinIdenticalVertices    | // join identical vertices/ optimize indexing

        aiProcess_ImproveCacheLocality     | // improve the cache locality of the output vertices
        aiProcess_RemoveRedundantMaterials | // remove redundant materials
        aiProcess_CalcTangentSpace         | // calculate tangents and bitangents if possible
//        aiProcess_ValidateDataStructure    | // perform a full validation of the loader's output
//        aiProcess_FindDegenerates          | // remove degenerated polygons from the import
//        aiProcess_FindInvalidData          | // detect invalid model data, such as invalid normal vectors
//        aiProcess_GenUVCoords              | // convert spherical, cylindrical, box and planar mapping to proper UVs
//        aiProcess_TransformUVCoords        | // preprocess UV transformations (scaling, translation ...)
//        aiProcess_FindInstances            | // search for instanced meshes and remove them by references to one master
//        aiProcess_LimitBoneWeights         | // limit bone weights to 4 per vertex
//        aiProcess_SplitByBoneCount         | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
        aiProcess_FlipUVs |
        aiProcess_Triangulate |             // triangulate polygons with more than 3 edges
        aiProcess_GenNormals |
        aiProcess_OptimizeMeshes           | // join small meshes, if possible;
        aiProcess_SplitLargeMeshes         | // split large, unrenderable meshes into submeshes
        aiProcess_SortByPType              | // make 'clean' meshes which consist of a single typ of primitives
        //aiProcess_OptimizeGraph            | // CAUTION!!!: A postprocessing step to optimize the scene hierarchy.
                                             // Nodes without animations, bones, lights or cameras assigned are
                                             // collapsed and joined.
        0;

        mp_scene = importer->ReadFile(path, ppsteps);
        // Check for errors
        if(!mp_scene || mp_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !mp_scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer->GetErrorString() << endl;
            return;
        }
//        Assimp::Exporter *exporter = new Assimp::Exporter();
//        const aiExportFormatDesc* exportFormatDesc = exporter->GetExportFormatDescription(0);
//        exporter->Export(mp_scene, exportFormatDesc->id, "C:/asd/exportado.obj");
//        delete exporter;
        InitFromScene(mp_scene, path);
        // Retrieve the directory path of the filepath
        this->directory = path.substr(0, path.find_last_of('/'));
        cout << "There are " << mp_scene->mNumMeshes << " meshes" << endl;
        // Process ASSIMP's root node recursively
        this->processNode(mp_scene->mRootNode, mp_scene, shader);
        cout << "Meshes creados " << this->meshes.size() << endl;
    }

    /**
    * Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    */
    void processNode(aiNode* node, const aiScene* scene, Shader *shader){
        // Process each mesh located at the current node
        for(GLuint i = 0; i < node->mNumMeshes; i++)
        {
            // The node object only contains indices to index the actual objects in the scene.
            // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes.push_back(this->processMesh(i, mesh, scene, shader));
        }
        // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(GLuint i = 0; i < node->mNumChildren; i++){
            this->processNode(node->mChildren[i], scene, shader);
        }
    }

    /**
    *
    */
    Mesh * processMesh(GLuint idMesh, aiMesh* mesh, const aiScene* scene, Shader *shader){
        // Data to fill
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Texture> textures;

        // Walk through each of the mesh's vertices
        for(GLuint i = 0; i < mesh->mNumVertices; i++){
            Vertex vertex;
            glm::vec3 vectorV; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // Positions
            vectorV.x = mesh->mVertices[i].x;
            vectorV.y = mesh->mVertices[i].y;
            vectorV.z = mesh->mVertices[i].z;
            vertex.Position = vectorV;
            // Normals
            if (mesh->HasNormals()){
                glm::vec3 vectorN;
                vectorN.x = mesh->mNormals[i].x;
                vectorN.y = mesh->mNormals[i].y;
                vectorN.z = mesh->mNormals[i].z;
                vertex.Normal = vectorN;
            }
            //else {
                //cout << "WARNING::ASSIMP:: " << "There are no Normals in model" << endl;
                //return nullptr;
            //}

            if (mesh->HasTangentsAndBitangents()){
                glm::vec3 vectorT, vectorBT;
                vectorT.x = mesh->mTangents[i].x;
                vectorT.y = mesh->mTangents[i].y;
                vectorT.z = mesh->mTangents[i].z;
                vertex.Tangent = vectorT;

                vectorBT.x = mesh->mTangents[i].x;
                vectorBT.y = mesh->mTangents[i].y;
                vectorBT.z = mesh->mTangents[i].z;
                vertex.Bittangent = vectorBT;
            }

            // Texture Coordinates
            if(mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // Retrieve all indices of the face and store them in the indices vector
            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // Process materials

        if(mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
            // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
            // Same applies to other texture as the following list summarizes:
            // Diffuse: texture_diffuseN
            // Specular: texture_specularN
            // Normal: texture_normalN

            testTextures(material);
            // 1. Diffuse maps
            vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            // 2. Specular maps
            vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            // 3. Opacity maps
            vector<Texture> opacityMaps = this->loadMaterialTextures(material, aiTextureType_OPACITY);
            textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());
            // 4. normalMaps
            vector<Texture> normalMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

            //cout << "opacityMaps size: " << opacityMaps.size() << endl;
        }

        vector<VertexBoneData> Bones;
        processBones(idMesh, scene, Bones);
        // Return a mesh object created from the extracted mesh data
        return new Mesh(&vertices, &indices, &textures, &Bones, shader);;
    }

    /**
    *
    */
    void processBones(int idMesh, const aiScene* scene, vector<VertexBoneData> &Bones){
        // Count the number of vertices and indices
        GLuint NumVertices = 0;
        NumVertices = scene->mMeshes[idMesh]->mNumVertices;
        aiMesh* mesh = scene->mMeshes[idMesh];

        if (mesh->mNumBones > 0){
            Bones.resize(NumVertices);
//            cout << "mesh " << idMesh << " tiene " << mesh->mNumBones << " bones" <<endl;
            for(GLuint j = 0; j < mesh->mNumBones; j++){
                uint32_t BoneIndex = 0;
                string BoneName(mesh->mBones[j]->mName.data);
//                cout << "BoneName: " << BoneName << endl;
                if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
                    // Allocate an index for a new bone
                    BoneIndex = m_NumBones;
                    m_NumBones++;
                    BoneInfo bi;
                    m_BoneInfo.push_back(bi);
                    m_BoneInfo[BoneIndex].BoneOffset = mesh->mBones[j]->mOffsetMatrix;
                    m_BoneMapping[BoneName] = BoneIndex;
                    //cout << "BoneName: " << BoneName << endl;
                } else {
                    BoneIndex = m_BoneMapping[BoneName];
                }

                //cout << "Hay: " << mesh->mBones[j]->mNumWeights << " Weights" << endl;
                for (uint32_t k = 0 ; k < mesh->mBones[j]->mNumWeights ; k++) {
                    uint32_t VertexID = mesh->mBones[j]->mWeights[k].mVertexId;
                    float Weight  = mesh->mBones[j]->mWeights[k].mWeight;
                    Bones[VertexID].AddBoneData(BoneIndex, Weight);
                }
            }
        }
    }

    void testTextures(aiMaterial* mat){
        if (mat->GetTextureCount(aiTextureType_NONE) > 0) cout << "aiTextureType_NONE" << ", ";
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) cout << "aiTextureType_DIFFUSE" << ", ";
        if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) cout << "aiTextureType_SPECULAR" << ", ";
        if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0) cout << "aiTextureType_AMBIENT" << ", ";
        if (mat->GetTextureCount(aiTextureType_EMISSIVE) > 0) cout << "aiTextureType_EMISSIVE" << ", ";
        if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0) cout << "aiTextureType_HEIGHT" << ", ";
        if (mat->GetTextureCount(aiTextureType_NORMALS) > 0) cout << "aiTextureType_NORMALS" << ", ";
        if (mat->GetTextureCount(aiTextureType_SHININESS) > 0) cout << "aiTextureType_SHININESS" << ", ";
        if (mat->GetTextureCount(aiTextureType_OPACITY) > 0) cout << "aiTextureType_OPACITY" << ", ";
        if (mat->GetTextureCount(aiTextureType_DISPLACEMENT) > 0) cout << "aiTextureType_DISPLACEMENT" << ", ";
        if (mat->GetTextureCount(aiTextureType_LIGHTMAP) > 0) cout << "aiTextureType_LIGHTMAP" << ", ";
        if (mat->GetTextureCount(aiTextureType_REFLECTION) > 0) cout << "aiTextureType_REFLECTION" << ", ";
        if (mat->GetTextureCount(aiTextureType_UNKNOWN) > 0) cout << "aiTextureType_UNKNOWN" << ", ";
        cout << endl;
    }


    /**
    * Checks all material textures of a given type and loads the textures if they're not loaded yet.
    * The required info is returned as a Texture struct.
    */
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type)
    {
        vector<Texture> textures;
        for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            aiString nombre;
            mat->Get(AI_MATKEY_NAME, nombre);
            aiColor3D colorTrans (0.f,0.f,0.f);
            mat->Get(AI_MATKEY_COLOR_TRANSPARENT, colorTrans);
            aiColor3D colorAmbient (0.f,0.f,0.f);
            mat->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient);
            aiColor3D colorDiffuse (0.f,0.f,0.f);
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse);
            aiColor3D colorSpecular (0.f,0.f,0.f);
            mat->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular);
            aiColor3D colorEmissive (0.f,0.f,0.f);
            mat->Get(AI_MATKEY_COLOR_EMISSIVE, colorEmissive);
            float shineStrength = 0;
            mat->Get(AI_MATKEY_SHININESS_STRENGTH, shineStrength);
            float shininess = 0;
            mat->Get(AI_MATKEY_SHININESS, shininess);
            int blend = 0;
            mat->Get(AI_MATKEY_BLEND_FUNC, blend);
            float opacity = 0;
            mat->Get(AI_MATKEY_OPACITY ,opacity);

//            cout << " name: " << nombre.data;
//            cout << " a: " << colorAmbient.r << "," << colorAmbient.g << "," << colorAmbient.b;
//            cout << " d: " << colorDiffuse.r << "," << colorDiffuse.g << "," << colorDiffuse.b;
//            cout << " s: " << colorSpecular.r << "," << colorSpecular.g << "," << colorSpecular.b;
//            cout << " e: " << colorEmissive.r << "," << colorEmissive.g << "," << colorEmissive.b;
//            cout << " shineStrength: " << shineStrength;
//            cout << " shininess: " << shininess;
//            cout << " opacity: " << opacity;
//            cout << " blend: " << blend;
//            cout << " trans: " << colorTrans.r << "," << colorTrans.g << "," << colorTrans.b << endl;


            // Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            GLboolean skip = false;
            for(GLuint j = 0; j < textures_loaded.size(); j++){
                if(textures_loaded[j].path == str){
                    textures.push_back(textures_loaded[j]);
                    skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }

            if(!skip)
            {   // If texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory, true);
                texture.type = type;
                texture.path = str;
                textures.push_back(texture);
                this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
//                cout << " LOADED!!";
            }

//            cout << endl;
        }
        return textures;
    }

    /**
    *
    */
    bool InitFromScene(const aiScene* pScene, const string& Filename)
    {
//        m_GlobalInverseTransform = glm::inverse(aiMatrix4x4ToGlm(&pScene->mRootNode->mTransformation));
        m_GlobalInverseTransform = pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform.Inverse();
        return GLCheckError();
    }


    /**
    *
    */
    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName){
        for (uint32_t i = 0 ; i < pAnimation->mNumChannels ; i++) {
            const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

            if (string(pNodeAnim->mNodeName.data) == NodeName) {
                return pNodeAnim;
            }
        }
        return NULL;
    }

    /**
    *
    */
    uint32_t FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim){
        assert(pNodeAnim->mNumScalingKeys > 0);
        for (uint32_t i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
                return i;
            }
        }
//        assert(0);
        return 0;
    }

    /**
    *
    */
    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumScalingKeys == 1) {
            Out = pNodeAnim->mScalingKeys[0].mValue;
            return;
        }

        uint32_t ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
        uint32_t NextScalingIndex = (ScalingIndex + 1);
        assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
        float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);

        float preFactor = AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
        if (preFactor < 0.0f) preFactor = 0.0f;
        float Factor = preFactor / DeltaTime;
//        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }

    /**
    *
    */
    uint32_t FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumRotationKeys > 0);

        for (uint32_t i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
                return i;
            }
        }

//        assert(0);

        return 0;
    }

    /**
    *
    */
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        // we need at least two values to interpolate...
        if (pNodeAnim->mNumRotationKeys == 1) {
            Out = pNodeAnim->mRotationKeys[0].mValue;
            return;
        }

        uint32_t RotationIndex = FindRotation(AnimationTime, pNodeAnim);
        uint32_t NextRotationIndex = (RotationIndex + 1);
        assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
        float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);

        float preFactor = AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
        if (preFactor < 0.0f) preFactor = 0.0f;

        float Factor = preFactor / DeltaTime;
//        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
        Out = Out.Normalize();
    }

    /**
    *
    */
    uint32_t FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        for (uint32_t i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
                return i;
            }
        }
//        assert(0);
        return 0;
    }

    /**
    *
    */
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumPositionKeys == 1) {
            Out = pNodeAnim->mPositionKeys[0].mValue;
            return;
        }

        uint32_t PositionIndex = FindPosition(AnimationTime, pNodeAnim);
        uint32_t NextPositionIndex = (PositionIndex + 1);
        assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
        float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);

        float preFactor = AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
        if (preFactor < 0.0f) preFactor = 0.0f;

        float Factor = preFactor / DeltaTime;
//        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }

    /**
    *
    */
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const Matrix4f& ParentTransform){
        string NodeName(pNode->mName.data);
        const aiAnimation* pAnimation = mp_scene->mAnimations[0];
        Matrix4f NodeTransformation(pNode->mTransformation);
        const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

        if (pNodeAnim) {
            // Interpolate scaling and generate scaling transformation matrix
            aiVector3D Scaling;
            CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
            Matrix4f ScalingM;
            ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

            // Interpolate rotation and generate rotation transformation matrix
            aiQuaternion RotationQ;
            CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
            Matrix4f RotationM = Matrix4f(RotationQ.GetMatrix());

            // Interpolate translation and generate translation transformation matrix
            aiVector3D Translation;
            CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
            Matrix4f TranslationM;
            TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

            // Combine the above transformations
            NodeTransformation = TranslationM * RotationM * ScalingM;
        }

        Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            const uint32_t BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
        }

        for (uint32_t i = 0 ; i < pNode->mNumChildren ; i++) {
            ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
        }
    }

//    void InitTranslationTransform(float x, float y, float z, glm::mat4 &m)
//    {
//        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = x;
//        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = y;
//        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = z;
//        m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
//    }
//
//    void InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ, glm::mat4 &m)
//    {
//        m[0][0] = ScaleX; m[0][1] = 0.0f;   m[0][2] = 0.0f;   m[0][3] = 0.0f;
//        m[1][0] = 0.0f;   m[1][1] = ScaleY; m[1][2] = 0.0f;   m[1][3] = 0.0f;
//        m[2][0] = 0.0f;   m[2][1] = 0.0f;   m[2][2] = ScaleZ; m[2][3] = 0.0f;
//        m[3][0] = 0.0f;   m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
//    }

//    void Matrix4f::InitRotateTransform(const Quaternion& quat)
//    {
//        float yy2 = 2.0f * quat.y * quat.y;
//        float xy2 = 2.0f * quat.x * quat.y;
//        float xz2 = 2.0f * quat.x * quat.z;
//        float yz2 = 2.0f * quat.y * quat.z;
//        float zz2 = 2.0f * quat.z * quat.z;
//        float wz2 = 2.0f * quat.w * quat.z;
//        float wy2 = 2.0f * quat.w * quat.y;
//        float wx2 = 2.0f * quat.w * quat.x;
//        float xx2 = 2.0f * quat.x * quat.x;
//        m[0][0] = - yy2 - zz2 + 1.0f;
//        m[0][1] = xy2 + wz2;
//        m[0][2] = xz2 - wy2;
//        m[0][3] = 0;
//        m[1][0] = xy2 - wz2;
//        m[1][1] = - xx2 - zz2 + 1.0f;
//        m[1][2] = yz2 + wx2;
//        m[1][3] = 0;
//        m[2][0] = xz2 + wy2;
//        m[2][1] = yz2 - wx2;
//        m[2][2] = - xx2 - yy2 + 1.0f;
//        m[2][3] = 0.0f;
//        m[3][0] = m[3][1] = m[3][2] = 0;
//        m[3][3] = 1.0f;
//    }

    /**
*
*/
bool isDir(string ruta){
    struct stat info;
    stat(ruta.c_str(), &info);

    if(S_ISDIR(info.st_mode)){
        return true;
    } else{
        return false;
    }

}

/**
* Comprueba si existe el directorio o fichero pasado por parámetro
*/
bool existe(string ruta){
    if(isDir(ruta)){
        return true;
    } else {
        FILE *archivo = fopen(ruta.c_str(), "r");
        if (archivo != NULL) {
            fclose(archivo);
            return true; //TRUE
        } else {
            return false; //FALSE
        }
    }
}

/**
* Obtiene el directorio de un fichero
*/
string getFileName(string file){
    if(isDir(file)){
        return file;
    } else {
        size_t found;
        found = file.find_last_of("\\");
        if (found != string::npos){
            return file.substr(found  + 1, file.length() - found - 1);
        } else {
            found = file.find_last_of("/");
            if (found != string::npos){
                return file.substr(found  + 1, file.length() - found - 1);
            } else {
                return file;
            }
        }
    }
}

/**
*
*/
GLint TextureFromFile(const char* path, string directory, GLboolean alpha = false)
{
     //Generate texture ID and load texture data
    string filename = directory + '/' + string(path);
    if (!existe(filename)){
        filename = directory + '/' + getFileName(path);
    }

    //With soil we can create new textures id with the flag "SOIL_CREATE_NEW_ID".
    //So we don't need to call "glGenTextures(1, &textureID);"
    GLuint textureID;
    int width,height;

    textureID = SOIL_load_OGL_texture(
            filename.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID, //    glGenTextures(1, &textureID);
            0
            | SOIL_FLAG_POWER_OF_TWO
            | SOIL_FLAG_MIPMAPS
            | SOIL_FLAG_COMPRESS_TO_DXT
            | SOIL_FLAG_DDS_LOAD_DIRECT
            //| SOIL_FLAG_MULTIPLY_ALPHA
            //| SOIL_FLAG_NTSC_SAFE_RGB
            //| SOIL_FLAG_CoCg_Y
            //| SOIL_FLAG_TEXTURE_RECTANGLE
            );

        if( textureID > 0 ){
            glBindTexture( GL_TEXTURE_2D, textureID );
			glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//unbinds texture
			glBindTexture(GL_TEXTURE_2D, 0);
//			std::cout << "the loaded texture ID was " << textureID << std::endl;
		} else {
//		    std::cout << "Attempting to load image" << std::endl;
            // Assign texture to ID if we failed from SOIL_load_OGL_texture
            glGenTextures(1, &textureID);
            unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB); //SOIL_LOAD_AUTO
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            //de allocates resources and unbinds texture
            SOIL_free_image_data(image);
            glBindTexture(GL_TEXTURE_2D, 0);
//            std::cout << "Image loaded" << std::endl;
		}

    return textureID;
}

};



#endif // MODEL_H_INCLUDED
