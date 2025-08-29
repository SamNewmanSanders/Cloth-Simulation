#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <memory>

#include "Rendering/Wrappers/Shader.h"
#include "Rendering/Lighting/Light.h"
#include "Entity.h"



class Renderer {
public:
    Renderer(std::shared_ptr<Shader> shader, GLFWwindow* window);

    void drawEntity(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos, std::shared_ptr<Entity> entity);

    void beginFrame();
    void endFrame();

private:
    std::shared_ptr<Shader> shaderProgram;
    GLFWwindow* window;
    Light lightSource;
};

