#ifndef POSEIDON_FREELIST_NODE_H
#define POSEIDON_FREELIST_NODE_H

#include "poseidon/region.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class FreeListNode;
 class FreeListNodeVisitor {
  protected:
   FreeListNodeVisitor() = default;
  public:
   virtual ~FreeListNodeVisitor() = default;
   virtual bool Visit(FreeListNode* node) = 0;
 };

 class FreeListNode : public Region {
   friend class FreeList;
  protected:
   RelaxedAtomic<RawObjectTag> tag_;
   RelaxedAtomic<uword> forwarding_;

   inline RawObjectTag raw_tag() const {
     return (RawObjectTag)tag_;
   }

   inline void set_raw_tag(const RawObjectTag& value) {
     tag_ = value;
   }

   inline ObjectTag tag() const {
     return ObjectTag(raw_tag());
   }

   inline void set_tag(const ObjectTag& value) {
     set_raw_tag(value.raw());
   }

   inline void SetSize(const ObjectSize size) {
     set_raw_tag(ObjectTag::SizeTag::Update(size, raw_tag())); //TODO: cleanup
   }

   inline void SetNextAddress(const uword value) {
     forwarding_ = value;
   }

   inline void SetNext(FreeListNode* node) {
     return SetNextAddress(node->GetStartingAddress());
   }

   inline void SetOldBit(bool value = true) {
     set_raw_tag(ObjectTag::OldBit::Update(value, raw_tag()));
   }

   inline void ClearOldBit() {
     return SetOldBit(false);
   }
  public:
   FreeListNode():
    Region(),
    tag_(),
    forwarding_() {
     SetSize(0);
     SetNextAddress(0);
   }
   explicit FreeListNode(const ObjectSize size):
    Region(),
    tag_(),
    forwarding_() {
     SetSize(size);
     SetNextAddress(GetStartingAddress() + GetSize());
   }
   explicit FreeListNode(const Region& region):
    Region(region),
    tag_(),
    forwarding_() {
     SetSize(region.GetSize());
     SetNextAddress(GetStartingAddress() + GetSize());
   }
   FreeListNode(const FreeListNode& rhs) = default;
   ~FreeListNode() override = default;

   uword GetStartingAddress() const override {
     return (uword)this;
   }

   inline uword GetAddress() const {
     return GetStartingAddress();
   }

   ObjectSize GetSize() const override {
     return ObjectTag::SizeTag::Decode(raw_tag());
   }

   bool IsOld() const {
     return ObjectTag::OldBit::Decode(raw_tag());
   }

   uword GetNextAddress() const {
     return (uword)forwarding_;
   }

   FreeListNode* GetNext() const {
     return (FreeListNode*)GetNextAddress();
   }

   FreeListNode& operator=(const FreeListNode& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const FreeListNode& val) {
     stream << "FreeListNode(";
     stream << "start=" <<  val.GetStartingAddressPointer() << ", ";
     stream << "size=" << val.GetSize() << ", ";
     stream << "next=" << val.GetNext();
     stream << ")";
     return stream;
   }

   friend bool operator==(const FreeListNode& lhs, const FreeListNode& rhs) {
     return ((const Region&)lhs) == ((const Region&)rhs) &&
            lhs.tag() == rhs.tag();
   }

   friend bool operator!=(const FreeListNode& lhs, const FreeListNode& rhs) {
     return !operator==(lhs, rhs);
   }

   //TODO: make comparable

   static inline FreeListNode* NewNode(const MemoryRegion& region) {
     return new (region.GetStartingAddressPointer())FreeListNode(region.GetSize());
   }
 };
}

#endif // POSEIDON_FREELIST_NODE_H