#ifndef POSEIDON_FLAGS_H
#define POSEIDON_FLAGS_H

#include <gflags/gflags.h>
#include "poseidon/common.h"

namespace poseidon{
 static constexpr const int64_t kDefaultNewZoneSize = 16 * kMB;
 DECLARE_int64(new_zone_size);
 static constexpr const int64_t kDefaultNewPageSize = 2 * kMB;
 DECLARE_int64(new_page_size);

 static constexpr const int64_t kDefaultOldZoneSize = 256 * kMB;
 DECLARE_int64(old_zone_size);
 static constexpr const int64_t kDefaultOldPageSize = 32 * kMB;
 DECLARE_int64(old_page_size);
 static constexpr const int32_t kDefaultNumberOfFreeListBuckets = 16;
 DECLARE_int32(free_list_buckets);

 static constexpr const int64_t kDefaultLargeObjectSize = 1 * kMB;
 DECLARE_int64(large_object_size);

 static constexpr const int64_t kDefaultNumberOfWorkers = 2;
 DECLARE_int64(num_workers);
 static constexpr const char* kDefaultReportDirectory = "";
 DECLARE_string(report_directory);

 static inline std::string
 GetReportDirectory(){
   return FLAGS_report_directory;
 }

 static inline bool
 HasReportDirectory(){
   return !GetReportDirectory().empty();
 }

 static inline int64_t
 GetNewZoneSize(){
   return FLAGS_new_zone_size;
 }

 static inline int64_t
 GetNewZoneSemispaceSize() {
   return GetNewZoneSize() / 2;
 }

 static inline int64_t
 GetNewPageSize() {
   return FLAGS_new_page_size;
 }

 static inline int64_t
 GetNumberOfNewPages() {
   return GetNewZoneSize() / GetNewPageSize();
 }

 static inline int64_t
 GetOldZoneSize(){
   return FLAGS_old_zone_size;
 }

 static inline int64_t
 GetOldPageSize(){
   return FLAGS_old_page_size;
 }

 static inline int64_t
 GetNumberOfOldPages() {
   return GetOldZoneSize() / GetOldPageSize();
 }

 static inline int64_t
 GetLargeObjectSize(){
   return FLAGS_large_object_size;
 }

 static inline int64_t
 GetNumberOfWorkers(){
   return FLAGS_num_workers;
 }

 static inline bool
 HasWorkers(){
   return GetNumberOfWorkers() > 0;
 }

 static inline int64_t
 GetTotalInitialHeapSize(){
   return GetNewZoneSize() + GetOldZoneSize();
 }

 static inline int64_t
 GetNumberOfFreeListBuckets() {
   return FLAGS_free_list_buckets;
 }
}

#endif //POSEIDON_FLAGS_H
