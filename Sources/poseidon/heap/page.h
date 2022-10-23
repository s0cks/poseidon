#ifndef POSEIDON_HEAP_PAGE_H
#define POSEIDON_HEAP_PAGE_H

#include <ostream>

#include "poseidon/utils.h"
#include "poseidon/heap/section.h"

namespace poseidon{
 typedef uword RawPageTag;

 static constexpr const RawPageTag kInvalidPageTag = 0x0;

 class PageTag {
  public:
   enum Layout {
     kMarkedBitOffset = 0,
     kBitsForMarkedBit = 1,

     kIndexTagOffset = kMarkedBitOffset + kBitsForMarkedBit,
     kBitsForIndexTag = 16,

     kSizeTagOffset = kIndexTagOffset + kBitsForIndexTag,
     kBitsForSizeTag = 32,

     kTotalBits = kBitsForMarkedBit + kBitsForIndexTag + kBitsForSizeTag,
   };

   class MarkedBit : public BitField<RawPageTag, bool, kMarkedBitOffset, kBitsForMarkedBit>{};
   class IndexTag : public BitField<RawPageTag, uint32_t, kIndexTagOffset, kBitsForIndexTag>{};
   class SizeTag : public BitField<RawPageTag, uint32_t, kSizeTagOffset, kBitsForSizeTag>{};

   static inline int
   Compare(const PageTag& lhs, const PageTag& rhs) {
     if(lhs.GetIndex() < rhs.GetIndex())
       return -1;
     else if(lhs.GetIndex() > rhs.GetIndex())
       return +1;

     if(lhs.GetSize() < rhs.GetSize())
       return -1;
     else if(lhs.GetSize() > rhs.GetSize())
       return +1;
     return 0;
   }
  private:
   RawPageTag raw_;
  public:
   explicit PageTag(const RawPageTag raw = kInvalidPageTag):
    raw_(raw) {
   }
   PageTag(const PageTag& rhs) = default;
   ~PageTag() = default;

   RawPageTag raw() const {
     return raw_;
   }

   bool IsMarked() const {
     return MarkedBit::Decode(raw());
   }

   void SetMarkedBit() {
     raw_ = MarkedBit::Update(true, raw());
   }

   void ClearMarkedBit() {
     raw_ = MarkedBit::Update(false, raw());
   }

   uint32_t GetIndex() const {
     return IndexTag::Decode(raw());
   }

   void SetIndex(const uint32_t val) {
     raw_ = IndexTag::Update(val, raw());
   }

   uint32_t GetSize() const {
     return SizeTag::Decode(raw());
   }

   void SetSize(const uint32_t val) {
     raw_ = SizeTag::Update(val, raw());
   }

   PageTag& operator=(const PageTag& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const PageTag& val) {
     stream << "PageTag(";
     stream << "marked=" << (val.IsMarked() ? "Y" : "N") << ", ";
     stream << "index=" << val.GetIndex() << ", ";
     stream << "size=" << val.GetSize();
     return stream << ")";
   }

   friend bool operator==(const PageTag& lhs, const PageTag& rhs) {
     return lhs.raw() == rhs.raw();
   }

   friend bool operator!=(const PageTag& lhs, const PageTag& rhs) {
     return lhs.raw() != rhs.raw();
   }

   friend bool operator<(const PageTag& lhs, const PageTag& rhs) {
     return Compare(lhs, rhs) < 0;
   }

   friend bool operator>(const PageTag& lhs, const PageTag& rhs) {
     return Compare(lhs, rhs) > 0;
   }
 };

 class Page : public AllocationSection{
  protected:
   Page(int64_t index, uword start, int64_t size):
    AllocationSection(start, size) {

   }

   void set_raw_tag(const RawPageTag val) {
     *((uword*) GetStartingAddress()) = val;
   }

   void set_tag(const PageTag& tag) {
     set_raw_tag(tag.raw());
   }

   void SetMarkedBit() {
     tag().SetMarkedBit();
   }

   void ClearMarkedBit() {
     tag().ClearMarkedBit();
   }
  public:
   Page() = default;
   Page(const Page& rhs) = default;
   ~Page() override = default;

   RawPageTag raw_tag() const {
     return GetStartingAddress();
   }

   PageTag tag() const {
     return PageTag(raw_tag());
   }

   bool IsMarked() const {
     return tag().IsMarked();
   }

   uint32_t index() const {
     return tag().GetIndex();
   }

   uint32_t size() const {
     return tag().GetSize();
   }

   Page& operator=(const Page& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const Page& val){
     stream << "Page(";
     stream << "tag=" << val.tag() << ", ";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize());
     return stream << ")";
   }

   friend bool operator==(const Page& lhs, const Page& rhs){
     return lhs.tag() == rhs.tag()
         && lhs.GetStartingAddress() == rhs.GetStartingAddress()
         && lhs.GetSize() == rhs.GetSize();
   }

   friend bool operator!=(const Page& lhs, const Page& rhs){
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const Page& lhs, const Page& rhs){
     return lhs.tag().GetIndex() < rhs.tag().GetIndex();
   }

   friend bool operator>(const Page& lhs, const Page& rhs) {
     return lhs.tag().GetIndex() > rhs.tag().GetIndex();
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H