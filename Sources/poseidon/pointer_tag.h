#ifndef POSEIDON_OBJECT_TAG_H
#define POSEIDON_OBJECT_TAG_H

#include "poseidon/utils.h"
#include "poseidon/platform/platform.h"

namespace poseidon {
 typedef uword RawPointerTag;

 static const constexpr RawPointerTag kInvalidPointerTag = 0x0;

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

     // Size
     kSizeTagOffset = kFreeBitOffset + kBitsForFreeBit,
     kBitsForSizeTag = 32,

     kTotalBits = kBitsForNewBit + kBitsForOldBit + kBitsForMarkedBit + kBitsForRememberedBit + kBitsForFreeBit + kBitsForSizeTag,
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
  private:
   RawPointerTag raw_;
  public:
   constexpr PointerTag(const RawPointerTag raw = kInvalidPointerTag):
     raw_(raw) {
   }
   PointerTag(const PointerTag& rhs) = default;
   ~PointerTag() = default;

   RawPointerTag raw() const{
     return raw_;
   }

   void SetNew(){
     raw_ = NewBit::Update(true, raw());
   }

   void ClearNew(){
     raw_ = NewBit::Update(false, raw());
   }

   bool IsNew() const{
     return NewBit::Decode(raw());
   }

   void SetOld(){
     raw_ = OldBit::Update(true, raw());
   }

   void ClearOld(){
     raw_ = OldBit::Update(false, raw());
   }

   bool IsOld() const{
     return OldBit::Decode(raw());
   }

   void SetMarked(){
     raw_ = MarkedBit::Update(true, raw());
   }

   void ClearMarked(){
     raw_ = MarkedBit::Update(false, raw());
   }

   bool IsMarked() const{
     return MarkedBit::Decode(raw());
   }

   void SetRemembered(){
     raw_ = RememberedBit::Update(true, raw());
   }

   void ClearRemembered(){
     raw_ = RememberedBit::Update(false, raw());
   }

   bool IsRemembered() const{
     return RememberedBit::Decode(raw());
   }

   void SetFreeBit(const bool value = true) {
     raw_ = FreeBit::Update(value, raw());
   }

   inline void ClearFreeBit() {
     return SetFreeBit(false);
   }

   bool IsFree() const {
     return FreeBit::Decode(raw());
   }

   void SetSize(int64_t size){
     raw_ = SizeTag::Update(size, raw());
   }

   inline void ClearSize() {
     return SetSize(0);
   }

   int64_t GetSize() const{
     return SizeTag::Decode(raw());
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