#ifndef POSEIDON_FLAGS_H
#define POSEIDON_FLAGS_H

#include <gflags/gflags.h>
#include "poseidon/common.h"

namespace poseidon{
 static const constexpr int64_t kDefaultNewZoneSize = 16 * kMB;
 DECLARE_int64(new_zone_size);

 static inline int64_t
 GetNewZoneSize(){
   return FLAGS_new_zone_size;
 }

 static const constexpr int64_t kDefaultOldZoneSize = 512 * kMB;
 DECLARE_int64(old_zone_size);

 static inline int64_t
 GetOldZoneSize(){
   return FLAGS_old_zone_size;
 }

 static const constexpr int64_t kDefaultOldPageSize = 16 * kMB;
 DECLARE_int64(old_page_size);

 static inline int64_t
 GetOldPageSize(){
   return FLAGS_old_page_size;
 }

 static const constexpr int64_t kDefaultLargeObjectSize = 1 * kMB;
 DECLARE_int64(large_object_size);

 static inline int64_t
 GetLargeObjectSize(){
   return FLAGS_large_object_size;
 }

 static const constexpr int64_t kDefaultNumberOfWorkers = 2;
 DECLARE_int64(num_workers);

 static inline int64_t
 GetNumberOfWorkers(){
   return FLAGS_num_workers;
 }

 static inline bool
 ShouldUseParallelScavenge(){
   return GetNumberOfWorkers() > 0;
 }

 static inline bool
 ShouldUseParallelMark(){
   return GetNumberOfWorkers() > 0;
 }

 static inline bool
 ShouldUseParallelSweep(){
   return GetNumberOfWorkers() > 0;
 }

 static inline int64_t
 GetTotalInitialHeapSize(){
   return GetNewZoneSize() + GetOldZoneSize();
 }
}

#endif //POSEIDON_FLAGS_H
