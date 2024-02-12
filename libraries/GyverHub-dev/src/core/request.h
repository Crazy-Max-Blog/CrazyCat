#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/types.h"

namespace gh {

class Request;
namespace core {
typedef bool (*RequestCallback)(gh::Request& request);
}

class Request {
   public:
    Request(gh::Client* client,
            gh::CMD cmd,
            const char* name = "",
            const char* value = "") : client(*client),
                                      cmd(cmd),
                                      name(name),
                                      value(value) {}

    // клиент
    gh::Client client;

    // событие
    gh::CMD cmd;

    // имя
    const char* name;

    // значение
    const char* value;
};

}  // namespace gh