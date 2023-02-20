#ifndef POSEIDON_PAGE_TAG_H
#define POSEIDON_PAGE_TAG_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/platform/platform.h"

namespace poseidon {
 typedef uint64_t RawPageTag;
 typedef word PageIndex;
 typedef word PageSize;

 static constexpr const RawPageTag kInvalidPageTag = 0x0;

 class PageTag { //TODO: add tests
   friend class Page;
   friend class PageTagTest;
  public:
   enum Layout {
     kNewBitOffset = 0,
     kBitsForNewBit = 1,

     kOldBitOffset = kNewBitOffset + kBitsForNewBit,
     kBitsForOldBit = 1,

     kIndexOffset = kOldBitOffset + kBitsForOldBit,
     kBitsForIndex = 16,

     kSizeOffset = kIndexOffset + kBitsForIndex,
     kBitsForSize = 32,

     kTotalBits = kBitsForNewBit + kBitsForOldBit + kBitsForIndex + kBitsForSize,
   };
  protected:
   class NewBit : public BitField<RawPageTag, ObjectSize, kNewBitOffset, kBitsForNewBit>{};
   class OldBit : public BitField<RawPageTag, ObjectSize, kOldBitOffset, kBitsForOldBit>{};
   class Index : public BitField<RawPageTag, PageIndex, kIndexOffset, kBitsForIndex>{};
   class Size : public BitField<RawPageTag, ObjectSize, kSizeOffset, kBitsForSize>{};

   RawPageTag raw_;

   inline void set_raw(const RawPageTag& value) {
     raw_ = value;
   }

   inline void SetNew(const bool value = true) {
     return set_raw(NewBit::Update(value, raw()));
   }

   inline void ClearNew() {
     return SetNew(false);
   }

   inline void SetOld(const bool value = true) {
     return set_raw(OldBit::Update(value, raw()));
   }

   inline void ClearOld() {
     return SetOld(false);
   }

   inline void SetIndex(const PageIndex& value) {
     return set_raw(Index::Update(value, raw()));
   }

   inline void SetSize(const PageSize& value) {
     return set_raw(Size::Update(value, raw()));
   }
  public:
   explicit constexpr PageTag(const RawPageTag raw = kInvalidPageTag):
    raw_(raw) {
   }
   PageTag(const PageTag& rhs) = default;
   ~PageTag() = default;

   RawPageTag raw() const {
     return raw_;
   }

   bool IsNew() const {
     return NewBit::Decode(raw());
   }

   bool IsOld() const {
     return OldBit::Decode(raw());
   }

   PageIndex GetIndex() const {
     return Index::Decode(raw());
   }

   PageSize GetSize() const {
     return Size::Decode(raw());
   }

   PageTag& operator=(const PageTag& rhs) {
     if(&rhs == this)
       return *this;
     raw_ = rhs.raw();
     return *this;
   }

   PageTag& operator=(const RawPageTag& rhs) {
     if(raw_ == rhs)
       return *this;
     raw_ = rhs;
     return *this;
   }

   friend bool operator==(const PageTag& lhs, const PageTag& rhs) {
     return lhs.raw() == rhs.raw();
   }

   friend bool operator!=(const PageTag& lhs, const PageTag& rhs) {
     return lhs.raw() != rhs.raw();
   }

   friend std::ostream& operator<<(std::ostream& stream, const PageTag& value) {
     stream << "PageTag(";
     stream << "new=" << value.IsNew() << ", ";
     stream << "old=" << value.IsOld() << ", ";
     stream << "index=" << value.GetIndex() << ", ";
     stream << "size=" << Bytes(value.GetSize());
     stream << ")";
     return stream;
   }
  public:
   static inline constexpr RawPageTag
   Empty() {
     return kInvalidPageTag;
   }

   static inline constexpr RawPageTag
   New(const PageIndex& index, const PageSize& size = flags::GetNewPageSize()) {
     return Empty() | NewBit::Encode(true) | Index::Encode(index) | Size::Encode(size);
   }

   static inline constexpr RawPageTag
   Old(const PageIndex& index, const PageSize& size = flags::GetOldPageSize()) {
     return Empty() | OldBit::Encode(true) | Index::Encode(index) | Size::Encode(size);
   }
 };
}

#endif // POSEIDON_PAGE_TAG_H