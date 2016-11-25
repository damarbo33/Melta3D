#ifndef MYDEBUG_H
#define MYDEBUG_H

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

#include "btBulletDynamicsCommon.h"

using namespace std;

class CibtDebugDraw : public btIDebugDraw
{
protected:
    int m_debugMode;

public:
    CibtDebugDraw();
    virtual ~CibtDebugDraw();
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
    virtual void drawSphere(const btVector3& p, btScalar radius, const btVector3& color);
    virtual void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color);
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
    virtual void reportErrorWarning(const char* warningString);
    virtual void draw3dText(const btVector3& location, const char* textString);
    virtual void setDebugMode(int debugMode);
    virtual int getDebugMode() const;
};

#endif // MYDEBUG_H
