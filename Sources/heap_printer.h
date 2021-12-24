#ifndef POSEIDON_HEAP_PRINTER_H
#define POSEIDON_HEAP_PRINTER_H

#include <glog/logging.h>
#include "raw_object.h"

namespace poseidon{
 class HeapPrinter : public RawObjectPointerVisitor{
  private:
   google::LogSeverity severity_;
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
     auto from = heap->GetFromSpace();
     auto to = heap->GetToSpace();

     HeapPrinter printer(Severity);
     LOG_AT_LEVEL(Severity) << heap->GetSpace() << " Heap:";
     LOG_AT_LEVEL(Severity) << "From Space: "
      << HumanReadableSize(from.GetAllocatedBytes()) << "/" << HumanReadableSize(from.GetTotalBytes())
      << " (" << PrettyPrintPercentage(from.GetAllocatedPercentage()) << "%)";
     LOG_AT_LEVEL(Severity) << "To Space: "
      << HumanReadableSize(to.GetAllocatedBytes()) << "/" << HumanReadableSize(to.GetTotalBytes())
      << " (" << PrettyPrintPercentage(to.GetAllocatedPercentage()) << "%)";
     return heap->VisitRawObjectPointers(&printer);
   }
 };
}

#endif //POSEIDON_HEAP_PRINTER_H
