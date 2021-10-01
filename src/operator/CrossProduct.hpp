#ifndef H_operator_CrossProduct
#define H_operator_CrossProduct
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include <memory>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// A cross product
class CrossProduct : public Operator {
   private:
   /// The input
   std::unique_ptr<Operator> left, right;
   /// Read the left side?
   bool readLeft;

   public:
   /// Constructor
   CrossProduct(std::unique_ptr<Operator>&& left, std::unique_ptr<Operator>&& right);
   /// Destructor
   ~CrossProduct() noexcept override;

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
