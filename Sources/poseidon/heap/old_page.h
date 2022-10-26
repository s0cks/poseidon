#ifndef POSEIDON_OLD_PAGE_H
#define POSEIDON_OLD_PAGE_H

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/common.h"
#include "poseidon/heap/page.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class OldZone;
 class OldPage : public Page{
   friend class OldPageTest;
   friend class SerialSweeperTest;
  protected:
   uword TryAllocate(ObjectSize size) override;
  public:
   OldPage():
     Page() {
   }
   OldPage(const PageIndex index, const MemoryRegion& region):
    Page(index, region) {
     if(!region.Protect(MemoryRegion::kReadWrite)) {
       LOG(FATAL) << "failed to protect memory region " << region;
       return;
     }
   }
   OldPage(const OldPage& rhs) = default;
   ~OldPage() override = default;

   OldPage& operator=(const OldPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const OldPage& val){
     stream << "OldPage(";
     stream << "tag=" << val.tag() << ", ";
     stream << "start=" << val.GetStartingAddress() << ", ";
     stream << "size=" << Bytes(val.GetSize());
     stream << ")";
     return stream;
   }

   friend bool operator==(const OldPage& lhs, const OldPage& rhs){
     return lhs.tag() == rhs.tag()
         && lhs.GetStartingAddress() == rhs.GetStartingAddress()
         && lhs.GetSize() == rhs.GetSize();
   }

   friend bool operator!=(const OldPage& lhs, const OldPage& rhs){
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const OldPage& lhs, const OldPage& rhs){
     return lhs.tag() < rhs.tag();
   }
 };
}

#endif//POSEIDON_OLD_PAGE_H