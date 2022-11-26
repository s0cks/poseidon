#ifndef POSEIDON_HEAP_ZONE_H
#define POSEIDON_HEAP_ZONE_H

#include "poseidon/bitset.h"
#include "poseidon/heap/page/page.h"
#include "poseidon/pointer.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 template<class P>
 class Zone : public Section {
   friend class ZoneTest;
   friend class Pointer;
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

     inline Pointer* current_ptr() const {
       return (Pointer*)current_address();
     }
    public:
     explicit ZoneIterator(Z* zone):
      RawObjectPointerIterator(),
      zone_(zone),
      current_(zone->GetStartingAddress()) {
     }
     ~ZoneIterator() override = default;

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };

   class ZonePageIterator {
    protected:
     Zone<P>* zone_;
     PageIndex current_;
     PageIndex last_;

     inline Zone<P>* zone() const {
       return zone_;
     }

     inline PageIndex current_index() const {
       return current_;
     }

     inline PageIndex last_index() const {
       return last_;
     }

     explicit ZonePageIterator(Zone<P>* zone):
      zone_(zone),
      current_(0),
      last_(zone->GetNumberOfPages()){
     }
    public:
     virtual ~ZonePageIterator() = default;

     virtual bool HasNext() const {
       return current_index() < zone()->GetNumberOfPages() &&
              zone()->pages(current_index()) != nullptr;
     }

     virtual P* Next() {
       auto next = zone()->pages(current_index());
       current_ += 1;
       return next;
     }
   };
  protected:
   static inline int64_t
   CalculateNumberOfPages(const MemoryRegion& region, const int64_t page_size) { //TODO: cleanup
     return region.GetSize() / page_size;
   }

   template<class Iterator, class Visitor>
   static inline bool IterateUnmarkedPages(Zone<P>* zone, Visitor* vis) {
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
       if(next->IsMarked()) {
         if(!vis->Visit(next))
           return false;
       }
     }
     return true;
   }
  protected:
   uword start_;
   int64_t size_;
   BitSet pages_table_;
   P* pages_;
   int64_t num_pages_;

   Zone(const uword start_address, const ObjectSize size, const ObjectSize page_size):
    start_(start_address),
    size_(size),
    pages_table_(size / page_size),
    pages_(new P[size / page_size]),
    num_pages_(size / page_size) {
     for(auto idx = 0; idx < num_pages_; idx++) {
       pages_[idx] = P(idx, start_address + (idx * page_size), page_size);
     }
   }
  public:
   Zone() = delete;
   ~Zone() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   int64_t GetSize() const override {
     return size_;
   }

   virtual int64_t GetNumberOfPages() const {
     return num_pages_;
   }

   virtual P* pages() const {
     return pages_;
   }

   virtual P* pages(const PageIndex index) {
     if(index < 0 || index > GetNumberOfPages())
       return nullptr;
     return &pages_[index];
   }

   virtual P* pages_begin() {
     return &pages_[0];
   }

   virtual P* pages_end() {
     return &pages_[num_pages_];
   }

   virtual bool IsMarked(const PageIndex index) const {
     return pages_table_.Test(index);
   }

   virtual bool IsMarked(const P* page) const {
     return IsMarked(page->GetIndex());
   }

   virtual void Mark(const PageIndex index) {
     return pages_table_.Set(index, true);
   }

   virtual void Mark(const P* page) {
     return Mark(page->GetIndex());
   }

   virtual void Unmark(const PageIndex index) {
     return pages_table_.Set(index, false);
   }

   virtual void Unmark(const P* page) {
     return Unmark(page->GetIndex());
   }
 };
}

#endif //POSEIDON_HEAP_ZONE_H
