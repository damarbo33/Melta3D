#include "sceneobjects.h"

SceneObjects::SceneObjects()
{
    this->stencil = false;
    physicsEngine = new Physics(1);
    physicsEngine->getDynamicsWorld()->setGravity(btVector3(0, -9.8f, 0));
}

SceneObjects::~SceneObjects()
{
    delete physicsEngine;

    for (int i=0; i < listObjects.size(); i++){
        delete listObjects.at(i);
    }
}

/**
*
*/
object3D *SceneObjects::getObjPointer(int i){
    btCollisionObject* obj = getPhysics()->getDynamicsWorld()->getCollisionObjectArray()[i];
    btRigidBody* capsuleBody = btRigidBody::upcast(obj);
    return (object3D *)capsuleBody->getUserPointer();
}


object3D::~object3D(){

}

/**
*
*/
btCollisionShape* object3D::createShapeWithVertices(Model *ourModel){
    //1
    if (convex){
        btConvexHullShape* originalConvexShape = new btConvexHullShape();
        //2
        for (int idMesh=0; idMesh < ourModel->getMeshes()->size(); idMesh++){
            vector<Vertex> *vertices = ourModel->getMeshes()->at(idMesh)->getVertices();
            for (int i = 0; i < vertices->size(); i += 4){
                const Vertex v = vertices->at(i);
                originalConvexShape->addPoint(btVector3(v.Position[0], v.Position[1], v.Position[2]));
            }
        }

        if (aproxHullShape == APROXHULL){
            //create a hull approximation
            btShapeHull* hull = new btShapeHull(originalConvexShape);
            btScalar margin = originalConvexShape->getMargin();
            hull->buildHull(margin);
            this->shape = new btConvexHullShape(hull->getVertexPointer()->m_floats, hull->numVertices());
            delete hull;
            delete originalConvexShape;
        } else if (aproxHullShape == APROXCYCLINDER){
            btTransform t;
            t.setIdentity();
            btVector3 aabmin, aabmax;
            originalConvexShape->getAabb(t, aabmin, aabmax);

            btTransform localTransform;
            localTransform.setIdentity();

            /**ATTENTION: ONLY VALID WHEN THE OBJECT HAVE NO ROTATIONS IN X OR Z AXIS*/
            int radius = (aabmax.x() * 0.5 + aabmax.z() * 0.5) * 0.5 ;
            int height = aabmax.y() * 0.5;
            localTransform.setOrigin(btVector3(position.x(), position.y() + height, position.z()));
            /**ATTENTION: ONLY VALID WHEN THE OBJECT HAVE NO ROTATIONS IN X OR Z AXIS*/
//            int radius = (aabmax.y() * 0.5 + aabmax.z() * 0.5) * 0.5 ;
//            int height = aabmax.x();

            shape = new btCompoundShape();
            btCylinderShape *shapeCyl = new btCylinderShape(btVector3(radius,height,radius));
            ((btCompoundShape*)shape)->addChildShape(localTransform,shapeCyl);

            delete originalConvexShape;
//            delete shapeCyl;

        } else {
            this->shape = originalConvexShape;
        }
    } else {
        //3
        /**ATTENTION: We are using a static-triangle mesh this->shape,
        *  it can only be used for fixed/non-moving objects
        */
        float maxX=0, minX=0, maxY=0, minY=0, maxZ=0, minZ=0;
        int nVertMeshes = 0;
        int nIndxMeshes = 0;
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
            if (verticesLeft != 0 && nVertices > 0 && index_count == 0){
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
        int cFace = 0;

        //Reservamos espacio para el modelo que no tiene
        if (ourModel->triMeshPhis == NULL){
            cout << "Reservando espacio para las fisicas del modelo: " << this->tag << endl;
            ourModel->initTriMeshPhis(totalFaces);

            for (int idMesh=0; idMesh < ourModel->getMeshes()->size(); idMesh++){
                vector<Vertex> *vertices = ourModel->getMeshes()->at(idMesh)->getVertices();
                vector<GLuint> *indices  = ourModel->getMeshes()->at(idMesh)->getIndices();
                int index_count = indices->size();

                if (!indexed){
                    unsigned int nFaces = vertices->size()/3;
                    for (int i=0; i < nFaces; i++){
                        addPhysMeshTriangle(ourModel,
                                            ourModel->triMeshPhis[cFace],
                                            vertices->at(i*3), vertices->at(i*3+1),
                                            vertices->at(i*3+2));

                        cFace++;
                    }
                } else {
                    unsigned int nFaces = index_count/3;
                    for (int i=0; i < nFaces; i++){
                        addPhysMeshTriangle(ourModel,
                                            ourModel->triMeshPhis[cFace],
                                            vertices->at(indices->at(i*3)), vertices->at(indices->at(i*3+1)),
                                            vertices->at(indices->at(i*3+2)));
                        cFace++;
                    }
                }
            }
        }

        cout << "Added " << cFace << " faces"<< endl;
        cout << "mesh with " << ourModel->physMesh->getNumTriangles() << " triangles" << endl;
        if (ourModel->physMesh->getNumTriangles() > 0){
            this->shape = new btBvhTriangleMeshShape(ourModel->physMesh, true);
        } else {
            cout << "createShapeWithVertices: Shape without indices" << endl;
        }
    }

    if (this->shape != NULL){
        btTransform t;
        t.setIdentity();
        btVector3 aabb_min, aabb_max;
        this->shape->getAabb(t,aabb_min, aabb_max);
        cout << this->tag <<". Bounding box original. x=" << aabb_max.x() << ", y=" << aabb_max.y() << ", z=" << aabb_max.z() << "," << endl;
        scaling = scaleToMeters(dimension, aabb_max);
        this->shape->setLocalScaling(scaling);
        this->shape->getAabb(t,aabb_min, aabb_max);
        cout << this->tag <<". Bounding box scaled. x=" << aabb_max.x() << ", y=" << aabb_max.y() << ", z=" << aabb_max.z() << "," << endl;
    }
    return this->shape;
}

/**
* Genera un vector de escalado teniendo en cuenta los limites especificados.
* Solo puede especificarse un eje en el que escalar. El resto se escalara
* en proporcion al escalado de este
*/
btVector3 object3D::scaleToMeters(btVector3 &scaleMeters, btVector3 &aabb){
    float relation = 0.0f;

    if (scaleMeters.x() > 0.0f && aabb.x() != 0.0f)
        relation = scaleMeters.x() / aabb.x();

    if (scaleMeters.y() > 0.0f && aabb.y() != 0.0f)
        relation = scaleMeters.y() / aabb.y();

    if (scaleMeters.z() > 0.0f && aabb.z() != 0.0f)
        relation = scaleMeters.z() / aabb.z();

    return btVector3(relation, relation, relation);
}

/**
*
*/
void object3D::addPhysMeshTriangle(Model *ourModel, btVector3* triMeshPhis, Vertex &vec1, Vertex &vec2, Vertex &vec3){
    triMeshPhis = new btVector3[3];
    triMeshPhis[0].setX(btScalar(vec1.Position.x));
    triMeshPhis[0].setY(btScalar(vec1.Position.y));
    triMeshPhis[0].setZ(btScalar(vec1.Position.z));
    triMeshPhis[1].setX(btScalar(vec2.Position.x));
    triMeshPhis[1].setY(btScalar(vec2.Position.y));
    triMeshPhis[1].setZ(btScalar(vec2.Position.z));
    triMeshPhis[2].setX(btScalar(vec3.Position.x));
    triMeshPhis[2].setY(btScalar(vec3.Position.y));
    triMeshPhis[2].setZ(btScalar(vec3.Position.z));

    ourModel->physMesh->addTriangle(triMeshPhis[0],
                                    triMeshPhis[1],
                                    triMeshPhis[2], false); // false, don’t remove duplicate vertices

}

/**
*
*/
int SceneObjects::initShape(object3D *obj){
    btCollisionShape *newRigidShape = obj->createShapeWithVertices(obj->meshModel);
    physicsEngine->getCollisionShapes()->push_back(newRigidShape);

    //set the initial position and transform. For this demo, we set the tranform to be none
    btTransform startTransform;
    startTransform.setIdentity();

    btQuaternion quat = btQuaternion(obj->rotation.x, obj->rotation.y, obj->rotation.z, obj->rotation.w);
    startTransform.setOrigin(obj->position);
    startTransform.setRotation(quat);

    btVector3 localInertia(0,0,0);
    newRigidShape->calculateLocalInertia(obj->mass, localInertia);

    //actually contruct the body and add it to the dynamics world
    btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(obj->mass, myMotionState, newRigidShape, localInertia);
    btRigidBody *body = new btRigidBody(rbInfo);

    body->setCenterOfMassTransform(startTransform);
    body->setRollingFriction(obj->rollingFriction);
    body->setFriction(obj->friction);
    body->setRestitution(obj->restitution);
    body->setUserPointer(obj);

    //restricting movement in planes
    body->setLinearFactor(btVector3(1,1,1));
    //Para que el objecto este siempre recto
    body->setAngularFactor(btVector3(0,0,0));
    //Para que no se desactiven los elementos
    body->setActivationState(DISABLE_DEACTIVATION);
    //Incluimos el cuerpo en la libreria de fisica
    physicsEngine->getDynamicsWorld()->addRigidBody(body);
    //physicsEngine->trackRigidBodyWithName(body, physicsCubeName);
}

/**
*
*/
bool SceneObjects::getObjectModel(int i, glm::vec3 scale, glm::vec3 offset,  glm::mat4 &model){
    btCollisionObject* obj = getPhysics()->getDynamicsWorld()->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    bool out = false;
    if (body && body->getMotionState()){
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);

        object3D *userPointer = (object3D *)body->getUserPointer();
        if (userPointer) {
            btQuaternion orientation = trans.getRotation();
            glm::quat MyQuaternion   = glm::quat(orientation.getW(),orientation.getX(), orientation.getY(), orientation.getZ());
            glm::mat4 RotationMatrix = glm::toMat4(MyQuaternion);
            model = glm::translate(model, glm::vec3(trans.getOrigin().getX()
                    + offset.x, trans.getOrigin().getY() + offset.y, trans.getOrigin().getZ() + offset.z));
            model = glm::scale(model, scale);	// Para el piloto mesh
            model =  model * RotationMatrix;
            out = true;
        }
    }

    return out;
}

