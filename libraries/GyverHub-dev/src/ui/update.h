#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/hub.h"
#include "utils/anyText.h"
#include "utils/packet.h"
#include "widget.h"

namespace gh {
using gh::core::AnyText;
using gh::core::Packet;
using gh::core::Tag;
using gh::core::Widget;

class Update {
   public:
    Update(GyverHub* hub, gh::Client* client = nullptr) : widget(&s), _hub(hub), _client(client) {
        widget._allowed = _hub->canSend();
        _init();
    }

    // начать обновление по имени виджета (или список)
    Widget& update(const AnyText& name) {
        if (widget._allowed) {
            _check();
            s.addKey(name);
            s.open();
        }
        return widget;
    }

    // начать обновление для кастомного виджета по имени (или список) + имя функции
    Widget& update(const AnyText& name, const AnyText& func) {
        if (widget._allowed) {
            update(name);
            s.addStr(Tag::func, func);
        }
        return widget;
    }

    // отправить пакет
    void send() {
        if (!widget._empty && widget._allowed) {
            if (!_first) s.closeComma();
            s.closeComma();
            s.comma();
            s.addID(_hub->id);
            if (_client) s.addClient(_client->id);
            s.addStr(Tag::type, Tag::update);
            s.end();
            _hub->send(s, _client);
        }
        _init();
    }

    // текущий виджет для установки значений
    Widget widget;

   private:
    GyverHub* _hub;
    gh::Client* _client = nullptr;
    Packet s;
    bool _first = true;

    void _check() {
        if (widget._allowed) {
            if (_first) _first = false;
            else {
                s.closeComma();
                s.comma();
            }
        }
    }
    void _init() {
        if (widget._allowed) {
            widget._empty = true;
            _first = true;
            s.begin();
            s.addKey(Tag::updates);
            s.open();
        }
    }
};

}  // namespace gh