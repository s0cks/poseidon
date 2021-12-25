#ifndef POSEIDON_LOCAL_H
#define POSEIDON_LOCAL_H

#include <utility>
#include <typeinfo>
#include "raw_object.h"

namespace poseidon{
  class Object;
  class RawObject;
  class LocalBase{
    friend class Allocator;
   protected:
    LocalBase* previous_;
    LocalBase* next_;
    RawObject** value_;

    LocalBase():
      previous_(nullptr),
      next_(nullptr),
      value_(nullptr){
    }

    explicit LocalBase(RawObject** value):
      previous_(nullptr),
      next_(nullptr),
      value_(value){
    }

    LocalBase* GetPrevious() const{
      return previous_;
    }

    void SetPrevious(LocalBase* local){
      previous_ = local;
    }

    bool HasPreviouss() const{
      return previous_ != nullptr;
    }

    LocalBase* GetNext() const{
      return next_;
    }

    void SetNext(LocalBase* local){
      next_ = local;
    }

    bool HasNext() const{
      return next_ != nullptr;
    }

    void SetValue(RawObject* val){
      (*value_) = val;
    }

    void SetValue(Object* val);
   public:
    LocalBase(const LocalBase& rhs) = default;
    ~LocalBase() = default;

    RawObject* GetRawObjectPointer() const;
    Object* GetObjectPointer() const;
    LocalBase& operator=(const LocalBase& rhs) = default;
  };

  template<typename T>
  class Local : public LocalBase{
   public:
    template<class S>
    friend class Local;

    friend class Allocator;
   private:
    explicit Local(RawObject** val):
      LocalBase(val){
    }
   public:
    Local(): LocalBase(){}

    Local(const Local&& rhs) noexcept :
      LocalBase(rhs){
    }

    template<typename U>
    explicit Local(const Local<U>& h):
      Local(h){
    }

    template<typename U>
    explicit Local(Local<U>&& h):
      Local(std::move(h)){
    }

    T* Get() const{
      return (T*)GetObjectPointer();
    }

    void Set(T* val){
      SetValue((Object*)val);
    }

    Local& operator=(const T& rhs){
      *value_ = (uword)rhs;//TODO: bad assignment
      return *this;
    }

    Local& operator=(const Local& rhs){
      if(this == &rhs)
        return *this;
      value_ = rhs.value_;
      return *this;
    }

    Local& operator=(Local&& rhs) noexcept {
      LocalBase::operator=(rhs);
      return *this;
    }

    template<typename U>
    Local& operator=(const Local<U>& rhs){
      return operator=(rhs.Get());
    }

    T* operator->() const{
      return static_cast<T*>(Get());
    }

    explicit operator T *() const{
      return Get();
    }

    explicit operator bool() const{
      return operator T*();
    }

    template<typename D>
    explicit operator D*() const{
      return static_cast<D*>(operator T*());
    }

    template<typename U>
    Local<U> CastTo() const{
      return static_cast<U*>(operator T*());
    }

    const std::type_info& TypeId() const{
      return typeid(*operator T*());
    }

    template<typename U>
    bool InstanceOf() const{
      return TypeId() == typeid(U);
    }

    template<typename U>
    Local<U> DynamicCastTo() const{
      return dynamic_cast<U*>(operator T*());
    }
  };

  class LocalGroup {
    friend class Allocator;
   public:
    static const uint64_t kMaxLocalsPerGroup = 65535;

    class Iterator : public RawObjectPointerIterator{
     private:
      const LocalGroup* head_;
      const LocalGroup* current_group_;
      uint64_t current_local_;
      bool recursive_ : 1;
     public:
      explicit Iterator(const LocalGroup* group, bool recursive):
        head_(group),
        current_group_(group),
        current_local_(0),
        recursive_(recursive){
      }
      ~Iterator() override = default;

      const LocalGroup* GetCurrentGroup() const{
        return current_group_;
      }

      const LocalGroup* GetHeadGroup() const{
        return head_;
      }

      bool IsRecursive() const{
        return recursive_;
      }

      bool HasNext() const override{
        if(current_group_ && current_local_ < GetCurrentGroup()->GetNumberOfLocals())
          return true;// has more in current group
        // no more in current group, check for recursive iteration
        return current_group_ && IsRecursive() && GetCurrentGroup()->HasNext();
      }

      RawObject* Next() override{
        auto next = GetCurrentGroup()->GetLocal(current_local_++);
        if(IsRecursive() && current_local_ >= GetCurrentGroup()->GetNumberOfLocals())
          current_group_ = GetCurrentGroup()->GetNext();
        return next;
      }

      RawObject** NextPointer(){
        auto* ptr = &current_group_->locals_[current_local_++];
        if(IsRecursive() && current_local_ >= GetCurrentGroup()->GetNumberOfLocals())
          current_group_ = GetCurrentGroup()->GetNext();
        return const_cast<RawObject**>(ptr);
      }
    };
   private:
    LocalGroup *next_;
    LocalGroup *previous_;
    RawObject *locals_[kMaxLocalsPerGroup];
    uword num_locals_;
   public:
    LocalGroup() = default;
    explicit LocalGroup(LocalGroup *parent) :
      previous_(nullptr),
      next_(parent),
      locals_(),
      num_locals_(0) {
      if (parent != nullptr)
        parent->SetPrevious(this);
    }
    ~LocalGroup() = default;

    LocalGroup *GetNext() const {
      return next_;
    }

    void SetNext(LocalGroup *group) {
      next_ = group;
    }

    bool HasNext() const {
      return next_ != nullptr;
    }

    LocalGroup *GetPrevious() const {
      return previous_;
    }

    void SetPrevious(LocalGroup *group) {
      previous_ = group;
    }

    bool HasPrevious() const {
      return previous_ != nullptr;
    }

    RawObject *GetLocal(const uword &idx) const {
      return locals_[idx];//TODO: bounds check
    }

    uword GetNumberOfLocals() const {
      return num_locals_;
    }

    bool IsEmpty() const {
      return GetNumberOfLocals() == 0;
    }

    void VisitLocals(RawObjectPointerVisitor *vis) const{
      if (IsEmpty())
        return;
      Iterator iter(this, false);
      while(iter.HasNext()){
        auto local = iter.Next();
        if(local && !vis->Visit(local))
          return;
      }
    }

    void VisitLocals(RawObjectPointerPointerVisitor* vis) const {
      if (IsEmpty())
        return;
      Iterator iter(this, false);
      while(iter.HasNext()){
        auto ptr = iter.NextPointer();
        if((*ptr) && !vis->Visit(ptr))
          return;
      }
    }
  };
}

#endif //POSEIDON_LOCAL_H