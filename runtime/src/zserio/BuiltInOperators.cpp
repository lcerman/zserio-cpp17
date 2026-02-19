#include <cstdint>

#include "zserio/BuiltInOperators.h"

namespace zserio
{

namespace builtin
{

::std::uint8_t numBits(::std::uint64_t numValues)
{
    if (numValues == 0)
    {
        return 0;
    }

    ::std::uint8_t result = 1;
    ::std::uint64_t current = (numValues - 1U) >> 1U;
    while (current > 0)
    {
        result++;
        current >>= 1U;
    }

    return result;
}

} // namespace builtin

} // namespace zserio
