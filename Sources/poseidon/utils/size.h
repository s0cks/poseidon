#ifndef POSEIDON_UTILS_MEM_SIZE_H
#define POSEIDON_UTILS_MEM_SIZE_H

#include <cmath>
#include <ostream>
#include <glog/logging.h>

#include "poseidon/relaxed_atomic.h"
#include "poseidon/platform/platform.h"

namespace poseidon {
 typedef word RawSize;

 static constexpr const RawSize kB = 1;
 static constexpr const RawSize kKB = kB * 1024;
 static constexpr const RawSize kMB = kKB * 1024;
 static constexpr const RawSize kGB = kMB * 1024;
 static constexpr const RawSize kTB = kGB * 1024;

 class Size {
  protected:
   static constexpr const char* kSuffix[] = {
       "b",
       "kb",
       "mb",
       "gb",
       "tb",
   };
   static constexpr const word kSuffixLength = (sizeof(kSuffix) / sizeof(kSuffix[0]));
   static constexpr const word kMaxRemaining = 1024;

   word bytes_;
  public:
   explicit constexpr Size(const word bytes = 0):
    bytes_(bytes) {
   }
   Size(const Size& rhs) = default;
   ~Size() = default;

   word bytes() const {
     return bytes_;
   }

   explicit operator const char*() const {
     uint8_t suffix = 0;
     auto remainder = static_cast<double>(bytes());
     while(remainder >= kMaxRemaining && suffix < kSuffixLength){
       suffix++;
       remainder /= kMaxRemaining;
     }

     static char result[128];
     if(remainder - floor(remainder) == 0.0){
       sprintf(result, "%d%s", (int)remainder, kSuffix[suffix]);
     } else{
       sprintf(result, "%.2lf%s", remainder, kSuffix[suffix]);
     }
     return result;
   }

   explicit constexpr operator word() const {
     return bytes();
   }

   Size& operator=(const Size& rhs) = default;

   Size& operator=(const word& rhs) {
     if(bytes_ == rhs)
       return *this;
     bytes_ = rhs;
     return *this;
   }

   friend bool operator==(const Size& lhs, const Size& rhs) {
     return lhs.bytes() == rhs.bytes();
   }

   friend bool operator!=(const Size& lhs, const Size& rhs) {
     return lhs.bytes() != rhs.bytes();
   }

   friend bool operator<(const Size& lhs, const Size& rhs) {
     return lhs.bytes() < rhs.bytes();
   }

   friend bool operator>(const Size& lhs, const Size& rhs) {
     return lhs.bytes() & rhs.bytes();
   }

   friend std::ostream& operator<<(std::ostream& stream, const Size& size) {
     uint8_t suffix = 0;
     auto remainder = static_cast<double>(size.bytes());
     while(remainder >= kMaxRemaining && suffix < kSuffixLength){
       suffix++;
       remainder /= kMaxRemaining;
     }

     static char result[128];
     if(remainder - floor(remainder) == 0.0){
       sprintf(result, "%d%s", (int)remainder, kSuffix[suffix]);
     } else{
       sprintf(result, "%.2lf%s", remainder, kSuffix[suffix]);
     }
     stream << result;
     return stream;
   }
 };

 template<word Scale>
 class ScaledSize : public Size {
  public:
   explicit constexpr ScaledSize(const word size = 0):
     Size(size * Scale) {
   }
   ~ScaledSize() = default;
 };

 static inline constexpr Size
 Bytes(const word size) {
   return ScaledSize<kB>(size);
 }

 static inline constexpr Size
 Kilobytes(const word size) {
   return ScaledSize<kKB>(size);
 }

 static inline constexpr Size
 Megabytes(const word size) {
   return ScaledSize<kMB>(size);
 }

 static inline constexpr Size
 Gigabytes(const word size) {
   return ScaledSize<kGB>(size);
 }

