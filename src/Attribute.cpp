#include "Attribute.hpp"
#include <cstdlib>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
Attribute::Attribute()
   : type(Type::Int), size(4), uniqueValues(100), key(false), index(false)
// Constructor
{
}
//---------------------------------------------------------------------------
/// Destructor
Attribute::~Attribute() noexcept = default;
//---------------------------------------------------------------------------
static string escape(string_view str)
// Escape
{
   if (str.find_first_of(" \\") == string_view::npos)
       return string{str};

   string result;
   result.reserve(str.size());

   for (char c : str) {
      if (c == ' ')
         result += "\\s";
      else if (c == '\\')
         result += "\\\\";
      else
         result += c;
   }
   return result;
}
//---------------------------------------------------------------------------
static string unescape(string_view str)
// Unescape
{
   if (str.find('\\') == string_view::npos)
      return string{str};

   string result;
   result.reserve(str.size());
   for (auto iter = str.begin(), limit = str.end(); iter != limit; ++iter) {
      char c = *iter;
      if (c == '\\') {
         ++iter;
         if (iter == limit)
            result += '\\';
         else if ((*iter) == 's')
            result += ' ';
         else
            result += *iter;
      } else
         result += *iter;
   }
   return result;
}
//---------------------------------------------------------------------------
static void getToSpace(istream& in, string& target)
// Read to the next space
{
   char c;
   target.resize(0);
   while (in.get(c)) {
      if (c == ' ') break;
      target += c;
   }
}
//---------------------------------------------------------------------------
void Attribute::read(istream& in)
// Read
{
   in >> name;
   {
      unsigned t;
      in >> t;
      type = static_cast<Type>(t);
   }
   in >> size >> uniqueValues;
   string s, s2;
   in.get();
   getToSpace(in, s);
   s2 = unescape(s);
   switch (type) {
      case Type::Int: minValue.setInt(stoi(s2)); break;
      case Type::Double: minValue.setDouble(stof(s2)); break;
      case Type::Bool: minValue.setBool(s2 == "true"); break;
      case Type::String: minValue.setString(s2); break;
   }
   getToSpace(in, s);
   s2 = unescape(s);
   switch (type) {
      case Type::Int: maxValue.setInt(stoi(s2)); break;
      case Type::Double: maxValue.setDouble(stof(s2)); break;
      case Type::Bool: maxValue.setBool(s2 == "true"); break;
      case Type::String: maxValue.setString(s2); break;
   }

   getline(in, s);
   key = false;
   if (s.substr(0, 4) == "key ") {
      key = true;
      s = s.substr(4);
   } else if (s == "key") {
      key = true;
      s = "";
   }

   index = false;
   if (s.substr(0, 6) == "index ") {
      index = true;
      s = s.substr(6);
   } else if (s == "index") {
      index = true;
      s = "";
   }
}
//---------------------------------------------------------------------------
void Attribute::write(ostream& out) const
// Write
{
   out << name << " " << static_cast<unsigned>(type) << " " << size << " " << uniqueValues << " ";

   switch (type) {
      case Type::Int:
         if (minValue.isUnbound())
            out << "0";
         else
            out << minValue.getInt();
         break;
      case Type::Double:
         if (minValue.isUnbound())
            out << "0";
         else
            out << minValue.getDouble();
         break;
      case Type::Bool:
         if (minValue.isUnbound())
            out << "false";
         else
            out << minValue.getBool();
         break;
      case Type::String:
         if (minValue.isUnbound())
            out << "";
         else
            out << escape(minValue.getString());
         break;
   }
   out << " ";
   switch (type) {
      case Type::Int:
         if (maxValue.isUnbound())
            out << "0";
         else
            out << maxValue.getInt();
         break;
      case Type::Double:
         if (maxValue.isUnbound())
            out << "0";
         else
            out << maxValue.getDouble();
         break;
      case Type::Bool:
         if (maxValue.isUnbound())
            out << "false";
         else
            out << maxValue.getBool();
         break;
      case Type::String:
         if (maxValue.isUnbound())
            out << "";
         else
            out << escape(maxValue.getString());
         break;
   }
   if (key) out << " key";
   if (index) out << " index";
   out << endl;
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
