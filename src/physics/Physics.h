#include "btBulletDynamicsCommon.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "../examples/CommonInterfaces/CommonRigidBodyBase.h"


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

    protected:
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        btBroadphaseInterface* overlappingPairCache;
        btSequentialImpulseConstraintSolver* solver;
        btDiscreteDynamicsWorld* dynamicsWorld;
        std::vector<btCollisionShape *> collisionShapes;
        std::map<std::string, btRigidBody *> physicsAccessors;

};
