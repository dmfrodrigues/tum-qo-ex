#ifndef H_operator_HashJoin
#define H_operator_HashJoin
//---------------------------------------------------------------------------
#include "Register.hpp"
#include "operator/Operator.hpp"
#include <memory>
#include <unordered_map>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// A hash join
class HashJoin : public Operator {
   private:
   /// The input
   std::unique_ptr<Operator> left, right;
   /// The registers
   const Register *leftValue, *rightValue;
   /// The copy mechanism
   std::vector<Register*> leftRegs;
   /// The hashtable
   std::unordered_multimap<Register, std::vector<Register>, Register::Hasher> table;
   /// Iterator
   std::unordered_multimap<Register, std::vector<Register>, Register::Hasher>::const_iterator iter, iterLimit;

   public:
   /// Constructor
   HashJoin(std::unique_ptr<Operator>&& left, std::unique_ptr<Operator>&& right, const Register* leftValue, const Register* rightValue);
   /// Destructor
   ~HashJoin() noexcept override;

   /// Open the operator
   void open() override;
   /// Get the next tuple
   bool next() override;
   /// Close the operator
   void close() override;

   /// Get all produced values
   [[nodiscard]] std::vector<const Register*> getOutput() const override;

   /// Print operator tree
   void printTree(std::ostream& out) const override;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
