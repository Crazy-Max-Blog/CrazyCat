#pragma once
#include <Arduino.h>

#include "hub_macro.hpp"
#include "types.h"

namespace gh::core {

gh::CMD getCMD(const char* str);

}