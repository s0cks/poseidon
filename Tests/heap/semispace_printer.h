#ifndef POSEIDON_SEMISPACE_PRINTER_H
#define POSEIDON_SEMISPACE_PRINTER_H

#include <glog/logging.h>
#include "poseidon/pointer.h"
#include "poseidon/heap/semispace.h"

namespace poseidon {
 template<const google::LogSeverity Severity = google::INFO>
 class SemispacePrinter : public RawObjectVisitor {
  private:
   static inline bool
   PrintSemispaceWithHeader(Semispace& semispace, const std::string& header) {
     LOG_AT_LEVEL(Severity) << header << ":";
     SemispacePrinter<Severity> printer;
     return semispace.VisitPointers(&printer);
   }
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
   PrintSemispace(Semispace& semispace) {
     return PrintSemispaceWithHeader(semispace, "Semispace");
   }

   static inline bool
   PrintFromspace(Semispace& semispace) {
     return PrintSemispaceWithHeader(semispace, "Fromspace");
   }

   static inline bool
   PrintTospace(Semispace& semispace) {
     return PrintSemispaceWithHeader(semispace, "Tospace");
   }
 };
}

#endif // POSEIDON_SEMISPACE_PRINTER_H