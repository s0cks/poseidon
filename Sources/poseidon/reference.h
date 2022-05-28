#ifndef POSEIDON_REFERENCE_H
#define POSEIDON_REFERENCE_H

#include "poseidon/raw_object.h"

namespace poseidon{
#define FOR_EACH_REFERENCE_TYPE(V) \
 V(Weak)                           \
 V(Strong)

#define FORWARD_DECLARE_REFERENCE_TYPE(Name) class Name##Reference;
 FOR_EACH_REFERENCE_TYPE(FORWARD_DECLARE_REFERENCE_TYPE)
#undef FORWARD_DECLARE_REFERENCE_TYPE

 class Reference{
  protected:
   uword owner_;
   uword target_;

   Reference(uword owner, uword target):
    owner_(owner),
    target_(target){ //TODO: we should probably assert owner & target are in the heap
     PSDN_ASSERT(owner > 0);
     PSDN_ASSERT(target > 0);
   }

   Reference(RawObject* owner, RawObject* target):
    Reference(owner->GetAddress(), target->GetAddress()){
   }
  public:
   Reference() = default;
   Reference(const Reference& rhs) = default;
   virtual ~Reference() = default;

   uword GetOwnerAddress() const{
     return owner_;
   }

   void* GetOwnerPointer() const{
     return (void*)GetOwnerAddress();
   }

   RawObject* GetOwner() const{
     return (RawObject*)GetOwnerAddress();
   }

   uword GetTargetAddress() const{
     return target_;
   }

   void* GetTargetPointer() const{
     return (void*)GetTargetAddress();
   }

   RawObject* GetTarget() const{
     return (RawObject*)GetTargetAddress();
   }

#define DEFINE_TYPE_CHECK(Name) \
   virtual Name##Reference* As##Name##Reference(){ return nullptr; } \
   bool Is##Name##Reference() { return As##Name##Reference() != nullptr; }
   FOR_EACH_REFERENCE_TYPE(DEFINE_TYPE_CHECK)
#undef DEFINE_TYPE_CHECK

   Reference& operator=(const Reference& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const Reference& val){
     stream << "Reference(";
     stream << ")";
     return stream;
   }

   friend bool operator==(const Reference& lhs, const Reference& rhs){

   }

   friend bool operator!=(const Reference& lhs, const Reference& rhs){
     return !operator==(lhs, rhs);
   }
 };

#define DEFINE_REFERENCE_TYPE(Name) \
  public:                           \
    Name##Reference* As##Name##Reference() override{ return this; }

 class WeakReference : public Reference{
  public:
   WeakReference() = default;
   WeakReference(const WeakReference& rhs) = default;
   ~WeakReference() override = default;

   WeakReference& operator=(const WeakReference& rhs) = default;

   DEFINE_REFERENCE_TYPE(Weak);
 };

 class StrongReference : public Reference{
  public:
   StrongReference() = default;
   StrongReference(const StrongReference& rhs) = default;
   ~StrongReference() override = default;

   StrongReference& operator=(const StrongReference& rhs) = default;

   DEFINE_REFERENCE_TYPE(Strong);
 };

#undef DEFINE_REFERENCE_TYPE
}

#endif//POSEIDON_REFERENCE_H