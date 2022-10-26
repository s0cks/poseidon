#ifndef POSEIDON_NEW_PAGE_H
#define POSEIDON_NEW_PAGE_H

#include "poseidon/heap/page.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class NewPage : public Page {
   friend class NewZone;
   friend class NewPageTest;
  public:
   NewPage() = default;
   NewPage(const PageIndex index, const MemoryRegion& region):
    Page(index, region) {
     SetNewBit();
     if(!region.Protect(MemoryRegion::kReadWrite)) {
       LOG(FATAL) << "cannot set " << MemoryRegion::kReadWrite << " for " << region;
     }
   }
   NewPage(const NewPage& rhs) = default;
   ~NewPage() override = default;

   uword TryAllocate(ObjectSize size) override; //TODO: change visibility

   NewPage& operator=(const NewPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const NewPage& val) {
     stream << "NewPage(";
     stream << "index=" << val.index() << ", ";
     stream << "marked=" << val.marked() << ", ";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "current=" << val.GetCurrentAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer();
     return stream << ")";
   }

   friend bool operator==(const NewPage& lhs, const NewPage& rhs) {
     return ((const Page&)lhs) == ((const Page&)rhs);
   }

   friend bool operator!=(const NewPage& lhs, const NewPage& rhs) {
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const NewPage& lhs, const NewPage& rhs) {
     return lhs.tag() < rhs.tag();
   }

   friend bool operator>(const NewPage& lhs, const NewPage& rhs) {
     return lhs.tag() > rhs.tag();
   }
 };
}

#endif // POSEIDON_NEW_PAGE_H