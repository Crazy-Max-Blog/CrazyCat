#pragma once
#include <Arduino.h>

#include "types.h"

namespace gh {

class Client {
   public:
    Client(gh::Connection connection = gh::Connection::System, const char* id = nullptr, gh::Source source = gh::Source::System) : connection(connection), source(source) {
        if (id && strlen(id) <= 8) strcpy(this->id, id);
    }

    // соединение
    gh::Connection connection;

    // источник
    gh::Source source;

    // id клиента
    char id[9] = {0};

    // id как String
    String idString() {
        return id;
    }

    bool operator==(Client& client) {
        return eq(client);
    }
    bool operator!=(Client& client) {
        return !eq(client);
    }

   private:
    bool eq(Client& client) {
        return (client.connection == connection && client.source == source && !strcmp(client.id, id));
    }
};

}  // namespace gh