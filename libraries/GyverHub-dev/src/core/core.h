#pragma once

#include <Arduino.h>

#include "build.h"
#include "client.h"
#include "core/request.h"
#include "core_class.h"
#include "event_p.h"
#include "focus.h"
#include "fs.h"
#include "hooks.h"
#include "hub_macro.hpp"
#include "modules.h"
#include "types.h"
#include "types_p.h"
#include "ui/builder.h"
#include "ui/info.h"
#include "ui/manual.h"
#include "utils/datatypes.h"
#include "utils/packet.h"
#include "utils/parse.h"

#ifdef GH_ESP_BUILD

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif  // ESP8266

#ifdef ESP32
#include <WiFi.h>
#endif  // ESP32

#include "esp/transfer/fetcher.h"

#ifndef GH_NO_OTA
#include "esp/transfer/updater.h"
#endif

#ifndef GH_NO_OTA_URL
#include "esp/transfer/ota_url.h"
#endif

#ifndef GH_NO_FS
#ifndef GH_NO_UPLOAD
#include "esp/transfer/uploader.h"
#endif
#endif  // GH_NO_FS

#ifndef GH_NO_HTTP
#include "esp/sync/http.h"
#endif
#ifndef GH_NO_MQTT
#include "esp/sync/mqtt.h"
#endif
#ifndef GH_NO_WS
#include "esp/sync/ws.h"
#endif

#endif  // GH_ESP_BUILD

#ifndef GH_NO_STREAM
#include "stream.h"
#endif

namespace gh::core {
typedef void (*CliCallback)(String& str);
typedef void (*ManualCallback)(gh::Manual manual);
typedef void (*UnixCallback)(uint32_t unix);
#ifdef GH_ESP_BUILD
typedef void (*RebootCallback)(gh::Reboot res);
typedef void (*FetchCallback)(gh::Fetcher& fetcher);
typedef void (*UploadCallback)(String& path);
#endif

using gh::core::Tag;

// ========================== CLASS ==========================
class HubCore {
   public:
    // доступна ли сейчас отправка обновлений и sendXxx функции
    bool canSend() {
        return _allow_send;
    }

    // отправить пакет указанному клиенту или всем
    void send(gh::core::AnyText text, gh::Client* client = nullptr, bool force = false) {
        text.calcLen();
        if (_manual_cb) {
            gh::Manual manual;
            String topic;
            manual.ws_all = true;
            manual.text = text;
            if (client) {
                if (_focus.focused(client->connection) || force) {
                    if (client->connection == gh::Connection::MQTT) topic = _topicSend(client->id);
                    manual.topic = topic.c_str();
                    manual.connection = client->connection;
                    _manual_cb(manual);
                }
            } else {
                topic = _topicSend();
                manual.topic = topic.c_str();
                for (uint8_t i = 0; i < 4; i++) {  // Serial, Bluetooth, WS, MQTT
                    manual.connection = (gh::Connection)i;
                    if (_focus.focused(manual.connection) || force) _manual_cb(manual);
                }
            }
        }

#ifndef GH_NO_STREAM
        if (stream.state() && (_focus.focused(stream.connection()) || force)) {
            if (!client) stream.send(text);
            else if (client->connection == stream.connection()) stream.send(text);
        }
#endif

#ifdef GH_ESP_BUILD
#ifndef GH_NO_MQTT
        if (_focus.focused(gh::Connection::MQTT) || force) {
            if (!client) {
                String topic = _topicSend();
                mqtt.send(topic.c_str(), text);
            } else if (client->connection == gh::Connection::MQTT) {
                String topic = _topicSend(client->id);
                mqtt.send(topic.c_str(), text);
            }
        }
#endif
#ifndef GH_NO_WS
        if (_focus.focused(gh::Connection::WS)) {
            if (!client) ws.send(text);
            else if (client->connection == gh::Connection::WS) ws.send(text);
        }
#endif
#endif
    }

