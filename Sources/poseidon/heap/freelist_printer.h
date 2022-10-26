#ifndef POSEIDON_FREELIST_PRINTER_H
#define POSEIDON_FREELIST_PRINTER_H

#include "poseidon/heap/freelist.h"

namespace poseidon {
 class FreeListPrinter : public FreeListNodeVisitor {
  protected:
   bool Visit(FreeListNode* node) override {
     LOG(INFO) << " - " << (*node);
     return true;
   }
  public:
   FreeListPrinter() = default;
   ~FreeListPrinter() override = default;

   static inline bool
   Print(FreeList& free_list) {
     LOG(INFO) << free_list << ":";
     FreeListPrinter printer;
     return free_list.VisitFreeNodes(&printer);
   }
 };
}

#endif // POSEIDON_FREELIST_PRINTER_H