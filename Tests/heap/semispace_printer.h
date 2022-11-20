#ifndef POSEIDON_SEMISPACE_PRINTER_H
#define POSEIDON_SEMISPACE_PRINTER_H

#include <glog/logging.h>
#include "poseidon/pointer.h"
#include "poseidon/heap/semispace.h"

namespace poseidon {
 template<const google::LogSeverity Severity = google::INFO>
 class SemispacePrinter : public RawObjectVisitor {
  public:
   SemispacePrinter() = default;
   ~SemispacePrinter() override = default;

   google::LogSeverity GetSeverity() const {
     return Severity;
   }

   bool Visit(Pointer* raw) override {
     LOG_AT_LEVEL(GetSeverity()) << " - " << (*raw);
     return true;
   }

   static inline bool
   PrintFromspace(Semispace& semispace) {
     LOG_AT_LEVEL(Severity) << "Fromspace:";
     SemispacePrinter<Severity> printer;
     return semispace.VisitPointers(&printer);
   }

   static inline bool
   PrintTospace(Semispace& semispace) {
     LOG_AT_LEVEL(Severity) << "Tospace:";
     SemispacePrinter<Severity> printer;
     return semispace.VisitPointers(&printer);
   }
 };
}

#endif // POSEIDON_SEMISPACE_PRINTER_H