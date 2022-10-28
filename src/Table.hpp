#ifndef H_Table
#define H_Table
//---------------------------------------------------------------------------
#include "Attribute.hpp"
#include <cassert>
#include <fstream>
#include <map>
#include <span>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
class Database;
class Tablescan;
class Indexscan;
//---------------------------------------------------------------------------
/// A database table
class Table {
   private:
   /// The name
   std::string name;
   /// The file
   std::string file;
   /// The index file
   std::string indexFile;
   /// The cardinality
   unsigned cardinality;
   /// The attributes
   std::vector<Attribute> attributes;
   /// The indices
   std::vector<std::map<Register, unsigned>> indices;
   /// The I/O interface
   std::fstream io;
   /// Unwritten changes?
   bool dirty;

   friend class Database;
   friend class Tablescan;
   friend class Indexscan;

   public:
   /// Constructor
   Table();
   /// Constructor
   Table(std::string name, std::string file, std::string indexFile);
   /// Copy constructor
   Table(const Table&) = delete;
   /// Destructor
   ~Table() noexcept;

   //// Dirty?
   bool isDirty() const { return dirty; }
   /// Add an attribute
   void addAttribute(const std::string& name, Attribute::Type type, bool key);
   /// Read
   void read(std::istream& in);
   /// Write
   void write(std::ostream& out);
   /// Initialize the table io
   void initIO();

   /// Insert a new tuple
   void insertValues(std::span<const Register> values);
   /// Insert a new tuple
   void insertValues(std::initializer_list<Register> values) { insertValues(std::span{std::data(values), values.size()}); };
   /// Update the statistics
   void runStats();

   /// The cardinality
   [[nodiscard]] unsigned getCardinality() const { return cardinality; }
   /// The number of attributes
   [[nodiscard]] unsigned getAttributeCount() const {
      assert(attributes.size() <= ~0u);
      return static_cast<unsigned>(attributes.size());
   }
   /// A specific attribute
   [[nodiscard]] const Attribute& getAttribute(unsigned index) const { return attributes[index]; }
   /// Search for a specific attribute. Returns -1 if attribute does not exist
   [[nodiscard]] int findAttribute(std::string_view name) const;
   /// Get the name
   [[nodiscard]] const std::string& getName() const { return name; }
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
