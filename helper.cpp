
#include "helper.hpp"


namespace helper
{
uint64_t ParseNumber(unsigned char* buffer, size_t byte_count) {
    uint64_t result = 0;
    for (size_t i = 0; i < byte_count; i++)
    {
        result = (result << 8ull) | (uint64_t) buffer[i];
    }
    return result;
}
}//helper
