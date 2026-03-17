#include <memory_resource>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

#include "any_test/AnyTest.h"
#include "gtest/gtest.h"
#include "zserio/Any.h"
#include "zserio/pmr/Any.h"
#include "zserio/ppmr/PropagatingPolymorphicAllocator.h"

#include "TrackingAllocator.h"

namespace zserio
{

namespace
{

class SmallObject
{
public:
    SmallObject() :
            m_value(0)
    {}

    bool operator==(const SmallObject& other) const
    {
        return m_value == other.m_value;
    }

private:
    int m_value;
};

struct BigObject
{
    BigObject() = default;

    bool operator==(const BigObject& other) const
    {
        return std::tie(value1, value2, value3, value4) ==
                std::tie(other.value1, other.value2, other.value3, other.value4);
    }

    uint64_t value1 = 1;
    uint64_t value2 = 2;
    uint64_t value3 = 3;
    uint64_t value4 = 4;
};

template <typename ALLOC>
class AllocatorArgObject
{
public:
    using allocator_type = ALLOC;

    explicit AllocatorArgObject(std::allocator_arg_t, const ALLOC& allocator, int value) :
            m_allocator(allocator),
            m_value(value)
    {}

    bool operator==(const AllocatorArgObject& other) const
    {
        return std::tie(m_allocator, m_value) == std::tie(other.m_allocator, other.m_value);
    }

    const ALLOC& getAllocator() const
    {
        return m_allocator;
    }

    int getValue() const
    {
        return m_value;
    }

private:
    ALLOC m_allocator;
    int m_value = 0;
};

class ThrowingOnCopyConstruction
{
public:
    explicit ThrowingOnCopyConstruction(int = 0) noexcept
    {}

    ThrowingOnCopyConstruction(const ThrowingOnCopyConstruction& other) :
            m_marker(other.m_marker)
    {
        throw std::runtime_error("copy construction failed");
    }

    ~ThrowingOnCopyConstruction();
    ThrowingOnCopyConstruction& operator=(const ThrowingOnCopyConstruction&) = default;
    ThrowingOnCopyConstruction(ThrowingOnCopyConstruction&&) = delete;
    ThrowingOnCopyConstruction& operator=(ThrowingOnCopyConstruction&&) = delete;

private:
    std::vector<int> m_marker;
};

ThrowingOnCopyConstruction::~ThrowingOnCopyConstruction() = default;

template <typename T>
class TrackingAllocatorSwapProp : public TrackingAllocator<T>
{
public:
    using propagate_on_container_swap = std::true_type;

    using TrackingAllocator<T>::TrackingAllocator;
};

BigObject createBigObject(uint64_t value1)
{
    BigObject value;
    value.value1 = value1;
    value.value2 = value1 + 1;
    value.value3 = value1 + 2;
    value.value4 = value1 + 3;
    return value;
}

template <typename ALLOC>
BasicAny<ALLOC> moveConstructWithAllocatorForTest(BasicAny<ALLOC>& any, const ALLOC& allocator)
{
    return BasicAny<ALLOC>(std::move(any), allocator);
}

} // namespace

class AnyTest : public ::testing::Test
{
protected:
    template <typename T, typename ALLOC>
    void testAny(const T& value, const ALLOC& allocator)
    {
        testEmptyConstructor<ALLOC>();
        testAllocatorConstructor(allocator);
        testLvalueConstructor(value, allocator);
        testRvalueConstructor(value, allocator);
        testCopyConstructor(value, allocator);
        testCopyConstructorWithAllocator(value, allocator);
        testCopyAssignmentOperator(value, allocator);
        testMoveConstructor(value, allocator);
        testMoveConstructorWithAllocator(value, allocator);
        testMoveAssignmentOperator(value, allocator);
        testValueAssignmentOperator<T, ALLOC>(value);
        testSwap(value, allocator);
        testReset(value, allocator);
        testSet(value, allocator);
        testEmplace(value, allocator);
        testAllocatorArgEmplace(allocator);
        testGet(value, allocator);
        testConstGet(value, allocator);
        testGetIf(value, allocator);
        testConstGetIf(value, allocator);
        testIsType(value, allocator);
        testHasValue(value, allocator);
    }

private:
    template <typename ALLOC>
    BasicAny<ALLOC> moveConstruct(BasicAny<ALLOC>& any)
    {
        return BasicAny<ALLOC>(std::move(any));
    }

