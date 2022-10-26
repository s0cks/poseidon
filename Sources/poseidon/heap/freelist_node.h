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
  public:
   FreeListNode():
    Region(),
    tag_(),
    forwarding_() {
   }
   explicit FreeListNode(const ObjectSize size):
    Region(),
    tag_(),
    forwarding_() {
     SetSize(size);
   }
   explicit FreeListNode(const Region& region):
    Region(region),
    tag_(),
    forwarding_() {
     SetSize(region.GetSize());
   }
   FreeListNode(const FreeListNode& rhs) = default;
   ~FreeListNode() override = default;

   uword GetStartingAddress() const override {
     return (uword)this;
   }

   ObjectSize GetSize() const override {
     return ObjectTag::SizeTag::Decode(raw_tag());
   }

   FreeListNode& operator=(const FreeListNode& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const FreeListNode& val) {
     stream << "FreeListNode(";
     stream << "start=" <<  val.GetStartingAddress() << ", ";
     stream << "size=" << val.GetSize();
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

   static inline FreeListNode* Of(const MemoryRegion& region) {
     return new (region.GetStartingAddressPointer())FreeListNode(region.GetSize());
   }
 };
}

#endif // POSEIDON_FREELIST_NODE_H