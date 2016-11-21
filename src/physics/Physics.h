#include "btBulletDynamicsCommon.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "../examples/CommonInterfaces/CommonRigidBodyBase.h"

#include "mydebug.h"

#include <vector>
#include <map>
#include <string>

using namespace std;

class Physics{
    public:
        Physics();
        ~Physics();
        void initObjects();

        btDiscreteDynamicsWorld* getDynamicsWorld(){return dynamicsWorld;}
        std::vector<btCollisionShape *>* getCollisionShapes(){return &collisionShapes;}
        int getCollisionObjectCount(){return dynamicsWorld->getCollisionObjectArray().size() ;}

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
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        btBroadphaseInterface* overlappingPairCache;
        btSequentialImpulseConstraintSolver* solver;
        btDiscreteDynamicsWorld* dynamicsWorld;
        std::vector<btCollisionShape *> collisionShapes;
        std::map<std::string, btRigidBody *> physicsAccessors;

};
