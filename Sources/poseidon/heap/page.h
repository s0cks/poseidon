#ifndef POSEIDON_HEAP_PAGE_H
#define POSEIDON_HEAP_PAGE_H

#include <ostream>

#include "poseidon/utils.h"
#include "poseidon/heap/section.h"

namespace poseidon{
 class Page : public AllocationSection{
  protected:
   int64_t index_;

   Page(int64_t index, uword start, int64_t size):
    AllocationSection(start, size),
    index_(index){
   }
  public:
   Page() = default;
   Page(const Page& rhs) = default;
   ~Page() override = default;

   int64_t GetIndex() const{
     return index_;
   }

   Page& operator=(const Page& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const Page& val){
     stream << "Page(";
     stream << "index=" << val.GetIndex() << ", ";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize());
     return stream << ")";
   }

   friend bool operator==(const Page& lhs, const Page& rhs){
     return lhs.GetIndex() == rhs.GetIndex()
         && lhs.GetStartingAddress() == rhs.GetStartingAddress()
         && lhs.GetSize() == rhs.GetSize();
   }

   friend bool operator!=(const Page& lhs, const Page& rhs){
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const Page& lhs, const Page& rhs){
     return lhs.GetIndex() < rhs.GetIndex();
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H