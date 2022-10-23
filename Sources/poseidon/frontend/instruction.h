#ifndef POSEIDON_INSTRUCTION_H
#define POSEIDON_INSTRUCTION_H

namespace poseidon {
 class Instruction {
  protected:
   Instruction* next_;
   Instruction* previous_;
  public:
   Instruction():
    next_(nullptr),
    previous_(nullptr) {
   }
   virtual ~Instruction() = default;
   virtual const char* GetName() const = 0;

   void SetNext(Instruction* val) {
     next_ = val;
   }

   Instruction* GetNext() const {
     return next_;
   }

   bool HasNext() const {
     return GetNext() != nullptr;
   }

   void SetPrevious(Instruction* val) {
     previous_ = val;
   }

   Instruction* GetPrevious() const {
     return previous_;
   }

   bool HasPrevious() const {
     return GetPrevious() != nullptr;
   }

  public:
   static inline void
   Prepend(Instruction** list, Instruction* val) {
     val->SetNext((*list));
     if((*list) != nullptr)
       (*list)->SetPrevious(val);
     (*list) = val;
   }

   static inline void
   Append(Instruction** list, Instruction* val) {
     if (!(*list)) {
       (*list) = val;
       return;
     }

     auto last = (*list);
     while(last->HasNext())
       last = last->GetNext();
     last->SetNext(val);
     val->SetPrevious(last);
   }

   static inline void
   InsertAfter(Instruction* previous, Instruction* val) {
     if(!previous)
       return;
     val->SetNext(previous->GetNext());
     previous->SetNext(val);
     val->SetPrevious(previous);
     if (val->HasNext())
       val->GetNext()->SetPrevious(val);
   }

   static inline void
   InsertBefore(Instruction* after, Instruction* val) {
     return InsertAfter(after->GetPrevious(), val);
   }
 };

 class InstructionIterator {
  protected:
   Instruction* current_;

   explicit InstructionIterator(Instruction* current):
    current_(current) {
   }
  public:
   virtual ~InstructionIterator() = default;
   virtual bool HasNext() const = 0;
   virtual Instruction* Next() = 0;
 };

 class ForwardInstructionIterator : public InstructionIterator {
  public:
   explicit ForwardInstructionIterator(Instruction* current):
     InstructionIterator(current) {
   }
   ~ForwardInstructionIterator() override = default;

   bool HasNext() const override {
     return current_ != nullptr;
   }

   Instruction* Next() override {
     auto next = current_;
     current_ = current_->GetNext();
     return next;
   }
 };

 class BackwardInstructionIterator : public InstructionIterator {
  public:
   explicit BackwardInstructionIterator(Instruction* current):
     InstructionIterator(current) {
   }
   ~BackwardInstructionIterator() override = default;

   bool HasNext() const override {
     return current_ != nullptr;
   }

   Instruction* Next() override {
     auto next = current_;
     current_ = current_->GetPrevious();
     return next;
   }
 };

#define DECLARE_INSTRUCTION(Name) \
 public:                          \
  const char* GetName() const override { return #Name; }
}

#endif//POSEIDON_INSTRUCTION_H