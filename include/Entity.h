#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Rendering/Mesh.h"
#include "Rendering/Lighting/Material.h"


class Entity {
public:
    Entity(std::shared_ptr<Mesh> mesh);

    void setPosition(const glm::vec3& pos);
    const glm::vec3& getPosition() const { return position; }
    void setRotation(const glm::vec3& rot);  // or use quaternion
    void setScale(const glm::vec3& scale);

    glm::mat4 getModelMatrix() const;

    std::shared_ptr<Mesh> getMesh() const;

    glm::vec3 getPosition() { return position; } 

    Material& getMaterial() { return material; } // Different things call this differently so provide two functions
    const Material& getMaterial() const { return material; }


private:
    std::shared_ptr<Mesh> mesh;
    Material material; 

    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
};