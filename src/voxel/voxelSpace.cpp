#include "voxel/voxelSpace.hpp"
#include "graphics/vertex.hpp"
#include "graphics/quad.hpp"
#include "graphics/descriptorSet.hpp"
#include "typeDefines.hpp"
#include "PerlinNoise.hpp"
#include "random.hpp"
#include <vector>
#include <array>
#include <glm/gtx/quaternion.hpp>

void voxelSpace::buildGeometryAtPosition(voxelChunk &chunk, unsigned int x, unsigned int y, unsigned int z)
    {
        std::vector<quad> quads;
        test.meshAtPosition(quads, x, y, z);
        test.meshAtPosition(quads, x - 1, y, z);
        test.meshAtPosition(quads, x + 1, y, z);
        test.meshAtPosition(quads, x, y - 1, z);
        test.meshAtPosition(quads, x, y + 1, z);
        test.meshAtPosition(quads, x, y, z - 1);
        test.meshAtPosition(quads, x, y, z + 1);
    }

void voxelSpace::buildGeometry(voxelChunk &chunk)
    {
        std::vector<quad> quads;
        chunk.mesh(quads);

        buildGeometry(quads);
    }

void voxelSpace::buildGeometry(const std::vector<quad> &quads)
    {
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
                v0.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];
                v1.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];
                v2.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];
                v3.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];

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

voxelSpace::~voxelSpace()
    {
        destroy();
    }

void voxelSpace::create()
    {
    }

void voxelSpace::destroy()
    {
        m_vertexBuffer.destroy();
        m_indexBuffer.destroy();
    }

std::vector<float> noiseMapGenerator(int sizeX, int sizeY, int sizeZ, int seed, float scale, int octaves, float persistance, float lacunarity, glm::vec3 offset)
    {
        std::vector<float> noise(sizeX * sizeY * sizeZ);
        const siv::PerlinNoise noiseGenerator(seed);

        std::vector<glm::vec3> octaveOffsets(octaves);
        for (int i = 0; i < octaves; octaves++)
            {
                float offsetX = fe::random::get().generate(-100000.f, 100000.f) + offset.x;
                float offsetY = fe::random::get().generate(-100000.f, 100000.f) + offset.y;
                float offsetZ = fe::random::get().generate(-100000.f, 100000.f) + offset.z;

                octaveOffsets[i] = { offsetX, offsetY, offsetZ };
            }

        if (scale <= 0.f)
            {
                scale = 0.000000001f;
            }

        float maxNoiseHeight = std::numeric_limits<float>::min();
        float minNoiseHeight = std::numeric_limits<float>::max();

        float halfWidth = static_cast<float>(sizeX) / 2.f;
        float halfHeight = static_cast<float>(sizeY) / 2.f;
        float halfDepth = static_cast<float>(sizeZ) / 2.f;

        for (int y = 0; y < sizeY; y++)
            {
                for (int z = 0; z < sizeZ; z++)
                    {
                        for (int x = 0; x < sizeX; x++)
                            {
                                float amplitude = 1.f;
                                float frequency = 1.f;
                                float noiseHeight = 0.f;

                                for (int i = 0; i < octaves; i++)
                                    {
                                        float sampleX = (x - halfWidth) / scale * frequency + octaveOffsets[i].x;
                                        float sampleY = (y - halfHeight) / scale * frequency + octaveOffsets[i].y;
                                        float sampleZ = (z - halfDepth) / scale * frequency + octaveOffsets[i].z;

                                        float perlinValue = noiseGenerator.noise3D(sampleX, sampleY, sampleZ);
                                        noiseHeight += perlinValue * amplitude;

                                        amplitude += persistance;
                                        frequency += lacunarity;
                                    }

                                if (noiseHeight > maxNoiseHeight)
                                    {
                                        maxNoiseHeight = noiseHeight;
                                    }
                                if (noiseHeight < minNoiseHeight)
                                    {
                                        minNoiseHeight = noiseHeight;
                                    }

                                noise[x + sizeX * (y + sizeZ * z)] = noiseHeight;
                            }
                    }
            }

        for (int y = 0; y < sizeY; y++)
            {
                for (int z = 0; z < sizeZ; z++)
                    {
                        for (int x = 0; x < sizeX; x++)
                            {
                                float currentNoise = noise[x + sizeX * (y + sizeZ * z)];
                                // inverse lerp: (v - a) / (a - b)
                                currentNoise = (currentNoise - minNoiseHeight) / (maxNoiseHeight - minNoiseHeight);

                                noise[x + sizeX * (y + sizeZ * z)] = currentNoise;
                            }
                    }
            }

        return noise;
    }

