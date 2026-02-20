#ifndef ZSERIO_PPMR_BIT_BUFFER_H_INC
#define ZSERIO_PPMR_BIT_BUFFER_H_INC

#include <cstdint>

#include "zserio/BitBuffer.h"
#include "zserio/ppmr/PropagatingPolymorphicAllocator.h"

namespace zserio
{
namespace ppmr
{

/** Typedef to BitBuffer provided for convenience - using PropagatingPolymorphicAllocator<std::uint8_t>. */
using BitBuffer = BasicBitBuffer<PropagatingPolymorphicAllocator<std::uint8_t>>;

/** Typedef to BitBufferView provided for convenience - using PropagatingPolymorphicAllocator<std::uint8_t>.
 */
using BitBufferView = std::reference_wrapper<const BitBuffer>;

} // namespace ppmr
} // namespace zserio

#endif // ZSERIO_PPMR_BIT_BUFFER_H_INC
