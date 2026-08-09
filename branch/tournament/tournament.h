#ifndef BRANCH_TOURNAMENT_H
#define BRANCH_TOURNAMENT_H

#include "modules.h"
#include "msl/fwcounter.h"
#include <array>
#include <cstdint>

struct tournament : champsim::modules::branch_predictor {
  // ---- 局部预测器参数 ----
  static constexpr std::size_t LOCAL_HISTORY_LENGTH = 10;
  static constexpr std::size_t NUM_LOCAL_BP         = 1024;  // LHT 项数

  // ---- 全局预测器参数 ----
  static constexpr std::size_t GLOBAL_HISTORY_LENGTH = 12;
  static constexpr std::size_t NUM_GLOBAL_BP         = 4096; // GPT 项数 (2^12)

  // ---- 选择器参数 ----
  static constexpr std::size_t NUM_CHOOSER = 1024;

  using branch_predictor::branch_predictor;

  void initialize_branch_predictor();

  // ========== 局部预测器 ==========
  // LHT：每个分支的局部历史（10 位移位寄存器）
  std::array<uint16_t, NUM_LOCAL_BP> local_history_table{};
  // LPT：用局部历史索引的 2 位饱和计数器
  std::array<champsim::msl::fwcounter<2>, (1 << LOCAL_HISTORY_LENGTH)> local_pred_table{};

  // ========== 全局预测器 ==========
  // GHR：全局历史寄存器
  uint16_t global_history_reg = 0;
  // GPT：用 (GHR ^ PC) 索引的 2 位饱和计数器
  std::array<champsim::msl::fwcounter<2>, NUM_GLOBAL_BP> global_pred_table{};

  // ========== 选择器（元预测器）==========
  // 2 位饱和计数器：>=2 偏向局部预测器，<2 偏向全局预测器
  std::array<champsim::msl::fwcounter<2>, NUM_CHOOSER> chooser_table{};

  bool predict_branch(champsim::address ip);
  void last_branch_result(champsim::address ip, champsim::address branch_target,
                          bool taken, uint8_t branch_type);
};

#endif