#ifndef POSEIDON_ALLOCATOR_H
#define POSEIDON_ALLOCATOR_H

#include <glog/logging.h>

#include "poseidon/heap.h"

namespace poseidon{
 template<typename T>
 class Local;

 struct Reference{
   uword src;
   uword dst;

   friend std::ostream& operator<<(std::ostream& stream, const Reference& val){
     return stream << "Reference(from=" << val.src << ", to=" << ((void*)val.dst) << ")";
   }
 };

 class LocalGroup;
 class Allocator{
   friend class Scavenger;
  private:
   class RootPage{
    public:
     static constexpr const int64_t kDefaultNumberOfRootsPerPage = 1024;
    private:
     RootPage* next_;
     RootPage* previous_;

     MemoryRegion region_;
     uword current_;

     inline uword starting_address() const{
       return region_.GetStartingAddress();
     }

     inline uword current_address() const{
       return current_;
     }

     inline uword ending_address() const{
       return region_.GetEndingAddress();
     }

     inline Reference* reference(int64_t idx) const{
       auto offset = idx * (kWordSize * 2);
       return (Reference*)(starting_address() + offset);
     }

     inline int64_t size() const{
       return static_cast<int64_t>(current_address() - starting_address());
     }

     inline Reference* begin() const{
       return (Reference*)starting_address();
     }

     inline Reference* end() const{
       return (Reference*)ending_address();
     }
    public:
     explicit RootPage(int64_t num_roots = kDefaultNumberOfRootsPerPage):
      next_(nullptr),
      previous_(nullptr),
      region_(num_roots * kWordSize),
      current_(region_.GetStartingAddress()){
       if(!region_.Protect(MemoryRegion::kReadWrite))
         LOG(FATAL) << "cannot protect RootPage MemoryRegion.";
     }
     ~RootPage() = default;

     RootPage* GetNext() const{
       return next_;
     }

     bool HasNext() const{
       return next_ != nullptr;
     }

     void SetNext(RootPage* page){
       next_ = page;
     }

     RootPage* GetPrevious() const{
       return previous_;
     }

     bool HasPrevious() const{
       return previous_ != nullptr;
     }

     void SetPrevious(RootPage* page){
       previous_ = page;
     }

     uword* CreateReference(){
       void* ptr = (void*)current_;
       current_ += (sizeof(kWordSize) * 2);
       auto reference = new (ptr)Reference();
       return &reference->dst;
     }

     void VisitPointers(const std::function<bool(uword, uword)>& vis) const{
       for(auto & it : *this){
         if(it.dst != 0 && !vis(it.src, it.dst))
           return;
       }
     }
   };
  private:
   static RootPage* roots_;
  public:
   Allocator() = delete;
   Allocator(const Allocator& rhs) = delete;
   ~Allocator() = delete;

   static void Initialize();
   static void InitializeForThread();

   static inline uword
   Allocate(int64_t size){
     auto heap = Heap::GetCurrentThreadHeap();
#ifdef PSDN_DEBUG
     assert(heap != nullptr);
#endif//PSDN_DEBUG
     return heap->Allocate(size);
   }

   template<typename T>
   static Local<T> AllocateLocal(){//TODO: create new page if current page is full.
     return Local<T>((uword*)roots_->CreateReference());
   }

   static inline void*
   New(size_t size){
     auto ptr = (RawObject*)Allocate(static_cast<int64_t>(size));
     return ptr != nullptr ? ptr->GetPointer() : nullptr;
   }

   template<typename T>
   static inline T*
   New(){
     auto ptr = (RawObject*)Allocate(static_cast<int64_t>(sizeof(T)));
     return ptr != nullptr ? (T*)ptr->GetPointer() : nullptr;
   }

   template<typename T>
   static inline T*
   New(const T& value){
     auto ptr = (RawObject*)Allocate(static_cast<int64_t>(sizeof(T)));
     if(ptr != nullptr)
       *((T*)ptr->GetPointer()) = value;
     return ptr != nullptr ? (T*)ptr->GetPointer() : nullptr;
   }

   static void VisitRoots(const std::function<bool(uword, uword)>& vis);

   static void VisitLocals(RawObjectVisitor* vis);
   static void VisitLocals(const std::function<bool(RawObject*)>& vis);

   Allocator& operator=(const Allocator& rhs) = delete;
 };
}

#endif //POSEIDON_ALLOCATOR_H