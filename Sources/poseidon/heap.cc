#include <glog/logging.h>

#include "heap.h"
#include "utils.h"
#include "object.h"
#include "raw_object.h"

namespace poseidon{
  static inline bool
  HasFlag(uint64_t flags, const HeapPrinter::Flags flag){
    return (flags & flag) == flag;
  }

  static inline bool
  HasDetailedFlag(uint64_t flags){
    return HasFlag(flags, HeapPrinter::kDetailed);
  }

  static inline bool
  HasHexDumpFlag(uint64_t flags){
    return HasFlag(flags, HeapPrinter::kHexDump);
  }

  static const uint64_t kDefaultFormattedHexDumpSize = 128;
  static const uint64_t kDefaultPaddingSize = 1;

  static inline std::string
  CreateHeader(const std::string& title,
               uint64_t max_length = kDefaultFormattedHexDumpSize,
               uint64_t padding_length = kDefaultPaddingSize,
               char c = '*'){
#ifdef PSDN_DEBUG
    assert(max_length % 2 == 0);
#endif//PSDN_DEBUG

    char result[max_length];

    const uint64_t kTotalLength = max_length;
    const uint64_t kPaddingLength = padding_length;
    const uint64_t kTotalPaddingLength = kPaddingLength * 2;
    const uint64_t kTitleLength = title.length();
    const uint64_t kTotalTitleAndPaddingLength = kTitleLength + kTotalPaddingLength;
    const uint64_t kFillLength = (kTotalLength - kTotalTitleAndPaddingLength) / 2;

    const uint64_t kPaddingStartPosition = 0;
    const uint64_t kTitlePosition = kPaddingStartPosition + kFillLength + kPaddingLength;
    const uint64_t kPaddingEndPosition = kTitlePosition + kTitleLength + kPaddingLength;

    memset(result, ' ', max_length);
    memset(&result[kPaddingStartPosition], c, kFillLength);
    memcpy(&result[kTitlePosition], title.data(), kTitleLength);
    memset(&result[kPaddingEndPosition], c, kFillLength + kPaddingLength);

    return { result, max_length };
  }

  static inline std::string
  CreateFooter(uint64_t max_length = kDefaultFormattedHexDumpSize, char c = '*'){
    char result[max_length];
    memset(result, c, max_length);
    return { result, max_length };
  }

  static inline std::string
  GetHexForRegion(const MemoryRegion& region){
    static const char* kHexAlphabet = "0123456789ABCDEF";
    char data[region.GetSize() * 2];

    int index = 0;
    for(auto it = region.bytes_begin(); it != region.bytes_end(); it++){
      data[index] = kHexAlphabet[(*it) >> 4];
      data[index + 1] = kHexAlphabet[(*it) & 0x0F];
      index += 2;
    }
    return {data, region.GetSize() * 2};
  }

  void HeapPrinter::Print(Heap* heap, uint64_t flags){
    LOG(INFO) << CreateHeader("Heap Dump");
    if(HasDetailedFlag(flags)){
      LOG(INFO) << "* Starting Address: " << ((void*)heap->GetStartingAddress());
      LOG(INFO) << "*   Ending Address: " << ((void*)heap->GetEndingAddress());
    }

    if((flags & kHexDump) == kHexDump){
      DLOG(INFO) << "Allocated: ";
    }
    LOG(INFO) << CreateFooter();
  }
}