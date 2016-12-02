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
    // Tangents
    glm::vec3 Tangent;
    // Bittangents
    glm::vec3 Bittangent;

    Vertex(){};
    ~Vertex(){};
};

#endif // STRUCTS_H_INCLUDED
