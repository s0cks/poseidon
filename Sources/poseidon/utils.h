#ifndef POSEIDON_UTILS_H
#define POSEIDON_UTILS_H

#include <cmath>
#include <string>
#include <chrono>
#include <cstdio>
#include <ostream>
#include <cstdint>
#include <cstring>

#include "poseidon/platform/platform.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon{
  template<typename T>
  static inline double
  GetPercentageOf(T part, T whole){
    return (static_cast<double>(part) / static_cast<double>(whole)) * 100.0;
  }

  static inline std::string
  PrettyPrintPercentage(const double percentage){
    char data[8];
    snprintf(data, 8, "%.2f%%", percentage);
    return {data};
  }

  template<typename T>
  static inline std::string
  PrettyPrintPercentage(T part, T whole){
    return PrettyPrintPercentage(GetPercentageOf<T>(part, whole));
  }

 static inline uintptr_t
 RoundUpPowTwo(intptr_t x){
   x = x - 1;
   x = x | (x >> 1);
   x = x | (x >> 2);
   x = x | (x >> 4);
   x = x | (x >> 8);
   x = x | (x >> 16);
#if defined(ARCHITECTURE_IS_X64)
   x = x | (x >> 32);
#endif
   return x + 1;
 }

 template<typename T>
 static inline bool
 IsPow2(T x){
   return ((x & (x - 1)) == 0) && (x != 0);
 }

 template<typename T>
 static inline bool
 IsAligned(T x, int64_t n){
   return (x & (n - 1)) == 0;
 }

  class BitVector{
   public:
    static const uint64_t kBitsPerByte = 8;
    static const uint64_t kBitsPerWord = sizeof(word) * 8;
   private:
    uint64_t size_;
    uint64_t asize_;
    uword* data_;
   public:
    explicit BitVector(const uint64_t& size):
      size_(size),
      asize_(1 + ((size - 1) / kBitsPerWord)),
      data_(nullptr){
      data_ = (uword*)(malloc(sizeof(uword)*size));
      Clear();
    }
    BitVector(const BitVector& rhs):
      size_(rhs.size_),
      asize_(rhs.asize_),
      data_(new uword[rhs.size_]){
      memcpy(data_, rhs.data_, sizeof(uword)*rhs.size_);
    }
    ~BitVector(){
      delete[] data_;
    }

    void Clear(){
      for(int idx = 0; idx < asize_; idx++)
        data_[idx] = 0x0;
    }

    void Add(word i){
      data_[i / kBitsPerWord] |= static_cast<uword>(1 << (i % kBitsPerWord));
    }

    void Remove(word i){
      data_[i / kBitsPerWord] &= ~static_cast<uword>(1 << (i % kBitsPerWord));
    }

    void Intersect(const BitVector& rhs){
      for(int i = 0; i < asize_; i++)
        data_[i] = data_[i] & rhs[i];
    }

    uword operator[](const int& i) const{
      return data_[i];
    }

    bool Contains(word i) const{
      uword block = data_[i / kBitsPerWord];
      return (block & static_cast<uword>(1 << (i % kBitsPerWord)));
    }

    bool AddAll(const BitVector& rhs){
      bool changed = false;
      for(int i = 0; i < asize_; i++){
        uword before = data_[i];
        uword after =data_[i] |= rhs[i];
        if(before != after){
          changed = true;
          data_[i] = after;
        }
      }
      return changed;
    }
  };

  template<typename S, typename T, int Position, int Size=(sizeof(S)*kBitsPerByte) - Position>
  class BitField{
   public:
    static constexpr S Mask(){
      return (kUWordOne << Size) - 1;
    }

    static constexpr S MaskInPlace(){
      return Mask() << Position;
    }

    static constexpr T Decode(S val){
      const auto u = static_cast<typename std::make_unsigned<S>::type>(val);
      return static_cast<T>((u >> Position) & Mask());
    }

    static constexpr S Encode(T val){
      const auto u = static_cast<typename std::make_unsigned<S>::type>(val);
      return (u & Mask()) << Position;
    }

    static constexpr S Update(T val, S original){
      return Encode(val) | (~MaskInPlace() & original);
    }
  };

  struct Bytes{
    int64_t value;

    explicit Bytes(int64_t V):
      value(V){
    }
    explicit Bytes(RelaxedAtomic<int64_t> V):
      Bytes((int64_t)V){
    }
    Bytes(const Bytes& rhs) = default;
    ~Bytes() = default;

    Bytes& operator=(const Bytes& rhs) = default;

    friend std::ostream& operator<<(std::ostream& stream, const Bytes& val){
      static const char* kSuffix[] = {
          "b",
          "kb",
          "mb",
          "gb",
          "tb",
      };
      static int kSuffixLength = sizeof(kSuffix) / sizeof(kSuffix[0]);

      uint8_t suffix = 0;
      auto remainder = static_cast<double>(val.value);
      while(remainder >= 1024 && suffix < kSuffixLength){
        suffix++;
        remainder /= 1024;
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

  typedef std::chrono::system_clock Clock;
  typedef Clock::time_point Timestamp;
  typedef Clock::duration Duration;

/**********************************************************************************************
 *  This code was borrowed this stack overflow answer, it is really cool!
 *  https://stackoverflow.com/a/22075960/2832700
 **********************************************************************************************/
 using day_t = std::chrono::duration<long long, std::ratio<3600 * 24>>;

 template<typename> struct duration_traits {};
#define DURATION_TRAITS(Duration, Abbreviation) \
	template<> struct duration_traits<Duration> {      \
		constexpr static const char* abbreviation = Abbreviation; \
	}

 DURATION_TRAITS(std::chrono::microseconds, "us");
 DURATION_TRAITS(std::chrono::milliseconds, "ms");
 DURATION_TRAITS(std::chrono::seconds, "s");
 DURATION_TRAITS(std::chrono::minutes, "m");
 DURATION_TRAITS(std::chrono::hours, "h");
 DURATION_TRAITS(day_t, "d");

 using divisions = std::tuple<std::chrono::microseconds,
                              std::chrono::milliseconds,
                              std::chrono::seconds,
                              std::chrono::minutes,
                              std::chrono::hours,
                              day_t>;

 namespace detail {
  template<typename...> struct print_duration_impl_ {};

  template<typename Head, typename... Tail>
  struct print_duration_impl_<Head, Tail...> {
    template <typename Duration>
    static bool print(std::ostream& os, Duration& dur) {
      const auto started_printing = print_duration_impl_<Tail...>::print(os, dur);

      const auto n = std::chrono::duration_cast<Head>(dur);
      const auto count = n.count();

      using traits = duration_traits<Head>;
      if (count == 0) {
        return started_printing;
      }

      if (started_printing) {
        os << ' ';
      }

      os << count << traits::abbreviation;
      dur -= n;

      return true;
    }
  };

  template<> struct print_duration_impl_<> {
    template <typename Duration>
    static bool print(std::ostream& os, Duration& dur) {
      return false;
    }
  };

  template<typename...> struct print_duration {};

  template<typename... Args>
  struct print_duration<std::tuple<Args...>> {
    template<typename Duration>
    static void print(std::ostream& os, Duration dur) {
      print_duration_impl_<Args...>::print(os, dur);
    }
  };
 }

 template<typename Rep, typename Period>
 std::ostream& operator<<(std::ostream& os, const std::chrono::duration<Rep, Period>& dur) {
   detail::print_duration<divisions>::print(os, dur);
   return os;
 }
/**********************************************************************************************
 *
 **********************************************************************************************/

 typedef RelaxedAtomic<int64_t> AtomicLong;
 typedef RelaxedAtomic<Timestamp> AtomicTimestamp;

 class RawObject;
 struct AtomicPointerCounter{
   AtomicLong count;
   AtomicLong bytes;

   AtomicPointerCounter() = default;
   AtomicPointerCounter(const AtomicPointerCounter& rhs) = default;
   ~AtomicPointerCounter() = default;

   AtomicPointerCounter& operator=(const AtomicPointerCounter& rhs) = default;

   AtomicPointerCounter& operator+=(RawObject* val);
   AtomicPointerCounter& operator-=(RawObject* val);

   AtomicPointerCounter& operator=(const int64_t& val){
     count = val;
     bytes = val;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const AtomicPointerCounter& val){
     return stream << val.count << " (" << Bytes(val.bytes) << ")";
   }
 };
}

#endif //POSEIDON_UTILS_H
