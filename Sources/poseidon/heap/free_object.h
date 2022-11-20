#ifndef POSEIDON_FREELIST_NODE_H
#define POSEIDON_FREELIST_NODE_H

#include "poseidon/flags.h"
#include "poseidon/region.h"
#include "poseidon/pointer.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class FreeObject;
 class FreeObjectVisitor {
  protected:
   FreeObjectVisitor() = default;
  public:
   virtual ~FreeObjectVisitor() = default;
   virtual bool Visit(FreeObject* node) = 0;
 };

 class FreeObject : public Region {
   friend class FreeList;
  protected:
   RelaxedAtomic<RawPointerTag> tag_;
   RelaxedAtomic<uword> forwarding_;

   inline RawPointerTag raw_tag() const {
     return (RawPointerTag)tag_;
   }

   inline void set_raw_tag(const RawPointerTag& value) {
     tag_ = value;
   }

   inline PointerTag tag() const {
     return { raw_tag() };
   }

   inline void set_tag(const PointerTag& value) {
     set_raw_tag(value.raw());
   }

   inline void SetSize(const ObjectSize size) {
     set_raw_tag(PointerTag::SizeTag::Update(size, raw_tag())); //TODO: cleanup
   }

   inline void SetNextAddress(const uword value) {
     forwarding_ = value;
   }

   inline void SetNext(FreeObject* node) {
     return SetNextAddress(node->GetStartingAddress());
   }

   inline void SetOldBit(const bool value = true) {
     set_raw_tag(PointerTag::OldBit::Update(value, raw_tag()));
   }

   inline void ClearOldBit() {
     return SetOldBit(false);
   }

   inline void SetFreeBit(const bool value = true) {
     return set_raw_tag(PointerTag::FreeBit::Update(value, raw_tag()));
   }

   inline void ClearFreeBit() {
     return SetFreeBit(false);
   }
  public:
   FreeObject():
    Region(),
    tag_(),
    forwarding_() {
     SetSize(0);
     SetNextAddress(0);
   }
   explicit FreeObject(const PointerTag& tag):
    Region(),
    tag_(tag.raw()),
    forwarding_(0) {
   }
   explicit FreeObject(const Region& region): //TODO: remove
    Region(region),
    tag_(),
    forwarding_() {
     SetSize(region.GetSize());
   }
   FreeObject(const FreeObject& rhs) = default;
   ~FreeObject() override = default;

   uword GetStartingAddress() const override {
     return (uword)this;
   }

   inline uword GetAddress() const {
     return GetStartingAddress();
   }

   ObjectSize GetSize() const override {
     return PointerTag::SizeTag::Decode(raw_tag());
   }

   ObjectSize GetTotalSize() const {
     return GetSize();
   }

   bool IsOld() const {
     return PointerTag::OldBit::Decode(raw_tag());
   }

   bool IsFree() const {
     return PointerTag::FreeBit::Decode(raw_tag());
   }

   uword GetNextAddress() const {
     return (uword)forwarding_;
   }

   FreeObject* GetNext() const {
     return (FreeObject*)GetNextAddress();
   }

   FreeObject& operator=(const FreeObject& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const FreeObject& val) {
     stream << "FreeObject(";
     stream << "tag=" << val.tag() << ", ";
     stream << "start=" <<  val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "next=" << val.GetNext();
     stream << ")";
     return stream;
   }

   friend bool operator==(const FreeObject& lhs, const FreeObject& rhs) {
     return lhs.GetStartingAddress() == rhs.GetStartingAddress() &&
            lhs.GetSize() == rhs.GetSize() &&
            lhs.raw_tag() == rhs.raw_tag();
   }

   friend bool operator!=(const FreeObject& lhs, const FreeObject& rhs) {
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const FreeObject& lhs, const FreeObject& rhs) {
     return Compare(lhs, rhs) < 0;
   }

   friend bool operator>(const FreeObject& lhs, const FreeObject& rhs) {
     return Compare(lhs, rhs) > 0;
   }
  public:
   static int Compare(const FreeObject& lhs, const FreeObject& rhs);
   static FreeObject* From(const Region& region);
 };
}

#endif // POSEIDON_FREELIST_NODE_H