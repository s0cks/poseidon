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
      // .space
      kSpaceFieldOffset = 0,
      kBitsForSpaceField = 2,

      // .color
      kColorFieldOffset = kSpaceFieldOffset+kBitsForSpaceField,
      kBitsForColorField = 2,

      // .generations
      kGenerationsFieldOffset = kColorFieldOffset+kBitsForColorField,
      kBitsForGenerationField = 8,

      // .size
      kSizeFieldOffset = kGenerationsFieldOffset+kBitsForGenerationField,
      kBitsForSizeField = 32,

      // .references
      kReferencesFieldOffset = kSizeFieldOffset+kBitsForSizeField,
      kBitsForReferencesField = 16,

      // .remembered
      kRememberedFieldOffset = kReferencesFieldOffset+kBitsForReferencesField,
      kBitsForRememberedField = 1,

      // .raw_type //TODO: remove
      kRawTypeFieldOffset = kRememberedFieldOffset+kBitsForRememberedField,
      kBitsForRawTypeField = 1,
    };

    class SpaceField : public BitField<ObjectTag, Space, kSpaceFieldOffset, kBitsForSpaceField>{};
    class ColorField : public BitField<ObjectTag, Color, kColorFieldOffset, kBitsForColorField>{};
    class GenerationsField : public BitField<ObjectTag, uint8_t, kGenerationsFieldOffset, kBitsForGenerationField>{};
    class SizeField : public BitField<ObjectTag, uint32_t, kSizeFieldOffset, kBitsForSizeField>{};
    class ReferencesField : public BitField<ObjectTag, uint16_t, kReferencesFieldOffset, kBitsForReferencesField>{};
    class RememberedField : public BitField<ObjectTag, bool, kRememberedFieldOffset, kBitsForRememberedField>{};
    class RawTypeField : public BitField<ObjectTag, bool, kRawTypeFieldOffset, kBitsForRawTypeField>{};

    ObjectTag tag_;
    uword ptr_;
    uword forwarding_;

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
      forwarding_(0){
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

    Space GetSpace() const{
      return SpaceField::Decode(tag());
    }

    void SetSpace(const Space& val){
      tag_ = SpaceField::Update(val, tag());
    }

    Color GetColor() const{
      return ColorField::Decode(tag());
    }

    void SetColor(const Color& val){
      tag_ = ColorField::Update(val, tag());
    }

    uint8_t GetGenerations() const{
      return GenerationsField::Decode(tag());
    }

    void SetGenerations(const uint8_t& val){
      tag_ = GenerationsField::Update(val, tag());
    }

    uint32_t GetPointerSize() const{
      return SizeField::Decode(tag());
    }

    void SetPointerSize(const uint32_t& val){
      tag_ = SizeField::Update(val, tag());
    }

    uint16_t GetReferences() const{
      return ReferencesField::Decode(tag());
    }

    void SetReferences(const uint16_t& val){
      tag_ = ReferencesField::Update(val, tag());
    }

    bool IsRemembered() const{
      return RememberedField::Decode(tag());
    }

    void ClearRemembered(){
      tag_ = RememberedField::Update(false, tag());
    }

    void SetRemembered(){
      tag_ = RememberedField::Update(true, tag());
    }

    bool IsRawType() const{
      return RawTypeField::Decode(tag());
    }

    void ClearRawType(){
      tag_ = RawTypeField::Update(false, tag());
    }

    void SetRawType(){
      tag_ = RawTypeField::Update(true, tag());
    }

    uint64_t GetTotalSize() const{
      return sizeof(RawObject)+GetPointerSize();
    }

    bool IsMarked() const{
      return GetColor() == Color::kMarked;
    }

    bool IsReadyForPromotion() const{
      return GetGenerations() >= 3;//TODO: externalize constant
    }

    void VisitPointers(RawObjectPointerVisitor* vis);
    void VisitPointers(RawObjectPointerPointerVisitor* vis);

    std::string ToString() const{
      std::stringstream ss;
      ss << "RawObject(";
      ss << "space=" << GetSpace() << ", ";
      ss << "color=" << GetColor() << ", ";
      ss << "generations=" << GetGenerations() << ", ";
      ss << "size=" << GetPointerSize() << ", ";
      ss << "references=" << GetReferences() << ", ";
      ss << "pointer=" << GetPointer() << ", ";
      ss << "forwarding=" << GetForwardingPointer();
      ss << ")";
      return ss.str();
    }
  };
}

#endif //POSEIDON_RAW_OBJECT_H
