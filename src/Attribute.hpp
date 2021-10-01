#ifndef H_Attribute
#define H_Attribute
//---------------------------------------------------------------------------
#include "Register.hpp"
#include <iostream>
#include <string>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
class Table;
//---------------------------------------------------------------------------
/// An table attribute
class Attribute {
   public:
   /// Possible types
   enum class Type : unsigned {
       Int,
       Double,
       Bool,
       String
   };

   private:
   /// Name of the attribute
   std::string name;
   /// Type of the attribute
   Type type;
   /// Average size
   double size;
   /// Number of unique values
   unsigned uniqueValues;
   /// Minimum value
   Register minValue;
   /// Maximum value
   Register maxValue;
   /// Key attribute?
   bool key;
   /// Index available?
   bool index;

   friend class Table;

   public:
   /// Constructor
   Attribute();
   /// Destructor
   ~Attribute() noexcept;

   /// The name
   [[nodiscard]] const std::string& getName() const { return name; }
   /// The type
   [[nodiscard]] Type getType() const { return type; }
   /// The average size
   [[nodiscard]] double getSize() const { return size; }
   /// The number of unique values
   [[nodiscard]] unsigned getUniqueValues() const { return uniqueValues; }
   /// The minimum value
   [[nodiscard]] const Register& getMinValue() const { return minValue; }
   /// The maximum value
   [[nodiscard]] const Register& getMaxValue() const { return maxValue; }
   /// Key attribute?
   [[nodiscard]] bool getKey() const { return key; }
   /// Index available?
   [[nodiscard]] bool getIndex() const { return index; }

   /// Read
   void read(std::istream& in);
   /// Write
   void write(std::ostream& out) const;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
