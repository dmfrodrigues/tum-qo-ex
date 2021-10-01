#ifndef H_operator_Chi
#define H_operator_Chi
//---------------------------------------------------------------------------
#include "Register.hpp"
#include "operator/Operator.hpp"
#include <memory>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// A chi operator
class Chi : public Operator {
   public:
   /// An operation
   typedef void (*Operation)(const Register& a, const Register& b, Register& result);
   /// Add
   static void add(const Register& a, const Register& b, Register& result);
   /// Divide
   static void div(const Register& a, const Register& b, Register& result);
   /// Compare
   static void equal(const Register& a, const Register& b, Register& result);
   /// Compare
   static void notEqual(const Register& a, const Register& b, Register& result);
   /// Compare
   static void less(const Register& a, const Register& b, Register& result);
   /// Compare
   static void lessOrEqual(const Register& a, const Register& b, Register& result);
   /// Cast to different type
   static void cast(const Register& a, Register::State state, Register& result);

   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// The operation
   Operation op;
   /// The input of the operation
   const Register *left, *right;
   /// The output of the operation
   Register output;

   public:
   /// Constructor. Sample usage: new Chi(move(input),Chi::add,reg1,reg2);
   Chi(std::unique_ptr<Operator>&& input, Operation op, const Register* left, const Register* right);
   /// Destructor
   ~Chi() noexcept override;

   /// Open the operation
   void open() override;
   /// Get the next tuple
   bool next() override;
   /// Close the operator
   void close() override;

   /// Get the produced value
   [[nodiscard]] const Register* getResult() const { return &output; }
   /// Get all produced values
   [[nodiscard]] std::vector<const Register*> getOutput() const override;

   /// Print operator tree
   void printTree(std::ostream& out) const override;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
