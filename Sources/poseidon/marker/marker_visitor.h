#ifndef POSEIDON_MARKER_VISITOR_H
#define POSEIDON_MARKER_VISITOR_H

#include "poseidon/local.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/page.h"

namespace poseidon {
 template<bool Parallel>
 class MarkerVisitor : public PageVisitor, public RawObjectVisitor {
  protected:
   MarkerVisitor() = default;

   virtual bool Mark(RawObject* raw) {
     DLOG(INFO) << "marking " << (*raw);
     TIMED_SECTION("MarkObject", {
       raw->SetMarkedBit();
       return raw->IsMarked();
     });
   }

   bool VisitPage(Page* page) override {
     return MarkPage(page);
   }

   bool Visit(RawObject* raw) override {
     return Mark(raw);
   }
  public:
   ~MarkerVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   virtual bool MarkPage(Page* page) = 0;

   virtual bool MarkAllRoots() {
     TIMED_SECTION("MarkAllRoots", {
       auto page = LocalPage::GetLocalPageForCurrentThread();
       while(page != nullptr){
         page->VisitObjects(this);
         page = page->GetNext();
       }
     });
     return true;
   }

   virtual bool MarkAllPages(Zone& zone) {
     TIMED_SECTION("MarkAllPages", {
       if(!zone.VisitPages(this))
         return false;
     });
     return true;
   }

   virtual bool MarkAll(Zone& zone) {
     TIMED_SECTION("MarkAll", {
       if(!MarkAllRoots()) {
         LOG(WARNING) << "cannot mark all roots";
         return false;
       }

       if(!MarkAllPages(zone)) {
         LOG(WARNING) << "cannot mark all pages";
         return false;
       }
     });
     return true;
   }
 };
}

#endif // POSEIDON_MARKER_VISITOR_H