/**
*
*/
void SceneObjects::createModelStencil(int i){
    object3D *userPointer = getObjPointer(i);
    if (userPointer) {
        //Stencil processing
        if (this->stencil){
            if (userPointer->stencil){
                // 1st. Render pass, draw objects as normal, filling the stencil buffer
                glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should update the stencil buffer
                glStencilMask(0xFF); // Enable writing to the stencil buffer
            } else {
                glStencilMask(0x00); // Disable writing to the stencil buffer
            }

            if (userPointer->stencilDepthTest){
                glEnable(GL_DEPTH_TEST);
            }
        }
    }
}

/**
*
*/
bool SceneObjects::mustProcessStencil(int i, glm::mat4 &model, Shader &shader){
    object3D *userPointer = getObjPointer(i);
    bool ret = false;
    if (userPointer) {
        //Stencil processing
        if (this->stencil){
            if (userPointer->stencil){
                //Stencil
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilMask(0x00); // Disable writing to the stencil buffer
                glDisable(GL_DEPTH_TEST);
                shader.Use();
                model = glm::scale(model, glm::vec3(userPointer->stencilScale, userPointer->stencilScale, userPointer->stencilScale));
                glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                //Calculamos la inversa de la matriz por temas de iluminacion y rendimiento
                glm::mat4 transInversMatrix = transpose(inverse(model));
                glUniformMatrix4fv(glGetUniformLocation(shader.Program, "transInversMatrix"),
                                   1, GL_FALSE, glm::value_ptr(transInversMatrix));
                ret = true;
            }
            glStencilMask(0xFF);
        }
    }
    return ret;
}

/**
*
*/
void SceneObjects::cleanScreen(){
    GLbitfield opt = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    if (this->stencil) opt |= GL_STENCIL_BUFFER_BIT;

    glClear(opt);
}

