#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

#include "poseidon/flags.h"

namespace poseidon{
 class Marker{
  private:
   static void SetMarking();
   static void ClearMarking();

   static void SerialMark();
   static void ParallelMark();
  public:
   Marker() = delete;
   Marker(const Marker& rhs) = delete;
   ~Marker() = delete;

   static bool IsMarking();

   static void Mark(){
     if(IsMarking()){
       DLOG(WARNING) << "already marking.";
       return;
     }

     SetMarking();
     if(false){
       TIMED_SECTION("ParallelMark", {
         ParallelMark();
       });
     } else{
       TIMED_SECTION("SerialMark", {
         SerialMark();
       });
     }
     ClearMarking();
   }

   Marker& operator=(const Marker& rhs) = delete;
 };
}

#endif //POSEIDON_MARKER_H