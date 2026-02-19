#ifndef ZSERIO_SIZE_CONVERT_UTIL_H_INC
#define ZSERIO_SIZE_CONVERT_UTIL_H_INC

#include <cstddef>
#include <cstdint>

namespace zserio
{

/**
 * Converts size (array size, string size or bit buffer size) of type ::std::size_t to ::std::uint32_t value.
 *
 * \param value Size of type ::std::size_t to convert.
 *
 * \return ::std::uint32_t value converted from size.
 *
 * \throw CppRuntimeException when input value is not convertible to ::std::uint32_t value.
 */
::std::uint32_t convertSizeToUInt32(::std::size_t value);

/**
 * Converts ::std::uint64_t value to size (array size, string size of bit buffer size).
 *
 * \param value ::std::uint64_t value to convert.
 *
 * \return ::std::size_t value converted from ::std::uint64_t value.
 *
 * \throw CppRuntimeException when input value is not convertible to ::std::size_t value.
 */
::std::size_t convertUInt64ToSize(::std::uint64_t value);

} // namespace zserio

#endif // ZSERIO_SIZE_CONVERT_UTIL_H_INC
