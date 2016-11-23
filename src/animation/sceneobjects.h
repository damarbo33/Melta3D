#ifndef SCENEOBJECTS_H
#define SCENEOBJECTS_H

#include "../Model.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "../examples/CommonInterfaces/CommonRigidBodyBase.h"
#include "../common/structs.h"
#include "../physics/Physics.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


using namespace std;

enum {
    APROXNONE,
    APROXHULL,
    APROXCYCLINDER
} eAproxHull;

class object3D{

    public:
        object3D(){
            stencil = false;
            spinningFriction = 1.0f;
            velocity = 2.0f;
            instantStop = true;
            mass = 0.0f;
            convex = true;
            scaling = btVector3(1,1,1);
            groundContact = false;
            shape = NULL;
            aproxHullShape = APROXHULL;
            position = btVector3(0,0,0);
            dimension = btVector3(1,1,1);
        }

        bool stencil;
        string tag;
        float spinningFriction;
        float velocity;
        float mass;
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

        btVector3 position;

        btCollisionShape* shape;

        btCollisionShape* createShapeWithVertices(Model *ourModel);
        btVector3 scaleToMeters(btVector3 &scaleMeters, btVector3 &aabb);

    private:
        void addPhysMeshTriangle(Model *ourModel, btVector3* triMeshPhis, Vertex &vec1, Vertex &vec2, Vertex &vec3);

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
        int initShape(btVector3 initialPosition, Model *ourModel, btVector3 dimension);
        bool getObjectModel(int i, glm::vec3 scale, glm::vec3 offset,  glm::mat4 &model);
        bool getWorldModel(int i, glm::vec3 scale, glm::vec3 offset,  glm::mat4 &model);
        object3D *getObjPointer(int i);

        Physics * getPhysics(){
            return physicsEngine;
        }


    protected:

    private:
        Physics *physicsEngine;
};



#endif // SCENEOBJECTS_H