    // ==========================================================================
    // ================================== VARS ==================================
    // ==========================================================================

    // CONST
    String prefix = "";
    String name = "";
    String icon = "";
    String version = "";
    const char* id = _id;

// модули
#ifndef GH_NO_MODULES
    gh::core::Modules modules;
#endif

#ifndef GH_NO_STREAM
    gh::core::HubStream stream;
#endif

#if defined(GH_ESP_BUILD) && !defined(GH_NO_MQTT)
    gh::core::MQTT mqtt;
#endif

   protected:
    char _id[9] = {'\0'};
    uint32_t _pin = 0;

    uint16_t _bufsize = 1000;
    bool _running_f = false;
    bool _build_busy = false;  // (билдер запущен) запрещает вызывать функции, которые вызывают билдер
    bool _allow_send = true;   // разрешает отправку sendXxx и класс Update только вне билдера + в Set билдере
    uint8_t _menu = 0;
    gh::core::Focus _focus;
    gh::Client* _client_p = nullptr;

    gh::core::BuildCallback _build_cb = nullptr;
    gh::core::ManualCallback _manual_cb = nullptr;
    gh::core::CliCallback _cli_cb = nullptr;
    gh::core::InfoCallback _info_cb = nullptr;
    gh::core::UnixCallback _unix_cb = nullptr;
#ifndef GH_NO_REQUEST
    gh::core::RequestCallback _req_cb = nullptr;
#endif

    bool _autoGet_f = true;

#ifdef GH_ESP_BUILD
    bool _safe_upl = true;
    gh::core::UploadCallback _upload_cb = nullptr;
    gh::core::FetchCallback _fetch_cb = nullptr;
    gh::core::RebootCallback _reboot_cb = nullptr;
    gh::Reboot _reason = gh::Reboot::None;

#ifndef GH_NO_FS
#ifndef GH_NO_FETCH
    gh::Fetcher* _fet_p = nullptr;
    void _errFetcher(gh::Error error) {
        _answerError(Tag::fetch_err, error, _client_p);
    }
#endif
#ifndef GH_NO_UPLOAD
    gh::core::Uploader* _upl_p = nullptr;
    void _errUploader(gh::Error error) {
        _answerError(Tag::upload_err, error, _client_p);
    }
#endif
#endif  // GH_NO_FS

#ifndef GH_NO_OTA
    gh::core::Updater* _ota_p = nullptr;
    void _errUpdater(gh::Error error) {
        _answerError(Tag::ota_err, error, _client_p);
    }
#endif
#ifndef GH_NO_OTA_URL
    gh::core::OtaUrl* _otaurl_p = nullptr;
#endif

#ifndef GH_NO_HTTP
    gh::core::HTTP http;
#endif
#ifndef GH_NO_WS
    gh::core::WS ws;
#endif
#endif

    // ==========================================================================
    // ================================== FUNC ==================================
    // ==========================================================================

    void _init() {
#ifndef GH_NO_STREAM
        stream.setup(this, _parseHook);
#endif

#ifdef GH_ESP_BUILD
#ifndef GH_NO_HTTP

#ifndef GH_NO_MODULES
        http.setup(this, _parseHook, _fetchHook, _requestHook, _uploadHook, &_safe_upl, &modules, &_reason);
#else
        http.setup(this, _parseHook, _fetchHook, _requestHook, _uploadHook, &_safe_upl, nullptr, &_reason);
#endif

#endif  // GH_NO_HTTP
#ifndef GH_NO_WS
        ws.setup(this, _parseHook, _focusHook);
#endif
#ifndef GH_NO_MQTT
        mqtt.setup(this, _parseHook, _id, &prefix);
#endif
#endif
    }
#ifndef GH_NO_REQUEST
    bool _request(gh::CMD cmd, gh::Client* client, const char* name = "", const char* value = "") {
        if (_req_cb) {
            gh::Request req(client, cmd, name, value);
            return _req_cb(req);
        }
        return 1;
    }
#endif
    void _discover(gh::Connection conn, const char* id, gh::Source source, gh::CMD cmd) {
        gh::Client client(conn, id, source);
        _client_p = &client;
#ifndef GH_NO_REQUEST
        if (_request(cmd, _client_p)) {
            _answerDiscover();
        } else {
            _answerError(Tag::error, gh::Error::Forbidden);
        }
#else
        _answerDiscover();
#endif
    }

