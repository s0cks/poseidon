#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

#include <deque>
#include "raw_object.h"

namespace poseidon{
  class Marker : public RawObjectPointerVisitor{
   protected:
    Color color_;

    inline void
    MarkObject(RawObject* obj) const{
      obj->SetRemembered();
      obj->SetColor(GetColor());
    }
   public:
    explicit Marker(const Color& color):
      RawObjectPointerVisitor(),
      color_(color){
    }
    ~Marker() override = default;

    Color GetColor() const{
      return color_;
    }
  };

  class LiveObjectMarker : public Marker{
   protected:
    std::deque<uword> work_;
   public:
    LiveObjectMarker(std::deque<uword>& work, const Color& color):
      Marker(color),
      work_(work){
    }
    ~LiveObjectMarker() override = default;

    bool Visit(RawObject* obj) override{
      if(obj->GetColor() != Color::kFree)
        return true;
      MarkObject(obj); //TODO: add references to work queue
      return true;
    }
  };
}

#endif //POSEIDON_MARKER_H
