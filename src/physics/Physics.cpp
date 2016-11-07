#include "Physics.h"

Physics::Physics(){
    initObjects();
}

Physics::~Physics(){
    delete collisionConfiguration;
    delete dispatcher;
    delete overlappingPairCache;
    delete solver;
    delete dynamicsWorld;
}

void Physics::initObjects() {
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
}
