#include "random.hpp"
#include "typeDefines.hpp"

fe::random *fe::random::m_instance = nullptr;

void fe::random::startUp()
    {
        if (!m_instance)
            {
                m_instance = this;
                randomSeed();
            }
    }

void fe::random::randomSeed()
    {
        pcg_extras::seed_seq_from<std::random_device> seedSource;
        m_rng = pcg32(seedSource);
    }

void fe::random::seed(unsigned int seed)
    {
        m_rng.seed(seed);
    }

fe::random &fe::random::get()
    {
        return *m_instance;
    }
