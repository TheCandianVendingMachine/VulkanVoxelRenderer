// typeDefines.hpp
// Common engine defines
#pragma once

namespace sf
    {
        class RenderWindow;
    }

constexpr long double operator "" _meters(long double pixels)
	{
		return pixels * 15.0;
	}

constexpr long double operator "" _meters(unsigned long long int pixels)
	{
		return pixels * 15.0;
	}

constexpr long double operator "" _pixels(long double meters)
	{
		return meters / 15.0;
	}

constexpr long double operator "" _pixels(unsigned long long int meters)
	{
		return meters / 15.0;
	}

namespace fe
	{
		using Handle = int;
        using str = unsigned long long;

        using int8 = signed char;
        using uInt8 = unsigned char;

        using int16 = signed short;
        using uInt16 = unsigned short;

        using int32 = signed int;
        using uInt32 = unsigned int;

        #if defined(_MSC_VER)
            using int64 = signed __int64;
            using uInt64 = unsigned __int64;
        #else
            using int64 = signed long long;
            using uInt64 = unsigned long long;
        #endif

        #if defined(_WIN64)
            using uIntPtr = uInt64;
            using intPtr = int64;
        #else
            using uIntPtr = uInt32;
            using intPtr = int32;
        #endif

		constexpr static double EPSILON = 0.0001;
        constexpr static double PI = 3.14159265359;

        using inputBit = unsigned short;

        template<typename T>
        constexpr T degrees(const T radians)
            {
                return static_cast<T>(radians * (180.0 / fe::PI));
            }

        template<typename T>
        constexpr T radians(const T degrees)
            {
                return static_cast<T>(degrees * (fe::PI / 180.0));
            }

        using index = uInt32;
	}
