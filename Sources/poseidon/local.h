#ifndef POSEIDON_LOCAL_H
#define POSEIDON_LOCAL_H

#include <utility>
#include <typeinfo>
#include <glog/logging.h>

#include "raw_object.h"

namespace poseidon{
 class RawObject;
 class LocalBase{
   friend class Allocator;
  protected:
   LocalBase* previous_;
   LocalBase* next_;
   uword* slot_;

   LocalBase():
     previous_(nullptr),
     next_(nullptr),
     slot_(nullptr){
   }
   explicit LocalBase(uword* slot):
     previous_(nullptr),
     next_(nullptr),
     slot_(slot){
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

   bool HasSlot() const{
     return slot_ != nullptr;
   }

   uword* slot() const{
     return slot_;
   }

   bool SetSlotAddress(uword address){
     if(!HasSlot())
       return false;
     (*slot()) = address;
     return true;
   }

   inline bool SetSlotPointer(RawObject* ptr){
     return SetSlotAddress(ptr->GetAddress());
   }

   uword GetSlotAddress() const{
     return HasSlot() ? (uword)(*slot()) : 0;
   }

   inline RawObject* GetSlotPointer() const{
     return (RawObject*)GetSlotAddress();
   }
  public:
   LocalBase(const LocalBase& rhs) = default;
   ~LocalBase() = default;

   LocalBase& operator=(const LocalBase& rhs) = default;
 };

 template<typename T>
 class Local : public LocalBase{
  public:
   template<class S>
   friend
   class Local;

   friend class Allocator;
  private:
   explicit Local(uword* slot):
    LocalBase(slot){
   }
  public:
   Local(): LocalBase(){}

   Local(const Local&& rhs) noexcept:
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
     if(!HasSlot())
       return nullptr;
     return (T*)GetSlotPointer()->GetPointer();
   }

   RawObject* raw() const{
     return GetSlotPointer();
   }

   Local& operator=(const Local& rhs){
     if(this == &rhs)
       return *this;
     slot_ = rhs.slot_;
     return *this;
   }

   Local& operator=(const uword& rhs){
     if(!HasSlot())
       return *this;//TODO: set from allocator
     SetSlotAddress(rhs);
     return *this;
   }

   Local& operator=(Local&& rhs) noexcept{
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

   explicit operator T*() const{
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

 class LocalGroup{
   friend class Allocator;
  public:
   static const uint64_t kMaxLocalsPerGroup = 65535;

   class Iterator : public RawObjectPointerIterator{
    private:
     const LocalGroup* head_;
     const LocalGroup* current_group_;
     uint64_t current_local_;
     bool recursive_: 1;
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
       // no more in current group, check the next group if available
       return current_group_ && IsRecursive() && GetCurrentGroup()->HasNext();
     }

     RawObject* Next() override{
       auto next = GetCurrentGroup()->GetLocal(current_local_);
       current_local_ += 1;
       if(IsRecursive() && current_local_ >= GetCurrentGroup()->GetNumberOfLocals())
         current_group_ = GetCurrentGroup()->GetNext();
       return (RawObject*)next;
     }
   };
  private:
   LocalGroup* next_;
   LocalGroup* previous_;
   RelaxedAtomic<uword> locals_[kMaxLocalsPerGroup];
   uword num_locals_;
  public:
   LocalGroup():
    next_(nullptr),
    previous_(nullptr),
    locals_(),
    num_locals_(0){
   }
   explicit LocalGroup(LocalGroup* parent):
     previous_(nullptr),
     next_(parent),
     locals_(),
     num_locals_(0){
     if(parent != nullptr)
       parent->SetPrevious(this);
   }
   ~LocalGroup() = default;

   LocalGroup* GetNext() const{
     return next_;
   }

   void SetNext(LocalGroup* group){
     next_ = group;
   }

   bool HasNext() const{
     return next_ != nullptr;
   }

   LocalGroup* GetPrevious() const{
     return previous_;
   }

   void SetPrevious(LocalGroup* group){
     previous_ = group;
   }

   bool HasPrevious() const{
     return previous_ != nullptr;
   }

   RelaxedAtomic<uword>* GetLocalSlot(uword idx){
#ifdef PSDN_DEBUG
     assert(idx >= 0 && idx <= num_locals_);
#endif//PSDN_DEBUG
     return &locals_[idx];
   }

   uword GetLocal(uword idx) const{
#ifdef PSDN_DEBUG
     assert(idx >= 0 && idx <= num_locals_);
#endif//PSDN_DEBUG
     return locals_[idx].load();
   }

   uword GetNumberOfLocals() const{
     return num_locals_;
   }

   bool IsEmpty() const{
     return GetNumberOfLocals() == 0;
   }

   void VisitLocals(RawObjectVisitor* vis) const{
     if(IsEmpty())
       return;
     Iterator iter(this, false);
     while(iter.HasNext()){
       auto local = iter.Next();
       if(local && !vis->Visit(local))
         return;
     }
   }

   void VisitLocals(const std::function<bool(RawObject*)>& vis) const{
     if(IsEmpty()){
       DLOG(WARNING) << "local group is empty.";
       return;
     }
     Iterator iter(this, false);
     while(iter.HasNext()){
       auto local = iter.Next();
       if(local && !vis(local))
         return;
     }
   }
 };
}

#endif //POSEIDON_LOCAL_H