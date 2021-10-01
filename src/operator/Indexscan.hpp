#ifndef H_operator_IndexScan
#define H_operator_IndexScan
//---------------------------------------------------------------------------
#include "Table.hpp"
#include "operator/Operator.hpp"
#include <map>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// An indexscan operator
class Indexscan : public Operator {
   private:
   /// The buffer size
   static const unsigned bufferSize = 4096;

   /// The table
   Table& table;
   /// The index
   std::map<Register, unsigned>& index;
   /// The iterator over the index
   std::map<Register, unsigned>::const_iterator iter, iterLimit;
   /// The bounds
   const Register *lowerBound, *upperBound;
   /// Buffer pointers
   unsigned bufferStart = 0, bufferStop = 0;
   /// Construction helper
   std::string buf;
   /// The output
   std::vector<Register> output;
   /// A small buffer
   char buffer[bufferSize]{};

   public:
   /// Constructor
   Indexscan(Table& scale, unsigned indexAttribute, const Register* lowerBounds, const Register* upperBounds);
   /// Destructor
   ~Indexscan() noexcept override;

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
