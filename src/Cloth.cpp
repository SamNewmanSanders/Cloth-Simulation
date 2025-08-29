// Cloth.cpp
#include "Cloth.h"
#include "Rendering/Mesh.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>

// ---------------------- Helper Mesh Generation ----------------------

// Interleaved: [x,y,z, nx,ny,nz]
std::vector<float> Cloth::generateVertices(int width, int height, float spacing) {
    std::vector<float> vertices;
    vertices.reserve(width * height * 6);

    float halfWidth  = (width - 1) * spacing * 0.5f;
    float halfHeight = (height - 1) * spacing * 0.5f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float xpos = x * spacing - halfWidth;
            float ypos = 0.0f;
            float zpos = y * spacing - halfHeight;

            // Position
            vertices.push_back(xpos);
            vertices.push_back(ypos);
            vertices.push_back(zpos);

            // Normal
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }
    }
    return vertices;
}

std::vector<unsigned int> Cloth::generateIndices(int width, int height) {
    std::vector<unsigned int> indices;
    indices.reserve((width - 1) * (height - 1) * 6);

    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            int topLeft     = y * width + x;
            int topRight    = topLeft + 1;
            int bottomLeft  = (y + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            // tri 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // tri 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    return indices;
}

// ---------------------- Cloth Class ----------------------

Cloth::Cloth(int width, int height, float spacing)
    : clothWidth(width), clothHeight(height), spacing(spacing),
        Entity(std::make_shared<Mesh>(generateVertices(width, height, spacing),
                                        generateIndices(width, height))) // Give base Entity the mesh immediately
{
    // Build CPU-side geometry
    vertexData = generateVertices(width, height, spacing);
    indexData  = generateIndices(width, height);

    // Init simulation arrays
    const int numVerts = width * height;
    positions.resize(numVerts);
    velocities.assign(numVerts, glm::vec3(0.0f));
    forces.assign(numVerts, glm::vec3(0.0f));

    // Extract positions from interleaved vertexData
    for (int i = 0; i < numVerts; ++i) {
        positions[i] = glm::vec3(
            vertexData[i * 6 + 0],
            vertexData[i * 6 + 1],
            vertexData[i * 6 + 2]
        );
    }
}

// ---------------------- Physics / Update ----------------------

void Cloth::update(float deltaTime) {
    // --- config ---
    const glm::vec3 gravity(0.0f, -0.1f, 0.0f);
    const float damping = 0.98f;
    const int width  = clothWidth;
    const int height = clothHeight;
    const float restLength = spacing;          // desired rest distance between structural neighbors
    const int constraintIterations = 30;       // tune (higher => stiffer, slower)
    const float maxCorrection = restLength * 0.5f; // clamp per-correction magnitude
    const float eps = 0.001f;                  // little offset to keep vertices outside the cube

    glm::vec3 worldOffset = getPosition();     // entity/world offset (cloth centre)

    // --- integrate velocities ---
    for (size_t i = 0; i < positions.size(); ++i) {
        velocities[i] += gravity * deltaTime;
        velocities[i] *= damping;
        positions[i] += velocities[i] * deltaTime;
    }

    // --- prepare cube (1.1 x 1.1 x 1.1 centered at origin) ---
    const glm::vec3 cubeMin(-0.55f);
    const glm::vec3 cubeMax( 0.55f);

    // --- constraint loop (structural) + collision enforcement ---
    for (int iter = 0; iter < constraintIterations; ++iter) {
        // structural constraints
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;

                // Right neighbor
                if (x < width - 1) {
                    int j = idx + 1;
                    glm::vec3 delta = positions[j] - positions[idx];
                    float d = glm::length(delta);
                    if (d > 1e-7f) {
                        glm::vec3 corr = delta * 0.5f * ((d - restLength) / d);
                        if (glm::length(corr) > maxCorrection) corr = glm::normalize(corr) * maxCorrection;
                        positions[idx] += corr;
                        positions[j]   -= corr;
                    }
                }

                // Down neighbor
                if (y < height - 1) {
                    int j = idx + width;
                    glm::vec3 delta = positions[j] - positions[idx];
                    float d = glm::length(delta);
                    if (d > 1e-7f) {
                        glm::vec3 corr = delta * 0.5f * ((d - restLength) / d);
                        if (glm::length(corr) > maxCorrection) corr = glm::normalize(corr) * maxCorrection;
                        positions[idx] += corr;
                        positions[j]   -= corr;
                    }
                }
            }
        }

        // collision enforcement
        for (size_t i = 0; i < positions.size(); ++i) {
            glm::vec3 worldPos = positions[i] + worldOffset;

            if (worldPos.x > cubeMin.x && worldPos.x < cubeMax.x &&
                worldPos.y > cubeMin.y && worldPos.y < cubeMax.y &&
                worldPos.z > cubeMin.z && worldPos.z < cubeMax.z)
            {
                float distLeft  = worldPos.x - cubeMin.x;
                float distRight = cubeMax.x - worldPos.x;
                float distBottom= worldPos.y - cubeMin.y;
                float distTop   = cubeMax.y - worldPos.y;
                float distBack  = worldPos.z - cubeMin.z;
                float distFront = cubeMax.z - worldPos.z;

                float minDist = distLeft;
                int face = 0;
                if (distRight < minDist) { minDist = distRight; face = 1; }
                if (distBottom < minDist){ minDist = distBottom; face = 2; }
                if (distTop < minDist)   { minDist = distTop; face = 3; }
                if (distBack < minDist)  { minDist = distBack; face = 4; }
                if (distFront < minDist) { minDist = distFront; face = 5; }

                glm::vec3 normal(0.0f);
                glm::vec3 targetWorld = worldPos;
                switch (face) {
                    case 0: normal = glm::vec3(-1,0,0); targetWorld.x = cubeMin.x - eps; break;
                    case 1: normal = glm::vec3( 1,0,0); targetWorld.x = cubeMax.x + eps; break;
                    case 2: normal = glm::vec3(0,-1,0); targetWorld.y = cubeMin.y - eps; break;
                    case 3: normal = glm::vec3(0, 1,0); targetWorld.y = cubeMax.y + eps; break;
                    case 4: normal = glm::vec3(0,0,-1); targetWorld.z = cubeMin.z - eps; break;
                    case 5: normal = glm::vec3(0,0, 1); targetWorld.z = cubeMax.z + eps; break;
                }

                positions[i] = targetWorld - worldOffset;

                float vn = glm::dot(velocities[i], normal);
                if (vn < 0.0f)
                    velocities[i] -= vn * normal;
            }
        }
    }

    // --- update vertex buffer ---
    for (size_t i = 0; i < positions.size(); ++i) {
        vertexData[i * 6 + 0] = positions[i].x;
        vertexData[i * 6 + 1] = positions[i].y;
        vertexData[i * 6 + 2] = positions[i].z;
    }

    recomputeNormals();
    updateMeshVertices();
}

