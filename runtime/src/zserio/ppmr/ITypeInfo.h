#ifndef ZSERIO_PPMR_I_TYPE_INFO_H_INC
#define ZSERIO_PPMR_I_TYPE_INFO_H_INC

#include <cstdint>

#include "zserio/ITypeInfo.h"
#include "zserio/ppmr/PropagatingPolymorphicAllocator.h"

namespace zserio
{
namespace ppmr
{

/**
 * Global function for type info of a generated type provided via specializations.
 *
 * \return Type info.
 */
template <typename T, typename ALLOC = PropagatingPolymorphicAllocator<std::uint8_t>>
const IBasicTypeInfo<ALLOC>& typeInfo()
{
    return detail::TypeInfo<T, ALLOC>::get();
}

/** Typedef provided for convenience - using default PropagatingPolymorphicAllocator<std::uint8_t>. */
/** \{ */
using ITypeInfo = IBasicTypeInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using FieldInfo = BasicFieldInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using ParameterInfo = BasicParameterInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using FunctionInfo = BasicFunctionInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using CaseInfo = BasicCaseInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using ColumnInfo = BasicColumnInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using TableInfo = BasicTableInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using TemplateArgumentInfo = BasicTemplateArgumentInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using MessageInfo = BasicMessageInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
using MethodInfo = BasicMethodInfo<PropagatingPolymorphicAllocator<std::uint8_t>>;
/** \} */

} // namespace ppmr
} // namespace zserio

#endif // ZSERIO_PPMR_I_TYPE_INFO_H_INC
