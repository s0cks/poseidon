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
  class RawObjectVisitor : public Visitor<Pointer> {
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

  class Pointer {
    friend class RawObjectTest;

    //TODO: cleanup friends
    friend class Semispace;
    friend class Page;
    friend class NewPage;
    friend class OldPage;
    friend class NewZone;
    friend class OldZone;
    friend class Compactor;
    friend class FreeList;
   protected:
    RelaxedAtomic<RawPointerTag> tag_;
    RelaxedAtomic<uword> forwarding_;

    explicit Pointer(PointerTag tag):
      tag_(tag.raw()),
      forwarding_(0){
    }
   public:
    Pointer(): //TODO: make private
      tag_(0),
      forwarding_(0){
    }
    ~Pointer() = default;

    uword GetStartingAddress() const {
      return (uword)this;
    }

    void* GetStartingAddressPointer() const {
      return (void*) GetStartingAddress();
    }

    word GetSize() const {
      return tag().GetSize();
    }

    uword GetObjectPointerAddress() const{
      return GetStartingAddress() + sizeof(Pointer);
    }

    void* GetPointer() const{
      return (void*)GetObjectPointerAddress();
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

    uword GetEndingAddress() const {
      return GetStartingAddress() + GetTotalSize();
    }

    bool IsForwarding() const{
      return GetForwardingAddress() != 0;
    }

    RawPointerTag raw_tag() const{
      return (RawPointerTag)tag_;
    }

    PointerTag tag() const{
      return (PointerTag)raw_tag();
    }

    void set_tag(const PointerTag& val){
      tag_ = (RawPointerTag)val;
    }

    bool IsNew() const{
      return PointerTag::NewBit::Decode(raw_tag());
    }

    void SetNewBit(){
      tag_ = PointerTag::NewBit::Update(true, raw_tag());
    }

    void ClearNewBit(){
      tag_ = PointerTag::NewBit::Update(false, raw_tag());
    }

    bool IsOld() const{
      return PointerTag::OldBit::Decode(raw_tag());
    }

    void SetOldBit(){
      tag_ = PointerTag::OldBit::Update(true, raw_tag());
    }

    void ClearOldBit(){
      tag_ = PointerTag::OldBit::Update(false, raw_tag());
    }

    bool IsMarked() const{
      return PointerTag::MarkedBit::Decode(raw_tag());
    }

    void SetMarkedBit(){
      tag_ = PointerTag::MarkedBit::Update(true, raw_tag());
    }

    void ClearMarkedBit(){
      tag_ = PointerTag::MarkedBit::Update(false, raw_tag());
    }

    bool IsRemembered() const{
      return PointerTag::RememberedBit::Decode(raw_tag());
    }

    void SetRememberedBit(){
      tag_ = PointerTag::RememberedBit::Update(true, raw_tag());
    }

    void ClearRememberedBit(){
      tag_ = PointerTag::RememberedBit::Update(false, raw_tag());
    }

    bool IsFree() const {
      return PointerTag::FreeBit::Decode(raw_tag());
    }

    void SetFreeBit(const bool value = true) {
      tag_ = PointerTag::FreeBit::Update(value, raw_tag());
    }

    inline void ClearFreeBit() {
      return SetFreeBit(false);
    }

    void ClearTag(){
      tag_ = kInvalidPointerTag;
    }

    uint32_t GetPointerSize() const{
      return PointerTag::SizeTag::Decode(raw_tag());
    }

    void SetPointerSize(const uint32_t& val){
      tag_ = PointerTag::SizeTag::Update(val, raw_tag());
    }

    int64_t GetTotalSize() const{
      return static_cast<int64_t>(sizeof(Pointer)) + GetPointerSize();
    }

    Region GetPointerRegion() const {
      return { GetObjectPointerAddress(), GetPointerSize() };
    }

    friend std::ostream& operator<<(std::ostream& stream, const Pointer& value) {
      stream << "Pointer(";
      stream << "tag=" << value.tag() << ", ";
      stream << "start=" << value.GetStartingAddressPointer() << ", ";
      stream << "size=" << Bytes(value.GetSize()) << ", ";
      stream << "pointer=" << value.GetPointer() << ", ";
      stream << "forwarding=" << value.GetForwardingPointer();
      stream << ")";
      return stream;
    }

    ObjectSize VisitPointers(RawObjectVisitor* vis);

    explicit operator Region() const {
      return { GetStartingAddress(), GetTotalSize() };
    }
   public:
    static inline Pointer*
    From(const Region& region, const PointerTag tag = PointerTag::Empty()) {
      //TODO: make assertions
      return new (region.GetStartingAddressPointer())Pointer(tag);
    }

    template<class T>
    static inline uword
    TryAllocateIn(T* area, int64_t size){
      auto total_size = static_cast<int64_t>(sizeof(Pointer) + size);
      if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
        PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
      }

      auto ptr = new (area->GetCurrentAddressPointer())Pointer();
      area->current_ += total_size;
      ptr->SetPointerSize(size);
      return ptr->GetStartingAddress();
    }

    template<class T>
    static inline uword
    TryAllocateNewIn(T* area, int64_t size){
      auto total_size = static_cast<int64_t>(sizeof(Pointer) + size);
      if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
        PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
      }

      auto ptr = new (area->GetCurrentAddressPointer())Pointer(PointerTag::New(size));
      area->current_ += total_size;
      return ptr->GetStartingAddress();
    }

    template<class T>
    static inline uword
    TryAllocateOldIn(T* area, int64_t size){
      auto total_size = static_cast<int64_t>(sizeof(Pointer) + size);
      if((area->GetCurrentAddress() + total_size) > area->GetEndingAddress()){
        PSDN_CANT_ALLOCATE(ERROR, total_size, (*area));
      }

      auto ptr = new (area->GetCurrentAddressPointer())Pointer(PointerTag::Old(size));
      area->current_ += total_size;
      return ptr->GetStartingAddress();
    }
  };
}

#endif //POSEIDON_RAW_OBJECT_H
