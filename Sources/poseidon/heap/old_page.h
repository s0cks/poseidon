#ifndef POSEIDON_OLD_PAGE_H
#define POSEIDON_OLD_PAGE_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/common.h"
#include "poseidon/heap/page/page.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class OldPage;
 class OldPageVisitor {
  public:
   OldPageVisitor() = default;
   virtual ~OldPageVisitor() = default;
   virtual bool Visit(OldPage* page) = 0;
 };

 class OldZone;
 class OldPage : public Page{
   template<class P>
   friend class Zone;

   friend class OldZone;
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
              current_ptr()->GetSize() > 0;
     }
   };
  public:
   OldPage() = default;
   explicit OldPage(const PageIndex index, const uword start, const ObjectSize size):
     Page(PageTag::Old(index, size), start) {
   }
   OldPage(const OldPage& rhs) = default;
   ~OldPage() override = default;

   PageTag tag() const {
     return (PageTag)raw_tag();
   }

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   OldPage& operator=(const OldPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const OldPage& value) {
     stream << "OldPage(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(value.GetSize()) << ", ";
     stream << "tag=" << value.tag();
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_OLD_PAGE_H