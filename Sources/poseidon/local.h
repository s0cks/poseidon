#ifndef POSEIDON_LOCAL_H
#define POSEIDON_LOCAL_H

#include <utility>
#include <typeinfo>
#include <glog/logging.h>

#include "poseidon/bitset.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/os_thread.h"

namespace poseidon{
 class RawObject;
 class LocalBase{
   friend class Allocator;
  protected:
   LocalBase* previous_;
   LocalBase* next_;
   uword* slot_;

   LocalBase();

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
     return SetSlotAddress(ptr->GetStartingAddress());
   }

   uword GetSlotAddress() const{
     if(!HasSlot()){
       DLOG(WARNING) << "no slot for local.";
       return 0;
     }
     return (*slot());
   }

   inline RawObject* GetSlotPointer() const{
     return (RawObject*)GetSlotAddress();
   }
  public:
   LocalBase(const LocalBase& rhs) = default;
   ~LocalBase() = default;

   LocalBase& operator=(const LocalBase& rhs){
     if(this == &rhs)
       return *this;
     slot_ = rhs.slot_;
     return *this;
   }
 };

 template<typename T>
 class Local : public LocalBase{
  public:
   template<class S>
   friend
   class Local;

   friend class Allocator;
  public:
   Local():
    LocalBase(){
   }

   Local(const Local<T>&& rhs) noexcept:
     LocalBase(rhs){
   }

   template<typename U>
   explicit Local(const Local<U>& h):
     Local(h){
   }

   template<typename U>
   Local(Local<U>&& h):
     LocalBase(std::move(h)){
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
     if(!HasSlot()){
       DLOG(WARNING) << "local slot is null.";
       return *this;//TODO: set from allocator
     }
     SetSlotAddress(rhs);
     return *this;
   }

   Local& operator=(Local&& rhs) noexcept{
     LocalBase::operator=(rhs);
     return *this;
   }

   template<typename U>
   Local& operator=(const Local<U>& rhs) {
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

 /**
  * l1      -> heap@0x1
  *
  * l1'     -> l1
  * l1''    -> l1
  */
 class LocalPage{
   friend class Allocator;
   friend class LocalPageIterator;
  public:
   static const uint64_t kMaxLocalsPerGroup = 65535;

   class LocalPageIterator : public RawObjectPointerIterator{
    private:
     const LocalPage* head_;
     const LocalPage* current_group_;
     uint64_t current_local_;
     bool recursive_: 1;
    public:
     explicit LocalPageIterator(const LocalPage* group, bool recursive = false):
       head_(group),
       current_group_(group),
       current_local_(0),
       recursive_(recursive){
     }
     ~LocalPageIterator() override = default;

     const LocalPage* GetCurrentGroup() const{
       return current_group_;
     }

     const LocalPage* GetHeadGroup() const{
       return head_;
     }

     bool IsRecursive() const{
       return recursive_;
     }

     bool HasNext() const override{
       if(current_group_ && current_local_ < kMaxLocalsPerGroup)
         return current_group_->locals_[current_local_] != 0;// has more in current group
       // no more in current group, check the next group if available
       return current_group_ && IsRecursive() && GetCurrentGroup()->HasNext();
     }

     RawObject* Next() override{
       auto next = (RawObject*)current_group_->locals_[current_local_];
       current_local_ += 1;
       if(IsRecursive() && current_local_ >= kMaxLocalsPerGroup)
         current_group_ = GetCurrentGroup()->GetNext();
       return next;
     }

     RawObject** NextPointer(){
       auto next = (RawObject**)&(current_group_->locals_[current_local_]);
       current_local_ += 1;
       if(IsRecursive() && current_local_ >= kMaxLocalsPerGroup)
         current_group_ = GetCurrentGroup()->GetNext();
       return next;
     }
   };
  private:
   LocalPage* next_;
   LocalPage* previous_;
   uword locals_[kMaxLocalsPerGroup];
  public:
   LocalPage():
    next_(nullptr),
    previous_(nullptr),
    locals_(){
   }
   ~LocalPage() = default;

   LocalPage* GetNext() const{
     return next_;
   }

   void SetNext(LocalPage* group){
     next_ = group;
   }

   bool HasNext() const{
     return next_ != nullptr;
   }

   LocalPage* GetPrevious() const{
     return previous_;
   }

   void SetPrevious(LocalPage* group){
     previous_ = group;
   }

   bool HasPrevious() const{
     return previous_ != nullptr;
   }

   uword* GetLocalSlot(uword idx){
#ifdef PSDN_DEBUG
     assert(idx >= 0 && idx <= kMaxLocalsPerGroup);
#endif//PSDN_DEBUG
     return &locals_[idx];
   }

   uword GetLocal(uword idx) const{
#ifdef PSDN_DEBUG
     assert(idx >= 0 && idx <= kMaxLocalsPerGroup);
#endif//PSDN_DEBUG
     return locals_[idx];
   }

   uword* GetFirstLocalSlotAvailable(){
     for(auto idx = 0; idx < kMaxLocalsPerGroup; idx++){
       if(locals_[idx] == 0)
         return &(locals_[idx]);
     }
     return nullptr;
   }

   void VisitObjects(RawObjectVisitor* vis) const;
   void VisitObjects(const std::function<bool(RawObject*)>& vis) const;
   void VisitPointers(RawObjectPointerVisitor* vis) const;
   void VisitPointers(const std::function<bool(RawObject**)>& vis) const;
  private:
   static pthread_key_t kThreadKey;

   static inline void
   SetLocalPageForCurrentThread(LocalPage* page){
     SetCurrentThreadLocal(kThreadKey, page);
   }
  public:
   static inline bool
   CurrentThreadHasLocalPage(){
     return GetCurrentThreadLocal(kThreadKey) != nullptr;
   }

   static inline LocalPage*
   GetLocalPageForCurrentThread(){
     return (LocalPage*)GetCurrentThreadLocal(kThreadKey);
   }

   static inline void
   ResetLocalPageForCurrentThread(){
     if(CurrentThreadHasLocalPage()){
       delete GetLocalPageForCurrentThread();
       SetLocalPageForCurrentThread(nullptr);
     }

     SetLocalPageForCurrentThread(new LocalPage());
   }

   static void Initialize(){
     InitializeThreadLocal(kThreadKey);
     ResetLocalPageForCurrentThread();
   }
 };
}

#endif //POSEIDON_LOCAL_H