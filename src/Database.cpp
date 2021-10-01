#include "Database.hpp"
#include <algorithm>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
Database::Database()
   : dirty(false)
// Constructor
{
}
//---------------------------------------------------------------------------
Database::~Database() noexcept
// Destructor
{
   close();
}
//---------------------------------------------------------------------------
void Database::open(const std::string& name)
// Open an existing database
{
   close();

   repoFile = name;
   baseDir = name + ".";
   read();
}
//---------------------------------------------------------------------------
void Database::create(const std::string& name)
// Create a new database
{
   close();

   repoFile = name;
   baseDir = name + ".";
   dirty = true;
}
//---------------------------------------------------------------------------
bool Database::isDirty() const
// Dirty?
{
   if (dirty)
      return true;
   return any_of(tables.begin(), tables.end(), [](const auto& p) { return p.second.isDirty(); });
}
//---------------------------------------------------------------------------
void Database::close()
// Close the database
{
   if (isDirty())
      write();
}
//---------------------------------------------------------------------------
void Database::read()
// Read the metadata information
{
   ifstream in(repoFile);
   if (!in.is_open()) {
      cerr << "unable to read " << repoFile << endl;
      throw std::runtime_error("unable to read");
   }

   string tableName;
   while (getline(in, tableName)) {
      Table& table = tables[tableName];
      table.name = tableName;
      table.file = baseDir + tableName;
      table.indexFile = baseDir + tableName + ".idx";
      table.read(in);
   }
}
//---------------------------------------------------------------------------
void Database::write()
// Write the metadata inforamtion
{
   ofstream out(repoFile);
   if (!out.is_open()) {
      cerr << "unable to write " << repoFile << endl;
      throw std::runtime_error("unable to write");
   }

   for (auto& [name, table] : tables) {
      out << name << endl;
      table.write(out);
   }
   dirty = false;
}
//---------------------------------------------------------------------------
Table& Database::createTable(const std::string& name)
// Create a table
{
   if (tables.count(name)) {
      cerr << "table " << name << " already exists" << endl;
      throw std::runtime_error("table already exists");
   }

   string tableFile = baseDir + name, indexFile = tableFile + ".idx";
   remove(tableFile.c_str());
   {
      ofstream out(tableFile);
      if (!out.is_open()) {
         cerr << "unable to create " << tableFile << endl;
         throw std::runtime_error("unable to create table file");
      }
   }
   remove(indexFile.c_str());
   {
      ofstream out(indexFile);
      if (!out.is_open()) {
         cerr << "unable to create " << indexFile << endl;
         throw std::runtime_error("unable to create index file");
      }
   }

   Table& table = tables[name];
   table.name = name;
   table.file = tableFile;
   table.indexFile = indexFile;
   dirty = true;
   table.initIO();

   return table;
}
//---------------------------------------------------------------------------
void Database::dropTable(const std::string& name)
// Drop a table
{
   if (!tables.count(name)) {
      cerr << "table " << name << " not found" << endl;
      throw std::runtime_error("table not found");
   }

   Table& table = tables[name];
   remove(table.file.c_str());
   remove(table.indexFile.c_str());

   tables.erase(name);
   dirty = true;
}
//---------------------------------------------------------------------------
Table& Database::getTable(const std::string& name)
// Get a table
{
   if (!tables.count(name)) {
      cerr << "table " << name << " not found" << endl;
      throw std::runtime_error("table not found");
   }
   return tables[name];
}
//---------------------------------------------------------------------------
void Database::runStats()
// Update the statistics
{
   for (auto& [name, table] : tables)
       table.runStats();
}
//---------------------------------------------------------------------------
void Database::createUniDb(const std::string& name)
/// Generate a sample university database
{
   Database& db = *this;
   db.create(name);
   auto& prof = db.createTable("professoren");
   prof.addAttribute("persnr", Attribute::Type::Int, true);
   prof.addAttribute("name", Attribute::Type::String, false);
   prof.addAttribute("rang", Attribute::Type::String, false);
   prof.addAttribute("raum", Attribute::Type::Int, false);
   auto& stud = db.createTable("studenten");
   stud.addAttribute("matrnr", Attribute::Type::Int, true);
   stud.addAttribute("name", Attribute::Type::String, false);
   stud.addAttribute("semester", Attribute::Type::Int, false);
   auto& vorl = db.createTable("vorlesungen");
   vorl.addAttribute("vorlnr", Attribute::Type::Int, true);
   vorl.addAttribute("titel", Attribute::Type::String, false);
   vorl.addAttribute("sws", Attribute::Type::Int, false);
   vorl.addAttribute("gelesenvon", Attribute::Type::Int, false);
   auto& vora = db.createTable("voraussetzen");
   vora.addAttribute("vorgaenger", Attribute::Type::Int, false);
   vora.addAttribute("nachfolger", Attribute::Type::Int, false);
   auto& hoer = db.createTable("hoeren");
   hoer.addAttribute("matrnr", Attribute::Type::Int, false);
   hoer.addAttribute("vorlnr", Attribute::Type::Int, false);
   auto& assi = db.createTable("assistenten");
   assi.addAttribute("persnr", Attribute::Type::Int, true);
   assi.addAttribute("name", Attribute::Type::String, false);
   assi.addAttribute("fachgebiet", Attribute::Type::String, false);
   assi.addAttribute("boss", Attribute::Type::Int, false);
   auto& prue = db.createTable("pruefen");
   prue.addAttribute("matrnr", Attribute::Type::Int, false);
   prue.addAttribute("vorlnr", Attribute::Type::Int, false);
   prue.addAttribute("persnr", Attribute::Type::Int, false);
   prue.addAttribute("note", Attribute::Type::Int, false);
   prof.insertValues({Register::makeInt(2125), Register::makeString("Sokrates"), Register::makeString("C4"), Register::makeInt(226)});
   prof.insertValues({Register::makeInt(2126), Register::makeString("Russel"), Register::makeString("C4"), Register::makeInt(232)});
   prof.insertValues({Register::makeInt(2127), Register::makeString("Kopernikus"), Register::makeString("C3"), Register::makeInt(310)});
   prof.insertValues({Register::makeInt(2133), Register::makeString("Popper"), Register::makeString("C3"), Register::makeInt(52)});
   prof.insertValues({Register::makeInt(2134), Register::makeString("Augustinus"), Register::makeString("C3"), Register::makeInt(309)});
   prof.insertValues({Register::makeInt(2136), Register::makeString("Curie"), Register::makeString("C4"), Register::makeInt(36)});
   prof.insertValues({Register::makeInt(2137), Register::makeString("Kant"), Register::makeString("C4"), Register::makeInt(7)});
   stud.insertValues({Register::makeInt(24002), Register::makeString("Xenokrates"), Register::makeInt(18)});
   stud.insertValues({Register::makeInt(25403), Register::makeString("Jonas"), Register::makeInt(12)});
   stud.insertValues({Register::makeInt(26120), Register::makeString("Fichte"), Register::makeInt(10)});
   stud.insertValues({Register::makeInt(26830), Register::makeString("Aristoxenos"), Register::makeInt(8)});
   stud.insertValues({Register::makeInt(27550), Register::makeString("Schopenhauer"), Register::makeInt(6)});
   stud.insertValues({Register::makeInt(28106), Register::makeString("Carnap"), Register::makeInt(3)});
   stud.insertValues({Register::makeInt(29120), Register::makeString("Theophrastos"), Register::makeInt(2)});
   stud.insertValues({Register::makeInt(29555), Register::makeString("Feuerbach"), Register::makeInt(2)});
   vorl.insertValues({Register::makeInt(5001), Register::makeString("Grundzüge"), Register::makeInt(4), Register::makeInt(2137)});
   vorl.insertValues({Register::makeInt(5041), Register::makeString("Ethik"), Register::makeInt(4), Register::makeInt(2125)});
   vorl.insertValues({Register::makeInt(5043), Register::makeString("Erkenntnistheorie"), Register::makeInt(3), Register::makeInt(2126)});
   vorl.insertValues({Register::makeInt(5049), Register::makeString("Mäeutik"), Register::makeInt(2), Register::makeInt(2125)});
   vorl.insertValues({Register::makeInt(4052), Register::makeString("Logik"), Register::makeInt(4), Register::makeInt(2125)});
   vorl.insertValues({Register::makeInt(5052), Register::makeString("Wissenschaftstheorie"), Register::makeInt(3), Register::makeInt(2126)});
   vorl.insertValues({Register::makeInt(5216), Register::makeString("Bioethik"), Register::makeInt(2), Register::makeInt(2126)});
   vorl.insertValues({Register::makeInt(5259), Register::makeString("Der Wiener Kreis"), Register::makeInt(2), Register::makeInt(2133)});
   vorl.insertValues({Register::makeInt(5022), Register::makeString("Glaube und Wissen"), Register::makeInt(2), Register::makeInt(2134)});
   vorl.insertValues({Register::makeInt(4630), Register::makeString("Die 3 Kritiken"), Register::makeInt(4), Register::makeInt(2137)});
   vora.insertValues({Register::makeInt(5001), Register::makeInt(5041)});
   vora.insertValues({Register::makeInt(5001), Register::makeInt(5043)});
   vora.insertValues({Register::makeInt(5001), Register::makeInt(5049)});
   vora.insertValues({Register::makeInt(5041), Register::makeInt(5216)});
   vora.insertValues({Register::makeInt(5043), Register::makeInt(5052)});
   vora.insertValues({Register::makeInt(5041), Register::makeInt(5052)});
   vora.insertValues({Register::makeInt(5052), Register::makeInt(5259)});
   hoer.insertValues({Register::makeInt(26120), Register::makeInt(5001)});
   hoer.insertValues({Register::makeInt(27550), Register::makeInt(5001)});
   hoer.insertValues({Register::makeInt(27550), Register::makeInt(4052)});
   hoer.insertValues({Register::makeInt(28106), Register::makeInt(5041)});
   hoer.insertValues({Register::makeInt(28106), Register::makeInt(5052)});
   hoer.insertValues({Register::makeInt(28106), Register::makeInt(5216)});
   hoer.insertValues({Register::makeInt(28106), Register::makeInt(5259)});
   hoer.insertValues({Register::makeInt(29120), Register::makeInt(5001)});
   hoer.insertValues({Register::makeInt(29120), Register::makeInt(5041)});
   hoer.insertValues({Register::makeInt(29120), Register::makeInt(5049)});
   hoer.insertValues({Register::makeInt(29555), Register::makeInt(5022)});
   hoer.insertValues({Register::makeInt(25403), Register::makeInt(5022)});
   hoer.insertValues({Register::makeInt(29555), Register::makeInt(5001)});
   assi.insertValues({Register::makeInt(3002), Register::makeString("Platon"), Register::makeString("Ideenlehre"), Register::makeInt(2125)});
   assi.insertValues({Register::makeInt(3003), Register::makeString("Aristoteles"), Register::makeString("Syllogistik"), Register::makeInt(2125)});
   assi.insertValues({Register::makeInt(3004), Register::makeString("Wittgenstein"), Register::makeString("Sprachtheorie"), Register::makeInt(2126)});
   assi.insertValues({Register::makeInt(3005), Register::makeString("Rhetikus"), Register::makeString("Planetenbewegung"), Register::makeInt(2127)});
   assi.insertValues({Register::makeInt(3006), Register::makeString("Newton"), Register::makeString("Keplersche Gesetze"), Register::makeInt(2127)});
   assi.insertValues({Register::makeInt(3007), Register::makeString("Spinoza"), Register::makeString("Gott und Natur"), Register::makeInt(2134)});
   prue.insertValues({Register::makeInt(28106), Register::makeInt(5001), Register::makeInt(2126), Register::makeInt(1)});
   prue.insertValues({Register::makeInt(25403), Register::makeInt(5041), Register::makeInt(2125), Register::makeInt(2)});
   prue.insertValues({Register::makeInt(27550), Register::makeInt(4630), Register::makeInt(2137), Register::makeInt(2)});
   db.write();
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
