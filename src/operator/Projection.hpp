#ifndef H_operator_Projection
#define H_operator_Projection
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include <memory>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// A projection
class Projection : public Operator {
   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// The output
   std::vector<const Register*> output;

   public:
   /// Constructor
   Projection(std::unique_ptr<Operator>&& input, const std::vector<const Register*>& output);
   /// Destructor
   ~Projection() noexcept override;

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
