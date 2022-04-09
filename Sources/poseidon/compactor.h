#ifndef POSEIDON_COMPACTOR_H
#define POSEIDON_COMPACTOR_H

#include "poseidon/platform.h"

namespace poseidon{
 class HeapPage;
 class Compactor{
   //TODO: convert to table based algorithm
  private:
   uword start_;
   uint64_t size_;

   uword live_;
   uword free_;

   inline uword GetStartingAddress() const{
     return start_;
   }

   inline uint64_t size() const{
     return size_;
   }

   inline uword GetEndingAddress() const{
     return GetStartingAddress() + size();
   }

   inline bool Contains(uword address) const{
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }

   inline RawObject* live_ptr() const{
     return (RawObject*)live_;
   }

   inline uint64_t ForwardObject(RawObject* val){
     DLOG(INFO) << "forwarding " << val->ToString() << " to " << (void*)free_;
     val->SetForwardingAddress(free_);
     return val->GetTotalSize();
   }

   void MarkLiveObjects();
   void ComputeForwardingAddresses();
   void UpdateLiveObjects();
   void ForwardAndFinalizeObjects();

   inline void MarkObject(RawObject* src){
     DLOG(INFO) << "marking " << src->ToString();
     src->SetMarkedBit();
   }

   inline uint64_t CopyObject(RawObject* src, RawObject* dst){
     DLOG(INFO) << "copying " << src->ToString() << " to " << dst->ToString();
     new (dst)RawObject();
     dst->SetPointerSize(src->GetPointerSize());
     dst->SetOldBit();

#ifdef PSDN_DEBUG
     assert(src->GetTotalSize() == dst->GetTotalSize());
#endif//PSDN_DEBUG
     memcpy(dst->GetPointer(), src->GetPointer(), src->GetPointerSize());
     return dst->GetTotalSize();
   }

   inline uint64_t Copy(RawObject* val){
     auto next_address = val->GetForwardingAddress();
     return CopyObject(val, (RawObject*)next_address);
   }
  public:
   explicit Compactor(OldPage* page):
    start_(page->GetStartingAddress()),
    size_(page->GetSize()),
    live_(page->GetStartingAddress()),
    free_(page->GetStartingAddress()){
   }
   Compactor(const Compactor& rhs) = delete;
   ~Compactor() = default;

   void Compact();

   Compactor& operator=(const Compactor& rhs) = delete;
 };
}

#endif //POSEIDON_COMPACTOR_H