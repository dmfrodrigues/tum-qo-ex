#include "operator/Printer.hpp"
#include "Register.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
Printer::Printer(unique_ptr<Operator>&& input, std::ostream& out)
   : input(move(input)), toPrint(this->input->getOutput()), out(out)
// Constructor
{
}
//---------------------------------------------------------------------------
Printer::Printer(unique_ptr<Operator>&& input, vector<const Register*> toPrint, std::ostream& out)
   : input(move(input)), toPrint(move(toPrint)), out(out)
// Constructor
{
}
//---------------------------------------------------------------------------
/// Destructor
Printer::~Printer() noexcept = default;
//---------------------------------------------------------------------------
void Printer::open()
// Open the operator
{
   input->open();
}
//---------------------------------------------------------------------------
bool Printer::next()
// Get the next tuple
{
   // Produce the next tuple
   if (!input->next())
      return false;

   // Print the entries
   for (size_t index = 0, limit = toPrint.size(); index < limit; ++index) {
      if (index) out << ' ';
      const Register& r = *toPrint[index];
      out << r;
   }
   out << endl;

   return true;
}
//---------------------------------------------------------------------------
void Printer::close()
// Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> Printer::getOutput() const
// Get all produced values
{
   return input->getOutput();
}
//---------------------------------------------------------------------------
void Printer::printTree(ostream& out) const
// Print operator tree
{
   out << "PRINT (";
   input->printTree(out);
   out << ")";
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
