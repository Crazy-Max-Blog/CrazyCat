#pragma once
#include <Arduino.h>

#ifndef GH_NO_PAIRS
#include <Pairs.h>
#endif

#include "anyptr.h"
#include "core/types.h"
#include "ui/button.h"
#include "ui/color.h"
#include "ui/flags.h"
#include "ui/log.h"
#include "utils/anyText.h"
#include "utils/anyptr.h"
#include "utils/packet.h"

namespace gh::core {

// void strToVar(const char* s, void* var, gh::Type type);
// void varToStr(gh::core::Packet* s, void* var, gh::Type type);

void strToVar(const char* s, gh::core::AnyPtr& data);
void varToStr(gh::core::Packet* s, const gh::core::AnyPtr& data);

}  // namespace gh::core