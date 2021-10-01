#include "Table.hpp"
#include "operator/Tablescan.hpp"
#include "operator/Chi.hpp"
#include "operator/Groupby.hpp"
#include <cstdlib>
#include <memory>
#include <unordered_set>
#include <sstream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
Table::Table()
   : cardinality(0), dirty(false)
// Constructor
{
}
//---------------------------------------------------------------------------
Table::Table(string name, string file, string indexFile)
   : name(move(name)), file(move(file)), indexFile(move(indexFile)), cardinality(0), dirty(false)
// Constructor
{
}
//---------------------------------------------------------------------------
/// Destructor
Table::~Table() noexcept = default;
//---------------------------------------------------------------------------
void Table::addAttribute(const string& name, Attribute::Type type, bool key)
// Add an attribute
{
   if (findAttribute(name) >= 0) {
      cerr << "attribute " << name << " already defined" << endl;
      throw;
   }

   Attribute a;
   a.name = name;
   a.type = type;
   a.key = key;
   attributes.push_back(a);
   indices.resize(indices.size() + 1);

   dirty = true;
}
//---------------------------------------------------------------------------
static void readIndex(istream& in, map<Register, unsigned>& index, Attribute::Type type, unsigned cardinality)
// Read an index
{
   unsigned count;
   in.read(reinterpret_cast<char*>(&count), sizeof(count));
   if (count != cardinality) {
      cerr << "index corruption encountered, reload the database" << endl;
      throw;
   }
   index.clear();

   switch (type) {
      case Attribute::Type::Int:
         for (unsigned index2 = 0; index2 < count; ++index2) {
            int val;
            unsigned pos;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
            Register r;
            r.setInt(val);
            index[r] = pos;
         }
         break;
      case Attribute::Type::Double:
         for (unsigned index2 = 0; index2 < count; ++index2) {
            double val;
            unsigned pos;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
            Register r;
            r.setDouble(val);
            index[r] = pos;
         }
         break;
      case Attribute::Type::Bool:
         for (unsigned index2 = 0; index2 < count; ++index2) {
            bool val;
            unsigned pos;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
            Register r;
            r.setBool(val);
            index[r] = pos;
         }
         break;
      case Attribute::Type::String:
         for (unsigned index2 = 0; index2 < count; ++index2) {
            unsigned valLen, pos;
            string val;
            in.read(reinterpret_cast<char*>(&valLen), sizeof(valLen));
            val.resize(valLen);
            in.read(const_cast<char*>(val.data()), valLen);
            in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
            Register r;
            r.setString(val);
            index[r] = pos;
         }
         break;
   }
}
//---------------------------------------------------------------------------
void Table::initIO()
// Initialize the table io
{
   io.open(file, ios_base::in | ios_base::out | ios_base::binary);
   if (!io.is_open()) {
      cerr << "unable to open " << file << endl;
      throw runtime_error("cannot initialize table IO");
   }
}
//---------------------------------------------------------------------------
void Table::read(istream& in)
// Read
{
   string line;
   getline(in, line);
   cardinality = stoi(line);

   attributes.clear();
   while (getline(in, line)) {
      if (line.empty()) break;
      Attribute a;
      stringstream s(line);
      a.read(s);
      attributes.push_back(a);
   }

   indices.resize(attributes.size());
   ifstream indicesIn(indexFile, ios_base::in | ios_base::binary);
   if (!indicesIn.is_open()) {
      cerr << "unable to open " << indexFile << endl;
      throw runtime_error("cannot read index");
   }
   for (unsigned index = 0, limit = attributes.size(); index < limit; ++index)
      if (attributes[index].key || attributes[index].index)
         readIndex(indicesIn, indices[index], attributes[index].type, cardinality);
   indicesIn.close();

   initIO();
}
//---------------------------------------------------------------------------
static void writeIndex(ostream& out, map<Register, unsigned>& index, Attribute::Type type)
// Write an index
{
   unsigned count = index.size();
   out.write(reinterpret_cast<char*>(&count), sizeof(count));

   switch (type) {
      case Attribute::Type::Int:
         for (const auto& [reg, pos] : index) {
            int val = reg.getInt();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            out.write(reinterpret_cast<const char*>(&pos), sizeof(pos));
         }
         break;
      case Attribute::Type::Double:
         for (const auto& [reg, pos] : index) {
            double val = reg.getDouble();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            out.write(reinterpret_cast<const char*>(&pos), sizeof(pos));
         }
         break;
      case Attribute::Type::Bool:
         for (const auto& [reg, pos] : index) {
            bool val = reg.getBool();
            out.write(reinterpret_cast<const char*>(&val), sizeof(bool));
            out.write(reinterpret_cast<const char*>(&pos), sizeof(pos));
         }
         break;
      case Attribute::Type::String:
         for (const auto& [reg, pos] : index) {
            const string& val = reg.getString();
            unsigned valLen = val.length();
            out.write(reinterpret_cast<const char*>(&valLen), sizeof(valLen));
            out.write(val.data(), valLen);
            out.write(reinterpret_cast<const char*>(&pos), sizeof(pos));
         }
         break;
   }
}
//---------------------------------------------------------------------------
void Table::write(ostream& out)
// Write
{
   out << cardinality << endl;
   for (auto& a : attributes)
      a.write(out);
   out << endl;

   ofstream indicesOut(indexFile, ios_base::out | ios_base::binary);
   if (!indicesOut.is_open()) {
      cerr << "unable to write " << indexFile << endl;
      throw;
   }
   for (unsigned index = 0, limit = attributes.size(); index < limit; ++index)
      if (attributes[index].key || attributes[index].index)
         writeIndex(indicesOut, indices[index], attributes[index].type);
   indicesOut.close();

   dirty = false;
}
//---------------------------------------------------------------------------
void Table::insertValues(std::span<const Register> values)
// Insert a new tuple
{
   // Check key constraints first
   for (size_t index = 0, limit = attributes.size(); index < limit; ++index)
      if (attributes[index].key && indices[index].count(values[index])) {
         cerr << "key constraint for " << attributes[index].name << " violated" << endl;
         throw;
      }

   // Seek to the end of file and write the new tuple
   io.seekg(0, ios::end);

   if (!io)
      throw runtime_error("table IO error");

   unsigned pos = io.tellg();
   for (unsigned index = 0, limit = attributes.size(); index < limit; ++index) {
      if (index) io << ';';
      const Register& v = values[index];
      switch (v.getState()) {
         case Register::State::Unbound: io << "NULL"; break;
         case Register::State::Int: io << v.getInt(); break;
         case Register::State::Double: io << v.getDouble(); break;
         case Register::State::Bool: io << (v.getBool() ? "true" : "false"); break;
         case Register::State::String:
            for (auto c : v.getString()) {
               if ((c == ':') || (c == '\\'))
                  io << '\\';
               io << c;
            }
            break;
      }
   }
   io << endl;

   // Update the indices
   for (size_t index = 0, limit = attributes.size(); index < limit; ++index)
      if (attributes[index].key)
         indices[index][values[index]] = pos;

   cardinality++;
   dirty = true;
}
//---------------------------------------------------------------------------
void Table::runStats()
// Update the statistics
{
   // Should use a group by operator instead...
   vector<unordered_set<Register, Register::Hasher>> stats;
   stats.resize(attributes.size());

   // Perform the scan
   unique_ptr<Operator> op = make_unique<Tablescan>(*this);
   vector<const Register*> output = op->getOutput();
   vector<const Register*> stringLens;
   Register empty;

   Register four, eight, one;
   four.setDouble(4);
   eight.setDouble(8);
   one.setDouble(1);
   struct Result {
      const Register *min = nullptr, *max = nullptr, *uniqueValues = nullptr, *size = nullptr;
   };
   vector<Result> results(attributes.size());

   auto gb = make_unique<Groupby>(move(op));

   for (size_t index = 0, limit = attributes.size(); index < limit; ++index) {
      auto& res = results[index];
      res.min = gb->addAggregation({Groupby::AggregationType::Min, output[index]});
      res.max = gb->addAggregation({Groupby::AggregationType::Max, output[index]});
      res.uniqueValues = gb->addAggregation({Groupby::AggregationType::CountDistinct, output[index]});
      switch (attributes[index].type) {
         case Attribute::Type::Int: res.size = &four; break;
         case Attribute::Type::Double: res.size = &eight; break;
         case Attribute::Type::Bool: res.size = &one; break;
         case Attribute::Type::String: {
            auto chi = make_unique<Chi>(move(gb->input), [](const Register& a, const Register&, Register& result) { result.setDouble(a.getString().length()); }, output[index], &empty);
            res.size = gb->addAggregation({Groupby::AggregationType::Avg, chi->getResult()});
            gb->input = move(chi);
            break;
         }
      }
   }

   gb->open();
   gb->next();

   // Update the statistics
   for (size_t index = 0, limit = attributes.size(); index < limit; ++index) {
      attributes[index].uniqueValues = results[index].uniqueValues->getInt();
      attributes[index].minValue = *results[index].min;
      attributes[index].maxValue = *results[index].max;
      attributes[index].size = results[index].size->isUnbound() ? 1.0 : results[index].size->getDouble();
   }

   gb->close();

   dirty = true;
}
//---------------------------------------------------------------------------
int Table::findAttribute(std::string_view name) const
// Search for a specific attribute
{
   for (unsigned index = 0, limit = attributes.size(); index < limit; ++index)
      if (attributes[index].getName() == name)
         return index;
   return -1;
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
