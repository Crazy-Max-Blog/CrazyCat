#pragma once

#include <Arduino.h>

#include "core.h"
#include "hub_macro.hpp"
#include "ui/update_inline.h"

using gh::core::Tag;

class GyverHub : public gh::core::HubCore {
   public:
    // настроить префикс, название и иконку. Опционально задать свой ID устройства вида 0xXXXXXX (для esp он генерируется автоматически)
    GyverHub(const String& prefix = "", const String& name = "", const String& icon = "", uint32_t id = 0) {
        config(prefix, name, icon, id);
        _init();
    }

    // обновить виджет. Указать имя виджета (или список), имя функции, клиента (опционально)
    gh::UpdateInline update(gh::core::AnyText name, gh::core::AnyText func, gh::Client* client = nullptr) {
        return gh::UpdateInline((canSend() ? this : nullptr), _sendHook, _id, name, func, client);
    }

    // обновить виджет. Указать имя виджета (или список), клиента (опционально)
    gh::UpdateInline update(gh::core::AnyText name, gh::Client* client = nullptr) {
        gh::core::AnyText func;
        return update(name, func, client);
    }

    // ========================= SYSTEM =========================

    // настроить префикс, название и иконку. Опционально задать свой ID устройства вида 0xXXXXXX (для esp он генерируется автоматически)
    void config(const String& prefix, const String& name = "", const String& icon = "", uint32_t id = 0) {
        this->prefix = prefix;
        this->name = name;
        this->icon = icon;

#ifdef GH_ESP_BUILD
        if (id) {
            if (id <= 0xfffff) id += 0xfffff;
            ultoa(id, this->_id, HEX);
        } else {
            uint8_t mac[6];
            WiFi.macAddress(mac);
            ultoa(*((uint32_t*)(mac + 2)), this->_id, HEX);
        }
#else
        if (id <= 0x100000) id += 0x100000;
        ultoa(id, this->_id, HEX);
#endif
    }

    // установить версию прошивки для отображения в Info
    void setVersion(const String& v) {
        version = v;
    }

    // установить размер буфера строки для сборки интерфейса (умолч. 1000)
    // 0 - интерфейс будет собран и отправлен цельной строкой, >0 - пакет будет отправляться частями
    void setBufferSize(uint16_t size) {
        _bufsize = size;
    }

    // установить пин-код для открытия устройства (значение больше 1000, не может начинаться с 000..)
    void setPIN(uint32_t pin) {
        _pin = pin;
    }

    // прочитать пин-код
    uint32_t getPIN() {
        return _pin;
    }

    // запустить
    void begin() {
#ifdef GH_ESP_BUILD
#ifndef GH_NO_MQTT
        mqtt.begin();
#endif
#ifndef GH_NO_HTTP
        http.begin();
#endif
#ifndef GH_NO_WS
        ws.begin();
#endif

#ifndef GH_NO_FS
        gh::FS.begin();  // TODO
#endif

#endif  // GH_ESP_BUILD
        _running_f = true;
    }

    // остановить
    void end() {
#ifdef GH_ESP_BUILD
#ifndef GH_NO_MQTT
        mqtt.end();
#endif
#ifndef GH_NO_HTTP
        http.end();
#endif
#ifndef GH_NO_WS
        ws.end();
#endif
#endif  // GH_ESP_BUILD
        _running_f = false;
    }

    // ========================= ATTACH =========================

    // подключить функцию-сборщик интерфейса вида f(gh::Builder& builder)
    void onBuild(gh::core::BuildCallback callback) {
        _build_cb = *callback;
    }

    // подключить функцию-обработчик запроса при ручном соединении вида f(gh::Manual manual)
    void onManual(gh::core::ManualCallback callback) {
        _manual_cb = *callback;
    }

    // подключить обработчик запроса клиента вида f(gh::Request& request)
    void onRequest(gh::core::RequestCallback callback) {
#ifndef GH_NO_REQUEST
        _req_cb = *callback;
#endif
    }

    // подключить функцию-сборщик инфо вида f(gh::Info& info)
    void onInfo(gh::core::InfoCallback callback) {
        _info_cb = *callback;
    }

    // подключить обработчик входящих сообщений с веб-консоли вида f(String& str)
    void onCLI(gh::core::CliCallback callback) {
        _cli_cb = *callback;
    }

    // подключить обработчик получения unix времени с клиента вида f(uint32_t unix)
    void onUnix(gh::core::UnixCallback callback) {
        _unix_cb = *callback;
    }

#ifdef GH_ESP_BUILD
    // подключить обработчик скачивания файлов вида f(gh::Fetcher& fetcher)
    void onFetch(gh::core::FetchCallback callback) {
        _fetch_cb = callback;
    }

    // подключить функцию-обработчик перезагрузки вида f(gh::Reboot res). Будет вызвана перед перезагрузкой
    void onReboot(gh::core::RebootCallback callback) {
        _reboot_cb = *callback;
    }

