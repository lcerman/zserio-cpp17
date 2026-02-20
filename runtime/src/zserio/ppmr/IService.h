#ifndef ZSERIO_I_SERVICE_H_INC
#define ZSERIO_I_SERVICE_H_INC

#include <cstdint>

#include "zserio/IService.h"
#include "zserio/ppmr/PropagatingPolymorphicAllocator.h"

namespace zserio
{
namespace ppmr
{

/** Typedef to service interface provided for convenience - using
 * PropagatingPolymorphicAllocator<std::uint8_t>. */
/** \{ */
using IServiceData = IBasicServiceData<PropagatingPolymorphicAllocator<std::uint8_t>>;
using IServiceDataPtr = IBasicServiceDataPtr<PropagatingPolymorphicAllocator<std::uint8_t>>;
using IService = IBasicService<PropagatingPolymorphicAllocator<std::uint8_t>>;
using IServiceClient = IBasicServiceClient<PropagatingPolymorphicAllocator<std::uint8_t>>;
/** \} */

/**
 * Typedef to service data implementation provided for convenience - using
 * PropagatingPolymorphicAllocator<std::uint8_t>.
 */
template <typename ZSERIO_OBJECT>
using IntrospectableServiceData =
        BasicIntrospectableServiceData<ZSERIO_OBJECT, PropagatingPolymorphicAllocator<std::uint8_t>>;
using ObjectServiceData = BasicObjectServiceData<PropagatingPolymorphicAllocator<std::uint8_t>>;
using RawServiceDataHolder = BasicRawServiceDataHolder<PropagatingPolymorphicAllocator<std::uint8_t>>;
using RawServiceDataView = BasicRawServiceDataView<PropagatingPolymorphicAllocator<std::uint8_t>>;

} // namespace ppmr
} // namespace zserio

#endif // ZSERIO_I_SERVICE_H_INC
