#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <utility>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/platform.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/freelist_node.h"

namespace poseidon{ //TODO: atomic support?
 class FreeList : public Region {
   friend class Sweeper;
   friend class OldZone;

   friend class FreeListTest;
   friend class SerialSweeperTest;
  public:
   class FreeListNode {
    protected:
     FreeObject* item_;
     int level;
    public:
   };

   class FreeListIterator {
    protected:
     FreeList* free_list_;
     uword current_;

     inline FreeList* free_list() const {
       return free_list_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline FreeObject* current_ptr() const {
       return (FreeObject*)current_address();
     }

     inline void set_current_address(FreeObject* node) {
       current_ = node ? node->GetAddress() : 0;
     }
    public:
     explicit FreeListIterator(FreeList* free_list):
      free_list_(free_list),
      current_(0) {
       set_current_address(free_list->GetHead());
     }
     virtual ~FreeListIterator() = default;

     virtual bool HasNext() const {
       return current_address() >= free_list()->GetStartingAddress() &&
              current_address() < free_list()->GetEndingAddress();
     }

     virtual FreeObject* Next() {
       auto node = current_ptr();
       current_ = node->GetNextAddress();
       return node;
     }
   };
  protected:
   MemoryRegion region_;
   FreeObject* head_;
   int64_t num_nodes_;
   int64_t level;

   virtual bool Remove(uword starting_address, ObjectSize size);
   virtual bool Insert(uword starting_address, ObjectSize size);

   virtual inline bool Insert(FreeObject* node) {
     return Insert(node->GetStartingAddress(), node->GetSize());
   }

   virtual bool InsertAtHead(FreeObject* node);
   virtual uword TryAllocate(ObjectSize size);
   virtual FreeObject* FindBestFit(ObjectSize size);
   virtual FreeObject* Split(FreeObject* parent, ObjectSize size);

   inline void SetHead(FreeObject* node) {
     head_ = node;
     num_nodes_ = node == nullptr ? 0 : num_nodes_;
   }
  public:
   FreeList():
    region_(),
    head_(nullptr),
    num_nodes_(0) {
   }
   explicit FreeList(const MemoryRegion& region):
    region_(region),
    head_(nullptr),
    num_nodes_(0) {
     if(!region.Protect(MemoryRegion::kReadWrite)) {
       LOG(FATAL) << "failed to protect " << region;
     }
     Insert(FreeObject::NewNode(region));
   }
   FreeList(const FreeList& rhs) = default;
   ~FreeList() override = default;

   uword GetStartingAddress() const override {
     return region_.GetStartingAddress();
   }

   int64_t GetSize() const override {
     return region_.GetSize();
   }

   virtual FreeObject* GetHead() const {
     return head_;
   }

   bool IsEmpty() const {
     return GetHead() == nullptr || GetHead()->GetSize() == 0;
   }

   virtual int64_t GetNumberOfNodes() const {
     return num_nodes_;
   }

   virtual bool VisitFreeNodes(FreeListNodeVisitor* vis);

   FreeList& operator=(const FreeList& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const FreeList& value) {
     stream << "FreeList(";
     if(!value.IsEmpty())
       stream << "head=" << (*value.GetHead());
     stream << ")";
     return stream;
   }

   friend bool operator==(const FreeList& lhs, const FreeList& rhs) {
     return (*lhs.GetHead()) == (*rhs.GetHead());
   }

   friend bool operator!=(const FreeList& lhs, const FreeList& rhs) {
     return !operator==(lhs, rhs);
   }
 };
}

#endif//POSEIDON_FREELIST_H