 static inline constexpr Size
 Terabytes(const word size) {
   return ScaledSize<kTB>(size);
 }

 class SizeParser {
  private:
   std::istream& stream_;
   int pos_row_;
   int pos_col_;

   inline std::istream& stream() {
     return stream_;
   }

   inline char PeekChar() {
     auto c = stream_.get();
     stream_.putback((char) c);
     return (char) c;
   }

   inline char NextChar() {
     char c = (char) stream_.get();
     switch(c) {
       case '\n': {
         pos_row_++;
         pos_col_ = 1;
         break;
       }
       default: {
         pos_col_++;
         break;
       }
     }
     return c;
   }

   static inline bool
   IsWhitespace(char c) {
     return isspace(c);
   }

   static inline bool
   IsDigit(char c) {
     return isdigit(c);
   }

   static inline bool
   IsAlpha(char c) {
     return isalpha(c);
   }

   inline char
   NextRealChar() {
     char next;
     while(IsWhitespace(next = NextChar()));
     return next;
   }

   template<const word Scale>
   inline bool ParseScaledBytes(word* result) {
     char c;
     switch((c = NextRealChar())) {
       case 'b':
       case 'B':
         (*result) = Scale;
         return true;
       default:
         LOG(FATAL) << "unknown token: " << c;
         (*result) = 0;
         return false;
     }
   }

   bool ParseScale(word* result) {
     char c;
     switch((c = NextRealChar())) {
       case 'b':
       case 'B':
         (*result) = kB;
         return true;
       case 'k':
       case 'K':
         return ParseScaledBytes<kKB>(result);
       case 'm':
       case 'M':
         return ParseScaledBytes<kMB>(result);
       case 'g':
       case 'G':
         return ParseScaledBytes<kGB>(result);
       case 't':
       case 'T':
         return ParseScaledBytes<kTB>(result);
       case EOF:
         LOG(FATAL) << "end of stream";
         return false;
       default:
         LOG(FATAL) << "unknown token: " << c;
         return false;
     }
   }

   bool ParseWord(word* result) {
     std::stringstream ss;

     char c;
     while((c = NextRealChar()) != EOF && IsDigit(c)) {
       ss << c;
       if(!IsDigit(PeekChar()))
         break;
     }

     std::string value = ss.str();
     const char* buff = value.c_str();
     char* end;

     errno = 0;
     const long sl = strtol(buff, &end, 10);
     if(buff == end) {
       LOG(FATAL) << "not a number: " << buff;
       return false;
     } else if(errno == ERANGE && sl == LONG_MIN) {
       LOG(FATAL) << "undeflow: " << buff;
       return false;
     } else if(errno == ERANGE && sl == LONG_MAX) {
       LOG(FATAL) << "overflow: " << buff;
       return false;
     } else if(errno == EINVAL) {
       LOG(FATAL) << buff << " invalid (unspecified)";
       return false;
     } else if(errno != 0 && sl == 0) {
       LOG(FATAL) << buff << " invalid (unspecified)";
       return false;
     } else if(errno == 0 && (*end) != 0) {
       LOG(FATAL) << buff << " valid but additional characters";
       return false;
     }

     (*result) = sl;
     return true;
   }
  public:
   explicit SizeParser(std::istream& stream):
    stream_(stream) {
   }
   ~SizeParser() = default;

   bool Parse(Size* size) {
     word num;
     if(!ParseWord(&num)) {
       LOG(ERROR) << "failed to parse num";
       return false;
     }

     word scale;
     if(!ParseScale(&scale)) {
       LOG(ERROR) << "failed to parse scale";
       return false;
     }
     (*size) = Bytes(num * scale);
     return true;
   }

   static inline bool
   ParseSize(const std::string& value, Size* result) {
     std::istringstream stream(value);
     SizeParser parser(stream);
     return parser.Parse(result);
   }
 };
}

#endif // POSEIDON_UTILS_MEM_SIZE_H