    template <typename ALLOC>
    BasicAny<ALLOC> moveConstructWithAllocator(BasicAny<ALLOC>& any, const ALLOC& allocator)
    {
        return BasicAny<ALLOC>(std::move(any), allocator);
    }

    template <typename ALLOC>
    void moveAssign(BasicAny<ALLOC>& destination, BasicAny<ALLOC>& source)
    {
        destination = std::move(source);
    }

    template <typename ALLOC>
    void testEmptyConstructor()
    {
        BasicAny<ALLOC> any;
        ASSERT_FALSE(any.hasValue());
    }

    template <typename ALLOC>
    void testAllocatorConstructor(const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        ASSERT_FALSE(any.hasValue());
        ASSERT_EQ(allocator, any.get_allocator());
    }

    template <typename T, typename ALLOC>
    void testLvalueConstructor(const T& value, const ALLOC& allocator)
    {
        const void* const originalAddress = &value;
        BasicAny<ALLOC> any(value, allocator);

        const T& anyValue = any.template get<T>();
        ASSERT_NE(originalAddress, &anyValue);
        ASSERT_EQ(value, anyValue);
        ASSERT_EQ(allocator, any.get_allocator());
    }

    template <typename ALLOC>
    void testRvalueConstructor(const std::vector<int>& value, const ALLOC& allocator)
    {
        std::vector<int> originalValue(value);
        const void* const originalAddress = originalValue.data();
        BasicAny<ALLOC> any(std::move(originalValue), allocator);

        const std::vector<int>& anyValue = any.template get<std::vector<int>>();
        ASSERT_EQ(originalAddress, anyValue.data());
        ASSERT_EQ(value, anyValue);
        ASSERT_EQ(allocator, any.get_allocator());
    }

    template <typename T, typename ALLOC>
    void testRvalueConstructor(const T& value, const ALLOC& allocator)
    {
        T originalValue(value);
        BasicAny<ALLOC> any(std::move(originalValue), allocator);

        ASSERT_EQ(value, any.template get<T>());
        ASSERT_EQ(allocator, any.get_allocator());
    }

    template <typename T, typename ALLOC>
    void testCopyConstructor(const T& value, const ALLOC& allocator)
    {
        const bool copiesSourceAllocator = (allocator.select_on_container_copy_construction() == allocator);
        BasicAny<ALLOC> any(allocator);

        BasicAny<ALLOC> emptyCopy(any);
        ASSERT_FALSE(emptyCopy.hasValue());
        if (copiesSourceAllocator)
        {
            ASSERT_EQ(allocator, emptyCopy.get_allocator());
        }
        else
        {
            ASSERT_NE(allocator, emptyCopy.get_allocator());
        }

        any.set(value);
        BasicAny<ALLOC> copy(any);
        ASSERT_EQ(value, copy.template get<T>());
        if (copiesSourceAllocator)
        {
            ASSERT_EQ(allocator, copy.get_allocator());
        }
        else
        {
            ASSERT_NE(allocator, copy.get_allocator());
        }
    }

    template <typename T, typename ALLOC>
    void testCopyConstructorWithAllocator(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        const ALLOC newAllocator;

        BasicAny<ALLOC> emptyCopy(any, newAllocator);
        ASSERT_FALSE(emptyCopy.hasValue());
        ASSERT_EQ(newAllocator, emptyCopy.get_allocator());

        any.set(value);
        const size_t numAllocations = allocator.numAllocs();
        BasicAny<ALLOC> copy(any, newAllocator);
        ASSERT_EQ(value, copy.template get<T>());
        ASSERT_EQ(newAllocator, copy.get_allocator());
        ASSERT_EQ(numAllocations, allocator.numAllocs());
    }

