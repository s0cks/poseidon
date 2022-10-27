#ifndef POSEIDON_FREELIST_NODE_H
#define POSEIDON_FREELIST_NODE_H

#include "poseidon/region.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class FreeObject;
 class FreeListNodeVisitor {
  protected:
   FreeListNodeVisitor() = default;
  public:
   virtual ~FreeListNodeVisitor() = default;
   virtual bool Visit(FreeObject* node) = 0;
 };

 class FreeObject : public Region {
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

   inline void SetNext(FreeObject* node) {
     return SetNextAddress(node->GetStartingAddress());
   }

   inline void SetOldBit(bool value = true) {
     set_raw_tag(ObjectTag::OldBit::Update(value, raw_tag()));
   }

   inline void ClearOldBit() {
     return SetOldBit(false);
   }
  public:
   FreeObject():
    Region(),
    tag_(),
    forwarding_() {
     SetSize(0);
     SetNextAddress(0);
   }
   explicit FreeObject(const ObjectSize size):
    Region(),
    tag_(),
    forwarding_() {
     SetSize(size);
     SetNextAddress(GetStartingAddress() + GetSize());
   }
   explicit FreeObject(const Region& region):
    Region(region),
    tag_(),
    forwarding_() {
     SetSize(region.GetSize());
     SetNextAddress(GetStartingAddress() + GetSize());
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
     return ObjectTag::SizeTag::Decode(raw_tag());
   }

   bool IsOld() const {
     return ObjectTag::OldBit::Decode(raw_tag());
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
     stream << "start=" <<  val.GetStartingAddressPointer() << ", ";
     stream << "size=" << val.GetSize() << ", ";
     stream << "next=" << val.GetNext();
     stream << ")";
     return stream;
   }

   friend bool operator==(const FreeObject& lhs, const FreeObject& rhs) {
     return ((const Region&)lhs) == ((const Region&)rhs) &&
            lhs.tag() == rhs.tag();
   }

   friend bool operator!=(const FreeObject& lhs, const FreeObject& rhs) {
     return !operator==(lhs, rhs);
   }

   //TODO: make comparable

   static inline FreeObject* NewNode(const MemoryRegion& region) {
     return new (region.GetStartingAddressPointer())FreeObject(region.GetSize());
   }
 };
}

#endif // POSEIDON_FREELIST_NODE_H