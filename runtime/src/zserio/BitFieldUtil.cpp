#include <cstddef>
#include <cstdint>

#include "zserio/BitFieldUtil.h"
#include "zserio/CppRuntimeException.h"

namespace zserio
{

static void checkBitFieldLength(::std::size_t length)
{
    if (length == 0 || length > 64)
    {
        throw CppRuntimeException("Asking for bound of bitfield with invalid length ") << length << "!";
    }
}

::std::int64_t getBitFieldLowerBound(::std::size_t length, bool isSigned)
{
    checkBitFieldLength(length);

    if (isSigned)
    {
        return -static_cast<::std::int64_t>((UINT64_C(1) << (length - 1)) - 1) - 1;
    }
    else
    {
        return 0;
    }
}

::std::uint64_t getBitFieldUpperBound(::std::size_t length, bool isSigned)
{
    checkBitFieldLength(length);

    if (isSigned)
    {
        return (UINT64_C(1) << (length - 1)) - 1;
    }
    else
    {
        return length == 64 ? UINT64_MAX : ((UINT64_C(1) << length) - 1);
    }
}

} // namespace zserio
