#include "poseidon/flags.h"
#include "poseidon/local.h"
#include "poseidon/marker.h"
#include "poseidon/raw_object.h"

namespace poseidon{


 template<bool Parallel>
 class MarkerVisitorBase : public RawObjectPointerVisitor{
  protected:
   MarkerVisitorBase() = default;
  public:
   ~MarkerVisitorBase() override = default;

   bool IsParallel() const{
     return Parallel;
   }
 };

 class SerialMarkerVisitor : public MarkerVisitorBase<false>{
  protected:
   void MarkRoots(){
     auto page = LocalPage::GetLocalPageForCurrentThread();
     while(page != nullptr){
       page->VisitPointers(this);
       page = page->GetNext();
     }
   }
  public:
   SerialMarkerVisitor() = default;
   ~SerialMarkerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(old_val->IsOld() && !old_val->IsMarked()){
       old_val->SetMarkedBit();
       GCLOG(3) << "marked " << old_val->ToString();
     }
     //TODO: mark references?
     return true;
   }

   void MarkAll(){
     MarkRoots();
   }
 };

 void Marker::SerialMark(){
   SerialMarkerVisitor visitor;
   visitor.MarkAll();
 }

 class ParallelMarkerVisitor : public MarkerVisitorBase<true>{
  public:
   ParallelMarkerVisitor() = default;
   ~ParallelMarkerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     NOT_IMPLEMENTED(ERROR);
     return true;
   }

   void MarkAll(){
     NOT_IMPLEMENTED(ERROR);
   }
 };

 void Marker::ParallelMark(){
   ParallelMarkerVisitor visitor;
   visitor.MarkAll();
 }

 void Marker::Mark(){
   if(ShouldUseParallelMark()){
     return ParallelMark();
   }
   return SerialMark();
 }
}