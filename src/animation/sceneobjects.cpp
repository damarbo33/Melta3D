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
        this->shape = new btConvexHullShape();
        for (int idMesh=0; idMesh < ourModel->getMeshes()->size(); idMesh++){

            vector<Vertex> *vertices = ourModel->getMeshes()->at(idMesh)->getVertices();
//            vector<GLuint> *indices  = ourModel->getMeshes()->at(idMesh)->getIndices();
//
//            int index_count = indices->size();
//
//            if (index_count > 0){
//                for (int i=0; i < index_count; i++){
//                    const Vertex vec1 = vertices->at(indices->at(i));
//                    _this->shape->addPoint(btVector3(vec1.Position[0], vec1.Position[1], vec1.Position[2]));
//                }
//            } else {
                for (int i = 0; i < vertices->size(); i+=4){
                    const Vertex v = vertices->at(i);
                    ((btConvexHullShape*)this->shape)->addPoint(btVector3(v.Position[0], v.Position[1], v.Position[2]));
                }
//            }


        }
        this->shape->setLocalScaling(scaling);
        return this->shape;

//        ((btConvexHullShape*)_this->shape)->setLocalScaling(scaling);
//        //create a hull approximation
//        btShapeHull* hull = new btShapeHull(((btConvexHullShape*)_this->shape));
//        btScalar margin = _this->shape->getMargin();
//        hull->buildHull(margin);
//        btConvexHullShape* simplifiedConvexShape = new btConvexHullShape(hull->getVertexPointer(),hull->numVertices());
//        delete _this->shape;
//        return simplifiedConvexShape;
    }
    else
    {
        //3
        /**ATTENTION: We are using a static-triangle mesh this->shape,
        *  it can only be used for fixed/non-moving objects
        */
        float maxX=0, minX=0, maxY=0, minY=0, maxZ=0, minZ=0;
        int nVertMeshes = 0;
        int nIndxMeshes = 0;
        bool createdNew = false;
        bool indexed = false;

        //Calculamos en primer lugar el numero de vertices de todos los meshes
        for (int idMesh=0; idMesh < ourModel->getMeshes()->size(); idMesh++){
            vector<Vertex> *vertices = ourModel->getMeshes()->at(idMesh)->getVertices();
            vector<GLuint> *indices  = ourModel->getMeshes()->at(idMesh)->getIndices();
            int index_count = indices->size();
            //Aseguramos que hay un numero correcto de vertices. En caso contrario (que no deberia pasar)
            //replicamos el ultimo vertice
            int nVertices = vertices->size();

            int verticesLeft =  nVertices % 3 == 0 ? 0 : 3 - nVertices % 3;
            if (verticesLeft != 0 && nVertices > 0){
                for (int i=0; i < verticesLeft; i++){
                    vertices->push_back(vertices->at(nVertices - 1));
                }
            }
            nVertices = vertices->size();
            nVertMeshes += nVertices;
            nIndxMeshes += index_count;
        }

        indexed = nIndxMeshes != 0;
        unsigned int totalFaces = indexed ? nIndxMeshes / 3 : nVertMeshes / 3;

        //Reservamos espacio para el modelo que no tiene
        if (ourModel->triMeshPhis == NULL){
            ourModel->triMeshPhis = new btVector3*[totalFaces];
            createdNew = true;
        }

        int cFace = 0;

        for (int idMesh=0; idMesh < ourModel->getMeshes()->size(); idMesh++){
            vector<Vertex> *vertices = ourModel->getMeshes()->at(idMesh)->getVertices();
            vector<GLuint> *indices  = ourModel->getMeshes()->at(idMesh)->getIndices();
            int index_count = indices->size();

            if (!indexed){
                if (createdNew){
                    unsigned int nFaces = vertices->size()/3;
                    for (int i=0; i < nFaces; i++){
                        ourModel->triMeshPhis[cFace] = new btVector3[3];
                        const Vertex vec1 = vertices->at(i*3);
                        ourModel->triMeshPhis[cFace][0].setX(btScalar(vec1.Position.x));
                        ourModel->triMeshPhis[cFace][0].setY(btScalar(vec1.Position.y));
                        ourModel->triMeshPhis[cFace][0].setZ(btScalar(vec1.Position.z));
                        const Vertex vec2 = vertices->at(i*3+1);
                        ourModel->triMeshPhis[cFace][1].setX(btScalar(vec2.Position.x));
                        ourModel->triMeshPhis[cFace][1].setY(btScalar(vec2.Position.y));
                        ourModel->triMeshPhis[cFace][1].setZ(btScalar(vec2.Position.z));
                        const Vertex vec3 = vertices->at(i*3+2);
                        ourModel->triMeshPhis[cFace][2].setX(btScalar(vec3.Position.x));
                        ourModel->triMeshPhis[cFace][2].setY(btScalar(vec3.Position.y));
                        ourModel->triMeshPhis[cFace][2].setZ(btScalar(vec3.Position.z));

                        ourModel->physMesh->addTriangle(ourModel->triMeshPhis[cFace][0],
                                                    ourModel->triMeshPhis[cFace][1],
                                                    ourModel->triMeshPhis[cFace][2], false); // false, don’t remove duplicate vertices
                        cFace++;
                    }
//                    if (idMesh == 0 && i == 0){
//                        maxX = v1.Position[0];
//                        minX = v1.Position[0];
//                        maxY = v1.Position[1];
//                        minY = v1.Position[1];
//                        maxZ = v1.Position[2];
//                        minZ = v1.Position[2];
//                    }

//                    if (v1.Position[0] > maxX) maxX = v1.Position[0];
//                    if (v2.Position[0] > maxX) maxX = v2.Position[0];
//                    if (v3.Position[0] > maxX) maxX = v3.Position[0];
//                    if (v1.Position[0] < minX) minX = v1.Position[0];
//                    if (v2.Position[0] < minX) minX = v2.Position[0];
//                    if (v3.Position[0] < minX) minX = v3.Position[0];
//
//                    if (v1.Position[1] > maxY) maxY = v1.Position[1];
//                    if (v2.Position[1] > maxY) maxY = v2.Position[1];
//                    if (v3.Position[1] > maxY) maxY = v3.Position[1];
//                    if (v1.Position[1] < minY) minY = v1.Position[1];
//                    if (v2.Position[1] < minY) minY = v2.Position[1];
//                    if (v3.Position[1] < minY) minY = v3.Position[1];
//
//                    if (v1.Position[2] > maxZ) maxZ = v1.Position[2];
//                    if (v2.Position[2] > maxZ) maxZ = v2.Position[2];
//                    if (v3.Position[2] > maxZ) maxZ = v3.Position[2];
//                    if (v1.Position[2] < minZ) minZ = v1.Position[2];
//                    if (v2.Position[2] < minZ) minZ = v2.Position[2];
//                    if (v3.Position[2] < minZ) minZ = v3.Position[2];

//                    ourModel->physMesh->addTriangle(bv1, bv2, bv3,false); // false, don’t remove duplicate vertices
                }
            } else {
                if (createdNew){
                    unsigned int nFaces = index_count/3;
                    for (int i=0; i < nFaces; i++){
                        ourModel->triMeshPhis[cFace] = new btVector3[3];
                        const Vertex vec1 = vertices->at(indices->at(i*3));
                        ourModel->triMeshPhis[cFace][0].setX(btScalar(vec1.Position.x));
                        ourModel->triMeshPhis[cFace][0].setY(btScalar(vec1.Position.y));
                        ourModel->triMeshPhis[cFace][0].setZ(btScalar(vec1.Position.z));
                        const Vertex vec2 = vertices->at(indices->at(i*3+1));
                        ourModel->triMeshPhis[cFace][1].setX(btScalar(vec2.Position.x));
                        ourModel->triMeshPhis[cFace][1].setY(btScalar(vec2.Position.y));
                        ourModel->triMeshPhis[cFace][1].setZ(btScalar(vec2.Position.z));
                        const Vertex vec3 = vertices->at(indices->at(i*3+2));
                        ourModel->triMeshPhis[cFace][2].setX(btScalar(vec3.Position.x));
                        ourModel->triMeshPhis[cFace][2].setY(btScalar(vec3.Position.y));
                        ourModel->triMeshPhis[cFace][2].setZ(btScalar(vec3.Position.z));

                        ourModel->physMesh->addTriangle(ourModel->triMeshPhis[cFace][0],
                                                    ourModel->triMeshPhis[cFace][1],
                                                    ourModel->triMeshPhis[cFace][2], false); // false, don’t remove duplicate vertices
                        cFace++;
                    }
                }
            }
        }

        cout << "Added " << cFace << " faces"<< endl;

        cout << "mesh with " << ourModel->physMesh->getNumTriangles() << " triangles" << endl;
        if (ourModel->physMesh->getNumTriangles() > 0){
            this->shape = new btBvhTriangleMeshShape(ourModel->physMesh, true);
            this->shape->setLocalScaling(scaling);
//        } else if (triangleIndex != NULL && triangleIndex->getNumSubParts() > 0){
//            _this->shape = new btBvhTriangleMeshShape(triangleIndex, true);
//            _this->shape->setLocalScaling(scaling);
        } else {
            cout << "createShapeWithVertices: Shape without indices" << endl;
        }

        cout << "maxX: " << maxX << " minX: " << minX <<
        "maxY: " << maxY << " minY: " << minY <<
        "maxZ: " << maxZ << " minZ: " << minZ << endl;

        return this->shape;
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

bool SceneObjects::getWorldModel(int i, glm::vec3 scale, glm::vec3 offset,  glm::mat4 &model){
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
//            cout << "origX: " << trans.getOrigin().getX() << endl;
//            cout << "origY: " << trans.getOrigin().getY() << endl;
//            cout << "origZ: " << trans.getOrigin().getZ() << endl;

            model = glm::translate(model, glm::vec3(trans.getOrigin().getX()
                    + offset.x, trans.getOrigin().getY() + offset.y, trans.getOrigin().getZ() + offset.z));
            model = glm::scale(model, glm::vec3(scale.x, scale.y, scale.z));	// Para el mundo
            model =  model * RotationMatrix;
            //model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            out = true;
        }
    }

    return out;
}