void voxelSpace::createWorld()
    {
        double globalFrequency = 0.85;
        std::array<std::array<double, 2>, 2> surfaceFrequencies = {{
            {{ 1.0, 1.5 }},
            {{ 0.2, 4.0 }},
        }};

        const siv::PerlinNoise noiseSurface(fe::random::get().generate<uint32_t>());
        test.create(48, 48, 48);

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

        m_translation = glm::translate(glm::mat4(1.f), glm::vec3{0.f, 0.f, 0.f});
        m_quaternion = glm::angleAxis(0.f, glm::normalize(glm::vec3{ 0, 1.f, 0.f }));
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

glm::mat4 voxelSpace::getModelTransformation() const
    {
        return m_translation * glm::toMat4(m_quaternion);
    }

glm::vec<3, int> voxelSpace::raycast(const glm::vec3 origin, const glm::vec3 direction)
    {
        constexpr float gridOffset = 0.000001f;

        glm::vec3 worldPosition = getModelTransformation() * glm::vec4(origin, 0.f);
        glm::vec<3, int> gridPosition = worldPosition / test.getVoxelSize();

        glm::vec3 deltaDistance = glm::abs(1.f / direction);
        glm::vec3 sideDistance;
        glm::vec<3, int> step;

        if (direction.x < 0.f)
            {
                step.x = -1;
                sideDistance.x = (worldPosition.x - gridPosition.x) * deltaDistance.x;
            }
        else
            {
                step.x = 1;
                sideDistance.x = (gridPosition.x + gridOffset - worldPosition.x) * deltaDistance.x;
            }

        if (direction.y < 0.f)
            {
                step.y = -1;
                sideDistance.y = (worldPosition.y - gridPosition.y) * deltaDistance.y;
            }
        else
            {
                step.y = 1;
                sideDistance.y = (gridPosition.y + gridOffset - worldPosition.y) * deltaDistance.y;
            }

        if (direction.z < 0.f)
            {
                step.z = -1;
                sideDistance.z = (worldPosition.z - gridPosition.z) * deltaDistance.z;
            }
        else
            {
                step.z = 1;
                sideDistance.z = (gridPosition.z + gridOffset - worldPosition.z) * deltaDistance.z;
            }

        int stepCount = 0;
        const int maxStepCount = test.getSizeX() + test.getSizeY() + test.getSizeZ();
        while (stepCount++ <= maxStepCount)
            {
                if (sideDistance.x < sideDistance.z)
                    {
                        if (sideDistance.y < sideDistance.x)
                            {
                                sideDistance.y += deltaDistance.y;
                                gridPosition.y += step.y;
                            }
                        else
                            {
                                sideDistance.x += deltaDistance.x;
                                gridPosition.x += step.x;
                            }
                    }
                else
                    {
                        if (sideDistance.y < sideDistance.z)
                            {
                                sideDistance.y += deltaDistance.y;
                                gridPosition.y += step.y;
                            }
                        else
                            {
                                sideDistance.z += deltaDistance.z;
                                gridPosition.z += step.z;
                            }
                    }

                if (test.withinBounds(gridPosition.x, gridPosition.y, gridPosition.z))
                    {
                        voxelType type = test.at(gridPosition.x, gridPosition.y, gridPosition.z);
                        if (type != voxelType::NONE)
                            {
                                test.at(gridPosition.x, gridPosition.y, gridPosition.z) = voxelType::TEST_1;
                                buildGeometryAtPosition(test, gridPosition.x, gridPosition.y, gridPosition.z);

                                return gridPosition;
                            }
                    }
            }

        return {};
    }
