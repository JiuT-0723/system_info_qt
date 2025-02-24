/**
 * @file sys_info.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-02-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#include <stdint.h>

#include <string>
namespace jiut {

struct cpu_occupy_t {
  uint64_t user;
  uint64_t nice;
  uint64_t system;
  uint64_t idle;
};

struct npu_info_t {
  int core0;
  int core1;
  int core2;

  uint64_t freq;
};

struct gpu_occupy_t {
  int usage;
  uint64_t freq;
};

struct mem_occupy_t {
  float total;
  float used;
  float usage;
  // int buffers;
  // int cached;
};

struct cpu_info_t {
  float usage;
  uint64_t freq[3];
};

class classSysInfo {
 public:
  classSysInfo();
  ~classSysInfo();

  auto getCpuInfo(cpu_info_t &cpu_info) -> int;

  /**
   * @brief 核心温度
   *
   */

  auto getCoreTemp() -> float;

  /**
   * @brief npu占用率
   *
   */
  auto getNpuInfo(npu_info_t &npu_occupy) -> int;

  /**
   * @brief gpu 占用率
   *
   */

  auto getGpuUsage(gpu_occupy_t &gpu_occupy) -> int;

  /**
   * @brief 内存使用情况
   *
   */

  auto getMemUsage(mem_occupy_t &mem_occupy) -> int;

  auto getDDRFreq() -> uint64_t;

 private:
  /**
   * @brief cpu 当前使用情况
   *
   */
  auto calcCpuTotalOccupy(cpu_occupy_t &cpu_occupy) -> int;

  auto run() -> void;

 private:
  float cpu_usage_ = 0.0;
};
}  // namespace jiut