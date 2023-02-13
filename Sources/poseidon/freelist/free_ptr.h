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

   inline void SetNextAddress(const uword value) {
     forwarding_ = value;
   }

   inline void SetNext(FreePointer* node) {
     return SetNextAddress(node != nullptr ? node->GetStartingAddress() : 0);
   }
  public:
   FreePointer() = delete;
   explicit FreePointer(const PointerTag& tag):
    tag_(tag.raw()),
    forwarding_(0) { }
   explicit FreePointer(const Region& region): //TODO: remove
    tag_(),
    forwarding_() {
     SetSize(region.GetSize());
   }
   FreePointer(const FreePointer& rhs) = delete;
   ~FreePointer() = delete;
   DEFINE_TAGGED_POINTER;

   inline uword GetAddress() const {
     return GetStartingAddress();
   }

   ObjectSize GetTotalSize() const {
     return GetSize();
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

   FreePointer& operator=(const FreePointer& rhs) = delete;

   explicit operator Region() const {
     return { GetStartingAddress(), GetSize() };
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
  public:
   static FreePointer* From(const Region& region);
 };
}

#endif // POSEIDON_FREELIST_NODE_H