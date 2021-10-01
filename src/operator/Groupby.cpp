#include "Config.hpp"
#include "operator/Chi.hpp"
#include "operator/Groupby.hpp"
#include <cassert>
#include <unordered_set>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// Internal logic for aggregations
struct Groupby::AggregationLogic {
   /// Destructor
   virtual ~AggregationLogic() = default;

   /// Initialize logic
   virtual void open() = 0;
   /// Add a value to the aggregate
   virtual void add(const Register* reg) = 0;
   /// Finalize logic
   virtual void close(Register& result) = 0;
};
//---------------------------------------------------------------------------
namespace {
//---------------------------------------------------------------------------
struct CountStarLogic : Groupby::AggregationLogic {
   /// The count
   int64_t count = 0;

   /// Open
   void open() override { count = 0; }
   /// Add
   void add(const Register* /*reg*/) override { ++count; }
   /// Close
   void close(Register& result) override { result.setInt(count); }
};
//---------------------------------------------------------------------------
struct CountLogic : Groupby::AggregationLogic {
   /// The count
   int64_t count = 0;

   /// Open
   void open() override { count = 0; }
   /// Add
   void add(const Register* reg) override { if (!reg->isUnbound()) ++count; }
   /// Close
   void close(Register& result) override { result.setInt(count); }
};
//---------------------------------------------------------------------------
struct CountDistinctLogic : Groupby::AggregationLogic {
   /// The set
   unordered_set<Register, Register::Hasher> values;

   /// Open
   void open() override { values.clear(); }
   /// Add
   void add(const Register* reg) override { if (!reg->isUnbound()) values.insert(*reg); }
   /// Close
   void close(Register& result) override { result.setInt(values.size()); }
};
//---------------------------------------------------------------------------
template <typename Func>
struct BasicLogic : Groupby::AggregationLogic {
   /// The callback function
   Func func;
   /// The set
   Register value;

   /// Open
   void open() override { value.setUnbound(); }
   /// Add
   void add(const Register* reg) override {
      assert(reg);
      if (value.isUnbound())
         value = *reg;
      else if (!reg->isUnbound())
         Chi::add(value, *reg, value);
   }
   /// Close
   void close(Register& result) override { result = value; }
};
//---------------------------------------------------------------------------
using SumLogic = BasicLogic<decltype([](Register& result, const Register& v){ Chi::add(result, v, result); })>;
using MinLogic = BasicLogic<decltype([](Register& result, const Register& v){ result = min(result, v); })>;
using MaxLogic = BasicLogic<decltype([](Register& result, const Register& v){ result = max(result, v); })>;
//---------------------------------------------------------------------------
struct AvgLogic : Groupby::AggregationLogic {
   /// The sum
   SumLogic sumLogic;
   /// The count
   CountLogic countLogic;

   /// Open
   void open() override {
      sumLogic.open();
      countLogic.open();
   }
   /// Add
   void add(const Register* reg) override {
      sumLogic.add(reg);
      countLogic.add(reg);
   }
   /// Close
   void close(Register& result) override {
      Register sumResult, countResult;
      sumLogic.close(sumResult);
      countLogic.close(countResult);
      Chi::cast(countResult, sumResult.getState(), countResult);
      Chi::div(sumResult, countResult, result);
   }
};
//---------------------------------------------------------------------------
unique_ptr<Groupby::AggregationLogic> makeLogic(Groupby::AggregationType type)
// Make corresponding aggregation logic
{
   switch (type) {
      case Groupby::AggregationType::CountStar: return make_unique<CountStarLogic>();
      case Groupby::AggregationType::Count: return make_unique<CountLogic>();
      case Groupby::AggregationType::CountDistinct: return make_unique<CountDistinctLogic>();
      case Groupby::AggregationType::Sum: return make_unique<SumLogic>();
      case Groupby::AggregationType::Min: return make_unique<MinLogic>();
      case Groupby::AggregationType::Max: return make_unique<MaxLogic>();
      case Groupby::AggregationType::Avg: return make_unique<AvgLogic>();
   }
   unreachable();
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
Groupby::Groupby(std::unique_ptr<Operator>&& input)
// Constructor
   : input(move(input))
{
}
//---------------------------------------------------------------------------
/// Destructor
Groupby::~Groupby() noexcept = default;
//---------------------------------------------------------------------------
const Register* Groupby::addAggregation(Groupby::Aggregation aggregation)
/// Add an aggregation
{
   aggregations.push_back(aggregation);
   logic.push_back(makeLogic(aggregation.type));
   output.push_back(make_unique<Register>());
   return output.back().get();
}
//---------------------------------------------------------------------------
void Groupby::open()
// Open the operator
{
   input->open();
   for (auto& l : logic)
      l->open();
   produced = false;
}
//---------------------------------------------------------------------------
bool Groupby::next()
// Get the next tuple
{
   if (produced)
      return false;

   while (input->next()) {
      for (size_t i = 0; i < aggregations.size(); i++)
         logic[i]->add(aggregations[i].input);
   }

   for (size_t i = 0; i < aggregations.size(); i++)
      logic[i]->close(*output[i]);

   produced = true;
   return true;
}
//---------------------------------------------------------------------------
void Groupby::close()
// Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
std::vector<const Register*> Groupby::getOutput() const
// Get all produced values
{
   auto res = input->getOutput();
   res.reserve(res.size() + output.size());
   for (auto& r : output)
      res.push_back(r.get());
   return res;
}
//---------------------------------------------------------------------------
void Groupby::printTree(ostream& out) const
// Print operator tree
{
   out << "GROUP BY (";
   input->printTree(out);
   out << ")";
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
