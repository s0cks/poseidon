#ifndef POSEIDON_NEW_PAGE_H
#define POSEIDON_NEW_PAGE_H

#include "poseidon/heap/page.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class NewPage : public Page {
   friend class NewZone;
   friend class NewPageTest;
   friend class SerialMarkerTest;
  public:
   class NewPageIterator : public PageIterator<NewPage> {
    public:
     explicit NewPageIterator(NewPage* page):
       PageIterator<NewPage>(page) {
     }
     ~NewPageIterator() override = default;

     bool HasNext() const override {
       return current_address() > 0 &&
              current_address() < page()->GetEndingAddress() &&
              current_ptr()->IsNew();
     }
   };
  protected:
   uword TryAllocate(ObjectSize size) override; //TODO: change visibility
  public:
   NewPage() = default;
   NewPage(const PageIndex index, const MemoryRegion& region):
    Page(index, region) {
     SetTag(PageTag::New(index));
     LOG_IF(FATAL, !region.Protect(MemoryRegion::kReadWrite)) << "failed to protect " << region;
   }
   NewPage(const NewPage& rhs) = default;
   ~NewPage() override = default;

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

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
     return ((const Page&)lhs) != ((const Page&)rhs);
   }

   friend bool operator<(const NewPage& lhs, const NewPage& rhs) {
     return lhs.index() < rhs.index();
   }
 };
}

#endif // POSEIDON_NEW_PAGE_H