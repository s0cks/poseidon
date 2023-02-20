#include "poseidon/flags.h"

#include "poseidon/page/new_page.h"

namespace poseidon::flags {
 DEFINE_string(new_zone_size, kDefaultNewZoneSize, "The size of the new zone.");

 word GetNewZoneSize() {
   Size size;
   LOG_IF(FATAL, !SizeParser::ParseSize(FLAGS_new_zone_size, &size)) << "cannot parse NewZoneSize from: " << FLAGS_new_zone_size;
   return size.bytes();
 }

 DEFINE_string(new_page_size, "2mb", "The size of the new page in bytes.");
 DEFINE_int32(max_new_pages, NewPage::kMaxNumberOfNewPages, "The maximum number of NewPages.");

 word GetNewPageSize() {
   Size size;
   LOG_IF(FATAL, !SizeParser::ParseSize(FLAGS_new_page_size, &size)) << "cannot parse NewPageSize from: " << FLAGS_new_page_size;
   return size.bytes();
 }

 DEFINE_string(old_zone_size, kDefaultOldZoneSize, "The size of the old zone in bytes.");

 word GetOldZoneSize() {
   Size size;
   LOG_IF(FATAL, !SizeParser::ParseSize(FLAGS_old_zone_size, &size)) << "cannot parse OldZoneSize from: " << FLAGS_old_zone_size;
   return size.bytes();
 }

 DEFINE_string(old_page_size, kDefaultOldPageSize, "The size of the old zone pages in bytes.");

 word GetOldPageSize() {
   Size size;
   LOG_IF(FATAL, !SizeParser::ParseSize(FLAGS_old_page_size, &size)) << "cannot parse OldPageSize from: " << FLAGS_old_page_size;
   return size.bytes();
 }

 DEFINE_int64(free_list_buckets, kDefaultNumberOfFreeListBuckets, "The number of buckets for the free list");

 DEFINE_string(large_object_size, kDefaultLargeObjectSize, "The max size of an object before it gets classified as a large object.");

 word GetLargeObjectSize() {
   Size size;
   LOG_IF(FATAL, !SizeParser::ParseSize(FLAGS_large_object_size, &size)) << "cannot parse LargeObjectSize from: " << FLAGS_large_object_size;
   return size.bytes();
 }

 DEFINE_int64(num_workers, kDefaultNumberOfWorkers, "The number of workers to use for collections.");

 DEFINE_string(report_directory, kDefaultReportDirectory, "The directory used for reports.");

 void FlagsPrinter::Print() const {
   LOG_AT_LEVEL(GetSeverity()) << "poseidon flags:";
   LOG_AT_LEVEL(GetSeverity()) << "- New Zone Size: " << Bytes(GetNewZoneSize());
   LOG_AT_LEVEL(GetSeverity()) << "- New Page Size: " << Bytes(GetNewPageSize());
   LOG_AT_LEVEL(GetSeverity()) << "- Old Zone Size: " << Bytes(GetOldZoneSize());
   LOG_AT_LEVEL(GetSeverity()) << "- Old Page Size: " << Bytes(GetOldPageSize());
   LOG_AT_LEVEL(GetSeverity()) << "- Number of Free List Buckets: " << GetNumberOfFreeListBuckets();
   LOG_AT_LEVEL(GetSeverity()) << "- Large Object Size: " << Bytes(GetLargeObjectSize());
   LOG_AT_LEVEL(GetSeverity()) << "- Number of Workers: " << GetNumberOfWorkers();
   LOG_AT_LEVEL(GetSeverity()) << "- Report Directory: " << GetReportDirectory();
 }
}