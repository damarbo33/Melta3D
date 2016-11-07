#ifndef LIGTH_H
#define LIGTH_H

enum lightType {
    AMBIENTLIGHT,
    POINTLIGHT,
    SPOTLIGHT,
    MAXLIGHTTYPE
};

class Light{
    public :
        GLint locAmbient;
        GLint locDiffuse;
        GLint locSpecular;
        GLint locPosition;
        GLint locDirection;

        glm::vec3 vAmbient;
        glm::vec3 vDiffuse;
        glm::vec3 vSpecular;
        glm::vec3 vPosition;
        glm::vec3 vDirection;

        int lightType;

        Light(){
           lightType = AMBIENTLIGHT;
        }

};

class Pointlight : public Light{
    public :
        GLint locConstant;
        GLint locLinear;
        GLint locQuadratic;

        GLfloat vConstant;
        GLfloat vLinear;
        GLfloat vQuadratic;

        Pointlight(){
           lightType = POINTLIGHT;
        }
};

class SpotLight : public Pointlight{
    public :
        GLint locCutOff;
        GLint locOuterCutOff;

        GLfloat vCutOff;
        GLfloat vOuterCutOff;

        SpotLight(){
           lightType = SPOTLIGHT;
        }
};


#endif // LIGTH_H
