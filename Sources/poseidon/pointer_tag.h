#ifndef POSEIDON_OBJECT_TAG_H
#define POSEIDON_OBJECT_TAG_H

#include "poseidon/type.h"
#include "poseidon/utils.h"
#include "poseidon/platform/platform.h"

namespace poseidon {
 typedef uword RawPointerTag;

 static const constexpr RawPointerTag kInvalidPointerTag = 0x0;

#define FOR_EACH_POINTER_TAG_BIT(V) \
 V(New)                              \
 V(Old)                              \
 V(Marked)                           \
 V(Remembered)                       \
 V(Free)

 class PointerTag{
  public:
   enum Layout{
     // NewBit
     kNewBitOffset = 0,
     kBitsForNewBit = 1,

     // OldBit
     kOldBitOffset = kNewBitOffset+kBitsForNewBit,
     kBitsForOldBit = 1,

     // MarkedBit
     kMarkedBitOffset = kOldBitOffset+kBitsForNewBit,
     kBitsForMarkedBit = 1,

     // RememberedBit
     kRememberedBitOffset = kMarkedBitOffset+kBitsForMarkedBit,
     kBitsForRememberedBit = 1,

     // FreeBit
     kFreeBitOffset = kRememberedBitOffset + kBitsForRememberedBit,
     kBitsForFreeBit = 1,

     // TypeId
     kTypeIdOffset = kFreeBitOffset + kBitsForFreeBit,
     kBitsForTypeId = 8,

     // Size
     kSizeTagOffset = kTypeIdOffset + kBitsForTypeId,
     kBitsForSizeTag = 32,

     kTotalBits = kBitsForNewBit + kBitsForOldBit + kBitsForMarkedBit + kBitsForRememberedBit + kBitsForFreeBit + kBitsForTypeId + kBitsForSizeTag,
   };

   // The object's size.
   class SizeTag : public BitField<RawPointerTag, int64_t, kSizeTagOffset, kBitsForSizeTag>{};
   // allocated in the new heap.
   class NewBit : public BitField<RawPointerTag, bool, kNewBitOffset, kBitsForNewBit>{};
   // allocated in the old heap.
   class OldBit : public BitField<RawPointerTag, bool, kOldBitOffset, kBitsForOldBit>{};
   // marked by the scavenger
   class MarkedBit : public BitField<RawPointerTag, bool, kMarkedBitOffset, kBitsForMarkedBit>{};
   // remembered by the scavenger
   class RememberedBit : public BitField<RawPointerTag, bool, kRememberedBitOffset, kBitsForRememberedBit>{};
   // is free space
   class FreeBit : public BitField<RawPointerTag, bool, kFreeBitOffset, kBitsForFreeBit>{};
   // typeid
   class TypeIdBits : public BitField<RawPointerTag, TypeId, kTypeIdOffset, kBitsForTypeId>{};
  private:
   RawPointerTag raw_;

   inline void set_raw(const RawPointerTag& raw) {
     raw_ = raw;
   }
  public:
   constexpr PointerTag(const RawPointerTag raw = kInvalidPointerTag): // NOLINT(google-explicit-constructor)
     raw_(raw) { }
   PointerTag(const PointerTag& rhs) = default;
   ~PointerTag() = default;

   RawPointerTag raw() const{
     return raw_;
   }

#define DEFINE_SET_BIT(Name) \
   inline void Set##Name(const bool val = true) { return set_raw(Name##Bit::Update(raw(), val)); }
#define DEFINE_CLEAR_BIT(Name) \
   inline void Clear##Name() { return Set##Name(false); }
#define DEFINE_IS(Name) \
   inline bool Is##Name() const { return Name##Bit::Decode(raw()); }
   FOR_EACH_POINTER_TAG_BIT(DEFINE_SET_BIT);
   FOR_EACH_POINTER_TAG_BIT(DEFINE_CLEAR_BIT);
   FOR_EACH_POINTER_TAG_BIT(DEFINE_IS);
#undef DEFINE_SET_BIT
#undef DEFINE_CLEAR_BIT
#undef DEFINE_IS

   void SetSize(int64_t size){
     raw_ = SizeTag::Update(size, raw());
   }

   inline void ClearSize() {
     return SetSize(0);
   }

   int64_t GetSize() const{
     return SizeTag::Decode(raw());
   }

   TypeId GetTypeId() const {
     return TypeIdBits::Decode(raw());
   }

   inline void SetTypeId(const TypeId val) {
     return set_raw(TypeIdBits::Update(val, raw()));
   }

   inline void ClearTypeId() {
     return SetTypeId(TypeId::kUnknownTypeId);
   }

   explicit operator RawPointerTag() const{
     return raw();
   }

   PointerTag& operator=(const PointerTag& rhs) = default;

   PointerTag& operator=(const RawPointerTag& rhs){
     if(raw_ == rhs)
       return *this;
     raw_ = rhs;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const PointerTag& val){
     stream << "PointerTag(";
     stream << "new=" << val.IsNew() << ", ";
     stream << "old=" << val.IsOld() << ", ";
     stream << "marked=" << val.IsMarked() << ", ";
     stream << "remembered=" << val.IsRemembered() << ", ";
     stream << "free=" << val.IsFree() << ", ";
     stream << "size=" << Bytes(val.GetSize());
     stream << ")";
     return stream;
   }

   friend bool operator==(const PointerTag& lhs, const PointerTag& rhs) {
     return lhs.raw() == rhs.raw();
   }

   friend bool operator!=(const PointerTag& lhs, const PointerTag& rhs) {
     return lhs.raw() != rhs.raw();
   }
  public:
   static inline constexpr RawPointerTag
   Empty() {
     return kInvalidPointerTag;
   }

   static inline constexpr RawPointerTag
   New(const ObjectSize size) {
     return Empty() | NewBit::Encode(true) | SizeTag::Encode(size);
   }

   static inline constexpr RawPointerTag
   NewMarked(const ObjectSize size) {
     return New(size) | MarkedBit::Encode(true);
   }

   static inline constexpr RawPointerTag
   NewRemembered(const ObjectSize size) {
     return New(size) | RememberedBit::Encode(true);
   }

   static inline constexpr RawPointerTag
   Old(const ObjectSize size) {
     return Empty() | OldBit::Encode(true) | SizeTag::Encode(size);
   }

   static inline constexpr RawPointerTag
   OldMarked(const ObjectSize size) {
     return Old(size) | MarkedBit::Encode(true);
   }

   static inline constexpr RawPointerTag
   OldRemembered(const ObjectSize size) {
     return Old(size) | RememberedBit::Encode(true);
   }

   static inline constexpr RawPointerTag
   OldFree(const ObjectSize size) {
     return Old(size) | FreeBit::Encode(true);
   }

   static inline constexpr RawPointerTag
   Free(const ObjectSize size) {
     return Empty() | SizeTag::Encode(size) | FreeBit::Encode(true);
   }
 };
}

#endif // POSEIDON_OBJECT_TAG_H