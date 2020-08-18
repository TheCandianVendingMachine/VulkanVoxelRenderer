#include "graphics/sphere.hpp"
#include "graphics/indexBuffer.hpp"
#include "graphics/vertexBuffer.hpp"
#include <cmath>

void sphere::generateVertices(int resolution)
    {
        const float positionOffset = 1.f / static_cast<float>(resolution);
        vertex v;
        v.m_colour = { 0.47f, 0.82f, 0.11f };
        v.m_textureCoord = { 0.f, 0.f };
        v.m_position = { 0.f, -1.f, 0.f };
        m_vertices.push_back(v);

        // the direction which we offset for each vertex
        constexpr int windDirections[] = {
            0, 1,
            1, 0,
            0, -1,
            -1, 0
        };

        for (int row = 1; row <= resolution; row++)
            {
                v.m_position.y += positionOffset;

                v.m_position.z -= positionOffset;
                v.m_position.x -= positionOffset;

                int vertexCounter = 0;
                int index = 2;
                int xDirection = 0;
                int zDirection = 1;
                for (int j = 0; j < row * 4; j++)
                    {
                        m_vertices.push_back(v);
                        v.m_position.x += 2.f * positionOffset * xDirection;
                        v.m_position.z += 2.f * positionOffset * zDirection;

                        if (++vertexCounter == row)
                            {
                                xDirection = windDirections[index + 0];
                                zDirection = windDirections[index + 1];
                                index += 2;

                                vertexCounter = 0;
                            }
                    }
            }
    }

void sphere::generateIndices(int resolution)
    {
        int currentVertexUp = 1;
        int currentVertexDown = 1;
        for (int i = 1; i <= resolution; i++)
            {
                // v(i) = 2r(i + 1) + 1
                const int totalVertexCount = (2 * i) * (i + 1) + 1;
                // = v(i) - v(i - 1)
                const int totalVerticesToProcess = (4 * i);
                // = v(i - 1)
                const int wrapVertex = (2 * i) * (i - 1) + 1;
                // = v(i - 2)
                const int previousVertex = (i > 1) ? ((2 * i) * (i - 3) + 5) : 0;

                // add first index to list since it is always the non pattern conforming
                m_indices.push_back(previousVertex);
                m_indices.push_back(totalVertexCount - 1);
                m_indices.push_back(wrapVertex);

                // up triangles
                int vertexCounter = 0;
                int increment = 0;
                for (int j = 0; j < totalVerticesToProcess - 1; j++)
                    {
                        m_indices.push_back(previousVertex + increment);
                        m_indices.push_back(currentVertexUp + 0);
                        m_indices.push_back(currentVertexUp + 1);
                        currentVertexUp++;

                        // if we are on a corner we have 2 triangles under us so generate them
                        if (i != 1)
                            {
                                if (++vertexCounter >= i)
                                    {
                                        m_indices.push_back(previousVertex + increment);
                                        m_indices.push_back(currentVertexUp + 0);
                                        m_indices.push_back(currentVertexUp + 1);

                                        // We are adding another vertex so we want to increment all values pertaining to the +1 vertex
                                        // vertexCounter = 1 since this is a processed vertex
                                        j++;
                                        currentVertexUp++;
                                        vertexCounter = 1;
                                    }
                                increment++;
                            }
                    }
                currentVertexUp++;

                // down triangles
                if (i < 2) { continue; }
                m_indices.push_back(totalVertexCount - 1);
                m_indices.push_back(wrapVertex - 1);
                m_indices.push_back(previousVertex);

                vertexCounter = 1;
                for (int j = 1; j < totalVerticesToProcess - 1; j++)
                    {
                        // if we are not on a corner we have down-triangles associated
                        if ((vertexCounter % i) != 0)
                            {
                                m_indices.push_back(wrapVertex + vertexCounter);
                                m_indices.push_back(currentVertexDown + 0);
                                m_indices.push_back(currentVertexDown + 1);

                                // We are adding another vertex so we want to increment all values pertaining to the +1 vertex
                                // vertexCounter = 1 since this is a processed vertex
                                currentVertexDown++;
                            }
                        vertexCounter++;
                    }
                currentVertexDown++;
            }
    }

void sphere::create(int resolution)
    {
        if (resolution <= 0)
            {
                return;
            }
        const int totalVerticesForLastRow = resolution * 4;
        const int totalVertexCount = (2 * resolution) * (resolution + 1) + 1;

        generateVertices(resolution);
        std::vector<vertex> mirroredVertices(m_vertices.begin(), m_vertices.end() - totalVerticesForLastRow);
        for (auto &vertex : mirroredVertices)
            {
                vertex.m_position.y = -vertex.m_position.y;
            }
        m_vertices.insert(m_vertices.end(), mirroredVertices.begin(), mirroredVertices.end());

        generateIndices(resolution);

        std::vector<fe::index> mirroredIndices(m_indices);
        for (auto &index : mirroredIndices)
            {
                if (static_cast<int>(totalVertexCount - index) > totalVerticesForLastRow)
                    {
                        index += totalVertexCount;
                    }
            }

        m_indices.insert(m_indices.end(), mirroredIndices.begin(), mirroredIndices.end());

        // Ensure all points are at a fixed distance away from origin
        for (auto &vert : m_vertices)
            {
                float modifier = std::sqrt(1.f / (vert.m_position.x * vert.m_position.x + vert.m_position.y * vert.m_position.y + vert.m_position.z * vert.m_position.z));
                vert.m_position.x *= modifier;
                vert.m_position.y *= modifier;
                vert.m_position.z *= modifier;
            }
    }

void sphere::mapToBuffers(vertexBuffer &vbo, indexBuffer &ibo)
    {
        vbo.addVertices(m_vertices.data(), static_cast<size_t>(m_vertices.size()));
        ibo.addIndices(m_indices.data(), static_cast<size_t>(m_indices.size()));
    }
