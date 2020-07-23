#define _CRT_SECURE_NO_WARNINGS
#include "clock.hpp"
#include <chrono>
#include <iomanip>

fe::clock::clock()
    {
        restart();
        m_stopped = false;
    }

void fe::clock::restart()
    {
        m_startTime = getTimeSinceEpoch();
    }

void fe::clock::stop(bool value)
    {
        fe::time currentTime = getTimeSinceEpoch();
        m_stopped = value;

        // if the clock is restarted we have to subtract the time the clock has been non-active from start_time
        if (!m_stopped)
            {
                m_offsetTime += m_stopTime - m_startTime;
            }

        m_stopTime = currentTime;
    }

fe::time fe::clock::getTime() const
    {
        if (!m_stopped)
            {
                return getTimeSinceEpoch() - m_startTime - m_offsetTime;
            }

        return m_stopTime - m_startTime;
    }

fe::time fe::clock::getTimeSinceEpoch()
    {
        auto currentTime = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now());
        return fe::microseconds(static_cast<long double>(currentTime.time_since_epoch().count()));
    }

const char *fe::clock::getFormattedTime(const char *format)
    {
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto timeInfo = std::localtime(&now);

        static char lastTimeAsked[500];
        strftime(lastTimeAsked, 25, format, timeInfo);

        return lastTimeAsked;
    }
