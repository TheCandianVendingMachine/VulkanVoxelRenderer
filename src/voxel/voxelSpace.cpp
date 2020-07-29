#include "voxel/voxelSpace.hpp"
#include "graphics/vertex.hpp"
#include "graphics/quad.hpp"
#include "typeDefines.hpp"
#include "PerlinNoise.hpp"
#include "random.hpp"
#include <vector>
#include <array>

void voxelSpace::buildGeometry(voxelChunk &chunk)
    {
        std::vector<quad> quads;
        chunk.mesh(quads);

        vertex vertexArr[4] = {};
        vertex &v0 = vertexArr[0];
        vertex &v1 = vertexArr[1];
        vertex &v2 = vertexArr[2];
        vertex &v3 = vertexArr[3];

        fe::index indexArrPositive[6] = {
            0, 1, 2, 0, 2, 3
        };
        fe::index indexArrNegative[6] = {
            2, 1, 0, 3, 2, 0
        };

        glm::vec3 colours[] = {
            { 0.23f, 0.48f, 0.34f },
            { 0.53f, 0.81f, 0.92f },
            { 0.37f, 0.50f, 0.22f }
        };

        for (auto &quad : quads)
            {
                v0.m_colour = colours[std::abs(quad.m_orientation) - 1];
                v1.m_colour = colours[std::abs(quad.m_orientation) - 1];
                v2.m_colour = colours[std::abs(quad.m_orientation) - 1];
                v3.m_colour = colours[std::abs(quad.m_orientation) - 1];

                switch (std::abs(quad.m_orientation))
                    {
                        case 1:
                            v0.m_position.x = quad.m_position.x;
                            v0.m_position.y = quad.m_position.y;
                            v0.m_position.z = quad.m_position.z;

                            v1.m_position.x = quad.m_position.x + quad.m_size.x;
                            v1.m_position.y = quad.m_position.y;
                            v1.m_position.z = quad.m_position.z;

                            v2.m_position.x = quad.m_position.x + quad.m_size.x;
                            v2.m_position.y = quad.m_position.y + quad.m_size.y;
                            v2.m_position.z = quad.m_position.z;

                            v3.m_position.x = quad.m_position.x;
                            v3.m_position.y = quad.m_position.y + quad.m_size.y;
                            v3.m_position.z = quad.m_position.z;
                            break;
                        case 2:
                            v0.m_position.x = quad.m_position.x;
                            v0.m_position.y = quad.m_position.z;
                            v0.m_position.z = quad.m_position.y;

                            v1.m_position.x = quad.m_position.x + quad.m_size.x;
                            v1.m_position.y = quad.m_position.z;
                            v1.m_position.z = quad.m_position.y;

                            v2.m_position.x = quad.m_position.x + quad.m_size.x;
                            v2.m_position.y = quad.m_position.z;
                            v2.m_position.z = quad.m_position.y + quad.m_size.y;

                            v3.m_position.x = quad.m_position.x;
                            v3.m_position.y = quad.m_position.z;
                            v3.m_position.z = quad.m_position.y + quad.m_size.y;
                            break;
                        case 3:
                            v0.m_position.x = quad.m_position.z;
                            v0.m_position.y = quad.m_position.x;
                            v0.m_position.z = quad.m_position.y;

                            v1.m_position.x = quad.m_position.z;
                            v1.m_position.y = quad.m_position.x + quad.m_size.x;
                            v1.m_position.z = quad.m_position.y;

                            v2.m_position.x = quad.m_position.z;
                            v2.m_position.y = quad.m_position.x + quad.m_size.x;
                            v2.m_position.z = quad.m_position.y + quad.m_size.y;

                            v3.m_position.x = quad.m_position.z;
                            v3.m_position.y = quad.m_position.x;
                            v3.m_position.z = quad.m_position.y + quad.m_size.y;
                            break;
                        default:
                            break;
                    }

                m_vertexBuffer.addVertices(vertexArr, 4);
                if (quad.m_orientation < 0)
                    {
                        m_indexBuffer.addIndices(indexArrNegative, 6);
                    }
                else 
                    {
                        m_indexBuffer.addIndices(indexArrPositive, 6);
                    }
                
                indexArrPositive[0] += 4;
                indexArrPositive[1] += 4;
                indexArrPositive[2] += 4;
                indexArrPositive[3] += 4;
                indexArrPositive[4] += 4;
                indexArrPositive[5] += 4;

                indexArrNegative[0] += 4;
                indexArrNegative[1] += 4;
                indexArrNegative[2] += 4;
                indexArrNegative[3] += 4;
                indexArrNegative[4] += 4;
                indexArrNegative[5] += 4;
            }
    }

voxelSpace::voxelSpace()
    {   
        double globalFrequency = 0.85;
        std::array<std::array<double, 2>, 3> surfaceFrequencies = {{
            {{ 1.0, 1.0 }},
            {{ 0.2, 4.0 }},
        }};

        const siv::PerlinNoise noiseSurface(fe::random::get().generate<uint32_t>());
        test.create(64, 64, 64);

        for (int x = 0; x < test.getSizeX(); x++)
            {
                double nx = (static_cast<double>(x) / test.getSizeX());
                for (int y = 0; y < test.getSizeY(); y++)
                    {
                        double ny = (static_cast<double>(y) / test.getSizeY());
                        for (int z = 0; z < test.getSizeZ(); z++)
                            {
                                double nz = (static_cast<double>(z) / test.getSizeZ());
                                double surfaceNoise = 0.0;
                                for (const auto &frequency : surfaceFrequencies)
                                    {
                                        surfaceNoise += globalFrequency * frequency[0] * noiseSurface.noise3D_0_1(frequency[1] * nx, frequency[1] * ny, frequency[1] * nz);
                                    }

                                voxelType type = voxelType::NONE;
                                if (surfaceNoise < 0.5)
                                    {
                                        type = voxelType::DEFAULT;
                                    }
                                test.at(x, y, z) = type;
                            }
                    }
            }

        buildGeometry(test);
    }

voxelSpace::~voxelSpace()
    {
        destroy();
    }

void voxelSpace::destroy()
    {
        m_vertexBuffer.destroy();
        m_indexBuffer.destroy();
    }

const vertexBuffer &voxelSpace::getVertexBuffer() const
    {
        return m_vertexBuffer;
    }

vertexBuffer &voxelSpace::getVertexBuffer()
    {
        return m_vertexBuffer;
    }

const indexBuffer &voxelSpace::getIndexBuffer() const
    {
        return m_indexBuffer;
    }

indexBuffer &voxelSpace::getIndexBuffer()
    {
        return m_indexBuffer;
    }
