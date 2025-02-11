#include "sys_info.h"

#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>

using namespace jiut;
classSysInfo::classSysInfo() { run(); }
classSysInfo::~classSysInfo() {}

auto classSysInfo::getCpuUsage() -> float { return cpu_usage_; }

auto classSysInfo::calcCpuTotalOccupy(cpu_occupy_t &cpu_occupy) -> int {
  FILE *fd;
  char buff[1024] = {0};

  fd = fopen("/proc/stat", "r");
  if (nullptr == fd) {
    return 0;
  }
  fgets(buff, sizeof(buff), fd);
  fclose(fd);
  char name[10] = {0};
  sscanf(buff, "%s %ld %ld %ld %ld", name, &cpu_occupy.user, &cpu_occupy.nice, &cpu_occupy.system, &cpu_occupy.idle);
  return 0;
}

auto classSysInfo::getCoreTemp() -> float {
  int temperature_all = 0;
  for (int i = 0; i < 7; i++) {
    std::string filePath = "/sys/class/thermal/thermal_zone" + std::to_string(i) + "/temp";
    std::ifstream file(filePath);
    if (!file.is_open()) {
      return -1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    // 将 stringstream 的内容转换为 std::string
    std::string fileContent = buffer.str();
    temperature_all += atoi(fileContent.c_str());
    // 关闭文件
  }
  return temperature_all / 7000.0;
}

auto classSysInfo::getNpuUsage(npu_occupy_t &npu_occupy) -> int {
  char buff[128] = {0};
  FILE *fd;

  fd = fopen("/sys/kernel/debug/rknpu/load", "r");
  if (fd == nullptr) {
    return -1;
  }
  fgets(buff, sizeof(buff), fd);
  fclose(fd);
  std::string str(buff);
  // 去除str内的所有%
  str.erase(std::remove(str.begin(), str.end(), '%'), str.end());
  sscanf(str.c_str(), "NPU load:  Core0:  %d, Core1:  %d, Core2:  %d,", &npu_occupy.core0, &npu_occupy.core1, &npu_occupy.core2);
  return 0;
}

auto classSysInfo::getGpuUsage(gpu_occupy_t &gpu_occupy) -> int {
  std::string filePath = "/sys/class/devfreq/fb000000.gpu/load";
  std::ifstream file(filePath);
  if (!file.is_open()) {
    return -1;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  // 将 stringstream 的内容转换为 std::string
  std::string fileContent = buffer.str();

  size_t pos = fileContent.find("@");
  gpu_occupy.usage = atoi(fileContent.substr(0, pos).c_str());
  gpu_occupy.freq = fileContent.substr(pos + 1, fileContent.find("Hz") - pos - 1);

  return 0;
}

auto classSysInfo::getMemUsage(mem_occupy_t &mem_occupy) -> int {
  std::string filePath = "/proc/meminfo";
  std::ifstream file(filePath);
  if (!file.is_open()) {
    return -1;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  uint64_t total_mem = 0;
  uint64_t free_mem = 0;
  uint64_t available_mem = 0;
  // 将 stringstream 的内容转换为 std::string
  std::string fileContent = buffer.str();
  sscanf(fileContent.c_str(), "MemTotal: %ld kB MemFree: %ld kB MemAvailable: %ld kB", &total_mem, &free_mem, &available_mem);
  mem_occupy.total = total_mem / 1024.0 / 1024.0;
  mem_occupy.used = (total_mem - available_mem) / 1024.0 / 1024.0;
  mem_occupy.usage = float(total_mem - available_mem) / (float)total_mem * 100.0;
  return 0;
}

auto classSysInfo::run() -> void {
  std::thread([this] {
    while (true) {
      cpu_occupy_t cpu_occupy_1;
      calcCpuTotalOccupy(cpu_occupy_1);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      cpu_occupy_t cpu_occupy_2;
      calcCpuTotalOccupy(cpu_occupy_2);
      cpu_usage_ =
          float(cpu_occupy_2.user + cpu_occupy_2.nice + cpu_occupy_2.system - cpu_occupy_1.user - cpu_occupy_1.nice - cpu_occupy_1.system) /
          float(cpu_occupy_2.user + cpu_occupy_2.nice + cpu_occupy_2.system + cpu_occupy_2.idle - cpu_occupy_1.user - cpu_occupy_1.nice - cpu_occupy_1.system - cpu_occupy_1.idle) *
          100.0;
    }
  }).detach();
}