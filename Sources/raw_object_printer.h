#ifndef POSEIDON_RAW_OBJECT_PRINTER_H
#define POSEIDON_RAW_OBJECT_PRINTER_H

#ifdef POSEIDON_DEBUG

#include <ostream>
#include <glog/logging.h>

#include "allocator.h"
#include "utils.h"

namespace poseidon{
  class RawObjectPrinter : public RawObjectPointerVisitor{
   private:
    google::LogSeverity severity_;

    explicit RawObjectPrinter(const google::LogSeverity& severity):
      severity_(severity){
    }
   public:
    ~RawObjectPrinter() override = default;

    google::LogSeverity GetSeverity() const{
      return severity_;
    }

    bool Visit(RawObject* obj) override{
      LOG_AT_LEVEL(GetSeverity()) << " - " << obj->ToString();
      return true;
    }

    template<const google::LogSeverity& Severity=google::INFO>
    static inline void
    PrintAll(Heap* heap){
      RawObjectPrinter printer(Severity);
      return heap->VisitRawObjectPointers(&printer);
    }

    template<const google::LogSeverity& Severity=google::INFO>
    static inline void
    PrintAll(Semispace& space){
      RawObjectPrinter printer(Severity);
      return space.VisitRawObjectPointers(&printer);
    }

    template<const google::LogSeverity& Severity=google::INFO>
    static inline void
    PrintAllMarked(Heap* heap){
      RawObjectPrinter printer(Severity);
      return heap->VisitMarkedRawObjectPointers(&printer);
    }

    template<const google::LogSeverity& Severity=google::INFO>
    static inline void
    PrintAllLocals(){
      RawObjectPrinter printer(Severity);
      return Allocator::VisitLocals(&printer);
    }
  };
}

#endif//POSEIDON_DEBUG
#endif//POSEIDON_RAW_OBJECT_PRINTER_H