    template <typename T, typename ALLOC>
    void testCopyAssignmentOperator(const T& value, const ALLOC& allocator)
    {
        using AllocTraits = std::allocator_traits<ALLOC>;

        BasicAny<ALLOC> any(allocator);

        BasicAny<ALLOC> emptyCopy;
        const ALLOC emptyCopyAllocator = emptyCopy.get_allocator();
        emptyCopy = any;
        ASSERT_FALSE(emptyCopy.hasValue());
        if constexpr (AllocTraits::propagate_on_container_copy_assignment::value)
        {
            ASSERT_EQ(allocator, emptyCopy.get_allocator());
        }
        else
        {
            ASSERT_EQ(emptyCopyAllocator, emptyCopy.get_allocator());
        }

        any.set(value);
        BasicAny<ALLOC>& anyRef = any;
        anyRef = any;
        ASSERT_EQ(value, anyRef.template get<T>());
        ASSERT_EQ(allocator, anyRef.get_allocator());

        BasicAny<ALLOC> copy;
        const ALLOC copyAllocator = copy.get_allocator();
        copy = any;
        ASSERT_EQ(value, copy.template get<T>());
        if constexpr (AllocTraits::propagate_on_container_copy_assignment::value)
        {
            ASSERT_EQ(allocator, copy.get_allocator());
        }
        else
        {
            ASSERT_EQ(copyAllocator, copy.get_allocator());
        }
    }

    template <typename T, typename ALLOC>
    void testMoveConstructor(const T& value, const ALLOC& allocator)
    {
        {
            BasicAny<ALLOC> any(allocator);
            BasicAny<ALLOC> moved = moveConstruct(any);
            ASSERT_FALSE(moved.hasValue());
            ASSERT_FALSE(any.hasValue());
            ASSERT_EQ(allocator, moved.get_allocator());
        }

        {
            BasicAny<ALLOC> any(allocator);
            any.set(value);
            const size_t numAllocations = allocator.numAllocs();
            BasicAny<ALLOC> moved = moveConstruct(any);
            ASSERT_EQ(value, moved.template get<T>());
            ASSERT_FALSE(any.hasValue());
            ASSERT_EQ(allocator, moved.get_allocator());
            ASSERT_EQ(numAllocations, allocator.numAllocs());
        }
    }

    template <typename T, typename ALLOC>
    void testMoveConstructorWithAllocator(const T& value, const ALLOC& allocator)
    {
        {
            BasicAny<ALLOC> any(allocator);
            BasicAny<ALLOC> moved = moveConstructWithAllocator(any, allocator);
            ASSERT_FALSE(moved.hasValue());
            ASSERT_FALSE(any.hasValue());
            ASSERT_EQ(allocator, moved.get_allocator());
        }

        {
            BasicAny<ALLOC> any(allocator);
            const ALLOC newAllocator;
            BasicAny<ALLOC> moved = moveConstructWithAllocator(any, newAllocator);
            ASSERT_FALSE(moved.hasValue());
            ASSERT_FALSE(any.hasValue());
            ASSERT_EQ(newAllocator, moved.get_allocator());
        }

        {
            BasicAny<ALLOC> any(allocator);
            any.set(value);
            const size_t numAllocations = allocator.numAllocs();
            BasicAny<ALLOC> moved = moveConstructWithAllocator(any, allocator);
            ASSERT_EQ(value, moved.template get<T>());
            ASSERT_FALSE(any.hasValue());
            ASSERT_EQ(allocator, moved.get_allocator());
            ASSERT_EQ(numAllocations, allocator.numAllocs());
        }

        {
            BasicAny<ALLOC> any(allocator);
            any.set(value);
            const size_t numAllocations = allocator.numAllocs();
            const ALLOC newAllocator;
            BasicAny<ALLOC> moved = moveConstructWithAllocator(any, newAllocator);
            ASSERT_EQ(value, moved.template get<T>());
            ASSERT_FALSE(any.hasValue());
            ASSERT_EQ(newAllocator, moved.get_allocator());
            ASSERT_TRUE(numAllocations >= allocator.numAllocs());
        }
    }

