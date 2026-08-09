#ifndef BRANCH_LOCAL_BP_H
#define BRANCH_LOCAL_BP_H

#include "modules.h"
#include "msl/fwcounter.h"
// #include "msl/lru_table.h"
#include <array>
#include <cstdint>

struct local_bp : champsim::modules::branch_predictor {
  static constexpr std::size_t LOCAL_HISTORY = 10; // 局部历史长度
  static constexpr std::size_t LOCAL_BITS = 8;     // 饱和计数器位数
  static constexpr std::size_t NUM_LOCAL_BP = 1024;

  using branch_predictor::branch_predictor;

  // 初始化钩子（可选但推荐）
  void initialize_branch_predictor();

  // Local History Table：记录每个分支的局部历史（10 位）
  std::array<uint16_t, NUM_LOCAL_BP> local_history_table{};

  // Pattern History Table：用局部历史索引的 8 位饱和计数器
  std::array<champsim::msl::fwcounter<8>, (1 << LOCAL_HISTORY)> pattern_history_table{};

  bool predict_branch(champsim::address ip);
  void last_branch_result(champsim::address ip, champsim::address branch_target, bool taken, uint8_t branch_type);
};

#endif