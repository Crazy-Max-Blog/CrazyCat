#pragma once
#include <Arduino.h>

#include "hub_macro.hpp"
#include "types.h"

namespace gh {

// получить текстовое значение типа CMD для вывода в порт
FSTR readCMD(gh::CMD cmd);

// получить текстовое значение типа Connection для вывода в порт
FSTR readConnection(gh::Connection connection);

// получить текстовое значение типа Action для вывода в порт
FSTR readAction(gh::Action action);

// получить текстовое значение типа Reboot для вывода в порт
FSTR readReboot(gh::Reboot reason);

// получить текстовое значение типа Source для вывода в порт
FSTR readSource(gh::Source source);

}  // namespace gh