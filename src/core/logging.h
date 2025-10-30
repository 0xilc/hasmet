#pragma once

#include <iostream>
#include <string>

#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl;
#define LOG_WARN(msg)                                                   \
  std::cerr << "[WARN] (" << __FILE__ << ":" << __LINE__ << ") " << msg \
            << std::endl;
#define LOG_ERROR(msg)                                                   \
  std::cerr << "[ERROR] (" << __FILE__ << ":" << __LINE__ << ") " << msg \
            << std::endl;
