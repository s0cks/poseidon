#ifndef POSEIDON_OLD_PAGE_H
#define POSEIDON_OLD_PAGE_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/common.h"
#include "page.h"
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
   friend class Zone;

   friend class OldZone;
   friend class OldPageTest;
   friend class SerialSweeperTest;
  public:
   OldPage() = default;
   explicit OldPage(const PageIndex index, const uword start, const ObjectSize size):
     Page(index, start, size) {
   }
   OldPage(const OldPage& rhs) = default;
   ~OldPage() override = default;

   bool VisitPointers(RawObjectVisitor* vis) const override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) const override;

   OldPage& operator=(const OldPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const OldPage& value) {
     stream << "OldPage(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(value.GetSize()) << ", ";
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_OLD_PAGE_H