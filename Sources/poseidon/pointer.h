#ifndef POSEIDON_RAW_OBJECT_H
#define POSEIDON_RAW_OBJECT_H

#include <sstream>
#include <glog/logging.h>

#include "utils.h"
#include "common.h"
#include "poseidon/region.h"
#include "poseidon/pointer_tag.h"

#include "poseidon/utils/visitor.h"

namespace poseidon{
 typedef const std::function<bool(Pointer*)>& RawObjectVisitorFunction;

 class Pointer;
 class RawObjectVisitor : public Visitor<Pointer>{
  protected:
   RawObjectVisitor() = default;
  public:
   ~RawObjectVisitor() override = default;
 };
 DEFINE_VISITOR_WRAPPER(RawObjectVisitor, Pointer);

 class RawObjectPointerVisitor{
  protected:
   RawObjectPointerVisitor() = default;
  public:
   virtual ~RawObjectPointerVisitor() = default;
   virtual bool Visit(Pointer** val) = 0;
 };

 class RawObjectPointerIterator{
  protected:
   RawObjectPointerIterator() = default;
  public:
   virtual ~RawObjectPointerIterator() = default;
   virtual bool HasNext() const = 0;
   virtual Pointer* Next() = 0;
 };

#define DEFINE_SET_BIT(Name) \
 inline void Set##Name(const bool val = true) { set_raw_tag(PointerTag::Name##Bit::Update(val, raw_tag())); }
#define DEFINE_CLEAR_BIT(Name) \
 inline void Clear##Name() { return Set##Name(false); }
#define DEFINE_IS(Name) \
 inline bool Is##Name() const { return PointerTag::Name##Bit::Decode(raw_tag()); }

#define DEFINE_TAGGED_POINTER \
  protected:                  \
   FOR_EACH_POINTER_TAG_BIT(DEFINE_SET_BIT); \
   FOR_EACH_POINTER_TAG_BIT(DEFINE_CLEAR_BIT); \
   inline void SetSize(const ObjectSize size) { return set_raw_tag(PointerTag::SizeTag::Update(size, raw_tag())); } \
   inline void SetTag(const PointerTag& tag) { return set_raw_tag(tag.raw()); }                                     \
   inline void ClearTag() { return SetTag(PointerTag::Empty()); }                                                   \
  public:                     \
   FOR_EACH_POINTER_TAG_BIT(DEFINE_IS);      \
   uword GetStartingAddress() const { return reinterpret_cast<uword>(this); }                                \
   void* GetStartingAddressPointer() const { return reinterpret_cast<void*>(GetStartingAddress()); }         \
   ObjectSize GetSize() const { return PointerTag::SizeTag::Decode(raw_tag()); }                             \
   uword GetEndingAddress() const { return GetStartingAddress() + GetSize(); }                               \
   void* GetEndingAddressPointer() const { return reinterpret_cast<void*>(GetEndingAddress()); }

 class Pointer{
   friend class RawObjectTest;

   //TODO: cleanup friends
   friend class Semispace;
   friend class Marker;
   friend class Page;
   friend class NewPage;
   friend class OldPage;
   friend class NewZone;
   friend class OldZone;
   friend class Compactor;
   friend class FreeList;
   friend class Finalizer;
  protected:
   RelaxedAtomic<RawPointerTag> tag_;
   RelaxedAtomic<uword> forwarding_;

   explicit Pointer(PointerTag tag):
    tag_(tag.raw()),
    forwarding_(0) { }

   inline void set_raw_tag(const RawPointerTag& raw) {
     tag_ = raw;
   }
  public:
   Pointer(): //TODO: make private
     tag_(0),
     forwarding_(0) { }
   ~Pointer() = default;

   inline RawPointerTag raw_tag() const{
     return (RawPointerTag) tag_;
   }

   PointerTag tag() const{
     return (PointerTag) raw_tag();
   }

   DEFINE_TAGGED_POINTER;

   uword GetObjectPointerAddress() const{
     return GetStartingAddress() + sizeof(Pointer);
   }

   void* GetPointer() const{
     return (void*) GetObjectPointerAddress();
   }

   void SetForwardingAddress(uword address){
     forwarding_ = address;
   }

   uword GetForwardingAddress() const{
     return (uword) forwarding_;
   }

   void* GetForwardingPointer() const{
     return (void*) GetForwardingAddress();
   }

   bool IsForwarding() const{
     return GetForwardingAddress() != 0;
   }

   uint32_t GetPointerSize() const{
     return PointerTag::SizeTag::Decode(raw_tag());
   }

   void SetPointerSize(const uint32_t& val){
     tag_ = PointerTag::SizeTag::Update(val, raw_tag());
   }

   TypeId GetTypeId() const {
     return PointerTag::TypeIdBits::Decode(raw_tag());
   }

   void SetTypeId(const TypeId type_id) {
     return set_raw_tag(PointerTag::TypeIdBits::Update(type_id, raw_tag()));
   }

   void ClearTypeId() {
     return SetTypeId(TypeId::kUnknownTypeId);
   }

   int64_t GetTotalSize() const{
     return static_cast<int64_t>(sizeof(Pointer)) + GetPointerSize();
   }

   Region GetPointerRegion() const{
     return {GetObjectPointerAddress(), GetPointerSize()};
   }

   friend std::ostream& operator<<(std::ostream& stream, const Pointer& value){
     stream << "Pointer(";
     stream << "new=" << value.IsNew() << ", ";
     stream << "old=" << value.IsOld() << ", ";
     stream << "marked=" << value.IsMarked() << ", ";
     stream << "remembered=" << value.IsRemembered() << ", ";
     stream << "free=" << value.IsFree() << ", ";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "type_id=" << value.GetTypeId() << ", ";
     stream << "size=" << Bytes(value.GetSize()) << ", ";
     stream << "pointer=" << value.GetPointer() << ", ";
     stream << "forwarding=" << value.GetForwardingPointer();
     stream << ")";
     return stream;
   }

   ObjectSize VisitPointers(RawObjectVisitor* vis);

   explicit operator Region() const{
     return {GetStartingAddress(), GetTotalSize()};
   }
  public:
   static inline Pointer* From(const Region& region, const PointerTag tag = PointerTag::Empty()){
     //TODO: make assertions
     return new(region.GetStartingAddressPointer())Pointer(tag);
   }

   template<class T>
   static inline uword TryAllocateIn(T* area, int64_t size){
     auto total_size = static_cast<int64_t>(sizeof(Pointer) + size);
     if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
       PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
     }
     auto ptr = new(area->GetCurrentAddressPointer())Pointer();
     area->current_ += total_size;
     ptr->SetPointerSize(size);
     return ptr->GetStartingAddress();
   }

   template<class T>
   static inline uword TryAllocateNewIn(T* area, int64_t size){
     auto total_size = static_cast<int64_t>(sizeof(Pointer) + size);
     if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
       PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
     }

     auto ptr = new(area->GetCurrentAddressPointer())Pointer(PointerTag::New(size));
     area->current_ += total_size;
     return ptr->GetStartingAddress();
   }

   template<class T>
   static inline uword TryAllocateOldIn(T* area, int64_t size){
     auto total_size = static_cast<int64_t>(sizeof(Pointer) + size);
     if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
       PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
     }

     auto ptr = new(area->GetCurrentAddressPointer())Pointer(PointerTag::Old(size));
     area->current_ += total_size;
     return ptr->GetStartingAddress();
   }
 };
}

#endif //POSEIDON_RAW_OBJECT_H
