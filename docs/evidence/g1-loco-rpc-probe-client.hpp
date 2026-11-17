// Transport-only test double. It never opens a DDS channel or network socket.
#pragma once
#include <cstdint>
#include <iostream>
#include <string>

#define UT_ROBOT_CLIENT_REG_API_NO_PROI(apiId) RegistApi(apiId, 0)

namespace unitree::robot {
class Client {
 public:
  Client(const std::string& service, bool /*enable_lease*/) : service_(service) {}

 protected:
  void SetApiVersion(const std::string&) {}
  void RegistApi(int32_t, int32_t) {}

  int32_t Call(int32_t id, const std::string& parameter, std::string& data) {
    std::cout << service_ << '\t' << id << '\t' << parameter << '\n';
    data = "{}";
    return 0;  // Test-double result, not a robot response.
  }

 private:
  std::string service_;
};
}  // namespace unitree::robot