// ---------------------- Normal Computation ----------------------

void Cloth::recomputeNormals() {
    // zero normals
    for (size_t i = 0; i < vertexData.size() / 6; ++i) {
        vertexData[i * 6 + 3] = 0.0f;
        vertexData[i * 6 + 4] = 0.0f;
        vertexData[i * 6 + 5] = 0.0f;
    }

    // accumulate face normals
    for (size_t i = 0; i < indexData.size(); i += 3) {
        unsigned int i0 = indexData[i + 0];
        unsigned int i1 = indexData[i + 1];
        unsigned int i2 = indexData[i + 2];

        glm::vec3 v0(
            vertexData[i0 * 6 + 0],
            vertexData[i0 * 6 + 1],
            vertexData[i0 * 6 + 2]
        );
        glm::vec3 v1(
            vertexData[i1 * 6 + 0],
            vertexData[i1 * 6 + 1],
            vertexData[i1 * 6 + 2]
        );
        glm::vec3 v2(
            vertexData[i2 * 6 + 0],
            vertexData[i2 * 6 + 1],
            vertexData[i2 * 6 + 2]
        );

        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        vertexData[i0 * 6 + 3] += n.x;
        vertexData[i0 * 6 + 4] += n.y;
        vertexData[i0 * 6 + 5] += n.z;

        vertexData[i1 * 6 + 3] += n.x;
        vertexData[i1 * 6 + 4] += n.y;
        vertexData[i1 * 6 + 5] += n.z;

        vertexData[i2 * 6 + 3] += n.x;
        vertexData[i2 * 6 + 4] += n.y;
        vertexData[i2 * 6 + 5] += n.z;
    }

    // normalize vertex normals
    for (size_t i = 0; i < vertexData.size() / 6; ++i) {
        glm::vec3 n(
            vertexData[i * 6 + 3],
            vertexData[i * 6 + 4],
            vertexData[i * 6 + 5]
        );
        n = glm::normalize(n);
        vertexData[i * 6 + 3] = n.x;
        vertexData[i * 6 + 4] = n.y;
        vertexData[i * 6 + 5] = n.z;
    }
}

// ---------------------- GPU Upload ----------------------

void Cloth::updateMeshVertices() {
    // Push CPU-side interleaved vertexData into the Mesh's VBO
    getMesh()->updateVertexBuffer(vertexData);
}
