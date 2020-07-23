// sphere.hpp
// A procedural sphere shape with alright control over vertex count
// The sphere is created by first creating an octahedron through triangles subdivided along the edges as vertices
// The vertices are then transformed to be distance 1 away from the centre
#pragma once
#include <vector>
#include "graphics/vertex.hpp"
#include "typeDefines.hpp"

class vertexBuffer;
class indexBuffer;
class sphere
    {
        private:
            std::vector<vertex> m_vertices;
            std::vector<fe::index> m_indices;

            void generateVertices(int resolution);
            void generateIndices(int resolution);

        public:
            void create(int resolution);
            void mapToBuffers(vertexBuffer &vbo, indexBuffer &ibo);
    };
