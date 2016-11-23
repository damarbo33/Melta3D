#include "Physics.h"

Physics::Physics(int debug){
    setDebug(debug);
    initObjects();
}

Physics::~Physics(){
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
}

void Physics::initObjects() {
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    if (getDebug() > 0){
        CibtDebugDraw *debuger = new CibtDebugDraw();
        dynamicsWorld->setDebugDrawer(debuger);
    }
}
