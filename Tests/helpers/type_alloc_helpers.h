#ifndef POSEIDON_TYPE_ALLOC_HELPERS_H
#define POSEIDON_TYPE_ALLOC_HELPERS_H

#include <gtest/gtest.h>
#include "poseidon/type.h"

#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"
#include "assertions/type_assertions.h"

namespace poseidon {
#define DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE(T) \
 template<class Z>                                                \
 static inline void                                               \
 GenerateUnmarked##T##sInZone(Z* zone, const int64_t num_objects) { \
  for(auto idx = 0; idx < num_objects; idx++) {                   \
    auto new_ptr = T::TryAllocateIn(zone, idx);                   \
    ASSERT_NE(new_ptr, nullptr);                                  \
    ASSERT_TRUE(Is##T(new_ptr->raw_ptr()));                       \
    ASSERT_TRUE(T##Eq(idx, new_ptr));                             \
  }                                                               \
 }

 DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE(Byte);
 DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE(Short);
 DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE(Int);
 DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE(Long);

#define DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(T) \
 template<class Z>                                                      \
 static inline void                                                     \
 GenerateUnmarked##T##sInZone(Z* zone, const int64_t num_objects, MockRawObjectVisitor& visitor) { \
  for(auto idx = 0; idx < num_objects; idx++) {                         \
    auto new_ptr = T::TryAllocateIn(zone, idx);                         \
    ASSERT_NE(new_ptr, nullptr);                                        \
    ASSERT_TRUE(Is##T(new_ptr->raw_ptr()));                             \
    ASSERT_TRUE(T##Eq(idx, new_ptr));                                   \
    EXPECT_CALL(visitor, Visit(IsPointerTo(new_ptr)))                   \
      .Times(1)                                                         \
      .WillOnce(::testing::Return(true));                               \
  }                                                                     \
 }

DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Byte);
DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Short);
DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Int);
DEFINE_GENERATE_UNMARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Long);

#define DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(T) \
 template<class Z>                                                    \
 static inline void                                                   \
 GenerateMarked##T##sInZone(Z* zone, const int64_t num_objects, MockRawObjectVisitor& visitor) { \
  for(auto idx = 0; idx < num_objects; idx++) {                       \
    auto new_ptr = T::TryAllocateIn(zone, idx);                       \
    ASSERT_NE(new_ptr, nullptr);                                      \
    ASSERT_TRUE(Is##T(new_ptr->raw_ptr()));                           \
    ASSERT_TRUE(T##Eq(idx, new_ptr));                                 \
    new_ptr->raw_ptr()->SetMarkedBit();                               \
    ASSERT_TRUE(IsMarked(new_ptr->raw_ptr()));                        \
    EXPECT_CALL(visitor, Visit(IsPointerTo(new_ptr->raw_ptr())))      \
      .Times(1)                                                       \
      .WillRepeatedly(::testing::Return(true));                       \
  }                                                                   \
 }

DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Byte);
DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Short);
DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Int);
DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE_WITH_MOCK_VISITOR(Long);

#define DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE(T) \
 template<class Z>                                  \
 static inline void                                 \
 GenerateMarked##T##sInZone(Z* zone, const int64_t num_objects) { \
  for(auto idx = 0; idx < num_objects; idx++) {     \
    auto new_ptr = T::TryAllocateIn(zone, idx);     \
    ASSERT_NE(new_ptr, nullptr);                    \
    ASSERT_TRUE(Is##T(new_ptr->raw_ptr()));         \
    ASSERT_TRUE(T##Eq(idx, new_ptr));               \
    new_ptr->raw_ptr()->SetMarkedBit();             \
    ASSERT_TRUE(IsMarked(new_ptr->raw_ptr()));      \
  }                                                 \
 }

DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE(Byte);
DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE(Short);
DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE(Int);
DEFINE_GENERATE_MARKED_NUM_TYPES_IN_ZONE(Long);
}

#endif // POSEIDON_TYPE_ALLOC_HELPERS_H