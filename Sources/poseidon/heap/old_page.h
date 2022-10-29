#ifndef POSEIDON_OLD_PAGE_H
#define POSEIDON_OLD_PAGE_H

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/common.h"
#include "poseidon/heap/page.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class OldPage;
 class OldPageVisitor : public PageVisitor {
  public:
   OldPageVisitor() = default;
   ~OldPageVisitor() override = default;
   virtual bool VisitOldPage(OldPage* page) = 0;
 };

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
   OldPage() = default;
  public:
   OldPage(const OldPage& rhs) = delete;
   ~OldPage() override = default;
   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;
 };
}

#endif//POSEIDON_OLD_PAGE_H