    // подключить обработчик загрузки файлов вида f(String& path). Будет вызван при сохранении файла
    void onUpload(gh::core::UploadCallback callback) {
        _upload_cb = callback;
    }

    // безопасная загрузка файлов (загружать во временный файл) (умолч. включен, true)
    void safeUpload(bool state) {
        _safe_upl = state;
    }
#endif

    // ========================= SEND =========================

    // отправить текст в веб-консоль. Опционально цвет
    void sendCLI(gh::core::AnyText str, gh::Colors col = gh::Colors::Default, gh::Client* client = nullptr) {
        if (!focused() || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::print);
        gs.addStrEsc(Tag::text, str);
        gs.addInt(Tag::color, (uint32_t)col);
        gs.end();
        send(gs, client);
    }

    // обновить панель управления в приложении
    void sendRefresh(gh::Client* client = nullptr) {
        if (!focused() || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::refresh);
        gs.end();
        send(gs, client);
    }

    // выполнить js код
    void sendScript(gh::core::AnyText script, gh::Client* client = nullptr) {
        if (!focused() || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::script);
        gs.addStrEsc(Tag::script, script);
        gs.end();
        send(gs, client);
    }

    // ========================= UPDATE =========================

    // отправить действие (обновить файл, вызвать Confirm/Prompt)
    void sendAction(gh::core::AnyText name, gh::Client* client = nullptr) {
        if (!focused() || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::update);
        gs.addKey(Tag::updates);
        gs.open();
        gs.addKey(name);
        gs += F("{\"action\":1}}");
        gs.end();
        send(gs, client);
    }

    // отправить value update на имя виджета [String]
    void sendUpdateStr(gh::core::AnyText name, const String& value, gh::Client* client = nullptr) {
        if (!focused() || !_allow_send) return;
        _sendUpdate(name, value, client);
    }

    // отправить value update на имя виджета [float]
    void sendUpdateFloat(gh::core::AnyText name, float value, int dec = 2, gh::Client* client = nullptr) {
        if (!focused() || !_allow_send) return;
        String s(value, dec);
        _sendUpdate(name, s, client);
    }

    // отправить value update на имя виджета [целочисленный]
    template <typename T>
    void sendUpdateInt(gh::core::AnyText name, T value, gh::Client* client = nullptr) {
        if (!focused() || !_allow_send) return;
        String s(value);
        _sendUpdate(name, s, client);
    }