    template <typename T, typename ALLOC>
    void testMoveAssignmentOperator(const T& value, const ALLOC& allocator)
    {
        using AllocTraits = std::allocator_traits<ALLOC>;

        {
            BasicAny<ALLOC> any(allocator);
            BasicAny<ALLOC> moved;
            const ALLOC movedAllocator = moved.get_allocator();
            moveAssign(moved, any);
            ASSERT_FALSE(moved.hasValue());
            ASSERT_FALSE(any.hasValue());
            if constexpr (AllocTraits::propagate_on_container_move_assignment::value)
            {
                ASSERT_EQ(allocator, moved.get_allocator());
            }
            else
            {
                ASSERT_EQ(movedAllocator, moved.get_allocator());
            }
        }

        {
            BasicAny<ALLOC> any(allocator);
            any.set(value);
            const size_t numAllocations = allocator.numAllocs();
            BasicAny<ALLOC>& anyRef = any;
            anyRef = std::move(any);
            ASSERT_EQ(value, anyRef.template get<T>());
            ASSERT_EQ(allocator, anyRef.get_allocator());
            ASSERT_EQ(numAllocations, allocator.numAllocs());
        }

        {
            BasicAny<ALLOC> any(allocator);
            any.set(value);
            const size_t numAllocations = allocator.numAllocs();
            BasicAny<ALLOC> moved;
            const ALLOC movedAllocator = moved.get_allocator();
            moveAssign(moved, any);
            ASSERT_EQ(value, moved.template get<T>());
            ASSERT_FALSE(any.hasValue());
            if constexpr (AllocTraits::propagate_on_container_move_assignment::value)
            {
                ASSERT_EQ(allocator, moved.get_allocator());
            }
            else
            {
                ASSERT_EQ(movedAllocator, moved.get_allocator());
            }
            ASSERT_TRUE(numAllocations >= allocator.numAllocs());
        }
    }

    template <typename T, typename ALLOC>
    void testValueAssignmentOperator(const T& value)
    {
        {
            T valueCopy(value);
            BasicAny<ALLOC> any;
            const ALLOC anyAllocator = any.get_allocator();
            any = std::move(valueCopy);
            ASSERT_EQ(value, any.template get<T>());
            ASSERT_EQ(anyAllocator, any.get_allocator());
        }

        {
            T valueCopy(value);
            BasicAny<ALLOC> any;
            any.set(3.14F);
            const ALLOC anyAllocator = any.get_allocator();
            any = std::move(valueCopy);
            ASSERT_EQ(value, any.template get<T>());
            ASSERT_EQ(anyAllocator, any.get_allocator());
        }
    }

    template <typename T, typename ALLOC>
    void testSwap(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> empty(allocator);
        BasicAny<ALLOC> any(allocator);
        any.set(value);

        any.swap(empty);
        ASSERT_FALSE(any.hasValue());
        ASSERT_TRUE(empty.template isType<T>());
        ASSERT_EQ(value, empty.template get<T>());

        any.set(150);
        any.swap(empty);
        ASSERT_TRUE(empty.template isType<int>());
        ASSERT_EQ(150, empty.template get<int>());
        ASSERT_TRUE(any.template isType<T>());
        ASSERT_EQ(value, any.template get<T>());

        BasicAny<ALLOC> left(allocator);
        BasicAny<ALLOC> right(allocator);
        const BigObject leftValue = createBigObject(11);
        const BigObject rightValue = createBigObject(21);
        left.set(leftValue);
        right.set(rightValue);
        const BigObject* const leftAddress = &left.template get<BigObject>();
        const BigObject* const rightAddress = &right.template get<BigObject>();

        left.swap(right);
        ASSERT_EQ(rightAddress, &left.template get<BigObject>());
        ASSERT_EQ(leftAddress, &right.template get<BigObject>());
        ASSERT_EQ(rightValue, left.template get<BigObject>());
        ASSERT_EQ(leftValue, right.template get<BigObject>());

        {
            BasicAny<ALLOC> emptyLeft(allocator);
            BasicAny<ALLOC> heapRight(allocator);
            const BigObject heapValue = createBigObject(41);
            heapRight.set(heapValue);

            emptyLeft.swap(heapRight);

            ASSERT_TRUE(emptyLeft.template isType<BigObject>());
            ASSERT_EQ(heapValue, emptyLeft.template get<BigObject>());
            ASSERT_FALSE(heapRight.hasValue());
        }

        {
            BasicAny<ALLOC> heapLeft(allocator);
            BasicAny<ALLOC> inPlaceRight(allocator);
            const BigObject heapValue = createBigObject(51);
            heapLeft.set(heapValue);
            inPlaceRight.set(23);

            heapLeft.swap(inPlaceRight);

            ASSERT_TRUE(heapLeft.template isType<int>());
            ASSERT_EQ(23, heapLeft.template get<int>());
            ASSERT_TRUE(inPlaceRight.template isType<BigObject>());
            ASSERT_EQ(heapValue, inPlaceRight.template get<BigObject>());
        }

        {
            const ALLOC otherAllocator;
            BasicAny<ALLOC> heapLeft(allocator);
            BasicAny<ALLOC> heapRight(otherAllocator);
            const BigObject leftHeapValue = createBigObject(61);
            const BigObject rightHeapValue = createBigObject(71);
            heapLeft.set(leftHeapValue);
            heapRight.set(rightHeapValue);

            heapLeft.swap(heapRight);

            ASSERT_EQ(allocator, heapLeft.get_allocator());
            ASSERT_EQ(otherAllocator, heapRight.get_allocator());
            ASSERT_TRUE(heapLeft.template isType<BigObject>());
            ASSERT_TRUE(heapRight.template isType<BigObject>());
            ASSERT_EQ(rightHeapValue, heapLeft.template get<BigObject>());
            ASSERT_EQ(leftHeapValue, heapRight.template get<BigObject>());
        }
    }

