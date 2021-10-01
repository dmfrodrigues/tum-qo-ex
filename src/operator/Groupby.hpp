#ifndef H_operator_Groupby
#define H_operator_Groupby
//---------------------------------------------------------------------------
#include "Register.hpp"
#include "operator/Operator.hpp"
#include <memory>
#include <unordered_map>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// A group-by
class Groupby : public Operator {
   public:
   /// Aggregation types
   enum class AggregationType {
      CountStar,
      Count,
      CountDistinct,
      Sum,
      Min,
      Max,
      Avg
   };

   /// An aggregation
   struct Aggregation {
      /// The type of the aggregate
      AggregationType type = AggregationType::CountStar;
      /// The input to the aggregate
      const Register* input = nullptr;
   };

   /// Internal logic for aggregations
   struct AggregationLogic;

   /// The input
   std::unique_ptr<Operator> input;

   private:
   /// The aggregations
   std::vector<Aggregation> aggregations;
   /// The output
   std::vector<std::unique_ptr<Register>> output;
   /// The logic for the aggregations
   std::vector<std::unique_ptr<AggregationLogic>> logic;
   /// Did we produce output?
   bool produced = false;

   public:
   /// Constructor
   explicit Groupby(std::unique_ptr<Operator>&& input);
   /// Destructor
   ~Groupby() noexcept override;

   /// Add an aggregation
   const Register* addAggregation(Aggregation aggregation);

   /// Open the operator
   void open() override;
   /// Get the next tuple
   bool next() override;
   /// Close the operator
   void close() override;

   /// Get all produced values
   [[nodiscard]] std::vector<const Register*> getOutput() const override;

   /// Print operator tree
   void printTree(std::ostream& out) const override;
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
