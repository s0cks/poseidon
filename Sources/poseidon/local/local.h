#ifndef POSEIDON_LOCAL_H
#define POSEIDON_LOCAL_H

#include "poseidon/local/local_page.h"

namespace poseidon {
 template<typename T>
 class Local {
   template<typename U>
   friend class Local;
  protected:
   uword address_;

   inline uword address() const {
     return address_;
   }

   inline void* address_ptr() const {
     return (void*)address();
   }

   RawObject** GetLocalPointer() const {
     return (RawObject**)address();
   }
  public:
   Local():
    address_(0) {
     auto page = LocalPage::GetForCurrentThread();
     if(page == nullptr) {
       LOG(WARNING) << "cannot allocate Local on thread " << GetCurrentThreadName() << " with missing LocalPage";
       return;
     }
     address_ = page->TryAllocate();
     LOG_IF(FATAL, IsEmpty()) << "cannot allocate Local on the " << GetCurrentThreadName() << " thread.";
   }

   explicit Local(RawObject* ptr):
    Local() {
     (*GetLocalPointer()) = ptr;
   }
   Local(Local<T>&& rhs) noexcept = default;

   template<typename U>
   explicit Local(const Local<U>& rhs):
    Local(rhs) {
   }

   template<typename U>
   explicit Local(Local<U>&& rhs):
    Local(std::move(rhs)) {
   }

   ~Local() = default;

   bool IsEmpty() const {
     return address() == UNALLOCATED;
   }

   RawObject* raw_ptr() const {
     return (*GetLocalPointer());
   }

   T* Get() const {
     if(IsEmpty())
       return nullptr;
     return raw_ptr()->GetPointer();
   }

   Local& operator=(RawObject* ptr) {
     if(!IsEmpty() && (*GetLocalPointer())->GetStartingAddress() == ptr->GetStartingAddress() && (*GetLocalPointer())->GetSize() == ptr->GetSize())
       return *this;
     (*GetLocalPointer()) = ptr;
     return *this;
   }

   Local& operator=(const Local& rhs){
     if(&rhs == this)
       return *this;
     address_ = rhs.address();
     return *this;
   }

   Local& operator=(Local&& rhs) noexcept {
     if(&rhs == this)
       return *this;
     address_ = rhs.address();
     return *this;
   }

   template<typename U>
   Local& operator=(const Local<U>& rhs) {
     operator=(rhs);
     return *this;
   }

   T* operator->() const {
     return Get();
   }

   explicit operator T*() const {
     return Get();
   }

   explicit operator bool() const {
     return operator T*();
   }

   template<typename U>
   explicit operator U*() const {
     return static_cast<U*>(operator T*());
   }

   template<typename U>
   Local<U> CastTo() const {
     return static_cast<U*>(operator T*());
   }

   const std::type_info& TypeId() const {
     return typeid(*operator T*());
   }

   template<typename U>
   bool InstanceOf() const {
     return TypeId() == typeid(U);
   }

   template<typename U>
   Local<U> DynamicCastTo() const {
     return dynamic_cast<U*>(operator T*());
   }
 };
}

#endif // POSEIDON_LOCAL_H