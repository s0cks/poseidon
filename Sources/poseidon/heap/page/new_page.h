#ifndef POSEIDON_NEW_PAGE_H
#define POSEIDON_NEW_PAGE_H

#include "poseidon/flags.h"
#include "poseidon/heap/page/page.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class NewPage;
 class NewPageVisitor {
  protected:
   NewPageVisitor() = default;
  public:
   virtual ~NewPageVisitor() = default;
   virtual bool Visit(NewPage* page) = 0;
 };

 class NewZone;
 class NewPage : public Page {
   friend class Zone;

   friend class NewZone;
   friend class NewPageTest;
   friend class SerialMarkerTest;
  public:
   class NewPageIterator : public RawObjectPointerIterator {
    private:
     const NewPage* page_;
     uword current_;

     inline const NewPage* page() const {
       return page_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline Pointer* current_ptr() const {
       return (Pointer*) current_address();
     }
    public:
     explicit NewPageIterator(const NewPage* page):
       RawObjectPointerIterator(),
       page_(page),
       current_(page->GetStartingAddress()) {
     }
     ~NewPageIterator() override = default;

     bool HasNext() const override {
       return current_address() > 0 &&
              current_address() < page()->GetEndingAddress() &&
              current_ptr()->IsNew() &&
              !current_ptr()->IsFree() &&
              current_ptr()->GetSize() > 0;
     }

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  public: //TODO: reduce access
   explicit NewPage(const PageIndex index = 0, const uword start_address = 0, const PageSize size = flags::GetNewPageSize()):
     Page(PageTag::New(index, size), start_address) {
   }
  public:
   NewPage(const NewPage& rhs) = default;
   ~NewPage() override = default;

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   word GetSize() const override {
     return flags::GetNewPageSize();
   }

   NewPage& operator=(const NewPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const NewPage& value) {
     stream << "NewPage(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(value.GetSize());
     stream << ")";
     return stream;
   }

   friend bool operator==(const NewPage& lhs, const NewPage& rhs) {
     return lhs.GetStartingAddress() == rhs.GetStartingAddress() &&
            lhs.GetSize() == rhs.GetSize();
   }

   friend bool operator!=(const NewPage& lhs, const NewPage& rhs) {
     return !operator==(lhs, rhs);
   }
 };
}

#endif // POSEIDON_NEW_PAGE_H