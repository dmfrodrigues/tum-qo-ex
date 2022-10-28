#ifndef H_Register
#define H_Register
//---------------------------------------------------------------------------
#include <iosfwd>
#include <string>
#include <variant>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// A runtime register
class Register {
   public:
   /// The int type
   using int_t = int;

   private:
   /// Values
   std::variant<std::monostate, int_t, double, bool, std::string> values;

   public:
   /// Possible states
   enum class State {
      Unbound,
      Int,
      Double,
      Bool,
      String
   };

   /// Constructor
   Register();
   /// Destructor
   ~Register() noexcept;

   /// Assign an unbound "value"
   void setUnbound() { values.emplace<std::monostate>(); }
   /// Assign an integer
   void setInt(int_t value) { values.emplace<int>(value); }
   /// Assign a double value
   void setDouble(double value) { values.emplace<double>(value); }
   /// Assign a bool value
   void setBool(bool value) { values.emplace<bool>(value); }
   /// Assign a string value
   void setString(std::string value) { values.emplace<std::string>(std::move(value)); }

   /// Make unbound register
   static Register makeUnbound() { return {}; }
   /// Make int register
   static Register makeInt(int_t v) { Register rv; rv.setInt(v); return rv; }
   /// Make double register
   static Register makeDouble(double v) { Register rv; rv.setDouble(v); return rv; }
   /// Make bool register
   static Register makeBool(bool v) { Register rv; rv.setBool(v); return rv; }
   /// Make string register
   static Register makeString(std::string v) { Register rv; rv.setString(std::move(v)); return rv; }

   /// Unbound?
   [[nodiscard]] bool isUnbound() const { return holds_alternative<std::monostate>(values); }
   /// Get the state
   [[nodiscard]] State getState() const { return static_cast<State>(values.index()); }
   /// Get the integer
   [[nodiscard]] int_t getInt() const { return get<int_t>(values); }
   /// Get the double
   [[nodiscard]] double getDouble() const { return get<double>(values); }
   /// Get the bool value
   [[nodiscard]] bool getBool() const { return get<bool>(values); }
   /// Get the string value
   [[nodiscard]] const std::string& getString() const { return get<std::string>(values); }

   /// Comparison
   [[nodiscard]] bool operator==(const Register& r) const;
   /// Comparison
   [[nodiscard]] bool operator<(const Register& r) const;
   /// Hash
   [[nodiscard]] uint64_t getHash() const;
   /// Hash
   struct Hasher {
      uint64_t operator()(const Register& r) const { return r.getHash(); };
   };
};
//---------------------------------------------------------------------------
/// Output to stream
std::ostream& operator<<(std::ostream& out, const Register& r);
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
