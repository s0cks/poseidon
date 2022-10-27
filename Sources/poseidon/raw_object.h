#ifndef POSEIDON_RAW_OBJECT_H
#define POSEIDON_RAW_OBJECT_H

#include <sstream>
#include <glog/logging.h>

#include "utils.h"
#include "common.h"
#include "poseidon/region.h"
#include "poseidon/object_tag.h"

namespace poseidon{
 typedef const std::function<bool(RawObject*)>& RawObjectVisitorFunction;

  class RawObject;
  class RawObjectVisitor{
   protected:
    RawObjectVisitor() = default;
   public:
    virtual ~RawObjectVisitor() = default;
    virtual bool Visit(RawObject* val) = 0;
  };

  class RawObjectPointerVisitor{
   protected:
    RawObjectPointerVisitor() = default;
   public:
    virtual ~RawObjectPointerVisitor() = default;
    virtual bool Visit(RawObject** val) = 0;
  };

  class RawObjectPointerIterator{
   protected:
    RawObjectPointerIterator() = default;
   public:
    virtual ~RawObjectPointerIterator() = default;
    virtual bool HasNext() const = 0;
    virtual RawObject* Next() = 0;
  };

  class RawObject : public Region {
    friend class RawObjectTest;

    //TODO: cleanup friends
    friend class Semispace;
    friend class Zone;
    friend class Page;
    friend class NewPage;
    friend class OldPage;
    friend class NewZone;
    friend class OldZone;
    friend class Compactor;
    friend class FreeList;
   private:
    RelaxedAtomic<RawObjectTag> tag_;
    RelaxedAtomic<uword> forwarding_;

    explicit RawObject(ObjectTag tag):
      Region(),
      tag_((RawObjectTag)tag),
      forwarding_(0){
    }
   public:
    RawObject():
      Region(), //TODO: make private
      tag_(0),
      forwarding_(0){
    }
    ~RawObject() override = default;

    uword GetStartingAddress() const override {
      return (uword)this;
    }

    int64_t GetSize() const override {
      return GetTotalSize(); //TODO: refactor
    }

    void* GetPointer() const{
      return (void*)(reinterpret_cast<uword>(this) + sizeof(RawObject));
    }

    uword GetObjectPointerAddress() const{
      return reinterpret_cast<uword>(this) + sizeof(RawObject);
    }

    void SetForwardingAddress(uword address){
      forwarding_ = address;
    }

    uword GetForwardingAddress() const{
      return (uword)forwarding_;
    }

    void* GetForwardingPointer() const{
      return (void*)GetForwardingAddress();
    }

    bool IsForwarding() const{
      return forwarding_ != 0;
    }

    RawObjectTag raw_tag() const{
      return (RawObjectTag)tag_;
    }

    ObjectTag tag() const{
      return (ObjectTag)raw_tag();
    }

    void set_tag(const ObjectTag& val){
      tag_ = (RawObjectTag)val;
    }

    bool IsNew() const{
      return ObjectTag::NewBit::Decode(raw_tag());
    }

    void SetNewBit(){
      tag_ = ObjectTag::NewBit::Update(true, raw_tag());
    }

    void ClearNewBit(){
      tag_ = ObjectTag::NewBit::Update(false, raw_tag());
    }

    bool IsOld() const{
      return ObjectTag::OldBit::Decode(raw_tag());
    }

    void SetOldBit(){
      tag_ = ObjectTag::OldBit::Update(true, raw_tag());
    }

    void ClearOldBit(){
      tag_ = ObjectTag::OldBit::Update(false, raw_tag());
    }

    bool IsMarked() const{
      return ObjectTag::MarkedBit::Decode(raw_tag());
    }

    void SetMarkedBit(){
      tag_ = ObjectTag::MarkedBit::Update(true, raw_tag());
    }

    void ClearMarkedBit(){
      tag_ = ObjectTag::MarkedBit::Update(false, raw_tag());
    }

    bool IsRemembered() const{
      return ObjectTag::RememberedBit::Decode(raw_tag());
    }

    void SetRememberedBit(){
      tag_ = ObjectTag::RememberedBit::Update(true, raw_tag());
    }

    void ClearRememberedBit(){
      tag_ = ObjectTag::RememberedBit::Update(false, raw_tag());
    }

    void ClearTag(){
      tag_ = kInvalidObjectTag;
    }

    uint32_t GetPointerSize() const{
      return ObjectTag::SizeTag::Decode(raw_tag());
    }

    void SetPointerSize(const uint32_t& val){
      tag_ = ObjectTag::SizeTag::Update(val, raw_tag());
    }

    int64_t GetTotalSize() const{
      return static_cast<int64_t>(sizeof(RawObject) + GetPointerSize());
    }

    friend std::ostream& operator<<(std::ostream& stream, const RawObject& value) {
      stream << "RawObject(";
      stream << "tag=" << value.tag() << ", ";
      stream << "start=" << value.GetStartingAddress() << ", ";
      stream << "size=" << value.GetSize() << ", ";
      stream << "pointer=" << value.GetPointer() << ", ";
      stream << "forwarding=" << value.GetForwardingPointer();
      stream << ")";
      return stream;
    }
   public:
    template<class T>
    static inline uword
    TryAllocateIn(T* area, int64_t size){
      auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);
      if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
        PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
      }

      auto ptr = new (area->GetCurrentAddressPointer())RawObject();
      area->current_ += total_size;
      ptr->SetPointerSize(size);
      return ptr->GetStartingAddress();
    }

    template<class T>
    static inline uword
    TryAllocateNewIn(T* area, int64_t size){
      auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);
      if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
        PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
      }

      auto ptr = new (area->GetCurrentAddressPointer())RawObject(ObjectTag::New(size));
      area->current_ += total_size;
      return ptr->GetStartingAddress();
    }

    template<class T>
    static inline uword
    TryAllocateOldIn(T* area, int64_t size){
      auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);
      if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
        PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
      }

      auto ptr = new (area->GetCurrentAddressPointer())RawObject(ObjectTag::Old(size));
      area->current_ += total_size;
      return ptr->GetStartingAddress();
    }
  };
}

#endif //POSEIDON_RAW_OBJECT_H
