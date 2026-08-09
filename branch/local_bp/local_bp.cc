#include "local_bp.h"
#include "instruction.h"

void local_bp::initialize_branch_predictor()
{
  // 将 PHT 初始化为中点（128），即“弱 not-taken”
  for (auto& cnt : pattern_history_table) {
    cnt = (1u << (LOCAL_BITS - 1));  // 128
  }
}

bool local_bp::predict_branch(champsim::address ip)
{
  // 用 PC 低 10 位索引 LHT
  auto lht_idx = (ip.to<uint64_t>()) % NUM_LOCAL_BP;
  auto history = local_history_table[lht_idx];

  // 用局部历史索引 PHT，读取 8 位饱和计数器
  const auto& counter = pattern_history_table[history];

  // >= 128 预测 taken，否则 not-taken
  return counter >= (1u << (LOCAL_BITS - 1));
}

void local_bp::last_branch_result(champsim::address ip, champsim::address branch_target,
                                  bool taken, uint8_t branch_type)
{
  // 只对条件分支进行训练（无条件分支由 BTB 保证方向）
  if (branch_type != BRANCH_CONDITIONAL) {
    return;
  }

  // 索引 LHT 获取当前局部历史
  auto lht_idx = (ip.to<uint64_t>()) % NUM_LOCAL_BP;
  auto history = local_history_table[lht_idx];

  // 更新对应的饱和计数器
  auto& counter = pattern_history_table[history];
  if (taken) {
    ++counter;   // fwcounter 自动饱和在 255
  } else {
    --counter;   // fwcounter 自动饱和在 0
  }

  // 更新 LHT：左移，最低位写入本次结果，保留 10 位
  local_history_table[lht_idx] = ((history << 1) | (taken ? 1u : 0u))
                                  & ((1u << LOCAL_HISTORY) - 1);
}