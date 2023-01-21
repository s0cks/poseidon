#ifndef PSDN_FREELIST_PRINTER_H
#define PSDN_FREELIST_PRINTER_H

#include "freelist.h"

namespace poseidon {
 class FreeListPrinter : public FreePointerVisitor {
  protected:
   FreeList* free_list_;
   const google::LogSeverity severity_;

   explicit FreeListPrinter(const google::LogSeverity severity,
                            FreeList* free_list):
    FreePointerVisitor(),
    severity_(severity),
    free_list_(free_list) {
   }
  public:
   ~FreeListPrinter() override = default;

   google::LogSeverity GetSeverity() const {
     return severity_;
   }

   FreeList* GetFreeList() const {
     return free_list_;
   }

   bool VisitFreeListStart() override {
     LOG_AT_LEVEL(GetSeverity()) << std::string(50, '=');
     LOG_AT_LEVEL(GetSeverity()) << "Freelist (" << GetFreeList()->GetStartingAddressPointer() << "-" << GetFreeList()->GetEndingAddressPointer() << " " << Bytes(GetFreeList()->GetSize()) << "):";
     return true;
   }

   bool VisitFreePointer(FreePointer* free_ptr) override {
     LOG_AT_LEVEL(GetSeverity()) << " - " << (*free_ptr);
     return true;
   }

   bool VisitFreeListEnd() override {
     LOG_AT_LEVEL(GetSeverity()) << std::string(50, '=');
     return true;
   }

   bool PrintFreeList() {
     if(!VisitFreeListStart())
       return false;
     if(!GetFreeList()->VisitFreePointers(this))
       return false;
     if(!VisitFreeListEnd())
       return false;
     return true;
   }

   template<const google::LogSeverity Severity=google::INFO>
   static inline bool Print(FreeList* free_list) {
     FreeListPrinter printer(Severity, free_list);
     return printer.PrintFreeList();
   }
 };
}

#endif //PSDN_FREELIST_PRINTER_H