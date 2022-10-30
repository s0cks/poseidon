#ifndef POSEIDON_OLD_PAGE_H
#define POSEIDON_OLD_PAGE_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/common.h"
#include "poseidon/heap/page.h"
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
              current_ptr()->GetPointerSize() > 0;
     }
   };
  protected:
   OldPage() = default;
  public:
   OldPage(const OldPage& rhs) = delete;
   ~OldPage() override = default;

   int64_t GetSize() const override {
     return GetOldPageSize();
   }

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   friend std::ostream& operator<<(std::ostream& stream, const OldPage& value) {
     stream << "OldPage(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << value.GetSize();
     stream << ")";
     return stream;
   }

   static inline OldPage* From(const uword start_address) {
     return new ((void*) start_address)OldPage();
   }
 };
}

#endif//POSEIDON_OLD_PAGE_H