    // ==========================================================================
    // ================================== HOOKS =================================
    // ==========================================================================

    static void _parseHook(void* hubptr, char* url, char* value, gh::Connection conn, gh::Source src) {
        if (value == nullptr) ((HubCore*)hubptr)->_parse(url, conn, src);
        else ((HubCore*)hubptr)->_parse(url, value, conn, src);
    }
    static void _answerHook(void* hubptr, gh::core::AnyText text, bool stop) {
        ((HubCore*)hubptr)->_answer(text, stop);
    }
    static void _sendHook(void* hubptr, String* text, gh::Client* client) {
        ((HubCore*)hubptr)->send(*text, client);
    }

#ifdef GH_ESP_BUILD
    static void _focusHook(void* hubptr, gh::Connection conn, bool focus) {
        ((HubCore*)hubptr)->_focus.write(conn, focus);
    }
    static void _fetchHook(void* hubptr, gh::Fetcher* fetcher) {
        if (((HubCore*)hubptr)->_fetch_cb) ((HubCore*)hubptr)->_fetch_cb(*fetcher);
    }
    static bool _requestHook(void* hubptr, gh::Request* request) {
#ifndef GH_NO_REQUEST
        if (((HubCore*)hubptr)->_req_cb) return ((HubCore*)hubptr)->_req_cb(*request);
#endif
        return 1;
    }
    static void _uploadHook(void* hubptr, String& path) {
        if (((HubCore*)hubptr)->_upload_cb) ((HubCore*)hubptr)->_upload_cb(path);
    }
#endif

    // ==========================================================================
    // ================================= ANSWER =================================
    // ==========================================================================

