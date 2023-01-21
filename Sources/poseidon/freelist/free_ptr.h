#ifndef POSEIDON_FREELIST_NODE_H
#define POSEIDON_FREELIST_NODE_H

#include "poseidon/flags.h"
#include "poseidon/region.h"
#include "poseidon/pointer.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class FreePointer;
 class FreePointerVisitor {
  protected:
   FreePointerVisitor() = default;
  public:
   virtual ~FreePointerVisitor() = default;
   virtual bool VisitFreeListStart() { return true; }
   virtual bool VisitFreePointer(FreePointer* free_ptr) = 0;
   virtual bool VisitFreeListEnd() { return true; }
 };

 class FreePointer {
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

   inline void SetNext(FreePointer* node) {
     return SetNextAddress(node != nullptr ? node->GetStartingAddress() : 0);
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
   FreePointer():
    tag_(),
    forwarding_() {
     SetSize(0);
     SetNextAddress(0);
   }
   explicit FreePointer(const PointerTag& tag):
    tag_(tag.raw()),
    forwarding_(0) {
   }
   explicit FreePointer(const Region& region): //TODO: remove
    tag_(),
    forwarding_() {
     SetSize(region.GetSize());
   }
   FreePointer(const FreePointer& rhs) = default;
   virtual ~FreePointer() = default;

   uword GetStartingAddress() const {
     return (uword)this;
   }

   void* GetStartingAddressPointer() const {
     return (void*) GetStartingAddress();
   }

   inline uword GetAddress() const {
     return GetStartingAddress();
   }

   uword GetEndingAddress() const {
     return GetStartingAddress() + GetSize();
   }

   void* GetEndingAddressPointer() const {
     return (void*) GetEndingAddress();
   }

   word GetSize() const {
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

   FreePointer* GetNext() const {
     return (FreePointer*)GetNextAddress();
   }

   bool HasNext() const {
     return GetNextAddress() != UNALLOCATED;
   }

   FreePointer& operator=(const FreePointer& rhs) = default;

   explicit operator Region() const {
     return Region(GetStartingAddress(), GetSize());
   }

   friend std::ostream& operator<<(std::ostream& stream, const FreePointer& val) {
     stream << "FreePointer(";
     stream << "tag=" << val.tag() << ", ";
     stream << "start=" <<  val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "next=" << val.GetNext();
     stream << ")";
     return stream;
   }

   friend bool operator==(const FreePointer& lhs, const FreePointer& rhs) {
     return lhs.GetStartingAddress() == rhs.GetStartingAddress() &&
            lhs.GetSize() == rhs.GetSize() &&
            lhs.raw_tag() == rhs.raw_tag();
   }

   friend bool operator!=(const FreePointer& lhs, const FreePointer& rhs) {
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const FreePointer& lhs, const FreePointer& rhs) {
     return Compare(lhs, rhs) < 0;
   }

   friend bool operator>(const FreePointer& lhs, const FreePointer& rhs) {
     return Compare(lhs, rhs) > 0;
   }
  public:
   static int Compare(const FreePointer& lhs, const FreePointer& rhs);
   static FreePointer* From(const Region& region);
 };
}

#endif // POSEIDON_FREELIST_NODE_H