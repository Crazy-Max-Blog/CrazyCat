#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/hub.h"
#include "ui/canvas.h"
#include "utils/anyText.h"
#include "utils/packet.h"

namespace gh {
using gh::core::Tag;

class CanvasUpdate : public Canvas {
   public:
    CanvasUpdate(gh::core::AnyText name, GyverHub* hub, Client* client = nullptr) : _hub(hub), _client(client) {
        if (!_hub->canSend()) return;
        Canvas::setBuffer(&gs);
        gs += F("#{\"updates\":{");
        gs.addKey(name);
        gs += F("{\"data\":[");
    }

    void send() {
        if (!_hub->canSend()) return;
        gs.closeSqComma();
        gs.close();
        gs.close();
        gs.comma();
        gs.addID(_hub->id);
        if (_client) gs.addClient(_client->id);
        gs.addStr(Tag::type, Tag::update);
        gs.end();
        _hub->send(gs, _client);
    }

   private:
    gh::core::Packet gs;
    GyverHub* _hub;
    gh::Client* _client = nullptr;
};

}  // namespace gh