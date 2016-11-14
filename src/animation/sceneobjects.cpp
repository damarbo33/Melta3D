#include "sceneobjects.h"

SceneObjects::SceneObjects()
{
    this->stencil = false;
    physicsEngine = new Physics();
    physicsEngine->getDynamicsWorld()->setGravity(btVector3(0, -9.8f, 0));
}

SceneObjects::~SceneObjects()
{
    //dtor
}

/**
*
*/
object3D2 *SceneObjects::getObjPointer(int i){
    btCollisionObject* obj = getPhysics()->getDynamicsWorld()->getCollisionObjectArray()[i];
    btRigidBody* capsuleBody = btRigidBody::upcast(obj);
    return (object3D2 *)capsuleBody->getUserPointer();
}


/**
*
*/
btCollisionShape* object3D2::createShapeWithVertices(Model *ourModel, bool convex, btVector3 scaling){
    //1
    if (convex){
        //2
        btCollisionShape* _shape = new btConvexHullShape();

        for (int idMesh=0; idMesh < ourModel->getMeshes()->size(); idMesh++){
            vector<Vertex> *vertices = ourModel->getMeshes()->at(idMesh)->getVertices();
            for (int i = 0; i < vertices->size(); i+=5){
                Vertex v = vertices->at(i);
                btVector3 btv = btVector3(v.Position[0], v.Position[1], v.Position[2]);
                ((btConvexHullShape*)_shape)->addPoint(btv);
            }
        }
        ((btConvexHullShape*)_shape)->setLocalScaling(scaling);
        return ((btConvexHullShape*)_shape);
    }
    else
    {
        //3
        btTriangleMesh* mesh = new btTriangleMesh();
        for (int idMesh=0; idMesh < ourModel->getMeshes()->size(); idMesh++){
            vector<Vertex> *vertices = ourModel->getMeshes()->at(idMesh)->getVertices();
            for (int i=0; i < vertices->size(); i += 3)
            {
                Vertex v1 = vertices->at(i);
                Vertex v2 = vertices->at(i+1);
                Vertex v3 = vertices->at(i+2);

                btVector3 bv1 = btVector3(v1.Position[0], v1.Position[1], v1.Position[2]);
                btVector3 bv2 = btVector3(v2.Position[0], v2.Position[1], v2.Position[2]);
                btVector3 bv3 = btVector3(v3.Position[0], v3.Position[1], v3.Position[2]);

                mesh->addTriangle(bv1, bv2, bv3);
            }
        }
       btCollisionShape* _shape = new btBvhTriangleMeshShape(mesh, true);
       return ((btBvhTriangleMeshShape*)_shape);
    }
}

/**
*
*/
int SceneObjects::initShape(btVector3 initialPosition, Model *ourModel, btVector3 scaling){

    btCollisionShape *newRigidShape = NULL;
    object3D2 *obj = new object3D2();

    if (ourModel != NULL){
        newRigidShape = obj->createShapeWithVertices(ourModel, true, scaling);
    } else {
        //create the new shape, and tell the physics that is a Box
        newRigidShape = new btCylinderShape(btVector3(0.3f, 1.0f, 0.3f));
    }

    physicsEngine->getCollisionShapes()->push_back(newRigidShape);

    //set the initial position and transform. For this demo, we set the tranform to be none
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setRotation(btQuaternion(0,0,0,1));

    //set the mass of the object. a mass of "0" means that it is an immovable object
    btScalar mass = 0.1f;
    btVector3 localInertia(0,0,0);
    //btVector3 localInertia(1.0f, 1.0f, 1.0f);

    startTransform.setOrigin(initialPosition);
    newRigidShape->calculateLocalInertia(mass, localInertia);

    //actually contruct the body and add it to the dynamics world
    btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, newRigidShape, localInertia);
    btRigidBody *body = new btRigidBody(rbInfo);
    //Set bounce property
    body->setRestitution(0.0);


    obj->spinningFriction = 10.0f;
    obj->tag = "model";
    obj->instantStop = false;
    obj->scaling = scaling;

    body->setUserPointer(obj);

    body->setRollingFriction(0.1f);
    body->setFriction(0.1f);

    //restricting movement in planes
    body->setLinearFactor(btVector3(1,1,1));
    body->setAngularFactor(btVector3(0,0,0));

    body->setActivationState(DISABLE_DEACTIVATION);

    physicsEngine->getDynamicsWorld()->addRigidBody(body);
    //physicsEngine->trackRigidBodyWithName(body, physicsCubeName);
}

/**
*
*/
bool SceneObjects::getOMWorld(int i, glm::vec3 scale, glm::vec3 offset,  glm::mat4 &model){
    btCollisionObject* obj = getPhysics()->getDynamicsWorld()->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    bool out = false;
    if (body && body->getMotionState()){
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);

        void *userPointer = body->getUserPointer();
        if (userPointer) {
            btQuaternion orientation = trans.getRotation();
            glm::quat MyQuaternion   = glm::quat(orientation.getW(),orientation.getX(), orientation.getY(), orientation.getZ());
            glm::mat4 RotationMatrix = glm::toMat4(MyQuaternion);
            model = glm::translate(model, glm::vec3(trans.getOrigin().getX()
                    + offset.x, trans.getOrigin().getY() + offset.y, trans.getOrigin().getZ() + offset.z));
            model = glm::scale(model, glm::vec3(scale.x, scale.y, scale.z));	// Para el piloto mesh
            model =  model * RotationMatrix;
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            out = true;
        }
    }

    return out;
}

