#include "Register.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// Constructor
Register::Register() = default;
//---------------------------------------------------------------------------
/// Destructor
Register::~Register() noexcept = default;
//---------------------------------------------------------------------------
bool Register::operator==(const Register& r) const
// Comparison
{
   return values == r.values;
}
//---------------------------------------------------------------------------
bool Register::operator<(const Register& r) const
// Comparison
{
   // Make sure that the state is the same
   if (values.index() != r.values.index())
      return values.index() < r.values.index();

   // Compare
   return visit([&]<typename T>(const T& v) -> bool {
      if constexpr (std::same_as<T, std::monostate>)
         return false;
      else
         return v < get<T>(r.values);
   }, values);
}
//---------------------------------------------------------------------------
uint64_t Register::getHash() const
// Hash
{
   return visit([&]<typename T>(const T& v) {
      return std::hash<T>{}(v);
   }, values);
}
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Register& r)
// Output to stream
{
   switch (r.getState()) {
      case Register::State::Unbound: out << "null"; break;
      case Register::State::Int: out << r.getInt(); break;
      case Register::State::Double: out << r.getDouble(); break;
      case Register::State::Bool: out << (r.getBool() ? "true" : "false"); break;
      case Register::State::String: out << r.getString(); break;
   }
   return out;
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
