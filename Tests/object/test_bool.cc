#include "object/test_bool.h"

#include "zone/mock_new_zone.h"
#include "matchers/size_eq.h"
#include "matchers/is_class.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 TEST_F(BoolTest, TestTryAllocateIn_WillPass_True) {
   MockNewZone zone;

   const auto size = Bool::GetClassAllocationSize();
   auto ptr = malloc(size);
   auto raw_ptr = Pointer::New((uword)ptr, PointerTag::New(size));
   ON_CALL(zone, TryAllocateClassBytes(IsClass<Bool>()))
    .WillByDefault([&](Class* cls) {
      DLOG(INFO) << "allocating " << (*cls) << " w/ " << (*raw_ptr);
      return raw_ptr->GetObjectPointerAddress();
    });

   auto value = Bool::TryAllocateIn(&zone, true);
   ASSERT_TRUE(IsAllocated(value));
   ASSERT_TRUE(IsNew(value));
   ASSERT_TRUE(IsBool(value));
   ASSERT_EQ(TypeId::kBoolTypeId, value->GetTypeId());
   ASSERT_EQ(true, value->Get());

   free(ptr);
 }
}