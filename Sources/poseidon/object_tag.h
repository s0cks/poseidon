#ifndef POSEIDON_OBJECT_TAG_H
#define POSEIDON_OBJECT_TAG_H

#include "poseidon/utils.h"
#include "poseidon/platform/platform.h"

namespace poseidon {
 typedef uword RawObjectTag;

 static const constexpr RawObjectTag kInvalidObjectTag = 0x0;

 class ObjectTag{
  private:
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

     // Size
     kSizeTagOffset = kRememberedBitOffset+kBitsForRememberedBit,
     kBitsForSizeTag = 32,

     kTotalBits = kBitsForNewBit + kBitsForOldBit + kBitsForMarkedBit + kBitsForRememberedBit + kBitsForSizeTag,
   };
  public:
   // The object's size.
   class SizeTag : public BitField<RawObjectTag, int64_t, kSizeTagOffset, kBitsForSizeTag>{};
   // allocated in the new heap.
   class NewBit : public BitField<RawObjectTag, bool, kNewBitOffset, kBitsForNewBit>{};
   // allocated in the old heap.
   class OldBit : public BitField<RawObjectTag, bool, kOldBitOffset, kBitsForOldBit>{};
   // marked by the scavenger
   class MarkedBit : public BitField<RawObjectTag, bool, kMarkedBitOffset, kBitsForMarkedBit>{};
   // remembered by the scavenger
   class RememberedBit : public BitField<RawObjectTag, bool, kRememberedBitOffset, kBitsForRememberedBit>{};
  private:
   RawObjectTag raw_;
  public:
   constexpr ObjectTag(const RawObjectTag raw = kInvalidObjectTag):
     raw_(raw) {
   }
   ObjectTag(const ObjectTag& rhs) = default;
   ~ObjectTag() = default;

   RawObjectTag raw() const{
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

   void SetSize(int64_t size){
     raw_ = SizeTag::Update(size, raw());
   }

   inline void ClearSize() {
     return SetSize(0);
   }

   int64_t GetSize() const{
     return SizeTag::Decode(raw());
   }

   explicit operator RawObjectTag() const{
     return raw();
   }

   ObjectTag& operator=(const ObjectTag& rhs) = default;

   ObjectTag& operator=(const RawObjectTag& rhs){
     if(raw_ == rhs)
       return *this;
     raw_ = rhs;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const ObjectTag& val){
     stream << "ObjectTag(";
     stream << "new=" << val.IsNew() << ", ";
     stream << "old=" << val.IsOld() << ", ";
     stream << "marked=" << val.IsMarked() << ", ";
     stream << "remembered=" << val.IsRemembered() << ", ";
     stream << "size=" << val.GetSize();
     stream << ")";
     return stream;
   }

   friend bool operator==(const ObjectTag& lhs, const ObjectTag& rhs) {
     return lhs.raw() == rhs.raw();
   }

   friend bool operator!=(const ObjectTag& lhs, const ObjectTag& rhs) {
     return lhs.raw() != rhs.raw();
   }
  public:
   static inline constexpr RawObjectTag
   Empty() {
     return kInvalidObjectTag;
   }

   static inline constexpr RawObjectTag
   New(const ObjectSize size) {
     return Empty() | NewBit::Encode(true) | SizeTag::Encode(size);
   }

   static inline constexpr RawObjectTag
   NewMarked(const ObjectSize size) {
     return New(size) | MarkedBit::Encode(true);
   }

   static inline constexpr RawObjectTag
   NewRemembered(const ObjectSize size) {
     return New(size) | RememberedBit::Encode(true);
   }

   static inline constexpr RawObjectTag
   Old(const ObjectSize size) {
     return Empty() | OldBit::Encode(true) | SizeTag::Encode(size);
   }

   static inline constexpr RawObjectTag
   OldMarked(const ObjectSize size) {
     return Old(size) | MarkedBit::Encode(true);
   }

   static inline constexpr RawObjectTag
   OldRemembered(const ObjectSize size) {
     return Old(size) | RememberedBit::Encode(true);
   }
 };
}

#endif // POSEIDON_OBJECT_TAG_H