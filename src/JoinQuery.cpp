#include "JoinQuery.hpp"
#include "Config.hpp"
#include "Database.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/Operator.hpp"
#include "operator/Printer.hpp"
#include "operator/Projection.hpp"
#include "operator/Selection.hpp"
#include "operator/Tablescan.hpp"
#include "operator/HashJoin.hpp"
#include <list>
#include <set>
#include <unordered_map>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
OperatorTree::OperatorTree(unique_ptr<Operator> root, vector<unique_ptr<Register>> constants) noexcept
   : root(move(root)), constants(move(constants))
// Constructor
{
}
//---------------------------------------------------------------------------
/// Move constructor
OperatorTree::OperatorTree(OperatorTree&&) noexcept = default;
//---------------------------------------------------------------------------
/// Destructor
OperatorTree::~OperatorTree() noexcept = default;
//---------------------------------------------------------------------------
/// Move assignment
OperatorTree& OperatorTree::operator=(OperatorTree&&) noexcept = default;
//---------------------------------------------------------------------------
/// Sql parser
struct SqlParser {
   /// The sql
   std::string_view sql;
   /// The position
   size_t pos = 0;

   /// Peek character
   char peekChar() {
      if (pos >= sql.size())
         return '\0';
      return sql[pos];
   }

   /// Read character
   char readChar() {
      char res = peekChar();
      if (pos < sql.size())
         pos++;
      return res;
   }

   /// Skip whitespace
   void skipWS() {
      while (isspace(peekChar()))
         pos++;
   }

   /// Read quoted
   std::string_view readWord() {
      size_t startPos = pos;
      auto isValid = [](char c) { return isalnum(c) || (c == '_'); };
      while (isValid(peekChar()))
         pos++;
      return sql.substr(startPos, pos - startPos);
   }

   /// Read quoted
   std::string_view readNumber() {
      auto w = readWord();
      for (char c : w)
         if (!isdigit(c))
            throw IncorrectSqlError{"expected number, found non-digit characters"};
      return w;
   }

   /// Read punctuation
   std::string_view readPunc() {
      auto rv = sql.substr(pos, 1);
      pos++;
      return rv;
   }

   /// Read quoted
   std::string_view readQuoted(char quote) {
      pos++;
      auto startPos = pos;
      while (readChar() != quote);
      return sql.substr(startPos, pos - 1 - startPos);
   }

   /// A token from the lexer
   struct Token {
      enum Type {
         Keyword,
         Punc,
         Text,
         Number,
         End
      };
      /// The token's type
      Type type;
      /// The token's content
      std::string_view content;
   };

   /// Read the next token
   Token readToken() {
      skipWS();
      char c = peekChar();
      if (c == '\0')
         return Token{Token::End, {}};
      if (c == ';')
         return Token{Token::End, {}};
      if (c == '\'' || c == '"')
         return Token{c == '\'' ? Token::Text : Token::Keyword, readQuoted(c)};
      if (isdigit(c))
         return Token{Token::Number, readNumber()};
      if (ispunct(c))
         return Token{Token::Punc, readPunc()};
      return Token{Token::Keyword, readWord()};
   }

   /// Peek the next token
   Token peekToken() {
      auto oldPos = pos;
      auto rv = readToken();
      pos = oldPos;
      return rv;
   }

   /// Case insensitive equality check
   static bool insensitiveEqual(string_view a, string_view b) {
      if (a.size() != b.size())
         return false;
      for (size_t i = 0; i < a.size(); i++)
         if (toupper(a[i]) != toupper(b[i]))
            return false;
      return true;
   }

   /// Read a binding and attribute pair
   JoinQuery::BindingAttribute readBindingAttribute() {
      auto binding = readToken();
      if (binding.type != Token::Keyword)
         throw IncorrectSqlError{"expected binding"};

      {
         auto dot = peekToken();
         if (dot.type != Token::Punc || dot.content != ".")
            return {{}, string{binding.content}};
      }
      // Read dot
      readToken();

      auto attribute = readToken();
      if (attribute.type != Token::Keyword)
         throw IncorrectSqlError{"expected attribute"};

      return {string{binding.content}, string{attribute.content}};
   }

