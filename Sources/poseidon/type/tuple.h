#ifndef POSEIDON_TUPLE_H
#define POSEIDON_TUPLE_H

#include "poseidon/type/instance.h"

namespace poseidon {
 class Tuple : public Instance {
   friend class Class;
  protected:
   static Field* kCarField;
   static Field* kCdrField;

   explicit Tuple():
    Instance(kClass, kTypeId) {
     SetCar(UNALLOCATED);
     SetCdr(UNALLOCATED);
   }
  public:
   ~Tuple() override = default;

   void SetCar(RawObject* ptr = UNALLOCATED) {
     return SetField(kCarField, ptr);
   }

   bool HasCar() const {
     return GetField(kCarField) != UNALLOCATED;
   }

   template<class T>
   T* GetCar() const {
     return (T*) GetField(kCarField);
   }

   void SetCdr(RawObject* ptr = UNALLOCATED) {
     return SetField(kCdrField, ptr);
   }

   bool HasCdr() const {
     return GetField(kCdrField) != UNALLOCATED;
   }

   template<class T>
   T* GetCdr() const {
     return (T*)GetField(kCdrField);
   }

   friend std::ostream& operator<<(std::ostream& stream, const Tuple& value) {
     stream << "Tuple(";
     if(value.HasCar())
       stream << "car=" << (*value.GetField(kCarField)->raw_ptr());
     if(value.HasCdr())
       stream << "cdr=" << (*value.GetField(kCdrField)->raw_ptr());
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Tuple);
  public:
   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;

   static inline Tuple* New() {
     return new Tuple();
   }
 };
}

#endif // POSEIDON_TUPLE_H