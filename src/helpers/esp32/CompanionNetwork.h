#pragma once

#include <IPAddress.h>

namespace companion_network {

bool begin(const char* hostname);
void loop();
bool hasIP();
IPAddress localIP();

}
