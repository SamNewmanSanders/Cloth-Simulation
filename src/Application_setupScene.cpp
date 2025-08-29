#include "Application.h"
#include "Cloth.h"
#include <vector>

#include <random>  // for random number generation

void Application::setupScene()
{
    // Test Cube (unchanged)
    std::vector<float> cubeVerts(cubeVertices, cubeVertices + sizeof(cubeVertices)/sizeof(float));
    std::vector<unsigned int> cubeInds(cubeIndices, cubeIndices + sizeof(cubeIndices)/sizeof(unsigned int));
    auto cubeMesh = std::make_shared<Mesh>(cubeVerts, cubeInds);
    auto cube = std::make_shared<Entity>(cubeMesh);
    cube->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    cube->getMaterial().diffuse = glm::vec3(0.5f, 0.1f, 0.8f);
    entities.push_back(cube);

    // Cloth
    cloth = std::make_shared<Cloth>(60, 60, 0.05f);
    cloth->setPosition(glm::vec3(0.0f, 1.5f, 0.0f));
    cloth->getMaterial().diffuse = glm::vec3(1.0f, 0.2f, 0.2f);
    cloth->getMaterial().specular = glm::vec3(0.3f, 0.3f, 0.3f);
    cloth->getMaterial().shininess = 16.0f; 
}