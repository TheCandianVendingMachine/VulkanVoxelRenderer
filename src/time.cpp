#include "time.hpp"
#include <utility>

fe::time::time(const fe::time &time) noexcept
    {
        m_time = time.m_time;
    }

fe::time::time(const fe::time &&time) noexcept
    {
        m_time = std::forward<const long long>(time.m_time);
    }

double fe::time::asSeconds() const
    {
        // 0.000001 = 1e-6
        return static_cast<double>(m_time) / 1000000;
    }

std::int64_t fe::time::asMilliseconds() const
    {
        return m_time / 1000;
    }

std::int64_t fe::time::asMicroseconds() const
    {
        return m_time;
    }

fe::time fe::time::operator+(const fe::time &rhs) const
    {
        return fe::time(fe::microseconds(static_cast<long double>(m_time + rhs.m_time)));
    }

fe::time fe::time::operator-(const fe::time &rhs) const
    {
        return fe::time(fe::microseconds(static_cast<long double>(m_time - rhs.m_time)));
    }

void fe::time::operator+=(const fe::time &rhs)
    {
        m_time += rhs.m_time;
    }

void fe::time::operator-=(const fe::time &rhs)
    {
        m_time -= rhs.m_time;
    }

fe::time &fe::time::operator=(const fe::time &rhs)
    {
        if (this != &rhs)
            {
                m_time = rhs.m_time;
            }
        return *this;
    }

bool fe::time::operator>(const fe::time &rhs)
    {
        return m_time > rhs.m_time;
    }

bool fe::time::operator>=(const fe::time &rhs)
    {
        return m_time >= rhs.m_time;
    }

bool fe::time::operator<(const fe::time &rhs)
    {
        return m_time < rhs.m_time;
    }

bool fe::time::operator<=(const fe::time &rhs)
    {
        return m_time <= rhs.m_time;
    }

bool fe::time::operator==(const fe::time &rhs)
    {
        return m_time == rhs.m_time;
    }


