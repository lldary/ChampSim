#ifndef BRANCH_MY_PERCEPTRON_H
#define BRANCH_MY_PERCEPTRON_H

#include <array>
#include <cstdint>
#include <tuple>
#include <vector>


#include "modules.h"
#include "msl/bits.h"
#include "msl/fwcounter.h"

class my_perceptron : champsim::modules::branch_predictor
{
  
public:
  using branch_predictor::branch_predictor;
  bool predict_branch(champsim::address pc);
  void last_branch_result(champsim::address pc, champsim::address branch_target, bool taken, uint8_t branch_type);
};

#endif
