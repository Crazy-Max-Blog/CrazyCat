#pragma once
#include <Arduino.h>

#include "core/types.h"
#include "utils/anyText.h"

namespace gh {

class Manual {
   public:
    gh::core::AnyText text;
    bool last = true;

    const char* topic = nullptr;
    bool ws_all = false;
    gh::Connection connection;
};

}  // namespace gh
