#ifndef POSEIDON_PAGE_TAG_H
#define POSEIDON_PAGE_TAG_H

#include "poseidon/utils.h"

namespace poseidon {
 typedef int64_t PageIndex;
 typedef uword RawPageTag;

 static constexpr const RawPageTag kInvalidPageTag = 0x0;

 class PageTag {
  public:
   enum Layout {
     kMarkedBitOffset = 0,
     kBitsForMarkedBit = 1,

     kNewBitOffset = kMarkedBitOffset + kBitsForMarkedBit,
     kBitsForNewBit = 1,

     kOldBitOffset = kNewBitOffset + kBitsForNewBit,
     kBitsForOldBit = 1,

     kIndexTagOffset = kOldBitOffset + kBitsForOldBit,
     kBitsForIndexTag = 32,

     kTotalBits = kBitsForMarkedBit + kBitsForNewBit + kBitsForOldBit + kBitsForIndexTag,
   };

   class MarkedBit : public BitField<RawPageTag, bool, kMarkedBitOffset, kBitsForMarkedBit>{};
   class NewBit : public BitField<RawPageTag, bool, kNewBitOffset, kBitsForNewBit>{};
   class OldBit : public BitField<RawPageTag, bool, kOldBitOffset, kBitsForOldBit>{};
   class IndexTag : public BitField<RawPageTag, PageIndex, kIndexTagOffset, kBitsForIndexTag>{};

   static inline int
   Compare(const PageTag& lhs, const PageTag& rhs) {
     if(lhs.GetIndex() < rhs.GetIndex())
       return -1;
     else if(lhs.GetIndex() > rhs.GetIndex())
       return +1;
     return 0;
   }
  private:
   RawPageTag raw_;
  public:
   constexpr PageTag(const RawPageTag raw = kInvalidPageTag):
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

   void SetMarkedBit(const bool value = true) {
     raw_ = MarkedBit::Update(value, raw());
   }

   inline void
   ClearMarkedBit() {
     return SetMarkedBit(false);
   }

   bool IsNew() const {
     return NewBit::Decode(raw());
   }

   void SetNewBit(const bool value = true) {
     raw_ = NewBit::Update(value, raw());
   }

   inline void
   ClearNewBit() {
     return SetNewBit(false);
   }

   bool IsOld() const {
     return OldBit::Decode(raw());
   }

   void SetOldBit(const bool value = true) {
     raw_ = OldBit::Update(value, raw());
   }

   inline void
   ClearOldBit() {
     return SetOldBit(false);
   }

   PageIndex GetIndex() const {
     return IndexTag::Decode(raw());
   }

   void SetIndex(const PageIndex val) {
     raw_ = IndexTag::Update(val, raw());
   }

   PageTag& operator=(const PageTag& rhs) {
     if(this == &rhs)
       return *this;
     raw_ = rhs.raw();
     return *this;
   }

   PageTag& operator=(const RawPageTag& rhs) {
     if(raw() == rhs)
       return *this;
     raw_ = rhs;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const PageTag& val) {
     stream << "PageTag(";
     stream << "marked=" << (val.IsMarked() ? "Y" : "N") << ", ";
     stream << "index=" << val.GetIndex();
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

   static inline constexpr RawPageTag
   Index(const PageIndex index) {
     return PageTag::IndexTag::Encode(index);
   }

   static inline constexpr RawPageTag
   Marked(const PageIndex index) {
     return Index(index) | PageTag::MarkedBit::Encode(true);
   }

   static inline constexpr RawPageTag
   Unmarked(const PageIndex index) {
     return Index(index) | PageTag::MarkedBit::Encode(false);
   }

   static inline constexpr RawPageTag
   NewMarked(const PageIndex index) {
     return Marked(index) | PageTag::NewBit::Encode(true);
   }

   static inline constexpr RawPageTag
   NewUnmarked(const PageIndex index) {
     return Unmarked(index) | PageTag::NewBit::Encode(true);
   }

   static inline constexpr RawPageTag
   OldMarked(const PageIndex index) {
     return Marked(index) | PageTag::OldBit::Encode(true);
   }

   static inline constexpr RawPageTag
   OldUnmarked(const PageIndex index) {
     return Unmarked(index) | PageTag::OldBit::Encode(false);
   }
 };
}

#endif // POSEIDON_PAGE_TAG_H