    void _answerDiscover() {
#ifndef GH_NO_MODULES
#ifdef GH_ESP_BUILD
#ifdef GH_NO_FS
        modules.clear(gh::Module::ModFiles | gh::Module::ModFormat | gh::Module::ModFetch | gh::Module::ModUpload | gh::Module::ModDelete | gh::Module::ModRename | gh::Module::ModCreate);
#endif
#ifdef GH_NO_OTA
        modules.clear(gh::Module::ModOta);
#endif
#ifdef GH_NO_OTA_URL
        modules.clear(gh::Module::ModOtaUrl);
#endif
#else
        modules.clear(gh::Module::ModReboot | gh::Module::ModFiles | gh::Module::ModFormat | gh::Module::ModFetch | gh::Module::ModUpload | gh::Module::ModOta | gh::Module::ModOtaUrl | gh::Module::ModDelete | gh::Module::ModRename | gh::Module::ModCreate);
#endif
#endif
        uint32_t hash = 0;
        if (_pin > 999) {
            char pin_s[11];
            ultoa(_pin, pin_s, 10);
            uint16_t len = strlen(pin_s);
            for (uint16_t i = 0; i < len; i++) {
                hash = (((uint32_t)hash << 5) - hash) + pin_s[i];
            }
        }
        gh::core::Packet gs(this, _answerHook, _bufsize);
        if (!_bufsize) gs.reserve(180);
        gs.begin();
        gs.addID(id);
        gs.addStr(Tag::type, Tag::discover);
        gs.addStr(Tag::name, name);
        gs.addStr(Tag::prefix, prefix);
        gs.addStr(Tag::icon, icon);
        gs.addInt(Tag::PIN, hash);
        gs.addStr(Tag::version, version);
        gs.addStr(Tag::platform, GH_PLATFORM);
        gs.addInt(Tag::max_upl, GH_UPL_CHUNK_SIZE);
        gs.addInt(Tag::api_v, GH_API_VERSION);

#if defined(GH_NO_HTTP_TRANSFER)
        gs.addInt(Tag::http_t, 0);
#else
        gs.addInt(Tag::http_t, 1);
#endif
#ifdef ATOMIC_FS_UPDATE
        gs.addStr(Tag::ota_t, F("gz"));
#else
        gs.addStr(Tag::ota_t, F("bin"));
#endif
#ifndef GH_NO_MODULES
        gs.addInt(Tag::ws_port, modules.read(gh::Module::ModWS) ? GH_WS_PORT : 0);
        gs.addInt(Tag::modules, modules.mods);
#else
        gs.addInt(Tag::ws_port, GH_WS_PORT);
        gs.addInt(Tag::modules, 0);
#endif
        gs.end();
        _answer(gs);
    }
    void _answerAck(const char* name) {
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        gs.addStr(Tag::type, Tag::ack);
        gs.addStr(Tag::name, name);
        gs.end();
        _answer(gs);
    }
    void _answerError(Tag cmd, gh::Error err, gh::Client* client = nullptr) {
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        gs.addStr(Tag::type, cmd);
        gs.addInt(Tag::code, (uint8_t)err);
        gs.end();
        if (client) _answer(gs, client);
        else _answer(gs);
    }
    void _answerOK() {
        _answerCmd(Tag::OK);
    }
    void _answerCmd(Tag type, gh::Client* client = nullptr) {
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        gs.addStr(Tag::type, type);
        gs.end();
        if (client) _answer(gs, client);
        else _answer(gs);
    }
    void _answerInfo() {
        gh::core::Packet gs(this, _answerHook, _bufsize);
        if (_bufsize) gs.reserve(300);
        gs.begin();
        gs.addID(id);
        gs.addStr(Tag::type, Tag::info);
        gh::Info::_build(_info_cb, gs, &version, _client_p);
        gs.end();
        _answer(gs);
    }
    void _answerUI() {
        gh::core::Packet gs(this, _answerHook, _bufsize);
        gs.begin();
        gs.addKey(Tag::controls);
        gs.openSq();
#ifndef GH_NO_MODULES
        if (modules.read(gh::Module::ModUI)) _uiBuild(&gs, _client_p);
#else
        _uiBuild(&gs, _client_p);
#endif
        gs.closeSq();
        gs.comma();
        gs.addID(id);
        gs.addStr(Tag::type, Tag::ui);
        gs.end();
        _answer(gs);
    }
    void _answerFiles() {
#if !defined(GH_NO_FS)
        if (!gh::FS.mounted()) return _answerCmd(Tag::fs_err);
        uint32_t count = 0;
        {
            String str;
            str.reserve(100);
            gh::FS.showFiles(str, "/", GH_FS_DEPTH, &count);
        }
        gh::core::Packet gs(this, _answerHook, _bufsize);
        gs.reserve(count + 50);
        gs += F("#{\"fs\":{\"/\":0,");
        gh::FS.showFiles(gs, "/", GH_FS_DEPTH);
        gs.closeComma();
        gs.comma();

        gs.addID(id);
        gs.addStr(Tag::type, Tag::files);
        gs.addInt(Tag::used, gh::FS.used());
        gs.addInt(Tag::total, gh::FS.totalSpace());
        gs.end();
        _answer(gs);
#endif
    }

