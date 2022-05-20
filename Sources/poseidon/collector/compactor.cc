#include "poseidon/heap/heap.h"
#include "poseidon/flags.h"
#include "poseidon/collector/compactor.h"
#include "poseidon/collector/finalizer.h"
#include "poseidon/raw_object.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon{
 class SerialCompactor;
 class ParallelCompactor;
 class ParallelCompactorTask;

 static RelaxedAtomic<bool> isCompacting(false);

 void Compactor::SetCompacting(bool active){
   isCompacting = active;
 }

 bool Compactor::IsCompacting(){
   return (bool)isCompacting;
 }

 template<bool Parallel>
 class CompactorVisitorBase : public RawObjectPointerVisitor{
  protected:
   CompactorVisitorBase() = default;
  public:
   ~CompactorVisitorBase() override = default;

   bool IsParallel() const{
     return Parallel;
   }
 };

 class SerialCompactor : public CompactorVisitorBase<false>{
  private:
   uword start_;
   int64_t size_;

   uword live_;
   uword free_;

   inline uword GetStartingAddress() const{
     return start_;
   }

   inline int64_t GetSize() const{
     return size_;
   }

   inline uword GetEndingAddress() const{
     return GetStartingAddress() + GetSize();
   }

   inline RawObject* live_ptr() const{
     return (RawObject*)live_;
   }

   inline RawObject* free_ptr() const{
     return (RawObject*)free_;
   }

   inline int64_t Forward(RawObject* ptr){ //TODO: need to copy object over
     DLOG(INFO) << "forwarding " << ptr->ToString() << " to " << free_ptr();
     ptr->SetForwardingAddress(free_ptr()->GetAddress());
     return ptr->GetTotalSize();
   }

   inline int64_t CopyObject(RawObject* src, RawObject* dst){
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

   inline int64_t Copy(RawObject* val){
     auto next_address = val->GetForwardingAddress();
     return CopyObject(val, (RawObject*)next_address);
   }

   void ComputeForwardingAddressAndFinalizeObjects(){
     while(live_ < GetEndingAddress() && live_ptr()->GetPointerSize() > 0){
       auto ptr = live_ptr();
       if(ptr->IsMarked()){
         free_ += Forward(ptr);
       }
       live_ += ptr->GetTotalSize();
     }
   }

   void ForwardAndFinalizeObjects(){
     auto next_address = GetStartingAddress();
     auto current_address = GetStartingAddress();
     while(current_address < GetEndingAddress()){
       auto current = (RawObject*)current_address;
       if(current->GetPointerSize() <= 0)
         break;

       auto length = current->GetTotalSize();

       if(current->IsMarked()){
#ifdef PSDN_DEBUG
         assert(current->IsForwarding());
#endif//PSDN_DEBUG
         next_address += Copy(current);
       } else{
         Finalizer::Finalize(current);
       }

       current_address += length;
     }
     //TODO: SetCurrent(next_address);
   }
  public:
   explicit SerialCompactor(OldZone* zone):
    start_(zone->GetStartingAddress()),
    size_(zone->GetSize()),
    live_(zone->GetStartingAddress()),
    free_(zone->GetStartingAddress()){
   }
   ~SerialCompactor() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }

   void Compact(){
     TIMED_SECTION("ComputeForwarding", {
       ComputeForwardingAddressAndFinalizeObjects();
     });

     TIMED_SECTION("CopyingAndFinalizing", {
       ForwardAndFinalizeObjects();//TODO: rename
     })
   }
 };

 class ParallelCompactor : public CompactorVisitorBase<true>{

 };

 void Compactor::SerialCompact(){
   auto heap = Heap::GetCurrentThreadHeap();
   auto old_zone = heap->old_zone();

   SerialCompactor compactor(old_zone);
   TIMED_SECTION("SerialCompact", {
     compactor.Compact();
   });
 }

 void Compactor::ParallelCompact(){
   auto heap = Heap::GetCurrentThreadHeap();
   auto old_zone = heap->old_zone();

   TIMED_SECTION("ParallelCompact", {
     NOT_IMPLEMENTED(ERROR);//TODO: implement
   });
 }

 void Compactor::Compact(){
   if(IsCompacting()){
     DLOG(WARNING) << "already compacting.";
     return;
   }

   SetCompacting();
   if(ShouldUseParallelCompact()){
     ParallelCompact();
   } else{
     SerialCompact();
   }
   ClearCompacting();
 }
}