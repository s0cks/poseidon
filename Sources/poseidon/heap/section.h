#ifndef POSEIDON_SECTION_H
#define POSEIDON_SECTION_H

#include "poseidon/platform/platform.h"

namespace poseidon{
 class Section{
  protected:
   uword start_;
   int64_t size_;

   Section():
    start_(0),
    size_(0){
   }

   Section(uword start, int64_t size):
    start_(start),
    size_(size){
   }

   template<class S, class I>
   static inline void
   IteratePointers(const S* section, RawObjectVisitor* vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(!vis->Visit(next))
         return;
     }
   }

   template<class S, class I>
   static inline void
   IterateMarkedPointers(const S* section, RawObjectVisitor* vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis->Visit(next))
         return;
     }
   }

   template<class S, class I>
   static inline void
   IteratePointers(const S* section, RawObjectVisitorFunction vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(!vis(next))
         return;
     }
   }

   template<class S, class I>
   static inline void
   IterateMarkedPointers(const S* section, RawObjectVisitorFunction vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis(next))
         return;
     }
   }
  public:
   Section(const Section& rhs):
    start_(rhs.GetStartingAddress()),
    size_(rhs.GetSize()){
   }
   virtual ~Section() = default;

   uword GetStartingAddress() const{
     return start_;
   }

   void* GetStartingAddressPointer() const{
     return (void*)GetStartingAddress();
   }

   int64_t GetSize() const{
     return size_;
   }

   uword GetEndingAddress() const{
     return GetStartingAddress() + GetSize();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   bool Contains(uword address) const{
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }
 };
}

#endif//POSEIDON_SECTION_H