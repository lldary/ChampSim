#include "tournament.h"

void tournament::initialize_branch_predictor()
{
  // 局部预测表：初始化为弱 not-taken (1)
  for (auto& cnt : local_pred_table) {
    cnt = 1;
  }
  // 全局预测表：初始化为弱 not-taken (1)
  for (auto& cnt : global_pred_table) {
    cnt = 1;
  }
  // 选择器：初始化为弱偏向局部 (2)
  for (auto& cnt : chooser_table) {
    cnt = 2;
  }
}

bool tournament::predict_branch(champsim::address ip)
{
  // ---- 局部预测 ----
  auto lht_idx = ip.to<uint64_t>() % NUM_LOCAL_BP;
  auto local_hist = local_history_table[lht_idx];
  bool local_pred = local_pred_table[local_hist] >= 2;  // >=2 预测 taken

  // ---- 全局预测 ----
  auto gpt_idx = (global_history_reg ^ static_cast<uint16_t>(ip.to<uint64_t>()))
                 & ((1 << GLOBAL_HISTORY_LENGTH) - 1);
  bool global_pred = global_pred_table[gpt_idx] >= 2;   // >=2 预测 taken

  // ---- 选择器仲裁 ----
  auto chooser_idx = ip.to<uint64_t>() % NUM_CHOOSER;
  bool use_local = chooser_table[chooser_idx] >= 2;     // >=2 选局部

  return use_local ? local_pred : global_pred;
}

void tournament::last_branch_result(champsim::address ip, champsim::address branch_target,
                                    bool taken, uint8_t branch_type)
{
  // ---- 局部预测器训练 ----
  auto lht_idx = ip.to<uint64_t>() % NUM_LOCAL_BP;
  auto local_hist = local_history_table[lht_idx];

  auto& local_cnt = local_pred_table[local_hist];
  if (taken) {
    ++local_cnt;
  } else {
    --local_cnt;
  }

  // 更新 LHT
  local_history_table[lht_idx] = ((local_hist << 1) | (taken ? 1u : 0u))
                                  & ((1u << LOCAL_HISTORY_LENGTH) - 1);

  // ---- 全局预测器训练 ----
  auto gpt_idx = (global_history_reg ^ static_cast<uint16_t>(ip.to<uint64_t>()))
                 & ((1 << GLOBAL_HISTORY_LENGTH) - 1);

  auto& global_cnt = global_pred_table[gpt_idx];
  if (taken) {
    ++global_cnt;
  } else {
    --global_cnt;
  }

  // 更新 GHR
  global_history_reg = ((global_history_reg << 1) | (taken ? 1u : 0u))
                        & ((1u << GLOBAL_HISTORY_LENGTH) - 1);

  // ---- 选择器训练（仅在两个预测器结果不同时更新）----
  bool local_pred = local_pred_table[local_hist] >= 2;
  bool global_pred = global_pred_table[gpt_idx] >= 2;

  if (local_pred != global_pred) {
    auto chooser_idx = ip.to<uint64_t>() % NUM_CHOOSER;
    auto& chooser_cnt = chooser_table[chooser_idx];

    if (local_pred == taken) {
      // 局部正确，全局错误 → 偏向局部
      ++chooser_cnt;
    } else {
      // 全局正确，局部错误 → 偏向全局
      --chooser_cnt;
    }
  }
}