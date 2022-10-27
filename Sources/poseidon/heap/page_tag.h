#ifndef POSEIDON_PAGE_TAG_H
#define POSEIDON_PAGE_TAG_H

#include "poseidon/utils.h"

namespace poseidon {
 typedef int64_t PageIndex;

 typedef uword RawPageTag;

 static constexpr const RawPageTag kInvalidPageTag = 0x0;

 class PageTag {
   friend class PageTagTest;
  public:
   enum Layout {
     kMarkedBitOffset = 0,
     kBitsForMarkedBit = 1,

     kNewBitOffset = kMarkedBitOffset + kBitsForMarkedBit,
     kBitsForNewBit = 1,

     kOldBitOffset = kNewBitOffset + kBitsForNewBit,
     kBitsForOldBit = 1,

     kIndexTagOffset = kOldBitOffset + kBitsForOldBit,
     kBitsForIndexTag = 8,

     kSizeTagOffset = kIndexTagOffset + kBitsForIndexTag,
     kBitsForSizeTag = 32,

     kTotalBits = kBitsForMarkedBit + kBitsForNewBit + kBitsForOldBit + kBitsForIndexTag + kBitsForSizeTag,
   };

   class MarkedBit : public BitField<RawPageTag, bool, kMarkedBitOffset, kBitsForMarkedBit>{};
   class NewBit : public BitField<RawPageTag, bool, kNewBitOffset, kBitsForNewBit>{};
   class OldBit : public BitField<RawPageTag, bool, kOldBitOffset, kBitsForOldBit>{};
   class IndexTag : public BitField<RawPageTag, PageIndex, kIndexTagOffset, kBitsForIndexTag>{};
   class SizeTag : public BitField<RawPageTag, ObjectSize, kSizeTagOffset, kBitsForSizeTag>{};
  private:
   RawPageTag raw_;

   inline void SetMarkedBit(const bool value = true) {
     raw_ = MarkedBit::Update(value, raw());
   }

   inline void ClearMarkedBit() {
     return SetMarkedBit(false);
   }

   inline void SetNewBit(const bool value = true) {
     raw_ = NewBit::Update(value, raw());
   }

   inline void ClearNewBit() {
     return SetNewBit(false);
   }

   inline void SetOldBit(const bool value = true) {
     raw_ = OldBit::Update(value, raw());
   }

   inline void ClearOldBit() {
     return SetOldBit(false);
   }

   inline void SetIndex(const PageIndex value) {
     raw_ = IndexTag::Update(value, raw());
   }

   inline void SetSize(const ObjectSize value) {
     raw_ = SizeTag::Update(value, raw());
   }
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

   bool IsNew() const {
     return NewBit::Decode(raw());
   }

   bool IsOld() const {
     return OldBit::Decode(raw());
   }

   PageIndex GetIndex() const {
     return IndexTag::Decode(raw());
   }

   ObjectSize GetSize() const {
     return SizeTag::Decode(raw());
   }

   explicit operator RawPageTag() const {
     return raw();
   }

   PageTag& operator=(const PageTag& rhs) = default;

   PageTag& operator=(const RawPageTag& rhs) {
     if(raw_ == rhs)
       return *this;
     raw_ = rhs;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const PageTag& val) {
     stream << "PageTag(";
     stream << "marked=" << val.IsMarked() << ", ";
     stream << "new=" << val.IsNew() << ", ";
     stream << "old=" << val.IsOld() << ", ";
     stream << "index=" << val.GetIndex() << ", ";
     stream << "size=" << val.GetSize();
     stream << ")";
     return stream;
   }

   friend bool operator==(const PageTag& lhs, const PageTag& rhs) {
     return lhs.raw() == rhs.raw();
   }

   friend bool operator!=(const PageTag& lhs, const PageTag& rhs) {
     return lhs.raw() != rhs.raw();
   }
  public:
   static inline constexpr RawPageTag
   Empty() {
     return kInvalidPageTag;
   }

   static inline constexpr RawPageTag
   New(const PageIndex& index = 0, const ObjectSize& size = 0) {
     return Empty() | NewBit::Encode(true) | IndexTag::Encode(index) | SizeTag::Encode(size);
   }

   static inline constexpr RawPageTag
   NewMarked(const PageIndex& index = 0, const ObjectSize size = 0) {
     return New(index, size) | MarkedBit::Encode(true);
   }

   static inline constexpr RawPageTag
   Old(const PageIndex& index = 0, const ObjectSize size = 0) {
     return Empty() | OldBit::Encode(true) | IndexTag::Encode(index) | SizeTag::Encode(size);
   }

   static inline constexpr RawPageTag
   OldMarked(const PageIndex& index = 0, const ObjectSize size = 0) {
     return Old(index, size) | MarkedBit::Encode(true);
   }
 };
}

#endif // POSEIDON_PAGE_TAG_H