#include "SimpleBox.h"


class SimpleBoxExample : public CommonRigidBodyBase {
   public:
        SimpleBoxExample(struct GUIHelperInterface* helper):CommonRigidBodyBase(helper) {}
        virtual ~SimpleBoxExample(){}
        virtual void initPhysics();
        virtual void renderScene();
        void resetCamera() {
          float dist = 41;
          float pitch = 52;
          float yaw = 35;
          float targetPos[3]={0,0.46,0};
          m_guiHelper->resetCamera(dist,pitch,yaw,targetPos[0],targetPos[1],targetPos[2]);
        }
};

void SimpleBoxExample::renderScene()
{
   CommonRigidBodyBase::renderScene();
}

void SimpleBoxExample::initPhysics()
{
    m_guiHelper->setUpAxis(1);
    createEmptyDynamicsWorld();
    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
    if (m_dynamicsWorld->getDebugDrawer())
        m_dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe+btIDebugDraw::DBG_DrawContactPoints);

    btBoxShape* groundShape = createBoxShape(btVector3(btScalar(50.),btScalar(50.),btScalar(50.)));
    m_collisionShapes.push_back(groundShape);
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0,-50,0));

}

//CommonExampleInterface* ET_SimpleBoxCreateFunc(CommonExampleOptions& options)
//{
//   return new SimpleBoxExample(options.m_guiHelper);
//}

