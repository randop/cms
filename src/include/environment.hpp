#pragma once

#ifndef CMS_ENVIRONMENT_HPP
#define CMS_ENVIRONMENT_HPP

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unordered_set>

#include <boost/optional.hpp>
#include <spdlog/spdlog.h>

namespace cms {
using string = std::string;

class Environment {
public:
  // Retrieves the value of the specified environment variable.
  // Returns boost::optional containing the value if found, or boost::none if
  // not set.
  static boost::optional<string> getVariable(const string &name);

  static void logOSinfo();

private:
  static string formatBytes(uint64_t bytes);
  static void getProcessorInfo();
};

boost::optional<string> Environment::getVariable(const string &name) {
  if (const char *value = std::getenv(name.c_str())) {
    return string(value);
  }
  return boost::none;
}

string Environment::formatBytes(uint64_t bytes) {
  const char *kUnits[] = {"B", "KB", "MB", "GB", "TB"};
  int unitIndex = 0;
  double size = static_cast<double>(bytes);

  while (size >= 1024 && unitIndex < 4) {
    size /= 1024;
    ++unitIndex;
  }

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << size << " " << kUnits[unitIndex];
  return oss.str();
}

void Environment::getProcessorInfo() {
  std::ifstream cpuInfo("/proc/cpuinfo");
  if (!cpuInfo.is_open()) {
    spdlog::error("Processor info error: Unable to open /proc/cpuinfo.");
    return;
  }

  string line;
  string processorName;
  int threadCount = 0;
  int coreCount = -1; // Initialize to -1 to detect if not found.
  std::unordered_set<string> physicalIds;

  while (std::getline(cpuInfo, line)) {
    if (processorName.empty() && line.find("model name") != string::npos) {
      size_t pos = line.find(":");
      if (pos != string::npos) {
        processorName = line.substr(pos + 2);
      }
    } else if (line.find("processor") != string::npos) {
      ++threadCount;
    } else if (coreCount == -1 && line.find("cpu cores") != string::npos) {
      size_t pos = line.find(":");
      if (pos != string::npos) {
        try {
          coreCount =
              std::stoi(line.substr(pos + 2)); // Cores per physical CPU.
        } catch (...) {
          coreCount = -1; // In case of parsing error.
        }
      }
    } else if (line.find("physical id") != string::npos) {
      size_t pos = line.find(":");
      if (pos != string::npos) {
        physicalIds.insert(line.substr(pos + 2));
      }
    }
  }

  // If core count not found, assume threads equal cores (fallback).
  if (coreCount == -1 || physicalIds.empty()) {
    coreCount = threadCount;
  } else {
    coreCount *= physicalIds.size(); // Total cores = cores per CPU * CPUs.
  }

  if (processorName.empty()) {
    processorName = "Unknown";
  }

  spdlog::info("Processor: {} <Cores: {}, Threads: {}>", processorName,
               coreCount, threadCount);
}

void Environment::logOSinfo() {
  struct utsname info;
  if (uname(&info) == -1) {
    spdlog::error("OS info error: ", std::strerror(errno));
  } else {
    spdlog::info("OS info: {} {} {} {} {}", info.sysname, info.nodename,
                 info.release, info.version, info.machine);
  }

  // Get memory info
  struct sysinfo mem_info;
  if (sysinfo(&mem_info) == -1) {
    spdlog::error("Error retrieving memory info: {}", std::strerror(errno));
  } else {
    // Print total memory
    uint64_t total_memory = mem_info.totalram * mem_info.mem_unit;
    spdlog::info("RAM: {}", formatBytes(total_memory));
  }

  getProcessorInfo();
}
} // namespace cms

#endif // CMS_ENVIRONMENT_HPP
