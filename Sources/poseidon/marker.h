#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

namespace poseidon{
 class Marker{
  private:
   static void SerialMark();
   static void ParallelMark();
  public:
   Marker() = delete;
   Marker(const Marker& rhs) = delete;
   ~Marker() = delete;

   static void Mark();

   Marker& operator=(const Marker& rhs) = delete;
 };
}

#endif //POSEIDON_MARKER_H