    template <typename T, typename ALLOC>
    void testReset(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(value, allocator);
        ASSERT_TRUE(any.hasValue());

        any.reset();
        ASSERT_FALSE(any.hasValue());

        any.set(value);
        ASSERT_TRUE(any.hasValue());
        ASSERT_EQ(value, any.template get<T>());
    }

    template <typename T, typename ALLOC>
    void testSet(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        any.set(value);
        ASSERT_TRUE(any.template isType<T>());
        ASSERT_EQ(value, any.template get<T>());

        const T valueCopy(value);
        any.set(valueCopy);
        ASSERT_TRUE(any.template isType<T>());
        ASSERT_EQ(value, any.template get<T>());

        any.set(0xDEAD);
        ASSERT_TRUE(any.template isType<int>());
        ASSERT_EQ(0xDEAD, any.template get<int>());

        any.set(3.14F);
        ASSERT_TRUE(any.template isType<float>());
        ASSERT_EQ(3.14F, any.template get<float>());
    }

    template <typename T, typename ALLOC>
    void testEmplace(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        const T& emplacedValue = any.template emplace<T>(value);
        ASSERT_EQ(value, emplacedValue);
        ASSERT_TRUE(any.template isType<T>());
        ASSERT_EQ(value, any.template get<T>());
    }

    template <typename ALLOC>
    void testAllocatorArgEmplace(const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        const auto& emplacedValue = any.template emplace<AllocatorArgObject<ALLOC>>(13);
        ASSERT_EQ(allocator, emplacedValue.getAllocator());
        ASSERT_EQ(13, emplacedValue.getValue());
        ASSERT_EQ(emplacedValue, any.template get<AllocatorArgObject<ALLOC>>());
    }

    template <typename T, typename ALLOC>
    void testGet(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        ASSERT_THROW(any.template get<int>(), CppRuntimeException);

        any.set(value);
        ASSERT_EQ(value, any.template get<T>());
        ASSERT_THROW(any.template get<long>(), CppRuntimeException);
    }

    template <typename T, typename ALLOC>
    void testConstGet(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        const BasicAny<ALLOC>& constAny = any;
        ASSERT_THROW(constAny.template get<int>(), CppRuntimeException);

        any.set(value);
        ASSERT_EQ(value, constAny.template get<T>());
        ASSERT_THROW(constAny.template get<long>(), CppRuntimeException);
    }

    template <typename T, typename ALLOC>
    void testGetIf(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        ASSERT_EQ(nullptr, any.template get_if<T>());

        any.set(value);
        ASSERT_NE(nullptr, any.template get_if<T>());
        ASSERT_EQ(any.template get_if<T>(), &any.template get<T>());
        ASSERT_EQ(nullptr, any.template get_if<std::string>());
        ASSERT_EQ(nullptr, any.template get_if<long>());

        any.reset();
        ASSERT_EQ(nullptr, any.template get_if<T>());

        BasicAny<ALLOC> throwingAny(allocator);
        const ThrowingOnCopyConstruction throwingValue(13);
        ASSERT_THROW(throwingAny.set(throwingValue), std::runtime_error);
        ASSERT_TRUE(throwingAny.template isType<ThrowingOnCopyConstruction>());
        ASSERT_EQ(nullptr, throwingAny.template get_if<ThrowingOnCopyConstruction>());
    }

