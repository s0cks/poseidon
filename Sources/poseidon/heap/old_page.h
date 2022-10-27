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
  public:
   class OldPageIterator : public PageIterator<OldPage> {
    public:
     explicit OldPageIterator(OldPage* page):
       PageIterator<OldPage>(page) {
     }
     ~OldPageIterator() override = default;

     bool HasNext() const override {
       return current_address() > 0 &&
              current_address() < page()->GetEndingAddress() &&
              current_ptr()->IsOld() &&
              current_ptr()->GetPointerSize() > 0;
     }
   };
  protected:
   uword TryAllocate(ObjectSize size) override;
  public:
   OldPage() = default;
   OldPage(const PageIndex index, const MemoryRegion& region):
    Page(index, region) {
     SetTag(PageTag::Old(index));
     LOG_IF(FATAL, !region.Protect(MemoryRegion::kReadWrite)) << "failed to protect " << region;
   }
   OldPage(const OldPage& rhs) = default;
   ~OldPage() override = default;

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   OldPage& operator=(const OldPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const OldPage& val){
     stream << "OldPage(";
     stream << "index=" << val.index() << ", ";
     stream << "marked=" << val.marked() << ", ";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "current=" << val.GetCurrentAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer();
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
 };
}

#endif//POSEIDON_OLD_PAGE_H