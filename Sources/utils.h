#ifndef POSEIDON_UTILS_H
#define POSEIDON_UTILS_H

#include <cmath>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include "platform.h"

namespace poseidon{
  static const uword kUWordOne = 1U;

  static inline std::string
  PrettyPrintPercentage(const double& percentage){
    char data[6];
    snprintf(data, 6, "%.2f", percentage);
    return {data};
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
    static S Mask(){
      return (kUWordOne << Size) - 1;
    }

    static S MaskInPlace(){
      return Mask() << Position;
    }

    static T Decode(S val){
      const auto u = static_cast<typename std::make_unsigned<S>::type>(val);
      return static_cast<T>((u >> Position) & Mask());
    }

    static S Encode(T val){
      const auto u = static_cast<typename std::make_unsigned<S>::type>(val);
      return (u & Mask()) << Position;
    }

    static S Update(T val, S original){
      return Encode(val) | (~MaskInPlace() & original);
    }
  };

  static inline const char*
  HumanReadableSize(uint64_t nbytes){
    static const char* kSuffix[] = {
        "b",
        "kb",
        "mb",
        "gb",
        "tb",
    };
    static int kSuffixLength = sizeof(kSuffix) / sizeof(kSuffix[0]);

    uint8_t suffix = 0;
    double remainder = static_cast<double>(nbytes);

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

    return result;
  }

  typedef std::chrono::system_clock Clock;
  typedef Clock::time_point Timestamp;
  typedef Clock::duration Duration;

  template<typename TimeUnit=std::chrono::milliseconds>
  static inline uint64_t
  GetElapsed(const Timestamp& from, const Timestamp& to = Clock::now()){
    return std::chrono::duration_cast<TimeUnit>(to - from).count();
  }

/**********************************************************************************************
 *  This code was borrowed this stack overflow answer, it is really cool!
 *  https://stackoverflow.com/a/22075960/2832700
 **********************************************************************************************/
 using day_t = std::chrono::duration<long long, std::ratio<3600 * 24>>;

 template<typename> struct duration_traits {};
#define DURATION_TRAITS(Duration, Singular, Plural) \
	template<> struct duration_traits<Duration> { \
		constexpr static const char* singular = Singular; \
		constexpr static const char* plural = Plural; \
	}

 DURATION_TRAITS(std::chrono::milliseconds, "millisecond", "milliseconds");
 DURATION_TRAITS(std::chrono::seconds, "second", "seconds");
 DURATION_TRAITS(std::chrono::minutes, "minute", "minutes");
 DURATION_TRAITS(std::chrono::hours, "hour", "hours");
 DURATION_TRAITS(day_t, "day", "days");

 using divisions = std::tuple<std::chrono::milliseconds,
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

      if (count == 0) {
        return started_printing;
      }

      if (started_printing) {
        os << ' ';
      }

      using traits = duration_traits<Head>;
      os << count << ' ' << (count == 1 ? traits::singular : traits::plural);
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
}

#endif //POSEIDON_UTILS_H