   /// Read sql statement
   void readSql(JoinQuery& jq) {
      {
         auto select = readToken();
         if (select.type != Token::Keyword || !insensitiveEqual(select.content, "select"))
            throw IncorrectSqlError{"expected select"};
      }

      bool foundStar = false;
      // Select
      do {
         auto star = peekToken();
         if (star.type == Token::Punc && star.content == "*") {
            readToken();
            foundStar = true;
            jq.projection.clear();
         } else {
            auto ba = readBindingAttribute();
            if (!foundStar)
               jq.projection.push_back(move(ba));
         }

         auto comma = readToken();
         if (comma.type == Token::End)
            return;
         if (comma.type != Token::Punc || comma.content != ",") {
            if (comma.type != Token::Keyword || !insensitiveEqual(comma.content, "from"))
               throw IncorrectSqlError{"expected from"};
            break;
         }
      } while (true);

      // From
      do {
         auto table = readToken();
         if (table.type != Token::Keyword)
            throw IncorrectSqlError{"expected table"};
         auto binding = readToken();
         if (binding.type != Token::Keyword)
            throw IncorrectSqlError{"expected binding"};

         jq.relations.push_back({string{table.content}, string{binding.content}});

         auto comma = readToken();
         if (comma.type == Token::End)
            return;
         if (comma.type != Token::Punc || comma.content != ",") {
            if (comma.type != Token::Keyword || !insensitiveEqual(comma.content, "where"))
               throw IncorrectSqlError{"expected where"};
            break;
         }
      } while (true);

      // Where
      do {
         auto ba = readBindingAttribute();

         auto eq = readToken();
         if (eq.type != Token::Punc || eq.content != "=")
            throw IncorrectSqlError{"expected equality"};

         auto binding2 = peekToken();
         if (binding2.type == Token::Keyword) {
            jq.joinConditions.push_back({move(ba), readBindingAttribute()});
         } else if (binding2.type == Token::Number) {
            readToken();
            jq.selections.push_back({move(ba), Register::makeInt(stoi(string{binding2.content}))});
         } else if (binding2.type == Token::Text) {
            readToken();
            jq.selections.push_back({move(ba), Register::makeString(string{binding2.content})});
         } else {
            throw IncorrectSqlError{"expected constant"};
         }

         auto comma = readToken();
         if (comma.type == Token::End)
            return;
         if (comma.type != Token::Keyword || !insensitiveEqual(comma.content, "and"))
            throw IncorrectSqlError{"expected end"};
      } while (true);
   }

   void readSql2(JoinQuery& jq) const {
      if (sql.empty())
         throw IncorrectSqlError{"empty query"};
      auto split = [&](string_view str, string_view sep, bool ignoreEmpty = false) {
         auto trim = [&](string_view str) {
            while (!str.empty() && isspace(str.front()))
               str = str.substr(1);
            while (!str.empty() && isspace(str.back()))
               str = str.substr(0, str.length() - 1);
            return string{str};
         };
         vector<string> res;
         while (!str.empty()) {
            auto pos = str.find(sep);
            pos = min(pos, str.length());
            auto trimmed = trim(str.substr(0, pos));
            if (!trimmed.empty() || !ignoreEmpty)
               res.push_back(move(trimmed));
            str = str.substr(min(pos + sep.size(), str.length()));
         }
         return res;
      };

      auto selp = sql.find("select");
      auto frop = sql.find("from");
      auto whep = sql.find("where");
      auto sel = sql.substr(selp + "select"sv.length(), frop - selp - "select"sv.length());
      auto fro = sql.substr(frop + "from"sv.length(), whep - frop- "from"sv.length());
      auto whe = sql.substr(whep + "where"sv.length(), sql.length() - whep - "where"sv.length());

      auto proj = split(sel, ",");
      auto rels = split(fro, ",");
      auto preds = split(whe, "and");

      auto getBA = [&](string_view str) {
         auto vs = split(str, ".");
         switch (vs.size()) {
            case 1: return JoinQuery::BindingAttribute{"", vs[0]};
            case 2: return JoinQuery::BindingAttribute{vs[0], vs[1]};
            default: throw IncorrectSqlError{"ba parse error"};
         }
      };

      for (auto& p : proj) {
         if (p == "*") {
            jq.projection.clear();
            break;
         }
         jq.projection.push_back(getBA(p));
      }
      for (auto& r : rels) {
         auto vs = split(r, " ", true);
         if (vs.size() != 2)
            throw IncorrectSqlError{"rel parse error"};
         jq.relations.push_back(JoinQuery::Relation{vs[0], vs[1]});
      }
      for (auto& p : preds) {
         auto vs = split(p, "=");
         if (vs.size() != 2)
            throw IncorrectSqlError{"rel parse error"};
         if (isalpha(vs[1][0]))
            jq.joinConditions.emplace_back(getBA(vs[0]), getBA(vs[1]));
         else if (vs[1][0] == '\'')
            jq.selections.emplace_back(getBA(vs[0]), Register::makeString(string{vs[1].substr(1, vs[1].length() - 2)}));
         else
            jq.selections.emplace_back(getBA(vs[0]), Register::makeInt(stoi(vs[1])));
      }
   }

