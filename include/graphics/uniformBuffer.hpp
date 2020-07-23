// uniformBuffer.hpp
// A buffer to allow transfer of data to GPU shaders
#pragma once
#include "graphics/vulkan/vulkanBuffer.hpp"

class vulkanDevice;
class vulkanCommandBuffer;
class uniformBuffer
    {
        private:
            vulkanBuffer m_uniformBuffer;

            unsigned int m_size = 0;
            bool m_dynamic = false;

        public:
            uniformBuffer() = default;
            uniformBuffer(unsigned int size);
            template<typename T>
            uniformBuffer(T *value);
            template<typename T>
            uniformBuffer(T &value);
            ~uniformBuffer();
            void create(unsigned int size);
            template<typename T>
            void create(T *value);
            template<typename T>
            void create(T &value);
            void destroy();
            
            template<typename T>
            void bind(T &data);
            void bind(void *data);

            vulkanBuffer &getUniformBuffer();
            const vulkanBuffer &getUniformBuffer() const;

            unsigned int getBufferSize() const;
    };

template<typename T>
inline uniformBuffer::uniformBuffer(T *value)
    {
        create(sizeof(T));
        bind(value);
    }

template<typename T>
inline uniformBuffer::uniformBuffer(T &value)
    {
        create(sizeof(T));
        bind(&value);
    }

template<typename T>
inline void uniformBuffer::create(T *value)
    {
        create(sizeof(T));
        bind(value);
    }

template<typename T>
inline void uniformBuffer::create(T &value)
    {
        create(sizeof(T));
        bind(&value);
    }

template<typename T>
inline void uniformBuffer::bind(T &data)
    {
        bind(&data);
    }
