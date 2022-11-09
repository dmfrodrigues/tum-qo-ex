#ifndef H_SQLParser
#define H_SQLParser
//---------------------------------------------------------------------------
#include "Register.hpp"
#include "operator/Tablescan.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <list>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
class Database;
class Operator;
//---------------------------------------------------------------------------
/// Semantic or syntactic errors
struct IncorrectSqlError : std::runtime_error {
   /// Constructor
   using std::runtime_error::runtime_error;

   /// Do not modify this
   [[nodiscard]] const char* what() const noexcept override { return "incorrect SQL"; }
};
//---------------------------------------------------------------------------
/// An operator tree with associated constants
struct OperatorTree {
   /// The root of the operator tree
   std::unique_ptr<Operator> root;
   /// Associated constant
   std::vector<std::unique_ptr<Register>> constants;

   /// Constructor
   explicit OperatorTree(std::unique_ptr<Operator> root, std::vector<std::unique_ptr<Register>> constants) noexcept;
   /// Move constructor
   OperatorTree(OperatorTree&&) noexcept;
   /// Destructor
   ~OperatorTree() noexcept;

   /// Move assignment
   OperatorTree& operator=(OperatorTree&&) noexcept;
};
//---------------------------------------------------------------------------
/// A parsed sql join query
struct JoinQuery {
   /// Name of a base table
   using TableName = std::string;
   /// Name of a binding
   using BindingName = std::string;
   /// Name of an attribute
   using AttributeName = std::string;
   /// Table and binding
   struct Relation {
      /// The name of the table
      TableName table;
      /// The name of the binding
      BindingName binding;

      /// Equality
      auto operator==(const Relation& o) const { return std::tie(table, binding) == std::tie(o.table, o.binding); }
      /// Comparison
      auto operator<(const Relation& o) const { return std::tie(table, binding) < std::tie(o.table, o.binding); }
   };
   /// Binding and attribute
   struct BindingAttribute {
      /// The name of the binding
      BindingName binding;
      /// The name of the attribute
      AttributeName attribute;

      /// Equality
      auto operator==(const BindingAttribute& o) const { return std::tie(binding, attribute) == std::tie(o.binding, o.attribute); }
      /// Comparison
      auto operator<(const BindingAttribute& o) const { return std::tie(binding, attribute) < std::tie(o.binding, o.attribute); }
   };
   /// Value of a constant
   using Constant = Register;

   /// The relations of the join query
   std::vector<Relation> relations;
   /// The projection of the join query
   std::vector<BindingAttribute> projection;
   /// The selections (filters) of the query
   std::vector<std::pair<BindingAttribute, Constant>> selections;
   /// The (equi-)join conditions
   std::vector<std::pair<BindingAttribute, BindingAttribute>> joinConditions;

   /// Parse and analyze a sql join query
   static JoinQuery parseAndAnalyseJoinQuery(Database& db, std::string_view sql);

   /// Build a (right-deep) canonical operator tree from the join query with pushed down predicates
   OperatorTree buildCanonicalTree(Database& db) const;

private:
   void applySelections(Tablescan *tablescan, std::unique_ptr<Operator> &tree, const std::list<std::pair<BindingAttribute, Constant>> &selections, std::vector<std::unique_ptr<Register>> &constants) const;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
