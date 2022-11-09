#include "operator/Operator.hpp"
#include <ostream>
//---------------------------------------------------------------------------
namespace tinydb {
//---------------------------------------------------------------------------
/// Constructor
Operator::Operator() = default;
//---------------------------------------------------------------------------
/// Destructor
Operator::~Operator() noexcept = default;
//---------------------------------------------------------------------------
bool Operator::containsRegister(const Register *r) const {
    std::vector<const Register*> output = getOutput();
    for(const Register *out: output){
        if(out == r) return true;
    }
    return false;
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
