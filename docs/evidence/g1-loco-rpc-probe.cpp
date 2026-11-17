// Compile the real G1 inline methods with a transport-only Client test double.
#include <unitree/robot/g1/loco/g1_loco_client.hpp>

int main() {
  unitree::robot::g1::LocoClient client;
  client.Init();
  std::string data;
  client.GetMimicMotion(data);
  client.SetFsmId(550507);
  client.SwitchToUserCtrl();
  client.SwitchToInternalCtrl(unitree::robot::g1::InternalFsmMode::LAST);
  client.SwitchToInternalCtrl(unitree::robot::g1::InternalFsmMode::PASSIVE);
  client.SwitchToInternalCtrl(unitree::robot::g1::InternalFsmMode::WALKRUN);
}
