#ifndef PSDN_VISITOR_H
#define PSDN_VISITOR_H

#include <functional>

namespace poseidon {
 template<typename T>
 class Visitor {
  public:
   typedef std::function<bool(T*)> VisitorFunction;
  public:
   Visitor() = default;
   virtual ~Visitor() = default;
   virtual bool Visit(T* value) = 0;
 };

#define DEFINE_VISITOR_WRAPPER(Visitor, T) \
  class Visitor##Wrapper : public Visitor {\
   protected:                              \
    const Visitor::VisitorFunction& function_; \
   public:                                 \
    explicit Visitor##Wrapper(const Visitor::VisitorFunction& function): \
      function_(function) { }              \
    ~Visitor##Wrapper() override = default;   \
    bool Visit(T* value) override { return function_(value); } \
  };
}

#endif //PSDN_VISITOR_H