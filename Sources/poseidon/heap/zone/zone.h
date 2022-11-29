#ifndef POSEIDON_HEAP_ZONE_H
#define POSEIDON_HEAP_ZONE_H

#include "poseidon/bitset.h"
#include "poseidon/heap/page/page.h"
#include "poseidon/pointer.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Class;
 class Zone : public Section {
   friend class ZoneTest;
  public:
   class ZonePointerIterator : public RawObjectPointerIterator {
    protected:
     const Zone* zone_;
     uword current_;

     inline const Zone* zone() const {
       return zone_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline Pointer* current_ptr() const {
       return (Pointer*) current_address();
     }
    public:
     explicit ZonePointerIterator(const Zone* zone):
      RawObjectPointerIterator(),
      zone_(zone),
      current_(zone->GetStartingAddress()) {
     }
     ~ZonePointerIterator() override = default;

     bool HasNext() const override {
       return zone()->Contains(current_address()) &&
              !current_ptr()->IsFree() &&
              current_ptr()->GetSize() > 0;
     }

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   Zone() = default;
   Zone(const uword start, const word size):
    Section(start, size) {
   }
  public:
   ~Zone() override = default;
   virtual uword TryAllocateBytes(word size) = 0;
   virtual uword TryAllocateClassBytes(Class* cls) = 0;
 };
}

#endif //POSEIDON_HEAP_ZONE_H
