#include "operator/Chi.hpp"
#include <ostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
void Chi::add(const Register& a, const Register& b, Register& result)
// Add
{
   if (a.getState() == b.getState()) switch (a.getState()) {
         case Register::State::Int: result.setInt(a.getInt() + b.getInt()); break;
         case Register::State::Double: result.setDouble(a.getDouble() + b.getDouble()); break;
         default: result.setUnbound();
      }
   else
      result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::div(const Register& a, const Register& b, Register& result)
// Divide
{
   if (a.getState() == b.getState()) switch (a.getState()) {
         case Register::State::Int: result.setInt(a.getInt() / b.getInt()); break;
         case Register::State::Double: result.setDouble(a.getDouble() / b.getDouble()); break;
         default: result.setUnbound();
      }
   else
      result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::equal(const Register& a, const Register& b, Register& result)
// Compare
{
   if (a.getState() == b.getState()) switch (a.getState()) {
         case Register::State::Int: result.setBool(a.getInt() == b.getInt()); break;
         case Register::State::Double: result.setBool(a.getDouble() == b.getDouble()); break;
         case Register::State::Bool: result.setBool(a.getBool() == b.getBool()); break;
         case Register::State::String: result.setBool(a.getString() == b.getString()); break;
         default: result.setUnbound();
      }
   else
      result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::notEqual(const Register& a, const Register& b, Register& result)
// Compare
{
   if (a.getState() == b.getState()) switch (a.getState()) {
         case Register::State::Int: result.setBool(a.getInt() != b.getInt()); break;
         case Register::State::Double: result.setBool(a.getDouble() != b.getDouble()); break;
         case Register::State::Bool: result.setBool(a.getBool() != b.getBool()); break;
         case Register::State::String: result.setBool(a.getString() != b.getString()); break;
         default: result.setUnbound();
      }
   else
      result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::less(const Register& a, const Register& b, Register& result)
// Compare
{
   if (a.getState() == b.getState()) switch (a.getState()) {
         case Register::State::Int: result.setBool(a.getInt() < b.getInt()); break;
         case Register::State::Double: result.setBool(a.getDouble() < b.getDouble()); break;
         case Register::State::Bool: result.setBool(a.getBool() < b.getBool()); break;
         case Register::State::String: result.setBool(a.getString() < b.getString()); break;
         default: result.setUnbound();
      }
   else
      result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::lessOrEqual(const Register& a, const Register& b, Register& result)
// Compare
{
   if (a.getState() == b.getState()) switch (a.getState()) {
         case Register::State::Int: result.setBool(a.getInt() <= b.getInt()); break;
         case Register::State::Double: result.setBool(a.getDouble() <= b.getDouble()); break;
         case Register::State::Bool: result.setBool(a.getBool() <= b.getBool()); break;
         case Register::State::String: result.setBool(a.getString() <= b.getString()); break;
         default: result.setUnbound();
      }
   else
      result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::cast(const Register& a, Register::State state, Register& result)
// Compare
{
   if (a.getState() == state) {
      result = a;
      return;
   }
   switch (a.getState()) {
      case Register::State::Unbound: result.setUnbound(); break;
      case Register::State::Int:
         switch (state) {
            case Register::State::Int: result.setInt(a.getInt()); break;
            case Register::State::Double: result.setDouble(a.getInt()); break;
            case Register::State::Bool: result.setBool(a.getInt()); break;
            case Register::State::Unbound:
            case Register::State::String: result.setUnbound(); break;
         }
         break;
      case Register::State::Double:
         switch (state) {
            case Register::State::Int: result.setInt(static_cast<Register::int_t>(a.getDouble())); break;
            case Register::State::Double: result.setDouble(a.getDouble()); break;
            case Register::State::Unbound:
            case Register::State::Bool:
            case Register::State::String: result.setUnbound(); break;
         }
         break;
      case Register::State::Bool:
         switch (state) {
            case Register::State::Int: result.setInt(a.getBool()); break;
            case Register::State::Unbound:
            case Register::State::Double:
            case Register::State::Bool:
            case Register::State::String: result.setUnbound(); break;
         }
      case Register::State::String: result.setUnbound(); break;
   }
}
//---------------------------------------------------------------------------
Chi::Chi(unique_ptr<Operator>&& input, Operation op, const Register* left, const Register* right)
   : input(move(input)), op(op), left(left), right(right)
// Constructor
{
}
//---------------------------------------------------------------------------
/// Destructor
Chi::~Chi() noexcept = default;
//---------------------------------------------------------------------------
void Chi::open()
// Open the operator
{
   input->open();
}
//---------------------------------------------------------------------------
bool Chi::next()
// Get the next tuple
{
   // Produce a tuple
   if (!input->next())
      return false;

   // Calculate the value
   op(*left, *right, output);

   return true;
}
//---------------------------------------------------------------------------
void Chi::close()
// Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> Chi::getOutput() const
// Get all produced values
{
   vector<const Register*> result = input->getOutput();
   result.push_back(&output);
   return result;
}
//---------------------------------------------------------------------------
void Chi::printTree(ostream& out) const
// Print operator tree
{
   out << "CHI(";
   input->printTree(out);
   out << ")";
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
