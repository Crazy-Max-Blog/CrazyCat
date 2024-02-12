#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "ui/color.h"
#include "ui/flags.h"
#include "ui/pos.h"

namespace gh {

struct Build {
    Build(gh::Action action,
          gh::Client* client,
          const char* name = nullptr,
          const char* value = nullptr) : action(action),
                                         client(*client),
                                         name(name),
                                         value(value) {}

    // тип билда
    gh::Action action;

    // клиент
    gh::Client& client;

    // имя компонента
    const char* name = nullptr;

    // значение компонента
    const char* value = nullptr;

    // билд-действие
    bool isSet() {
        return action == gh::Action::Set;
    }

    // билд-запрос виджетов
    bool isUI() {
        return action == gh::Action::UI;
    }

    // value
    // получить значение как int (32 бит)
    int32_t valueInt() {
        return atol(value);
    }

    // получить значение как float
    float valueFloat() {
        return atof(value);
    }

    // получить значение как const char*
    const char* valueStr() {
        return value;
    }

    // получить значение как bool
    bool valueBool() {
        return (value[0] == '1');
    }

    // получить значение как String
    String valueString() {
        return value;
    }

    // получить значение как Color
    gh::Color valueColor() {
        return gh::Color(atol(value), HEX);
    }

    // получить значение как Flags
    gh::Flags valueFlags() {
        return gh::Flags(atoi(value));
    }

    // получить значение как Pos
    gh::Pos valuePos() {
        uint32_t xy = atol(value);
        return gh::Pos(xy >> 16, xy & 0xffff, true);
    }

    // name
    // получить имя как String
    String nameString() {
        return name;
    }

    // получить имя как const char*
    const char* nameStr() {
        return name;
    }
};

}  // namespace gh