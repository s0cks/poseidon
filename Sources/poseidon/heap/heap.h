#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"

#include "poseidon/zone/zone.h"
#include "poseidon/zone/new_zone.h"
#include "poseidon/zone/old_zone.h"

#include "poseidon/platform/os_thread.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Heap {
   friend class Scavenger;
   friend class Compactor;
   friend class Allocator;
  private:
   NewZone new_zone_;
   OldZone old_zone_;

   uword AllocateNewObject(ObjectSize size);
   uword AllocateOldObject(ObjectSize size);
   uword AllocateLargeObject(ObjectSize size);

   static void SetForCurrentThread(Heap* value);

   Heap() = default;
  public:
   ~Heap() = delete;
   DEFINE_NON_COPYABLE_TYPE(Heap);

   NewZone* new_zone() {
     return &new_zone_;
   }

   OldZone* old_zone() {
     return &old_zone_;
   }

   Pointer* TryAllocatePointer(ObjectSize size);
   uword TryAllocateBytes(ObjectSize size);
   uword TryAllocateClassBytes(Class* cls);

   template<typename T>
   T* TryAllocate() {
     return TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClass() {
     return TryAllocateClassBytes(T::GetClass());
   }

   friend std::ostream& operator<<(std::ostream& stream, const Heap& heap) {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return stream;
   }
  public:
   static inline Heap* New() {
     return new Heap();
   }

   static Heap* GetForCurrentThread();
   static bool ExistsForCurrentThread();

   static inline void
   InitializeForCurrentThread() {
     if(ExistsForCurrentThread()) {
       auto existing = GetForCurrentThread();
       DLOG(WARNING) << (*existing) << " already exists for `" << GetCurrentThreadName() << "` thread.";
       return;
     }
     return SetForCurrentThread(New());
   }
 };
}

#endif //POSEIDON_HEAP_H
