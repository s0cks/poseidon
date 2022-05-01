#ifndef POSEIDON_RAW_OBJECT_H
#define POSEIDON_RAW_OBJECT_H

#include <sstream>
#include <glog/logging.h>

#include "utils.h"
#include "common.h"

namespace poseidon{
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

  class RawObject{
    friend class RawObjectTest;

    friend class Semispace;
    friend class Zone;
    friend class NewZone;
    friend class OldPage;
    friend class Compactor;
   private:
    typedef uword ObjectTag;

    enum{ // 61
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
    };

    // The object's size.
    class SizeTag : public BitField<ObjectTag, uint32_t, kSizeTagOffset, kBitsForSizeTag>{};
    // allocated in the new heap.
    class NewBit : public BitField<ObjectTag, bool, kNewBitOffset, kBitsForNewBit>{};
    // allocated in the old heap.
    class OldBit : public BitField<ObjectTag, bool, kOldBitOffset, kBitsForOldBit>{};
    // marked by the scavenger
    class MarkedBit : public BitField<ObjectTag, bool, kMarkedBitOffset, kBitsForMarkedBit>{};
    // remembered by the scavenger
    class RememberedBit : public BitField<ObjectTag, bool, kRememberedBitOffset, kBitsForRememberedBit>{};

    RelaxedAtomic<ObjectTag> tag_;
    RelaxedAtomic<uword> forwarding_;

    explicit RawObject(int64_t size):
      RawObject(){
      SetPointerSize(size);
    }

    inline ObjectTag
    tag() const{
      return (ObjectTag)tag_;
    }
   public:
    RawObject()://TODO: make private
      tag_(0),
      forwarding_(0){
    }
    virtual ~RawObject() = default;

    uword GetAddress() const{
      return (uword)this;
    }

    void* GetPointer() const{
      return (void*)(reinterpret_cast<uword>(this) + sizeof(RawObject));
    }

    uword GetObjectPointerAddress() const{
      return reinterpret_cast<uword>(this) + sizeof(RawObject);
    }

    void SetForwardingAddress(const uword& address){
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

    bool IsNew() const{
      return NewBit::Decode(tag());
    }

    void SetNewBit(){
      tag_ = NewBit::Update(true, tag());
    }

    void ClearNewBit(){
      tag_ = NewBit::Update(false, tag());
    }

    bool IsOld() const{
      return OldBit::Decode(tag());
    }

    void SetOldBit(){
      tag_ = OldBit::Update(true, tag());
    }

    void ClearOldBit(){
      tag_ = OldBit::Update(false, tag());
    }

    bool IsMarked() const{
      return MarkedBit::Decode(tag());
    }

    void SetMarkedBit(){
      tag_ = MarkedBit::Update(true, tag());
    }

    void ClearMarkedBit(){
      tag_ = MarkedBit::Update(false, tag());
    }

    bool IsRemembered() const{
      return RememberedBit::Decode(tag());
    }

    void SetRememberedBit(){
      tag_ = RememberedBit::Update(true, tag());
    }

    void ClearRememberedBit(){
      tag_ = RememberedBit::Update(false, tag());
    }

    uint32_t GetPointerSize() const{
      return SizeTag::Decode(tag());
    }

    void SetPointerSize(const uint32_t& val){
      tag_ = SizeTag::Update(val, tag());
    }

    uint64_t GetTotalSize() const{
      return sizeof(RawObject) + GetPointerSize();
    }

    std::string ToString() const{
      std::stringstream ss;
      ss << "RawObject(";
      ss << "new=" << NewBit::Decode(tag()) << ", ";
      ss << "old=" << OldBit::Decode(tag()) << ", ";
      ss << "marked=" << MarkedBit::Decode(tag()) << ", ";
      ss << "remembered=" << RememberedBit::Decode(tag()) << ", ";
      ss << "size=" << GetPointerSize() << ", ";
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
      if((area->current_ + total_size) > area->GetEndingAddress()){
        DLOG(ERROR) << "cannot allocate object of " << Bytes(total_size) << " in Semispace.";
        return 0;
      }

      auto address = area->current_;
      area->current_ += total_size;
      new ((void*)address)RawObject(size);
      return address;
    }
  };
}

#endif //POSEIDON_RAW_OBJECT_H
