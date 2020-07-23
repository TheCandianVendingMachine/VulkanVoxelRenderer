// string.hpp
// a hashed-string. Used for GUID's since it takes up no memory, can be compared quickly, and is easy to store
#pragma once
#include <unordered_map>
#include <string>
#include "typeDefines.hpp"

namespace fe
    {
    #if _DEBUG
        namespace impl
            {
                struct debugString
                    {
                        std::unordered_map<fe::str, std::string> strs;
                    };
                extern fe::impl::debugString g_debugStrings;
            }
    #endif

        constexpr fe::str hash(const char *input, fe::str hash)
            {
                unsigned long long index = 0;
                while (input[index] != '\0')
                    {
                        hash = ((hash << 5) + hash) + input[index++];
                    }
                return hash;
            }

#if _DEBUG
        // implicitely calls the hash with the magic number "5381"
        inline const fe::str hashImpl(const char *input)
            {
                fe::str h = hash(input, 5381);
                fe::impl::g_debugStrings.strs[h] = input;
                return h;
            }
#endif
    }

// Creates a GUID based on the input string
#if _DEBUG
    #define FE_STR(input) fe::hashImpl(input)
    #define FE_GET_STR(hash) fe::impl::g_debugStrings.strs[hash]
    #define FE_STR_CONST(input) fe::hash(input, 5381)
#else
    #define FE_STR(input) fe::hash(input, 5381)
    #define FE_GET_STR(hash) "DEBUG STRINGS UNAVALIABLE"
    #define FE_STR_CONST(input) fe::hash(input, 5381)
#endif
