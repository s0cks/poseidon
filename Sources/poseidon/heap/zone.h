#ifndef POSEIDON_HEAP_ZONE_H
#define POSEIDON_HEAP_ZONE_H

#include "poseidon/raw_object.h"
#include "poseidon/heap/page.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Zone : public Section {
   friend class ZoneTest;
   friend class RawObject;
   friend class Scavenger;
  protected:
   template<class Z>
   class ZoneIterator : public RawObjectPointerIterator {
    protected:
     Z* zone_;
     uword current_;

     inline Z* zone() const {
       return zone_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline RawObject* current_ptr() const {
       return (RawObject*)current_address();
     }
    public:
     explicit ZoneIterator(Z* zone):
      RawObjectPointerIterator(),
      zone_(zone),
      current_(zone->GetStartingAddress()) {
     }
     ~ZoneIterator() override = default;

     RawObject* Next() override {
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };

   template<class Z, class P>
   class ZonePageIterator {
    protected:
     Z* zone_;
     int64_t total_;
     int64_t current_;

     inline Z* zone() const {
       return zone_;
     }

     inline int64_t current() const {
       return current_;
     }

     inline int64_t total() const {
       return total_;
     }

     explicit ZonePageIterator(Z* zone, const int64_t total):
      zone_(zone),
      total_(total),
      current_(0) {
     }
    public:
     virtual ~ZonePageIterator() = default;

     virtual bool HasNext() const {
       return current() < total();
     }

     virtual P* Next() {
       auto next = zone()->GetPageAt(current_);
       current_ += 1;
       return next;
     }
   };
  protected:
   static inline int64_t
   CalculateNumberOfPages(const MemoryRegion& region, const int64_t page_size) { //TODO: cleanup
     return region.GetSize() / page_size;
   }

   template<class Z, class Iterator, class Visitor>
   static inline bool IteratePages(Z* zone, Visitor* vis) {
     Iterator iter(zone);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(!vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator, class Visitor>
   inline bool IterateMarkedPages(Z* zone, Visitor* vis) {
     Iterator iter(zone);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(zone->IsPageMarked(next) && !vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator, class Visitor>
   static inline bool IterateUnmarkedPages(Z* zone, Visitor* vis) {
     Iterator iter(zone);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(!IsPageMarked(next) && !vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator, class Visitor>
   static inline bool IteratePointers(Z* zone, Visitor* vis) {
     Iterator iter(zone);
     while(iter.HasNext()) {
       auto next = iter.Next();
       DLOG(INFO) << "next: " << (*next);
       if(!vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator, class Visitor>
   static inline bool IterateMarkedPointers(Z* zone, Visitor* vis) {
     Iterator iter(zone);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(next->IsMarked() && !vis->Visit(next))
         return false;
     }
     return true;
   }
  protected:
   Zone() = default;

   virtual void Clear();

   virtual inline void Reset() {
     return Clear(); //TODO: refactor
   }
  public:
   ~Zone() override = default;
 };
}

#endif //POSEIDON_HEAP_ZONE_H
