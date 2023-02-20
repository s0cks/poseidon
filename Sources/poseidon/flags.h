#ifndef POSEIDON_FLAGS_H
#define POSEIDON_FLAGS_H

#include <gflags/gflags.h>

#include "poseidon/common.h"
#include "poseidon/utils/size.h"
#include "poseidon/platform/platform.h"

namespace poseidon::flags{
#if defined(ARCHITECTURE_IS_ARM64) || defined(ARCHITECTURE_IS_X64)
#define DECLARE_word(Name) DECLARE_int64(Name)
#elif defined(ARCHITECTURE_IS_X32)
#define DECLARE_word(Name) DECLARE_int32(Name)
#else
#error "unsupported architecture"
#endif // DECLARE_word

#define DECLARE_size(Name, Flag, Default) \
  static constexpr const char* kDefault##Name##Size = (Default); \
  DECLARE_string(Flag);                        \
  word Get##Name##Size();

 DECLARE_size(NewZone, new_zone, "16mb");
 DECLARE_size(NewPage, new_page, "2mb");

 static inline word
 GetNewZoneSemispaceSize() {
   return GetNewZoneSize() / 2;
 }

 DECLARE_int32(max_new_pages);

 DECLARE_size(OldZone, old_zone, "256mb");
 DECLARE_size(OldPage, old_page, "32mb");

 static constexpr const word kDefaultNumberOfFreeListBuckets = 16;
 DECLARE_word(free_list_buckets);

 static inline word
 GetNumberOfFreeListBuckets() {
   return FLAGS_free_list_buckets;
 }

 DECLARE_size(LargeObject, large_object, "16mb");

 static constexpr const int64_t kDefaultNumberOfWorkers = 2;
 DECLARE_int64(num_workers);

 static constexpr const char* kDefaultReportDirectory = "";
 DECLARE_string(report_directory);

 static inline word
 GetTotalInitialHeapSize() {
   return GetNewZoneSize() + GetOldZoneSize();
 }

 static inline word
 GetNumberOfNewPages() {
   return GetNewZoneSize() / GetNewPageSize();
 }

 static inline word
 GetNumberOfWorkers() {
   return FLAGS_num_workers;
 }

 static inline std::string
 GetReportDirectory() {
   return FLAGS_report_directory;
 }

 class FlagsPrinter {
  private:
   const google::LogSeverity severity_;

   explicit FlagsPrinter(const google::LogSeverity severity):
    severity_(severity) {
   }
  public:
   ~FlagsPrinter() = default;

   inline google::LogSeverity
   GetSeverity() const {
     return severity_;
   }

   void Print() const;
  public:
   template<const google::LogSeverity Severity = google::INFO>
   static inline void PrintFlags() {
     FlagsPrinter printer(Severity);
     return printer.Print();
   }
 };
}

#endif //POSEIDON_FLAGS_H
