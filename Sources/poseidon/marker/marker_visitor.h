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

   virtual bool Mark(RawObject* raw) = 0;
   virtual bool MarkPage(Page* page) = 0;

   virtual bool MarkAllRoots() {
     TIMED_SECTION("MarkAllRoots", {
       auto page = LocalPage::GetLocalPageForCurrentThread();
       while(page != nullptr){
         page->VisitObjects(this);
         page = page->GetNext();
       }
       return true;
     });
   }

   virtual bool MarkAllPages(Zone& zone) {
     TIMED_SECTION("MarkAllPages", {
       zone.VisitPages(this);
     });
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
       return true;
     });
   }
 };
}

#endif // POSEIDON_MARKER_VISITOR_H