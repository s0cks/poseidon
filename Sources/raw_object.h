#ifndef POSEIDON_RAW_OBJECT_H
#define POSEIDON_RAW_OBJECT_H

#include <sstream>
#include "common.h"
#include "utils.h"

namespace poseidon{
  class RawObject;
  class RawObjectPointerVisitor{
   protected:
    RawObjectPointerVisitor() = default;
   public:
    virtual ~RawObjectPointerVisitor() = default;
    virtual bool Visit(RawObject* obj) = 0;
  };

  class RawObjectPointerPointerVisitor{//TODO: rename
   protected:
    RawObjectPointerPointerVisitor() = default;
   public:
    virtual ~RawObjectPointerPointerVisitor() = default;
    virtual bool Visit(RawObject** obj) = 0;
  };

  class RawObjectPointerIterator{
   protected:
    RawObjectPointerIterator() = default;
   public:
    virtual ~RawObjectPointerIterator() = default;
    virtual bool HasNext() const = 0;
    virtual RawObject* Next() = 0;
  };

  class Object;
  class RawObject{
    friend class Semispace;
   private:
    typedef uword ObjectTag;

    enum{ // 61
      kSizeFieldOffset = 0,
      kBitsForSizeField = 32,

      // EdenBit
      kEdenFieldOffset = kSizeFieldOffset+kBitsForSizeField,
      kBitsForEdenField = 1,

      kTenuredFieldOffset = kEdenFieldOffset+kBitsForEdenField,
      kBitsForTenuredField = 1,

      kMarkedFieldOffset = kTenuredFieldOffset+kBitsForTenuredField,
      kBitsForMarkedField = 1,

      kRememberedFieldOffset = kMarkedFieldOffset+kBitsForMarkedField,
      kBitsForRememberedField = 1,
    };

    // The object's size
    class SizeField : public BitField<ObjectTag, uint32_t, kSizeFieldOffset, kBitsForSizeField>{};
    // allocated in the eden heap
    class EdenBit: public BitField<ObjectTag, bool, kEdenFieldOffset, kBitsForEdenField>{};
    // allocated in the tenured heap
    class TenuredBit : public BitField<ObjectTag, bool, kTenuredFieldOffset, kBitsForTenuredField>{};
    // marked as not garbage
    class MarkedBit : public BitField<ObjectTag, bool, kMarkedFieldOffset, kBitsForMarkedField>{};
    // remembered by GC
    class RememberedBit : public BitField<ObjectTag, bool, kRememberedFieldOffset, kBitsForRememberedField>{};

    ObjectTag tag_;
    uword ptr_;
    uword forwarding_;
    uint32_t num_generations_;
    uint64_t num_references_;

    inline ObjectTag&
    tag(){
      return tag_;
    }

    inline ObjectTag
    tag() const{
      return tag_;
    }
   public:
    RawObject():
      tag_(0),
      ptr_(0),
      forwarding_(0),
      num_generations_(0),
      num_references_(0){
    }
    virtual ~RawObject() = default;

    uword GetAddress() const{
      return (uword)this;
    }

    uword GetPointerAddress() const{
      return ptr_;
    }

    void* GetPointer() const{
      return (void*)ptr_;
    }

    Object* GetObjectPointer() const{
      return (Object*)GetPointer();
    }

    void SetPointerAddress(const uword& address){
      ptr_ = address;
    }

    void SetForwardingAddress(const uword& address){
      forwarding_ = address;
    }

    void* GetForwardingPointer() const{
      return ((void*)forwarding_);
    }

    uword GetForwardingAddress() const{
      return forwarding_;
    }

    bool IsForwarding() const{
      return forwarding_ != 0;
    }

    bool IsEden() const{
      return EdenBit::Decode(tag());
    }

    void SetEdenBit(){
      tag_ = EdenBit::Update(true, tag());
    }

    void ClearEdenBit(){
      tag_ = EdenBit::Update(false, tag());
    }

    bool IsTenured() const{
      return TenuredBit::Decode(tag());
    }

    void SetTenuredBit(){
      tag_ = TenuredBit::Update(true, tag());
    }

    void ClearTenuredBit(){
      tag_ = TenuredBit::Update(false, tag());
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
      return SizeField::Decode(tag());
    }

    void SetPointerSize(const uint32_t& val){
      tag_ = SizeField::Update(val, tag());
    }

    uint64_t GetTotalSize() const{
      return sizeof(RawObject)+GetPointerSize();
    }

    bool IsReadyForPromotion() const{
      return GetNumberOfGenerationsSurvived() >= 3;//TODO: externalize constant
    }

    uint64_t GetNumberOfReferences() const{
      return num_references_;
    }

    uint32_t GetNumberOfGenerationsSurvived() const{
      return num_generations_;
    }

    void VisitPointers(RawObjectPointerVisitor* vis);
    void VisitPointers(RawObjectPointerPointerVisitor* vis);

    std::string ToString() const{
      std::stringstream ss;
      ss << "RawObject(";
      ss << "eden=" << (IsEden() ? "true" : "false") << ", ";
      ss << "tenured=" << (IsTenured() ? "true": "false") << ", ";
      ss << "marked=" << (IsMarked() ? "true": "false") << ", ";
      ss << "remembered=" << (IsRemembered() ? "true" : "false") << ", ";
      ss << "size=" << GetPointerSize() << ", ";
      ss << "references=" << GetNumberOfReferences() << ", ";
      ss << "pointer=" << GetPointer() << ", ";
      ss << "forwarding=" << GetForwardingPointer();
      ss << ")";
      return ss.str();
    }
  };
}

#endif //POSEIDON_RAW_OBJECT_H
