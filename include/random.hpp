// random.hpp
// allows for easy randomization of items
#pragma once
#include "pcg_random.hpp"
#include <random>
#include <limits>

namespace fe
    {
        class random
            {
                private:
                    static random *m_instance;
                    pcg32 m_rng;

                public:
                    void startUp();

                    void randomSeed();
                    void seed(unsigned int seed = 0);

                    static random &get();
                    template<typename T>
                    T generate(T min, T max);
                    template<typename T>
                    T generate();

            };

        template<typename T>
        inline T random::generate(T min, T max)
            {
                if constexpr (std::is_floating_point<T>::value) 
                    {
                        std::uniform_real_distribution<T> dist(min, max);
                        return dist(m_rng);
                    }
                else if constexpr(std::numeric_limits<T>::is_integer)
                    {
                        std::uniform_int_distribution<T> dist(min, max);
                        return dist(m_rng);
                    }

                return T();
            }
        template<typename T>
        inline T random::generate()
            {
                return generate(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
            }
    }