#ifndef SCENEOBJECTS_H
#define SCENEOBJECTS_H

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include "../Model.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "../examples/CommonInterfaces/CommonRigidBodyBase.h"
#include "../common/structs.h"
#include "../physics/Physics.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


using namespace std;

enum {
    APROXNONE,
    APROXHULL,
    APROXCYCLINDER
} eAproxHull;

enum {
    IMPULSEX=0,
    IMPULSEY,
    IMPULSEZ
}eImpulseAxis;

enum {
    IMPULSEPOS  = 1,
    IMPULSEZERO = 0,
    IMPULSENEG  = -1
}eImpulseSense;

class object3D{

    public:
        object3D(){
            stencil = false;
            stencilDepthTest = true;
            stencilScale = 1.1f;
            spinningFriction = 1.0f;
            mass = 0.1f;
            friction = 0.1f;
            rollingFriction = 0.1f;
            restitution = 0.1f;
            velocity = 2.0f;
            instantStop = true;
            convex = true;
            scaling = btVector3(1,1,1);
            groundContact = false;
            shape = NULL;
            aproxHullShape = APROXHULL;
            position = btVector3(0,0,0);
            dimension = btVector3(1,1,1);
            rotation = glm::quat(0,0,0,0);
            meshModel = NULL;
            impulseSense = IMPULSEPOS;
            impulseAxis = IMPULSEZ;
            rotationSense = IMPULSEPOS;
        }

        virtual ~object3D();

        //flag to highlight the object
        bool  stencil;
        float stencilScale;
        bool  stencilDepthTest;
        //Name to identify object
        string tag;
        float spinningFriction;
        //Velocity of the object
        float velocity;
        //set the mass of the object. a mass of "0" means that it is an immovable object
        float mass;
        float friction;
        float rollingFriction;
        float restitution;
        //Specify if the object is concave or convex. Always convex=true is better for performance
        bool convex;
        //This makes some calcules that implies some overhead to reset the forces
        //when an object touches floor
        bool instantStop;
        //flag to know if a object touches floor. By now, it is informed only when instantStop = true
        bool groundContact;
        //Scaling for the object
        btVector3 scaling;
        //To aproximate the hull shape and reduce the number of poligons
        int aproxHullShape;
        //Dimension of the object
        btVector3 dimension;
        //Initial position of the object
        btVector3 position;
        //Rotation of object
        glm::quat rotation;
        //Mesh of the object
        Model *meshModel;
        //Sense of the vector
        int impulseSense;
        //Axis of the reference object for impulse
        int impulseAxis;
        //Sense of the rotation
        int rotationSense;

        btCollisionShape* createShapeWithVertices(Model *ourModel);
        btVector3 scaleToMeters(btVector3 &scaleMeters, btVector3 &aabb);

    private:
        void addPhysMeshTriangle(Model *ourModel, btVector3* triMeshPhis, Vertex &vec1, Vertex &vec2, Vertex &vec3);
        btCollisionShape* shape;


};

class SceneObjects
{
    public:
        /** Default constructor */
        SceneObjects();
        /** Default destructor */
        virtual ~SceneObjects();

        void activateStencil(bool stencil){
            this->stencil = stencil;

            if (stencil){
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            } else {
                glDisable(GL_STENCIL_TEST);
            }
        }

        bool stencil;
        vector <object3D *> listObjects;

        int initShape(object3D *obj);
        bool getObjectModel(int i, glm::vec3 scale, glm::vec3 offset,  glm::mat4 &model);

        object3D *getObjPointer(int i);

        Physics * getPhysics(){
            return physicsEngine;
        }

        void createModelStencil(int i);
        bool mustProcessStencil(int i, glm::mat4 &model, Shader &shader);
        void cleanScreen();


    protected:

    private:
        Physics *physicsEngine;
};



#endif // SCENEOBJECTS_H
