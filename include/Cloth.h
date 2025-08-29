#pragma once
#include "Entity.h"
#include "Rendering/Mesh.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>


class Cloth : public Entity {
public:
    Cloth(int width, int height, float spacing);

    // Called every frame from your main loop
    void update(float deltaTime);

    // Recompute per-vertex normals after deformation
    void recomputeNormals();

private:
    // These are dynamically updated as the mesh changes so can't just put in a mesh constructor
    std::vector<float>  vertexData;
    std::vector<unsigned int> indexData;

    // Helpers
    std::vector<float> generateVertices(int width, int height, float spacing);
    std::vector<unsigned int> generateIndices(int width, int height);

    void updateMeshVertices(); // As the mesh dynamically changes

    // Cloth layout
    int   clothWidth, clothHeight;
    float spacing;

    // Simulation state (CPU)
    std::vector<glm::vec3> positions;      // one per vertex
    std::vector<glm::vec3> velocities;     // one per vertex
    std::vector<glm::vec3> forces;         // one per vertex


};
