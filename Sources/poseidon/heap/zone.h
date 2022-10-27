#ifndef POSEIDON_HEAP_ZONE_H
#define POSEIDON_HEAP_ZONE_H

#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/page_table.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Zone : public Section {
   friend class ZoneTest;
   friend class RawObject;
   friend class Scavenger;
  public:
   template<class T>
   class ZoneIterator : public RawObjectPointerIterator {
    protected:
     T* zone_;
     uword current_;

     inline T* zone() const {
       return zone_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline RawObject* current_ptr() const {
       return (RawObject*)current_address();
     }
    public:
     explicit ZoneIterator(T* zone):
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
  protected:
   static inline int64_t
   CalculateNumberOfPages(const MemoryRegion& region, const int64_t page_size) { //TODO: cleanup
     return region.GetSize() / page_size;
   }
  protected:
   MemoryRegion region_;
   void Clear();
  public:
   Zone():
    Section(),
    region_() {
   }

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(const MemoryRegion& region):
    Section(),
    region_(region) {
   }

   Zone(const Zone& rhs) = default;
   ~Zone() override = default;

   uword GetStartingAddress() const override {
     return region_.GetStartingAddress();
   }

   int64_t GetSize() const override {
     return region_.GetSize();
   }

   virtual bool VisitPages(PageVisitor* vis) = 0;
   virtual bool VisitMarkedPages(PageVisitor* vis) = 0;
   virtual bool VisitUnmarkedPages(PageVisitor* vis) = 0;

   Zone& operator=(const Zone& rhs){
     if(*this == rhs)
       return *this;
     Section::operator=(rhs);
     region_ = rhs.region_;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Zone& zone){
     stream << "Zone(";
     stream << "starting_address=" << zone.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << Bytes(zone.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif //POSEIDON_HEAP_ZONE_H
