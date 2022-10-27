#ifndef POSEIDON_HEAP_PAGE_H
#define POSEIDON_HEAP_PAGE_H

#include <ostream>

#include "poseidon/heap/section.h"
#include "poseidon/heap/page_tag.h"
#include "poseidon/relaxed_atomic.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Page;
 class PageVisitor {
  protected:
   PageVisitor() = default;
  public:
   virtual ~PageVisitor() = default;
   virtual bool VisitPage(Page* page) = 0;
 };

 class Page : public AllocationSection {
   friend class PageTable;
   friend class NewZone;
   friend class OldZone;

   friend class RawObject;//TODO: remove
   friend class NewPageTest; //TODO: remove
   friend class OldPageTest; //TODO: remove
  public:
   static inline constexpr ObjectSize
   CalculatePageSize(const ObjectSize size) {
     return size + kWordSize;
   }
  protected:
   template<class T>
   class PageIterator : public RawObjectPointerIterator {
    protected:
     T* page_;
     uword current_address_;

     inline T* page() const {
       return page_;
     }

     inline uword current_address() const {
       return current_address_;
     }

     inline RawObject* current_ptr() const {
       return (RawObject*)current_address();
     }
    public:
     explicit PageIterator(T* page):
       page_(page),
       current_address_(page->GetStartingAddress()) {
     }
     ~PageIterator() override = default;

     RawObject* Next() override {
       auto next = current_ptr();
       current_address_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   RelaxedAtomic<RawPageTag> tag_;
   MemoryRegion region_;
   uword current_;

   inline void
   SetTag(const RawPageTag tag) {
     tag_ = tag;
   }

   inline void
   SetMarkedBit(const bool value = true) {
     SetTag(PageTag::MarkedBit::Update(value, raw_tag()));
   }

   inline void
   ClearMarkedBit() {
     return SetMarkedBit(false);
   }

   inline void
   SetNewBit(const bool value = true) {
     SetTag(PageTag::NewBit::Update(value, raw_tag()));
   }

   inline void
   ClearNewBit() {
     return SetNewBit(false);
   }

   inline void
   SetOldBit(const bool value = true) {
     SetTag(PageTag::OldBit::Update(value, raw_tag()));
   }

   inline void
   ClearOldBit() {
     return SetOldBit(false);
   }

   inline void
   SetIndex(const PageIndex value) {
     SetTag(PageTag::IndexTag::Update(value, raw_tag()));
   }

   inline int64_t
   GetAllocatableSize() {
     return GetSize();
   }

   uword TryAllocate(ObjectSize size) override;
   void Clear() override;
  public:
   Page() = default;
   explicit Page(const PageIndex index, const MemoryRegion& region):
    AllocationSection(),
    tag_(),
    current_(region.GetStartingAddress()),
    region_(region) {
     SetIndex(index);
   }
   Page(const Page& rhs) = default;
   ~Page() override = default;

   uword GetStartingAddress() const override {
     return region_.GetStartingAddress();
   }

   uword GetCurrentAddress() const override {
     return current_;
   }

   int64_t GetSize() const override {
     return region_.GetSize();
   }

   RawPageTag raw_tag() const {
     return (RawPageTag)tag_;
   }

   PageTag tag() const {
     return { raw_tag() };
   }

   bool marked() const {
     return PageTag::MarkedBit::Decode(raw_tag());
   }

   PageIndex index() const {
     return PageTag::IndexTag::Decode(raw_tag());
   }

   Page& operator=(const Page& rhs) {
     if(this == &rhs)
       return *this;
     AllocationSection::operator=(rhs);
     region_ = rhs.region_;
     current_ = rhs.current_;
     tag_ = rhs.raw_tag();
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Page& val) {
     stream << "Page(";
     stream << "index=" << val.index() << ", ";
     stream << "marked=" << val.marked() << ", ";
     stream << "start" << val.GetStartingAddressPointer() << ", ";
     stream << "current=" << val.GetCurrentAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer();
     stream << ")";
     return stream;
   }

   friend bool operator==(const Page& lhs, const Page& rhs) {
     return ((const AllocationSection&)lhs) == ((const AllocationSection&)rhs) &&
            lhs.raw_tag() == rhs.raw_tag();
   }

   friend bool operator!=(const Page& lhs, const Page& rhs) {
     return !operator==(lhs, rhs);
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H