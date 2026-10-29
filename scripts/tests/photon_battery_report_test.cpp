#include <cassert>
#include <cmath>
#include <cstring>
#include "helpers/PhotonBatteryReport.h"
int main() {
  char out[160];
  assert(photonBatteryReport(out, sizeof out, 3600, 25.0f, -1.25f));
  assert(strcmp(out, "battery=3.60v 50% charge=-1.25%/hr temp=77.0F") == 0);
  assert(photonBatteryReport(out, sizeof out, 4200, 0.0f, 2.0f));
  assert(strcmp(out, "battery=4.20v 100% charge=+2.00%/hr temp=32.0F") == 0);
  assert(photonBatteryReport(out, sizeof out, 3000, -40.0f, NAN));
  assert(strcmp(out, "battery=3.00v 0% temp=-40.0F") == 0);
  assert(photonBatteryReport(out, sizeof out, 0, NAN, INFINITY));
  assert(strcmp(out, "battery=na") == 0);
  assert(photonBatteryReport(out, sizeof out, 5000, INFINITY, NAN));
  assert(strcmp(out, "battery=5.00v 100%") == 0);
  char small[8];
  assert(!photonBatteryReport(small, sizeof small, 4200, 25.0f, 1.0f));
  assert(small[7] == 0);
  assert(!photonBatteryReport(nullptr, 0, 4200, 25.0f, 1.0f));
}
