#ifndef H_operator_Printer
#define H_operator_Printer
//---------------------------------------------------------------------------
#include "Operator.hpp"
#include <memory>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// Prints tuple attributes
class Printer : public Operator {
   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// Registers to print
   std::vector<const Register*> toPrint;
   /// The output stream
   std::ostream& out;

   public:
   /// Constructor
   Printer(std::unique_ptr<Operator>&& input, std::ostream& out);
   /// Constructor
   Printer(std::unique_ptr<Operator>&& input, std::vector<const Register*> toPrint, std::ostream& out);
   /// Destructor
   ~Printer() noexcept override;

   /// Open the operator
   void open() override;
   /// Get the next ruple
   bool next() override;
   /// CLose the operator
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
