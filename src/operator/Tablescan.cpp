#include "operator/Tablescan.hpp"
#include "Table.hpp"
#include <cassert>
#include <cstdlib>
#include <ostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
Tablescan::Tablescan(Table& table)
   : table(table), bufferStart(0), bufferStop(0), filePos(0)
// Constructor
{
   output.resize(table.getAttributeCount());
}
//--------------------------------------------------------------------------
/// Destructor
Tablescan::~Tablescan() noexcept = default;
//---------------------------------------------------------------------------
void Tablescan::open()
// Open the operator
{
   bufferStart = bufferStop = 0;
   filePos = 0;
}
//---------------------------------------------------------------------------
bool Tablescan::next()
// Get the next tuple
{
   bool escape = false;
   for (size_t index = 0, limit = output.size(); index < limit; ++index) {
      buf.resize(0);
      while (true) {
         if (bufferStart >= bufferStop) {
            [[maybe_unused]] auto ind = table.io.tellg();
            table.io.seekg(filePos, ios_base::beg);
            table.io.read(buffer, bufferSize);
            auto len = table.io.gcount();
            if (len < 1) {
               table.io.clear();
               return false;
            }
            bufferStart = 0;
            assert(static_cast<size_t>(filePos) + len <= ~0u);
            bufferStop = static_cast<unsigned>(len);
            filePos = static_cast<unsigned>(filePos + len);
         }
         char c = buffer[bufferStart++];
         if (escape) {
            escape = false;
            buf += c;
            continue;
         }
         if (c == '\r') continue;
         if ((c == ';') || (c == '\n')) {
            Register& r = output[index];
            switch (table.attributes[index].getType()) {
               case Attribute::Type::Int: r.setInt(stoi(buf)); break;
               case Attribute::Type::Double: r.setDouble(stof(buf)); break;
               case Attribute::Type::Bool: r.setBool(buf == "true"); break;
               case Attribute::Type::String: r.setString(buf); break;
            }
            break;
         } else if (c == '\\') {
            escape = true;
         } else
            buf += c;
      }
   }

   table.io.clear();
   return true;
}
//---------------------------------------------------------------------------
void Tablescan::close()
// Close the operator
{
}
//---------------------------------------------------------------------------
vector<const Register*> Tablescan::getOutput() const
// Get all produced values
{
   vector<const Register*> result;
   for (auto& o : output)
      result.push_back(&o);
   return result;
}
//---------------------------------------------------------------------------
const Register* Tablescan::getOutput(const std::string& name) const
// Get one produced value
{
   int slot = table.findAttribute(name);
   if (slot < 0) return nullptr;
   return &output[slot];
}
//---------------------------------------------------------------------------
void Tablescan::printTree(ostream& out) const
// Print operator tree
{
   out << "TABLE SCAN (" << table.name << ")";
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
