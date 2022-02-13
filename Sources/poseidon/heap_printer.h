#ifndef POSEIDON_HEAP_PRINTER_H
#define POSEIDON_HEAP_PRINTER_H

#include <glog/logging.h>
#include "raw_object.h"

namespace poseidon{
 class HeapPrinter : public RawObjectPointerVisitor{
  private:
   google::LogSeverity severity_;

   inline void
   PrintHeader(Heap* heap) const{
     auto from_space = heap->GetFromSpace();
     auto to_space = heap->GetToSpace();
     LOG_AT_LEVEL(GetSeverity()) << heap->GetSpace() << " Heap ("
        << "from=" << HumanReadableSize(from_space.GetAllocatedBytes()) << "/" << HumanReadableSize(from_space.GetTotalBytes()) << " " << PrettyPrintPercentage(from_space.GetAllocatedPercentage()) << "; "
        << "to=" << HumanReadableSize(to_space.GetAllocatedBytes()) << "/" << HumanReadableSize(to_space.GetTotalBytes()) << " " << PrettyPrintPercentage(to_space.GetAllocatedPercentage()) << "):";
   }
  public:
   explicit HeapPrinter(const google::LogSeverity& severity):
    RawObjectPointerVisitor(),
    severity_(severity){
   }
   ~HeapPrinter() override = default;

   google::LogSeverity GetSeverity() const{
     return severity_;
   }

   bool Visit(RawObject* raw) override{
     LOG_AT_LEVEL(GetSeverity()) << " - " << raw->ToString();
     return true;
   }

   template<const google::LogSeverity& Severity=google::INFO>
   static inline void
   Print(Heap* heap){
     HeapPrinter printer(Severity);
     printer.PrintHeader(heap);
     if(heap->IsEmpty()){
       LOG_AT_LEVEL(Severity) << " - N/A";
       return;
     }
     return heap->VisitRawObjectPointers(&printer);
   }
 };
}

#endif //POSEIDON_HEAP_PRINTER_H
