#ifndef SCENEOBJECTS_H
#define SCENEOBJECTS_H

#include <vector>
#include <map>
#include <string>

// GLM Mathemtics
#include <glm/glm.hpp>

// GLFW
#include <GLFW/glfw3.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "../examples/CommonInterfaces/CommonRigidBodyBase.h"
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

using namespace std;

class object3D : public btRigidBody{

    public:
        object3D(btRigidBody::btRigidBodyConstructionInfo rbInfo)
            : btRigidBody(rbInfo){
            stencil = false;
        }
        bool stencil;

    private:

};

class object3D2{

    public:
        object3D2(){
            stencil = false;
            spinningFriction = 1.0f;
            velocity = 1.5f;
            instantStop = true;
        }
        bool stencil;
        string name;
        float spinningFriction;
        float velocity;
        bool instantStop;

    private:

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

        struct MyContactResultCallback : public btCollisionWorld::ContactResultCallback
        {
            bool contact;

            MyContactResultCallback(){
                contact = false;
            }

            btScalar addSingleResult(btManifoldPoint& cp,
                const btCollisionObjectWrapper* colObj0Wrap,
                int partId0,
                int index0,
                const btCollisionObjectWrapper* colObj1Wrap,
                int partId1,
                int index1)
            {
                contact = true;
            }

            bool isContact(){return contact;}
        };

    protected:

    private:
};



#endif // SCENEOBJECTS_H
