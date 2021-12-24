#ifndef POSEIDON_COMMON_H
#define POSEIDON_COMMON_H

#include <cstdint>
#include <iostream>
#include "platform.h"

#define NOT_IMPLEMENTED(Level) \
  LOG(Level) << __FUNCTION__ << " is not implemented!"

namespace poseidon{
  static inline uword
  RoundUpPow2(uword x){
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
#ifdef ARCHITECTURE_IS_X64
    x = x | (x >> 32);
#endif
    return x + 1;
  }

  enum Space{//TODO: move
    kEdenSpace = 0, //      0000
    kTenuredSpace, //       0001
    kLargeObjectSpace, //   0010
  };

  static inline std::ostream&
  operator<<(std::ostream& stream, const Space& val){
    switch(val){
      case Space::kEdenSpace:
        return stream << "Eden";
      case Space::kTenuredSpace:
        return stream << "Tenured";
      case Space::kLargeObjectSpace:
        return stream << "LargeObject";
      default:
        return stream << "Unknown";
    }
  }

  enum Color{
    kBlack = 0, //          0000
    kGray, //               0001
    kWhite, //              0010
    kFree = kWhite,
    kMarked = kBlack,
  };

  static inline std::ostream&
  operator<<(std::ostream& stream, const Color& val){
    switch(val){
      case Color::kBlack:
        return stream << "Black";
      case Color::kGray:
        return stream << "Gray";
      case Color::kWhite:
        return stream << "White";
      default:
        return stream << "Unknown";
    }
  }
}

#endif //POSEIDON_COMMON_H