    // отправить value update по имени компонента (значение будет прочитано в build). Нельзя вызывать из build. Имена можно передать списком через ;
    void sendUpdate(const String& name, gh::Client* client = nullptr) {
        if (!_build_cb || !focused() || _build_busy) return;
        gh::Client read_client;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::update);
        gs.addKey(Tag::updates);
        gs.open();
        for (gh::core::Parser p((char*)name.c_str()); p.next();) {
            gs.addKey(p.get());
            gs += F("{\"value\":\"");
            gs.reserve(gs.length() + 30);
            _readBuild(&gs, &read_client, p.get());
            gs += F("\"},");
        }
        gs.closeComma();
        gs.end();
        send(gs, client);
    }

    // ========================= MQTT =========================

    // автоматически отправлять новое состояние на get-топик при изменении через set (умолч. false)
    void sendGetAuto(bool state) {
        _autoGet_f = state;
    }

    // отправить имя-значение на get-топик (MQTT)
    void sendGetStr(gh::core::AnyText name, gh::core::AnyText text) {
        _sendGet(name, text);
    }

    // отправить имя-значение на get-топик (MQTT) [float]
    void sendGetFloat(gh::core::AnyText name, float value, int dec = 2) {
        String s(value, dec);
        _sendGet(name, s);
    }

    // отправить имя-значение на get-топик (MQTT) [целочисленный]
    template <typename T>
    void sendGetInt(gh::core::AnyText name, T value) {
        String s(value);
        _sendGet(name, s);
    }

    // отправить значение по имени компонента на get-топик (MQTT) (значение будет прочитано в build). Имена можно передать списком через ;
    void sendGet(const String& name) {
        if (!_running_f || !_build_cb || _build_busy) return;
        gh::Client client;
        gh::core::Packet value;
        value.reserve(30);

        for (gh::core::Parser p((char*)name.c_str()); p.next();) {
            bool ok = _readBuild(&value, &client, p.get());
            if (ok) _sendGet(p.get(), value);
        }
    }

    // отправить MQTT LWT команду на включение/выключение
    void sendStatus(bool status) {
#ifndef GH_NO_MQTT
        String topic = _topicStatus();
        _sendMQTT(topic, status ? F("online") : F("offline"));
#endif
    }

    // топик статуса для отправки
    String topicStatus() {
        String t(prefix);
        t += F("/hub/");
        t += id;
        t += F("/status");
        return t;
    }

    // общий топик для подписки
    String topicDiscover() {
        return prefix;
    }

    // топик устройства для подписки
    String topicHub() {
        String t(prefix);
        t += '/';
        t += id;
        t += "/#";
        return t;
    }

    // ========================= NOTIFY ==========================

    // отправить пуш уведомление
    void sendPush(gh::core::AnyText text, gh::Client* client = nullptr) {
        if (!_running_f || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::push);
        gs.addStrEsc(Tag::text, text);
        gs.end();
        send(gs, client, true);
    }

    // отправить всплывающее уведомление
    void sendNotice(gh::core::AnyText text, gh::Colors col = gh::Colors::Green, gh::Client* client = nullptr) {
        if (!_running_f || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::notice);
        gs.addStrEsc(Tag::text, text);
        gs.addInt(Tag::color, (uint32_t)col);
        gs.end();
        send(gs, client, true);
    }

    // показать окно с ошибкой
    void sendAlert(gh::core::AnyText text, gh::Client* client = nullptr) {
        if (!_running_f || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::alert);
        gs.addStrEsc(Tag::text, text);
        gs.end();
        send(gs, client, true);
    }

    // ========================== STATUS ==========================

    // вернёт true, если система запущена
    bool running() {
        return _running_f;
    }

    // true - интерфейс устройства сейчас открыт на сайте или в приложении
    bool focused() {
        return _focus.focused();
    }

    // проверить фокус по указанному типу связи
    bool focused(gh::Connection conn) {
        return _focus.focused(conn);
    }
    // ========================== PARSE ==========================

    // парсить строку вида PREFIX/ID/CLIENT_ID/CMD/NAME=VALUE
    void parse(char* url, gh::Connection conn) {
        _parse(url, conn, gh::Source::Manual);
    }

    // парсить строку вида PREFIX/ID/CLIENT_ID/CMD/NAME + VALUE
    void parse(char* url, char* value, gh::Connection conn) {
        _parse(url, value, conn, gh::Source::Manual);
    }

    // ========================= MISC =========================

    // получить полный JSON пакет панели управления. Флаг enclose true - обернуть в результат в []
    String getUI(bool enclose = false) {
        if (_build_busy) return String();
        gh::Client client;
        gh::core::Packet gs;
        if (enclose) gs.openSq();
        _uiBuild(&gs, &client);
        if (enclose) gs.closeSq();
        return gs;
    }

    // получить JSON объект {имя:значение, ...} виджетов (из билдера)
    String getValues() {
        if (_build_busy) return String();
        gh::core::Packet str;
        str.open();
        if (_build_cb) {
            _build_busy = true;
            _allow_send = false;
            gh::Client client;
            gh::Build build(gh::Action::Get, &client);
            gh::Builder b(&build, &str);
            _build_cb(b);
            _allow_send = true;
            _build_busy = false;
        }
        str.closeComma();
        return str;
    }

    // получить значение компонента по имени (из билдера)
    String getValue(const String& name) {
        if (_build_busy) return String();
        gh::Client client;
        gh::core::Packet value;
        value.reserve(30);
        _readBuild(&value, &client, name.c_str());
        return value;
    }

    // ========================= TICK =========================

    // тикер, вызывать в loop
    bool tick() {
        _focus.tick();
        if (!_running_f) return 0;

#ifndef GH_NO_STREAM
        stream.tick();
#endif

#ifdef GH_ESP_BUILD
#ifndef GH_NO_MQTT
        mqtt.tick();
#endif
#ifndef GH_NO_HTTP
        http.tick();
#endif
#ifndef GH_NO_WS
        ws.tick();
#endif

#ifndef GH_NO_FS
#ifndef GH_NO_FETCH
        if (_fet_p && _fet_p->timeout()) {
            _answerError(Tag::fetch_err, _fet_p->error, &_fet_p->client);
            delete _fet_p;
            _fet_p = nullptr;
        }
#endif
#ifndef GH_NO_UPLOAD
        if (_upl_p && _upl_p->timeout()) {
            _answerError(Tag::upload_err, _upl_p->error, &_upl_p->client);
            delete _upl_p;
            _upl_p = nullptr;
        }
#endif
#endif  // GH_NO_FS

#ifndef GH_NO_OTA
        if (_ota_p && _ota_p->timeout()) {
            _answerError(Tag::ota_err, _ota_p->error, &_ota_p->client);
            delete _ota_p;
            _ota_p = nullptr;
        }
#endif
        if (_reason != gh::Reboot::None) {
            if (_reboot_cb) _reboot_cb(_reason);
            delay(1500);
            ESP.restart();
        }

#ifndef GH_NO_OTA_URL
        if (_otaurl_p) {
            if (_otaurl_p->update()) {
                _answerCmd(Tag::ota_url_ok, &(_otaurl_p->client));
                _reason = gh::Reboot::OtaUrl;
            } else {
                _answerError(Tag::ota_url_err, gh::Error::End, &(_otaurl_p->client));
            }
            delete _otaurl_p;
            _otaurl_p = nullptr;
        }
#endif

#endif  // GH_ESP_BUILD
        return 1;
    }  // tick

   private:
};