   /* Regex example (not verified)
   string word = "[a-zA-Z_][a-zA-Z0-9_]+";
   string ba = word + "|" + word + "\\." + word;
   string proj = "(\\*|" + ba + ")";
   string rel = "(" + word + "\\s+" + word + ")";
   string cst = "\\d+|'.*'";
   string cond = "(" + ba + ")\\s*=\\s*(" + ba + "|" + cst + ")";

   string select = "select\\s+" + proj + "(?:\\s*," + proj + ")*";
   string from = "from\\s+" + rel + "(?:\\s*,\\s*" + rel + ")*";
   string where = "where\\s+" + cond + "(?:\\s+and\\s+" + cond + ")*";

   string query = select + "\\s+" + from + "\\s+" + where + "\\s?;?$";

   // select\s+(\*|[a-zA-Z_]+|[a-zA-Z_]+\.[a-zA-Z_]+)\s*(?:,\s*(\*|[a-zA-Z_]+|[a-zA-Z_]+\.[a-zA-Z_]+))*\sfrom\s+([a-zA-Z_]+\s+[a-zA-Z_]+)\s*(?:,\s*([a-zA-Z_]+\s+[a-zA-Z_]+))*\swhere\s+(([a-zA-Z_]+|[a-zA-Z_]+\.[a-zA-Z_]+)\s*=\s*([a-zA-Z_]+|[a-zA-Z_]+\.[a-zA-Z_]+|\d+|'.*'))\s*(?:\s*and\s*([a-zA-Z_]+|[a-zA-Z_]+\.[a-zA-Z_]+)\s*=\s*([a-zA-Z_]+|[a-zA-Z_]+\.[\w_]+|\d+|'[^']*'))*\s*;?\s*$
   */
};
//---------------------------------------------------------------------------
JoinQuery JoinQuery::parseAndAnalyseJoinQuery(Database& db, std::string_view sql)
// Parse and analyze a sql join query
{
    cout << sql << endl;
   JoinQuery jq;
   SqlParser parser{sql};
   parser.readSql2(jq);

   unordered_map<string, Table*> bindingTables;
   unordered_map<string, string> attributeBindings;
   for (auto& tbl : jq.relations) {
      try {
         auto& dbTbl = db.getTable(tbl.table);
         bindingTables[tbl.binding] = &dbTbl;

         for (unsigned i = 0; i < dbTbl.getAttributeCount(); i++) {
            const auto& attrName = dbTbl.getAttribute(i).getName();
            auto it = attributeBindings.find(attrName);
            if (it == attributeBindings.end()) {
               attributeBindings.emplace_hint(it, attrName, tbl.binding);
            } else {
               // We mark the binding as invalid as there are multiple possible bindings
               it->second = "";
            }
         }
      } catch (const runtime_error&) {
         throw IncorrectSqlError{"table not found"};
      }
   }

   auto fixBA = [&](JoinQuery::BindingAttribute& ba) {
      if (!ba.binding.empty()) {
         auto it = bindingTables.find(ba.binding);
         if (it == bindingTables.end())
            throw IncorrectSqlError{"binding not found"};
         if (it->second->findAttribute(ba.attribute) < 0)
            throw IncorrectSqlError{"attribute not found"};
         return;
      }

      auto it = attributeBindings.find(ba.attribute);
      if (it == attributeBindings.end())
         throw IncorrectSqlError{"attribute not found"};
      if (it->second.empty())
         throw IncorrectSqlError{"attribute's source is ambiguous"};
      ba.binding = it->second;
   };

   for (auto& ba : jq.projection)
      fixBA(ba);

   for (auto& ba : jq.selections)
      fixBA(ba.first);

   for (auto& ba : jq.joinConditions) {
      fixBA(ba.first);
      fixBA(ba.second);
   }

   return jq;
}
//---------------------------------------------------------------------------
void JoinQuery::applySelections(Tablescan *tablescan, unique_ptr<Operator> &tree, const list<pair<BindingAttribute, Constant>> &selections, vector<unique_ptr<Register>> &constants) const {
   for(const auto &p: selections){
      const Register* lhs = tablescan->getOutput(p.first.attribute);
      constants.emplace_back(make_unique<Register>());
      Register* rhs = constants.back().get();

      switch(p.second.getState()) {
         case Register::State::Bool: rhs->setBool(p.second.getBool()); break;
         case Register::State::Int: rhs->setInt(p.second.getInt()); break;
         case Register::State::Double: rhs->setDouble(p.second.getDouble()); break;
         case Register::State::String: rhs->setString(p.second.getString()); break;
         default: throw logic_error("State not allowed");
      }

      tree = make_unique<Selection>(move(tree), lhs, rhs);
   }
}

