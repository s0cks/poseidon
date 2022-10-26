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
   virtual bool Visit(Page* page) = 0;
 };

 class Page : public AllocationSection {
   friend class PageTable;
   friend class PageTest;
  protected:
   MemoryRegion region_;
   RelaxedAtomic<RawPageTag> tag_;

   Page() = default;

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
     return Section::GetSize();
   }
  public:
   explicit Page(const PageIndex index, const MemoryRegion& region):
    AllocationSection(region.GetStartingAddress(), region.GetSize()),
    tag_(),
    region_(region) {
     ClearMarkedBit();
     ClearNewBit();
     ClearOldBit();
     SetIndex(index);
   }
   ~Page() override = default;

   RawPageTag raw_tag() const {
     return (RawPageTag)tag_;
   }

   PageTag tag() const {
     return PageTag(raw_tag());
   }

   bool marked() const {
     return PageTag::MarkedBit::Decode(raw_tag());
   }

   PageIndex index() const {
     return PageTag::IndexTag::Decode(raw_tag());
   }

   uword TryAllocate(ObjectSize size) override; //TODO: make only accessible to page table

   Page& operator=(const Page& rhs) {
     if(this == &rhs)
       return *this;
     AllocationSection::operator=(rhs);
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
     return stream << ")";
   }

   friend bool operator==(const Page& lhs, const Page& rhs) {
     return lhs.tag() == rhs.tag()
         && ((const AllocationSection&)lhs) == ((const AllocationSection&)rhs);
   }

   friend bool operator!=(const Page& lhs, const Page& rhs) {
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const Page& lhs, const Page& rhs) {
     return lhs.tag() < rhs.tag();
   }

   friend bool operator>(const Page& lhs, const Page& rhs) {
     return lhs.tag() > rhs.tag();
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H