    // ответить указанному клиенту
    void _answer(gh::core::AnyText text, gh::Client* client, bool last = true) {
        text.calcLen();
        if (!client) return;
        switch (client->source) {
            case gh::Source::ESP:
#ifdef GH_ESP_BUILD
                switch (client->connection) {
#ifndef GH_NO_MQTT
                    case gh::Connection::MQTT: {
                        String topic = _topicSend(client->id);
                        mqtt.send(topic.c_str(), text);
                    } break;
#endif
#ifndef GH_NO_WS
                    case gh::Connection::WS:
                        ws.answer(text);
                        break;
#endif
#ifndef GH_NO_HTTP
                    case gh::Connection::HTTP:
                        http.answer(text);
                        break;
#endif
                    default:
                        break;
                }
#endif
                break;  // gh::Source::ESP

            case gh::Source::Manual:
                if (_manual_cb) {
                    gh::Manual manual;
                    String topic;
                    if (client->connection == gh::Connection::MQTT) {
                        topic = _topicSend(client->id);
                    }
                    manual.topic = topic.c_str();
                    manual.text = text;
                    manual.connection = client->connection;
                    manual.last = last;
                    _manual_cb(manual);
                }
                break;

            case gh::Source::Stream:
#ifndef GH_NO_STREAM
                stream.send(text);
#endif
                break;

            default:
                break;
        }
    }

    // ответить сохранённому клиенту. stop - последний пакет (очистить указатель)
    void _answer(gh::core::AnyText text, bool last = true) {
        if (!_client_p) return;
        _answer(text, _client_p, last);
        if (last) _client_p = nullptr;
    }

    // ==========================================================================
    // ================================== SEND ==================================
    // ==========================================================================

    void _sendUpdate(gh::core::AnyText name, gh::core::AnyText value, gh::Client* client = nullptr) {
        if (!_focus.focused() || !_allow_send) return;
        gh::core::Packet gs;
        gs.reserve(50);
        gs.begin();
        gs.addID(id);
        if (client) gs.addClient(client->id);
        gs.addStr(Tag::type, Tag::update);
        gs.addKey(Tag::updates);
        gs.open();
        gs.addKey(name);
        gs.open();
        gs.addStrEsc(Tag::value, value);
        gs.closeComma();
        gs.close();
        gs.end();
        send(gs, client);
    }
    void _sendGet(gh::core::AnyText name, gh::core::AnyText text) {
        if (!_running_f || !_allow_send) return;
        String topic = _topicGet(name);
        _sendMQTT(topic, text);
    }
    void _sendMQTT(const String& topic, gh::core::AnyText text) {
        text.calcLen();
        if (_manual_cb) {
            gh::Manual manual;
            manual.connection = gh::Connection::MQTT;
            manual.text = text;
            manual.topic = topic.c_str();
            _manual_cb(manual);
        }
#if defined(GH_ESP_BUILD) && !defined(GH_NO_MQTT)
        mqtt.send(topic.c_str(), text);
#endif
    }
    String _topicSend(const char* clientID = nullptr) {
        String topic(prefix);
        topic += F("/hub");
        if (clientID) {
            topic += '/';
            topic += clientID;
            topic += '/';
            topic += id;
        }
        return topic;
    }
    String _topicGet(gh::core::AnyText name) {
        String topic(prefix);
        topic += F("/hub/");
        topic += id;
        topic += F("/get/");
        if (name.pgm) topic += (FSTR)name.str;
        else topic += name.str;
        return topic;
    }
    String _topicStatus() {
        String topic(prefix);
        topic += F("/hub/");
        topic += id;
        topic += F("/status");
        return topic;
    }

    // ==========================================================================
    // ================================= BUILD ==================================
    // ==========================================================================

    bool _readBuild(gh::core::Packet* gs, gh::Client* client, const char* name) {
        if (_build_cb) {
            gh::Build build(gh::Action::Read, client, name);
            gh::Builder b(&build, gs);
            _build_busy = true;
            _allow_send = false;
            _build_cb(b);
            _allow_send = true;
            _build_busy = false;
            return b._stop;
        }
        return 0;
    }
    void _uiBuild(gh::core::Packet* gs, gh::Client* client) {
        if (!_build_cb) return;
        _build_busy = true;
        _allow_send = false;
        // count
        uint32_t count = 0;
        if (_bufsize == 0) {  // count ui
            gh::core::Packet str;
            str.reserve(50);
            gh::Build build(gh::Action::Count, client);
            gh::Builder builder(&build, &str, &count);
            _build_cb(builder);
            count += str.length();
        }

        // build
        if (_bufsize == 0) gs->reserve(count + gs->length());
        else gs->reserve(gs->length() + _bufsize);
        gh::Build build(gh::Action::UI, client);
        gh::Builder b(&build, gs);
        b._menu = &_menu;
        _build_cb(b);
        if (!b._first) gs->closeComma();
        _allow_send = true;
        _build_busy = false;
    }

