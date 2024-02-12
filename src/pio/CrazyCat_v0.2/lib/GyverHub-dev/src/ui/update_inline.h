#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/core.h"
#include "core/hooks.h"
#include "utils/anyText.h"
#include "utils/packet.h"
#include "widget.h"

namespace gh {
using gh::core::Tag;

class UpdateInline : public gh::core::Widget {
   public:
    UpdateInline(void* hub,
                 gh::core::SendHook hook,
                 char* id,
                 gh::core::AnyText name,
                 gh::core::AnyText func,
                 gh::Client* client) : gh::core::Widget(&gs),
                                       hub(hub),
                                       hook(hook),
                                       client(client) {
        if (!hub) return;
        _allowed = 1;
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::update);
        gs.addKey(Tag::updates);
        gs.open();
        gs.addKey(name);
        gs.open();
        if (func.str) gs.addStr(Tag::func, func);
    }

    ~UpdateInline() {
        if (_allowed) {
            gs.closeComma();
            gs.close();
            gs.end();
            hook(hub, &gs, client);
        }
    }

   private:
    void* hub;
    gh::core::SendHook hook;
    gh::Client* client;
    gh::core::Packet gs;
};

}  // namespace gh