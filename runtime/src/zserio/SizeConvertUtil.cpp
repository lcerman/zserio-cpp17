#include <cstddef>
#include <cstdint>
#include <limits>

#include "zserio/CppRuntimeException.h"
#include "zserio/RuntimeArch.h"
#include "zserio/SizeConvertUtil.h"

namespace zserio
{

::std::uint32_t convertSizeToUInt32(::std::size_t value)
{
#ifdef ZSERIO_RUNTIME_64BIT
    if (value > static_cast<::std::size_t>(std::numeric_limits<::std::uint32_t>::max()))
    {
        throw CppRuntimeException("SizeConvertUtil: ::std::size_t value '")
                << value << "' is out of bounds for conversion to ::std::uint32_t type!";
    }
#endif

    return static_cast<::std::uint32_t>(value);
}

::std::size_t convertUInt64ToSize(::std::uint64_t value)
{
#ifndef ZSERIO_RUNTIME_64BIT
    if (value > static_cast<::std::uint64_t>(std::numeric_limits<::std::size_t>::max()))
    {
        throw CppRuntimeException("SizeConvertUtil: ::std::uint64_t value '")
                << value << "' is out of bounds for conversion to ::std::size_t type!";
    }
#endif

    return static_cast<::std::size_t>(value);
}

} // namespace zserio
