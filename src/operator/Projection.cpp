#include "operator/Projection.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
Projection::Projection(unique_ptr<Operator>&& input, const vector<const Register*>& output)
   : input(move(input)), output(output)
// Constructor
{
}
//---------------------------------------------------------------------------
/// Destructor
Projection::~Projection() noexcept = default;
//---------------------------------------------------------------------------
void Projection::open()
// Open the operator
{
   input->open();
}
//---------------------------------------------------------------------------
bool Projection::next()
// Get the next tuple
{
   return input->next();
}
//---------------------------------------------------------------------------
void Projection::close()
// Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> Projection::getOutput() const
// Get all produced values
{
   return output;
}
//---------------------------------------------------------------------------
void Projection::printTree(ostream& out) const
// Print operator tree
{
   out << "PROJECTION (";
   input->printTree(out);
   out << ")";
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