    template <typename T, typename ALLOC>
    void testConstGetIf(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        const BasicAny<ALLOC>& constAny = any;
        ASSERT_EQ(nullptr, constAny.template get_if<T>());

        any.set(value);
        ASSERT_NE(nullptr, constAny.template get_if<T>());
        ASSERT_EQ(constAny.template get_if<T>(), &constAny.template get<T>());
        ASSERT_EQ(nullptr, constAny.template get_if<std::string>());
        ASSERT_EQ(nullptr, constAny.template get_if<long>());

        any.reset();
        ASSERT_EQ(nullptr, constAny.template get_if<T>());

        BasicAny<ALLOC> throwingAny(allocator);
        const ThrowingOnCopyConstruction throwingValue(13);
        ASSERT_THROW(throwingAny.set(throwingValue), std::runtime_error);
        ASSERT_TRUE(throwingAny.template isType<ThrowingOnCopyConstruction>());
        const BasicAny<ALLOC>& constThrowingAny = throwingAny;
        ASSERT_EQ(nullptr, constThrowingAny.template get_if<ThrowingOnCopyConstruction>());
    }

    template <typename T, typename ALLOC>
    void testIsType(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        ASSERT_FALSE(any.template isType<int>());

        any.set(value);
        ASSERT_TRUE(any.template isType<T>());
        ASSERT_FALSE(any.template isType<long>());

        any.set(0xDEAD);
        ASSERT_TRUE(any.template isType<int>());

        any.set(3.14F);
        ASSERT_TRUE(any.template isType<float>());
        ASSERT_FALSE(any.template isType<int>());
    }