OperatorTree JoinQuery::buildCanonicalTree(Database& db) const
// Build a (right-deep) canonical operator tree from the join query with pushed down predicates
{
   if(relations.empty()){
      return OperatorTree(nullptr, vector<unique_ptr<Register>>());
   }

   vector<std::unique_ptr<Register>> constants;

   map<std::string, Tablescan*> tables;

   // Pre-process selections
   map<string, list<pair<BindingAttribute, Constant>>> selectionsMap;
   for(auto it = selections.rbegin(); it != selections.rend(); ++it) {
      selectionsMap[it->first.binding].push_back(*it);
   }

   // Relations
   Table& table = db.getTable(relations.back().table);
   Tablescan* rightChildPtr = new Tablescan(table);

   unique_ptr<Operator> root(rightChildPtr);

   tables[relations.back().binding] = rightChildPtr;

   // Apply selections
   applySelections(rightChildPtr, root, selectionsMap[relations.back().binding], constants);

   auto joinConditionsCopy = joinConditions;

   // Relations (and apply selections)
   for(auto it = ++relations.rbegin(); it != relations.rend(); ++it){
      Table& table = db.getTable(it -> table);
      Tablescan* leftChildPtr = new Tablescan(table);

      unique_ptr<Operator> leftChild(leftChildPtr);

      tables[it -> binding] = leftChildPtr;

      applySelections(leftChildPtr, leftChild, selectionsMap[it->binding], constants);

      // Joins
      auto out = root->getOutput();
      set<const Register*> outSet(out.begin(), out.end());
      bool isJoin = false;

      for(auto it = joinConditionsCopy.begin(); it != joinConditionsCopy.end(); ++it){
            if(tables.count(it -> first.binding) && tables.count(it -> second.binding)){

               const Register* lhs = tables.at(it -> first.binding)->getOutput(it -> first.attribute);
               const Register* rhs = tables.at(it -> second.binding)->getOutput(it -> second.attribute);

               if(leftChild->containsRegister(rhs)){
                  swap(lhs, rhs);
               }
               root = make_unique<HashJoin>(move(leftChild), move(root), lhs, rhs);

               joinConditionsCopy.erase(it);
               isJoin = true;
               break;
            }
      }

      if(!isJoin){
         root = make_unique<CrossProduct>(move(leftChild), move(root));
      }

      for(auto it = joinConditionsCopy.begin(); it != joinConditionsCopy.end(); ){
            if(tables.count(it -> first.binding) && tables.count(it -> second.binding)){
               const Register* lhs = tables.at(it -> first.binding)->getOutput(it -> first.attribute);
               const Register* rhs = tables.at(it -> second.binding)->getOutput(it -> second.attribute);
               root = make_unique<Selection>(move(root), lhs, rhs);

               it = joinConditionsCopy.erase(it);
            } else {
               ++it;
            }
      }

   }




   /*
   // Joins
   for(auto it = joinConditions.rbegin(); it != joinConditions.rend(); ++it) {
      const Register* lhs = tables.at(it -> first.binding)->getOutput(it -> first.attribute);
      const Register* rhs = tables.at(it -> second.binding)->getOutput(it -> second.attribute);
      root = make_unique<Selection>(move(root), lhs, rhs);
   }
   */
   // Projection
   vector<const Register*> projectRegisters;
   if(!projection.empty()){
      projectRegisters.resize(projection.size());
      for(int i = 0; i < projection.size(); ++i){
         projectRegisters.at(i) = tables.at(projection.at(i).binding)->getOutput(projection.at(i).attribute);
      }
   } else {
      projectRegisters = root->getOutput();
   }

   root = make_unique<Projection>(move(root), projectRegisters);

   //root ->printTree(cout);
   /*
   Printer out(move(root), cout);
   out.open();
   while (out.next());
   out.close();
   */
   OperatorTree tree (move(root), move(constants));
   // Push down Join Relations

   return tree;






}
}
//---------------------------------------------------------------------------
