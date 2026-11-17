// Request-only comparison using real G1/H1/H2 headers and the Client double.
#include <unitree/robot/g1/loco/g1_loco_client.hpp>
#include <unitree/robot/h1/loco/h1_loco_client.hpp>
#include <unitree/robot/h2/loco/h2_loco_client.hpp>

int main() {
  unitree::robot::g1::LocoClient g1;
  g1.Init();
  g1.ZeroTorque();
  g1.Damp();
  g1.Squat();
  g1.Sit();
  g1.StandUp();
  g1.Start();

  unitree::robot::h1::LocoClient h1;
  h1.Init();
  h1.ZeroTorque();
  h1.Damp();
  h1.StandUp();
  h1.Start();

  unitree::robot::h2::LocoClient h2;
  h2.Init();
  h2.ZeroTorque();
  h2.Damp();
  h2.Squat();
  h2.Sit();
  h2.StandUp();
  h2.Start();
  h2.EnableArmSDK();
  h2.DisableArmSDK();
  std::vector<int> ids;
  std::vector<std::string> names;
  h2.GetAvailableFsmIds(ids, names);
}