    // ==========================================================================
    // ================================== PARSE =================================
    // ==========================================================================

    void _parse(char* url, gh::Connection conn, gh::Source source) {
        if (!_running_f) return;
        char* eq = strchr(url, '=');
        char val[1] = {0};
        if (eq) *eq = 0;
        _parse(url, eq ? (eq + 1) : val, conn, source);
    }
    void _parse(char* url, char* value, gh::Connection conn, gh::Source source) {
        if (!_running_f) return;
#ifndef GH_NO_MODULES
        if (!modules.checkConnection(conn)) return;
#endif
        if (!strcmp(url, prefix.c_str())) return _discover(conn, value, source, gh::CMD::Search);

        gh::core::Splitter<5> sp(url);

        if (strcmp(sp.str[0], prefix.c_str())) return;  // wrong prefix
        if (strcmp(sp.str[1], id)) return;              // wrong id
        if (sp.size == 2) return _discover(conn, value, source, gh::CMD::Discover);
        if (sp.size == 3) return;

        const char* name = sp.str[4];
        gh::CMD cmd = gh::core::getCMD(sp.str[3]);  // cmd
        if (cmd == gh::CMD::Unknown) return;

        gh::Client client(conn, sp.str[2], source);  // client id
        _client_p = &client;
#ifndef GH_NO_REQUEST
        if (!_request(cmd, _client_p, name, value)) return _answerError(Tag::error, gh::Error::Forbidden);
#endif
#ifndef GH_NO_MODULES
        if (!modules.checkCMD(cmd)) return _answerError(Tag::error, gh::Error::Disabled);
#endif

        // PREFIX/ID/CLIENT_ID/CMD
        if (sp.size == 4) {
            _focus.set(conn);
            switch (cmd) {
                case gh::CMD::UI:
                    _answerUI();
                    break;

                case gh::CMD::Ping:
                    _answerOK();
                    break;

                case gh::CMD::Unfocus:
                    _focus.clear(conn);
                    break;

                case gh::CMD::Info:
#ifdef GH_NO_INFO
                    _answerOK();
#else
                    _answerInfo();
#endif
                    break;

#ifdef GH_ESP_BUILD
                case gh::CMD::Reboot:
                    _reason = gh::Reboot::Button;
                    _answerOK();
                    break;

#ifndef GH_NO_FS
                case gh::CMD::Files:
                    _answerFiles();
                    break;

                case gh::CMD::Format:
                    gh::FS.format();
                    gh::FS.end();
                    gh::FS.begin();
                    _answerFiles();
                    break;
#ifndef GH_NO_FETCH
                case gh::CMD::FetchNext:
                    if (_fet_p && _fet_p->client == client) {
                        gh::core::Packet gs;
                        gs.reserve(50);
                        gs.begin();
                        gs.addID(id);
                        gs.addStr(Tag::type, Tag::fetch_chunk);
                        gs.addKey(Tag::data);
                        gs.dquot();
                        _fet_p->writeChunk(&gs);
                        gs.dquot();
                        gs.comma();
                        if (_fet_p->isLast()) {
                            gs.addInt(Tag::last, 1);
                            gs.addInt(Tag::crc32, _fet_p->crc32);
                        }
                        gs.end();
                        _answer(gs);
                        if (_fet_p->isLast()) {
                            _fet_p->end();
                            delete _fet_p;
                            _fet_p = nullptr;
                        }
                    } else {
                        _errFetcher(gh::Error::WrongClient);
                    }
                    break;
#endif
#endif  // GH_NO_FS
#endif  // GH_ESP_BUILD
                default:
                    break;
            }
            return;
        }  // size == 4

        _focus.set(conn);
        // PREFIX/ID/CLIENT_ID/CMD/NAME + VALUE
        switch (cmd) {
            case gh::CMD::Data:
                // TODO data
                break;

            case gh::CMD::Unix:  // name = unix
#ifdef GH_ESP_BUILD
                if (_unix_cb) _unix_cb(atoll(name));
#else
                if (_unix_cb) _unix_cb(atol(name));
#endif
                _answerOK();
                break;

            case gh::CMD::Set: {
                if (!_build_cb) return _answerOK();
                gh::Build build(gh::Action::Set, &client, name, value);
                gh::Builder b(&build);
                b._menu = &_menu;
                _build_busy = true;
                _build_cb(b);
                _build_busy = false;

                if (b.isRefresh()) _answerUI();
                else _answerAck(name);

                if (b._needs_update) {
#ifndef GH_NO_GET
                    if (_autoGet_f) _sendGet(name, value);
#endif
                    _sendUpdate(name, value);
                }

            } break;

            case gh::CMD::Get: {
#ifndef GH_NO_GET
                gh::core::Packet gs;
                gs.reserve(50);
                gs.begin();
                gs.addID(id);
                gs.addStr(Tag::type, Tag::get);
                gs.addKey(Tag::value);
                gs.dquot();
                _readBuild(&gs, _client_p, name);
                gs.dquot();
                gs.end();
                _answer(gs);
#endif
            } break;

            case gh::CMD::Read: {
                gh::core::Packet gs;
                _readBuild(&gs, _client_p, name);
#ifndef GH_NO_GET
                if (_autoGet_f) _sendGet(name, gs);
#endif
                _answer(gs);
            } break;

            case gh::CMD::CLI:
                _answerOK();
                if (_cli_cb) {
                    String str(value);
                    _cli_cb(str);
                }
                break;

#ifdef GH_ESP_BUILD
#ifndef GH_NO_FS
            case gh::CMD::Delete:
                gh::FS.remove(name);
                _answerFiles();
                break;

            case gh::CMD::Rename:
                if (gh::FS.rename(name, value)) _answerFiles();
                break;

            case gh::CMD::Create: {  // name == path
                gh::FS.mkdir(value);
                File f = gh::FS.openWrite(name);
                f.close();
                _answerFiles();
            } break;

            case gh::CMD::FsAbort:
                switch (name[0]) {  // name == type
                    case 'u':       // upload
#ifndef GH_NO_UPLOAD
                        if (_upl_p) {
                            _upl_p->abort();
                            delete _upl_p;
                            _upl_p = nullptr;
                        }
#endif
                        break;
                    case 'f':  // fetch/fetch_file
#ifndef GH_NO_FETCH
                        if (_fet_p) {
                            _fet_p->abort();
                            delete _fet_p;
                            _fet_p = nullptr;
                        }
#endif
                        break;
                    case 'o':  // ota
#ifndef GH_NO_OTA
                        if (_ota_p) {
                            _ota_p->abort();
                            delete _ota_p;
                            _ota_p = nullptr;
                        }
#endif
                        break;
                }
                break;

#ifndef GH_NO_FETCH
            case gh::CMD::Fetch:  // name == path
                if (!_fet_p) {
                    _fet_p = new gh::Fetcher(this, client, name, _fetchHook);
                    if (_fet_p) {
                        if (_fet_p->begin()) {
                            gh::core::Packet gs;
                            gs.reserve(50);
                            gs.begin();
                            gs.addID(id);
                            gs.addStr(Tag::type, Tag::fetch_start);
                            gs.addInt(Tag::len, _fet_p->len);
                            gs.end();
                            _answer(gs);
                        } else {
                            _answerError(Tag::fetch_err, _fet_p->error);
                            delete _fet_p;
                            _fet_p = nullptr;
                        }
                    } else {
                        _errFetcher(gh::Error::Memory);
                    }
                } else {
                    _errFetcher(gh::Error::Busy);
                }
                break;
#endif
#ifndef GH_NO_UPLOAD
            case gh::CMD::Upload:  // name == path, value == size
                if (!_upl_p) {
                    _upl_p = new gh::core::Uploader(client, _safe_upl);
                    if (_upl_p) {
                        if (_upl_p->begin(name, atoll(value))) {
                            _answerCmd(Tag::upload_next);
                        } else {
                            _answerError(Tag::upload_err, _upl_p->error);
                            delete _upl_p;
                            _upl_p = nullptr;
                        }
                    } else {
                        _errUploader(gh::Error::Memory);
                    }
                } else {
                    _errUploader(gh::Error::Busy);
                }
                break;

            case gh::CMD::UploadChunk:  // name == next/last/crc, value == data64
                if (_upl_p && _upl_p->client == client) {
                    if (name[0] == 'c') _upl_p->setCRC(value);  // 'crc'
                    else _upl_p->write64(value);

                    if (name[0] == 'l') {  // last
                        _upl_p->end();
                        if (!_upl_p->hasError()) {
                            if (_upload_cb) _upload_cb(_upl_p->path);
                            _answerCmd(Tag::upload_done);
                            delete _upl_p;
                            _upl_p = nullptr;
                            return;
                        }
                    } else {
                        if (!_upl_p->hasError()) _answerCmd(Tag::upload_next);
                    }

                    if (_upl_p->hasError()) {
                        _answerError(Tag::upload_err, _upl_p->error);
                        delete _upl_p;
                        _upl_p = nullptr;
                    }
                } else {
                    _errUploader(gh::Error::WrongClient);
                }
                break;
#endif
#endif  // GH_NO_FS
#ifndef GH_NO_OTA
            case gh::CMD::Ota:  // name == flash/fs
                if (!_ota_p) {
                    _ota_p = new gh::core::Updater(client, &_reason);
                    if (_ota_p) {
                        if (_ota_p->begin(name)) {
                            _answerCmd(Tag::ota_next);
                        } else {
                            _answerError(Tag::ota_err, _ota_p->error);
                            delete _ota_p;
                            _ota_p = nullptr;
                        }
                    } else {
                        _errUpdater(gh::Error::Memory);
                    }
                } else {
                    _errUpdater(gh::Error::Busy);
                }
                break;

            case gh::CMD::OtaChunk:  // name == next/last, value == data
                if (_ota_p && _ota_p->client == client) {
                    _ota_p->write64(value);

                    if (name[0] == 'l') {  // last
                        _ota_p->end();
                        if (!_ota_p->hasError()) {
                            _answerCmd(Tag::ota_done);
                            delete _ota_p;
                            _ota_p = nullptr;
                            return;
                        }
                    } else {
                        if (!_ota_p->hasError()) _answerCmd(Tag::ota_next);
                    }
                    if (_ota_p->hasError()) {
                        _answerError(Tag::ota_err, _ota_p->error);
                        delete _ota_p;
                        _ota_p = nullptr;
                    }
                } else {
                    _errUpdater(gh::Error::WrongClient);
                }
                break;
#endif
#ifndef GH_NO_OTA_URL
            case gh::CMD::OtaUrl:  // name == type, value == url
                if (!_otaurl_p) {
                    _otaurl_p = new gh::core::OtaUrl(value, name, client);
                    if (!_otaurl_p) _answerError(Tag::ota_url_err, gh::Error::Memory);
                } else {
                    _answerError(Tag::ota_url_err, gh::Error::Busy);
                }
                break;
#endif
#endif  // GH_ESP_BUILD
            default:
                break;
        }
    }
};

}  // namespace gh::core