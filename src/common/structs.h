#ifndef STRUCTS_H_INCLUDED
#define STRUCTS_H_INCLUDED

#include <string>
using namespace std;

struct Vertex {
    // Position
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // TexCoords
    glm::vec2 TexCoords;

    Vertex(){};
    ~Vertex(){};
};

#endif // STRUCTS_H_INCLUDED
