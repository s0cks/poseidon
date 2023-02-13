#ifndef POSEIDON_COLLECTOR_H
#define POSEIDON_COLLECTOR_H

#include <ostream>

namespace poseidon{
#define FOR_EACH_COLLECTOR_STATE(V) \
 V(Idle)                            \
 V(MinorCollection)                 \
 V(MajorCollection)

 class Collector{
  public:
   enum State : int8_t{
#define DEFINE_STATE(Name) k##Name,
     FOR_EACH_COLLECTOR_STATE(DEFINE_STATE)
#undef DEFINE_STATE
   };

   inline friend std::ostream&
   operator<<(std::ostream& stream, const State& val){
#define DEFINE_TOSTRING(Name) \
      case k##Name:           \
        return stream << #Name;
     switch(val){
       FOR_EACH_COLLECTOR_STATE(DEFINE_TOSTRING)
       default:
         return stream << "[Unknown State: " << (int8_t)val << "]";
     }
#undef DEFINE_TOSTRING
   }
  private:
   static void SetState(State state);
  public:
   Collector() = delete;
   Collector(const Collector& rhs) = delete;
   ~Collector() = delete;

   static State GetState();

   static void MinorCollection();
   static void MajorCollection();

#define DEFINE_STATE_CHECK(Name) static inline bool Is##Name(){ return GetState() == State::k##Name; }
   FOR_EACH_COLLECTOR_STATE(DEFINE_STATE_CHECK)
#undef DEFINE_STATE_CHECK

   Collector& operator=(const Collector& rhs) = delete;
 };
}

#endif //POSEIDON_COLLECTOR_H