    template <typename T, typename ALLOC>
    void testHasValue(const T& value, const ALLOC& allocator)
    {
        BasicAny<ALLOC> any(allocator);
        ASSERT_FALSE(any.hasValue());

        any.set(value);
        ASSERT_TRUE(any.hasValue());

        any.set(0xDEAD);
        ASSERT_TRUE(any.hasValue());

        any.reset();
        ASSERT_FALSE(any.hasValue());
    }
};

TEST_F(AnyTest, integerPropagatingAllocator)
{
    const int value = 0xDEAD;
    const TrackingAllocator<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, integerNonPropagatingAllocator)
{
    const int value = 0xDEAD;
    const TrackingAllocatorNonProp<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, vectorPropagatingAllocator)
{
    const std::vector<int> value{1, 2, 3};
    const TrackingAllocator<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, vectorNonPropagatingAllocator)
{
    const std::vector<int> value{1, 2, 3};
    const TrackingAllocatorNonProp<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, smallObjectPropagatingAllocator)
{
    const SmallObject value;
    const TrackingAllocator<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, smallObjectNonPropagatingAllocator)
{
    const SmallObject value;
    const TrackingAllocatorNonProp<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, bigObjectPropagatingAllocator)
{
    const BigObject value;
    const TrackingAllocator<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, bigObjectNonPropagatingAllocator)
{
    const BigObject value;
    const TrackingAllocatorNonProp<uint8_t> allocator;
    testAny(value, allocator);
}

TEST_F(AnyTest, emptyStdAllocatorCopy)
{
    using StdAny = BasicAny<std::allocator<uint8_t>>;

    const std::allocator<uint8_t> allocator;
    const StdAny empty(allocator);

    const StdAny copy(empty);
    ASSERT_FALSE(copy.hasValue());
    ASSERT_EQ(allocator, copy.get_allocator());

    const StdAny copyWithAllocator(empty, allocator);
    ASSERT_FALSE(copyWithAllocator.hasValue());
    ASSERT_EQ(allocator, copyWithAllocator.get_allocator());

    StdAny assigned;
    assigned.set(13);
    assigned = empty;
    ASSERT_FALSE(assigned.hasValue());
    ASSERT_EQ(allocator, assigned.get_allocator());
}

TEST_F(AnyTest, pmrAnyCopyAndMove)
{
    std::pmr::monotonic_buffer_resource leftResource;
    std::pmr::monotonic_buffer_resource rightResource;
    const std::pmr::polymorphic_allocator<uint8_t> leftAllocator(&leftResource);
    const std::pmr::polymorphic_allocator<uint8_t> rightAllocator(&rightResource);

    {
        const pmr::Any empty(leftAllocator);
        const pmr::Any copy(empty, leftAllocator);

        ASSERT_FALSE(copy.hasValue());
        ASSERT_EQ(leftAllocator.resource(), copy.get_allocator().resource());
    }

    {
        pmr::Any source(leftAllocator);
        const BigObject value = createBigObject(81);
        source.set(value);

        const pmr::Any moved = moveConstructWithAllocatorForTest(source, leftAllocator);

        ASSERT_EQ(value, moved.template get<BigObject>());
        ASSERT_FALSE(source.hasValue());
        ASSERT_EQ(leftAllocator.resource(), moved.get_allocator().resource());
    }

    {
        pmr::Any source(leftAllocator);
        const BigObject value = createBigObject(83);
        source.set(value);

        const pmr::Any moved = moveConstructWithAllocatorForTest(source, rightAllocator);

        ASSERT_EQ(value, moved.template get<BigObject>());
        ASSERT_FALSE(source.hasValue());
        ASSERT_EQ(rightAllocator.resource(), moved.get_allocator().resource());
    }
}

TEST_F(AnyTest, propagatingPmrAnyCopyAndMove)
{
    using Alloc = ppmr::PropagatingPolymorphicAllocator<uint8_t>;
    using PpmrAny = BasicAny<Alloc>;

    std::pmr::monotonic_buffer_resource leftResource;
    std::pmr::monotonic_buffer_resource rightResource;
    const Alloc leftAllocator(&leftResource);
    const Alloc rightAllocator(&rightResource);

    {
        const PpmrAny empty(leftAllocator);
        const PpmrAny copy(empty);

        ASSERT_FALSE(copy.hasValue());
        ASSERT_EQ(leftAllocator, copy.get_allocator());
    }

    {
        PpmrAny source(leftAllocator);
        const BigObject value = createBigObject(85);
        source.set(value);

        const PpmrAny moved = moveConstructWithAllocatorForTest(source, leftAllocator);

        ASSERT_EQ(value, moved.template get<BigObject>());
        ASSERT_FALSE(source.hasValue());
        ASSERT_EQ(leftAllocator, moved.get_allocator());
    }

    {
        PpmrAny source(leftAllocator);
        source.set(17);

        const PpmrAny moved = moveConstructWithAllocatorForTest(source, leftAllocator);

        ASSERT_EQ(17, moved.template get<int>());
        ASSERT_FALSE(source.hasValue());
        ASSERT_EQ(leftAllocator, moved.get_allocator());
    }

    {
        PpmrAny source(leftAllocator);
        const BigObject value = createBigObject(87);
        source.set(value);

        const PpmrAny moved = moveConstructWithAllocatorForTest(source, rightAllocator);

        ASSERT_EQ(value, moved.template get<BigObject>());
        ASSERT_FALSE(source.hasValue());
        ASSERT_EQ(rightAllocator, moved.get_allocator());
    }

    {
        PpmrAny empty(leftAllocator);

        const PpmrAny moved = moveConstructWithAllocatorForTest(empty, rightAllocator);

        ASSERT_FALSE(moved.hasValue());
        ASSERT_FALSE(empty.hasValue());
        ASSERT_EQ(rightAllocator, moved.get_allocator());
    }
}

TEST_F(AnyTest, swapPropagatingAllocator)
{
    using Alloc = TrackingAllocatorSwapProp<uint8_t>;

    {
        const Alloc leftAllocator;
        const Alloc rightAllocator;
        BasicAny<Alloc> left(leftAllocator);
        BasicAny<Alloc> right(rightAllocator);
        const BigObject leftValue = createBigObject(31);
        const BigObject rightValue = createBigObject(35);
        left.set(leftValue);
        right.set(rightValue);

        left.swap(right);

        ASSERT_EQ(rightValue, left.template get<BigObject>());
        ASSERT_EQ(leftValue, right.template get<BigObject>());
        ASSERT_EQ(rightAllocator, left.get_allocator());
        ASSERT_EQ(leftAllocator, right.get_allocator());
    }

    {
        const Alloc allocator;
        BasicAny<Alloc> left(allocator);
        BasicAny<Alloc> right(allocator);
        const BigObject leftValue = createBigObject(37);
        const BigObject rightValue = createBigObject(41);
        left.set(leftValue);
        right.set(rightValue);

        left.swap(right);

        ASSERT_EQ(rightValue, left.template get<BigObject>());
        ASSERT_EQ(leftValue, right.template get<BigObject>());
        ASSERT_EQ(allocator, left.get_allocator());
        ASSERT_EQ(allocator, right.get_allocator());
    }

    {
        const Alloc allocator;
        BasicAny<Alloc> left(allocator);
        BasicAny<Alloc> right(allocator);
        const BigObject rightValue = createBigObject(43);
        right.set(rightValue);

        left.swap(right);

        ASSERT_EQ(rightValue, left.template get<BigObject>());
        ASSERT_FALSE(right.hasValue());
        ASSERT_EQ(allocator, left.get_allocator());
        ASSERT_EQ(allocator, right.get_allocator());
    }

    {
        const Alloc allocator;
        BasicAny<Alloc> left(allocator);
        BasicAny<Alloc> right(allocator);
        const BigObject leftValue = createBigObject(45);
        left.set(leftValue);

        left.swap(right);

        ASSERT_FALSE(left.hasValue());
        ASSERT_EQ(leftValue, right.template get<BigObject>());
        ASSERT_EQ(allocator, left.get_allocator());
        ASSERT_EQ(allocator, right.get_allocator());
    }

    {
        const Alloc allocator;
        BasicAny<Alloc> left(allocator);
        BasicAny<Alloc> right(allocator);
        left.set(17);
        right.set(createBigObject(47));

        left.swap(right);

        ASSERT_TRUE(left.template isType<BigObject>());
        ASSERT_EQ(createBigObject(47), left.template get<BigObject>());
        ASSERT_TRUE(right.template isType<int>());
        ASSERT_EQ(17, right.template get<int>());
        ASSERT_EQ(allocator, left.get_allocator());
        ASSERT_EQ(allocator, right.get_allocator());
    }

    {
        const Alloc allocator;
        BasicAny<Alloc> left(allocator);
        BasicAny<Alloc> right(allocator);
        const BigObject leftValue = createBigObject(41);
        left.set(leftValue);
        right.set(23);

        left.swap(right);

        ASSERT_EQ(23, left.template get<int>());
        ASSERT_EQ(leftValue, right.template get<BigObject>());
        ASSERT_EQ(allocator, left.get_allocator());
        ASSERT_EQ(allocator, right.get_allocator());
    }
}

TEST_F(AnyTest, unexceptedCallsHeapHolder)
{
    const TrackingAllocator<uint8_t> allocator;
    detail::HeapHolder<BigObject, TrackingAllocator<uint8_t>>* holder =
            detail::HeapHolder<BigObject, TrackingAllocator<uint8_t>>::create(allocator);
    ASSERT_THROW(holder->clone(nullptr), CppRuntimeException);
    ASSERT_THROW(holder->move(nullptr), CppRuntimeException);
    holder->destroy(allocator);
}

TEST_F(AnyTest, unexceptedCallsNonHeapHolder)
{
    const TrackingAllocator<uint8_t> allocator;
    using MaxInPlaceType = std::aligned_storage<3 * sizeof(void*), alignof(void*)>::type;
    MaxInPlaceType inPlace = MaxInPlaceType();
    detail::NonHeapHolder<uint8_t, TrackingAllocator<uint8_t>>* holder =
            detail::NonHeapHolder<uint8_t, TrackingAllocator<uint8_t>>::create(&inPlace);
    ASSERT_THROW(holder->clone(allocator), CppRuntimeException);
    ASSERT_THROW(holder->move(allocator), CppRuntimeException);
    holder->destroy(allocator);
}

TEST_F(AnyTest, sharedLib)
{
    auto any1 = AnyTestLib::createAnyStdInt();
    ASSERT_TRUE(any1.isType<int>());
    ASSERT_FALSE(any1.isType<float>());
    auto any2 = AnyTestLib::createAnyInt32();
    ASSERT_TRUE(any2.isType<zserio::Int32>());
    ASSERT_FALSE(any2.isType<zserio::Int64>());
    auto any3 = AnyTestLib::createAnyString();
    ASSERT_TRUE(any3.isType<zserio::String>());
    ASSERT_FALSE(any3.isType<zserio::Int32>());
}

TEST_F(AnyTest, prettyFunction)
{
    Any any;
    any.set(std::vector<int>());
    ASSERT_TRUE(any.isType<std::vector<int>>());
    ASSERT_TRUE((any.isType<std::vector<int, std::allocator<int>>>()));
    any.set(std::vector<int, std::allocator<int>>());
    ASSERT_TRUE(any.isType<std::vector<int>>());
    ASSERT_TRUE((any.isType<std::vector<int, std::allocator<int>>>()));
}

} // namespace zserio
