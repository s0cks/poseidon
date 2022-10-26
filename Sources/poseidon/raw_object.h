#ifndef POSEIDON_RAW_OBJECT_H
#define POSEIDON_RAW_OBJECT_H

#include <sstream>
#include <glog/logging.h>

#include "utils.h"
#include "common.h"
#include "poseidon/region.h"

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

  typedef uword RawObjectTag;

  static const constexpr RawObjectTag kInvalidObjectTag = 0;

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
    explicit ObjectTag(RawObjectTag raw=kInvalidObjectTag):
      raw_(raw){
    }
    ObjectTag(const ObjectTag& rhs) = default;
    ~ObjectTag() = default;

    inline RawObjectTag raw() const{
      return raw_;
    }

    ObjectTag& operator=(const ObjectTag& rhs) = default;

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

    int64_t GetSize() const{
      return SizeTag::Decode(raw());
    }

    explicit operator RawObjectTag() const{
      return raw();
    }

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
    static inline ObjectTag
    New(){
      auto raw = NewBit::Encode(true)
           | OldBit::Encode(false)
           | MarkedBit::Encode(false)
           | RememberedBit::Encode(false)
           | SizeTag::Encode(0);
      return ObjectTag(raw);
    }

    static inline ObjectTag
    NewWithSize(int64_t size){
      auto raw = NewBit::Encode(true)
               | OldBit::Encode(false)
               | MarkedBit::Encode(false)
               | RememberedBit::Encode(false)
               | SizeTag::Encode(size);
      return ObjectTag(raw);
    }

    static inline ObjectTag
    NewMarkedWithSize(int64_t size){
      auto raw = NewBit::Encode(true)
          | OldBit::Encode(false)
          | MarkedBit::Encode(true)
          | RememberedBit::Encode(false)
          | SizeTag::Encode(size);
      return ObjectTag(raw);
    }

    static inline ObjectTag
    NewRememberedWithSize(int64_t size){
      auto raw = NewBit::Encode(true)
          | OldBit::Encode(false)
          | MarkedBit::Encode(false)
          | RememberedBit::Encode(true)
          | SizeTag::Encode(size);
      return ObjectTag(raw);
    }

    static inline ObjectTag
    Old(){
      auto raw = NewBit::Encode(false)
          | OldBit::Encode(true)
          | MarkedBit::Encode(false)
          | RememberedBit::Encode(false)
          | SizeTag::Encode(0);
      return ObjectTag(raw);
    }

    static inline ObjectTag
    OldWithSize(int64_t size){
      auto raw = NewBit::Encode(false)
          | OldBit::Encode(true)
          | MarkedBit::Encode(false)
          | RememberedBit::Encode(false)
          | SizeTag::Encode(size);
      return ObjectTag(raw);
    }

    static inline ObjectTag
    OldMarkedWithSize(int64_t size){
      auto raw = NewBit::Encode(false)
          | OldBit::Encode(true)
          | MarkedBit::Encode(true)
          | RememberedBit::Encode(false)
          | SizeTag::Encode(size);
      return ObjectTag(raw);
    }

    static inline ObjectTag
    OldRememberedWithSize(int64_t size){
      auto raw = NewBit::Encode(false)
          | OldBit::Encode(true)
          | MarkedBit::Encode(false)
          | RememberedBit::Encode(true)
          | SizeTag::Encode(size);
      return ObjectTag(raw);
    }
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
    RelaxedAtomic<uword> forwarding_; //TODO: merge w/ ObjectTag

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

    std::string ToString() const{
      std::stringstream ss;
      ss << "RawObject(";
      ss << "new=" << IsNew() << ", ";
      ss << "old=" << IsOld() << ", ";
      ss << "marked=" << IsMarked() << ", ";
      ss << "remembered=" << IsRemembered() << ", ";
      ss << "size=" << Bytes(GetPointerSize()) << ", ";
      ss << "address=" << this << ", ";
      ss << "pointer=" << GetPointer() << ", ";
      ss << "forwarding=" << GetForwardingPointer();
      ss << ")";
      return ss.str();
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

      auto ptr = new (area->GetCurrentAddressPointer())RawObject(ObjectTag::NewWithSize(size));
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

      auto ptr = new (area->GetCurrentAddressPointer())RawObject(ObjectTag::OldWithSize(size));
      area->current_ += total_size;
      return ptr->GetStartingAddress();
    }
  };
}

#endif //POSEIDON_RAW_OBJECT_H
