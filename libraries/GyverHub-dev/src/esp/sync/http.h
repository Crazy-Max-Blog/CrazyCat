#pragma once
#include <Arduino.h>

#include "hub_macro.hpp"

#ifdef GH_ESP_BUILD

#include "core/core_class.h"
#include "core/fs.h"
#include "core/hooks.h"
#include "core/hub_class.h"
#include "core/modules.h"
#include "core/request.h"
#include "core/types.h"
#include "esp/upload_portal.h"
#include "utils/anyText.h"
#include "utils/crc32.h"
#include "utils/mime.h"

#ifndef GH_NO_HTTP_TRANSFER
#ifndef GH_NO_HTTP_OTA
#include "esp/transfer/updater.h"
#endif

#ifndef GH_NO_FS

#ifndef GH_NO_HTTP_FETCH
#include "esp/transfer/fetcher.h"
#endif

#ifndef GH_NO_HTTP_UPLOAD
#include "esp/transfer/uploader.h"
#endif

#endif  // GH_NO_FS
#endif  // GH_NO_HTTP_TRANSFER

#ifdef ESP8266
#include <ESP8266WebServer.h>
#define GH_SERVER_T ESP8266WebServer
#else
#include <WebServer.h>
#define GH_SERVER_T WebServer
#endif

#ifndef GH_NO_HTTP_DNS
#include <DNSServer.h>
#endif

#ifdef GH_INCLUDE_PORTAL
#include "esp/esp_h/index.h"
#include "esp/esp_h/script.h"
#include "esp/esp_h/style.h"
#endif

namespace gh::core {

class HTTP {
    friend class HubCore;
    friend class ::GyverHub;

   public:
    GH_SERVER_T server;

    // ======================= PRIVATE =======================
   private:
    HTTP() : server(GH_HTTP_PORT) {}

    void setup(void* hub, ParseHook prh, FetchHook fh, RequestHook rh, UploadHook uh, bool* safe_upl, void* mod, gh::Reboot* reason) {
        this->hubptr = hub;
        this->parse_h = prh;
        this->fetch_h = fh;
        this->request_h = rh;
        this->upload_h = uh;
        this->safe_upl = safe_upl;
        this->modules = mod;
        this->reason = reason;
    }

    void answer(gh::core::AnyText& text) {
        handled = true;
        if (text.pgm) server.sendContent_P(text.str, text.len);
        else server.sendContent(text.str);
    }

    void begin() {
        server.onNotFound([this]() {
            // command uri
            if (server.uri().startsWith(F("/hub/"))) {
                handled = false;
                server.setContentLength(CONTENT_LENGTH_UNKNOWN);
                server.send(200, "text/plain");
                parse_h(hubptr, ((char*)server.uri().c_str()) + 5, nullptr, gh::Connection::HTTP, gh::Source::ESP);  // +5 == "/hub/"
                server.sendContent("");
                server.client().stop();
                if (handled) return;
            }

// fetch file from GH_PUBLIC_PATH
#if !defined(GH_NO_HTTP_PUBLIC) && !defined(GH_NO_HTTP_FETCH) && !defined(GH_NO_HTTP_TRANSFER) && !defined(GH_NO_FS)
            else if (server.uri().indexOf('.') > 0) {
                String path(GH_PUBLIC_PATH);
                path += server.uri();
                _handleFetch(path);
                return;
            }
#endif

// captive portal
#ifndef GH_NO_HTTP_DNS
            else {
#if defined(GH_INCLUDE_PORTAL)
                gzip_h();
                cache_h();
                server.send_P(200, "text/html", (PGM_P)hub_index_h, (size_t)hub_index_h_len);
                return;

#elif defined(GH_FILE_PORTAL)
                File f = gh::FS.openRead(F("/hub/index.html.gz"));
                if (f) server.streamFile(f, "text/html");
                else _send_portal();
                return;
#endif
            }
#endif
            server.send(404);
        });  // onNotFound

// fetch /hub/fetch?path=...&client_id=...
#if !defined(GH_NO_HTTP_FETCH) && !defined(GH_NO_HTTP_TRANSFER) && !defined(GH_NO_FS)
        server.on("/hub/fetch", HTTP_GET, [this]() {
            String path = server.arg(F("path"));
            _handleFetch(path);
        });
#endif

// upload /hub/upload_portal
#if !defined(GH_NO_HTTP_UPLOAD_PORTAL) && !defined(GH_NO_FS)
        server.on("/hub/upload_portal", HTTP_GET, [this]() {
            _send_portal();
        });

        server.on(
            "/hub/upload_portal", HTTP_POST, [this]() { _send_portal(); }, [this]() {
                HTTPUpload& upload = server.upload();

                switch (upload.status) {
                    case UPLOAD_FILE_START: {
                        String path(F("/hub/"));
                        gh::FS.mkdir(path.c_str());
                        path += upload.filename;
                        file_upl = gh::FS.openWrite(path.c_str());
                        if (!file_upl) server.send(400);
                    } break;

                    case UPLOAD_FILE_WRITE:
                        if (file_upl) {
                            size_t bytesWritten = file_upl.write(upload.buf, upload.currentSize);
                            if (bytesWritten != upload.currentSize) server.send(400);
                        }
                        break;

                    case UPLOAD_FILE_END:
                    case UPLOAD_FILE_ABORTED:
                        if (file_upl) file_upl.close();
                        break;
                } });
#endif

// upload /hub/upload?path=...&crc32=...&client_id=...&size=...
#if !defined(GH_NO_HTTP_UPLOAD) && !defined(GH_NO_HTTP_TRANSFER) && !defined(GH_NO_FS)
        server.on(
            "/hub/upload", HTTP_POST, [this]() {
                    if (_upl_p) {
                        if (_upl_p->hasError()) _err(_upl_p->error);
                        else _ok();
                        delete _upl_p;
                        _upl_p = nullptr;
                    } else _err(gh::Error::End); }, [this]() {
#ifndef GH_NO_MODULES
                if (!((gh::core::Modules*)modules)->read(gh::Module::ModUpload)) return _err(gh::Error::Disabled);
#endif
                HTTPUpload& upload = server.upload();
                gh::Client client(gh::Connection::HTTP, server.arg(F("client_id")).c_str(), gh::Source::ESP);

                switch (upload.status) {
                    case UPLOAD_FILE_START:
                        if (!_upl_p) {
                            String path = server.arg(F("path"));
                            String crc = server.arg(F("crc32"));
#ifndef GH_NO_REQUEST
                            gh::Request req(&client, gh::CMD::Upload, path.c_str());
                            if (!request_h(hubptr, &req)) return _err(gh::Error::Forbidden);
#endif

                            uint32_t size = (uint32_t)atoll(server.arg(F("size")).c_str());
                            _upl_p = new gh::core::Uploader(client, *safe_upl);
                            if (_upl_p) {
                                _upl_p->setCRC(crc.c_str());
                                if (!_upl_p->begin(path.c_str(), size)) {
                                    _err(_upl_p->error);
                                    delete _upl_p;
                                    _upl_p = nullptr;
                                }
                            } else _err(gh::Error::Memory);
                        } else _err(gh::Error::Busy);
                        break;

                    case UPLOAD_FILE_WRITE:
                    case UPLOAD_FILE_END:
                        if (_upl_p) {
                            if (upload.status == UPLOAD_FILE_WRITE) {
                                if (!_upl_p->write(upload.buf, upload.currentSize)) _err(_upl_p->error);
                            } else {    // UPLOAD_FILE_END
                                if (_upl_p->end()) upload_h(hubptr, _upl_p->path);
                                else _err(_upl_p->error);
                            }
                        } else _err(gh::Error::WrongClient);
                        break;

                    case UPLOAD_FILE_ABORTED:
                        if (_upl_p) _upl_p->abort();
                        break;
                } });
#endif

// ota /hub/ota?type=...&client_id=...
#if !defined(GH_NO_HTTP_OTA) && !defined(GH_NO_HTTP_TRANSFER)
        server.on(
            "/hub/ota", HTTP_POST, [this]() {
                    server.sendHeader(F("Connection"), F("close"));
                    if (_ota_p) {
                        if (_ota_p->hasError()) _err(_ota_p->error);
                        else _ok();
                        delete _ota_p;
                        _ota_p = nullptr;
                    } else _err(gh::Error::End); },
            [this]() {
#ifndef GH_NO_MODULES
                if (!((gh::core::Modules*)modules)->read(gh::Module::ModOta)) return _err(gh::Error::Disabled);
#endif
                HTTPUpload& upload = server.upload();
                gh::Client client(gh::Connection::HTTP, server.arg(F("client_id")).c_str(), gh::Source::ESP);

                switch (upload.status) {
                    case UPLOAD_FILE_START:
                        if (!_ota_p) {
                            String type = server.arg(F("type"));
#ifndef GH_NO_REQUEST
                            gh::Request req(&client, gh::CMD::Upload, type.c_str());
                            if (!request_h(hubptr, &req)) _err(gh::Error::Forbidden);
#endif

                            _ota_p = new gh::core::Updater(client, reason);
                            if (_ota_p) {
                                if (!_ota_p->begin(type.c_str())) {
                                    _err(_ota_p->error);
                                    delete _ota_p;
                                    _ota_p = nullptr;
                                }
                            } else _err(gh::Error::Memory);
                        } else _err(gh::Error::Busy);
                        break;

                    case UPLOAD_FILE_WRITE:
                    case UPLOAD_FILE_END:
                        if (_ota_p) {
                            if (upload.status == UPLOAD_FILE_WRITE) {
                                if (!_ota_p->write(upload.buf, upload.currentSize)) _err(_ota_p->error);
                            } else {
                                if (!_ota_p->end()) _err(_ota_p->error);
                            }
                        } else _err(gh::Error::WrongClient);
                        break;

                    case UPLOAD_FILE_ABORTED:
                        if (_ota_p) _ota_p->abort();
                        break;
                }
                yield();
            });
#endif

// PORTAL_H
#if defined(GH_INCLUDE_PORTAL)
        server.on("/", [this]() {
            gzip_h();
            cache_h();
            server.send_P(200, "text/html", (PGM_P)hub_index_h, (size_t)hub_index_h_len);
        });
        server.on("/script.js", [this]() {
            gzip_h();
            cache_h();
            server.send_P(200, "text/javascript", (PGM_P)hub_script_h, (size_t)hub_script_h_len);
        });
        server.on("/style.css", [this]() {
            gzip_h();
            cache_h();
            server.send_P(200, "text/css", (PGM_P)hub_style_h, (size_t)hub_style_h_len);
        });

// PORTAL FILE
#elif defined(GH_FILE_PORTAL) && !defined(GH_NO_FS)
        server.on("/", [this]() {
            File f = gh::FS.openRead(F("/hub/index.html.gz"));
            if (f) server.streamFile(f, "text/html");
            else _send_portal();
        });
        server.on("/script.js", [this]() {
            cache_h();
            File f = gh::FS.openRead(F("/hub/script.js.gz"));
            if (f) server.streamFile(f, F("text/javascript"));
            else server.send(404);
        });
        server.on("/style.css", [this]() {
            cache_h();
            File f = gh::FS.openRead(F("/hub/style.css.gz"));
            if (f) server.streamFile(f, F("text/css"));
            else server.send(404);
        });
#endif

// DNS for captive portal
#if (defined(GH_INCLUDE_PORTAL) || defined(GH_FILE_PORTAL)) && !defined(GH_NO_HTTP_DNS)
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            dns_f = 1;
            dns.start(53, "*", WiFi.softAPIP());
        }
#endif

        server.begin(GH_HTTP_PORT);
        server.enableCORS(true);
    }

    void end() {
        server.stop();
#ifndef GH_NO_HTTP_DNS
        if (dns_f) dns.stop();
#endif
    }

    void tick() {
        server.handleClient();
#ifndef GH_NO_HTTP_DNS
        if (dns_f) dns.processNextRequest();
#endif

#if !defined(GH_NO_HTTP_UPLOAD) && !defined(GH_NO_HTTP_TRANSFER) && !defined(GH_NO_FS)
        if (_upl_p && _upl_p->timeout()) {
            delete _upl_p;
            _upl_p = nullptr;
        }
#endif
#if !defined(GH_NO_HTTP_OTA) && !defined(GH_NO_HTTP_TRANSFER) && !defined(GH_NO_FS)
        if (_ota_p && _ota_p->timeout()) {
            delete _upl_p;
            _upl_p = nullptr;
        }
#endif
    }

    // ======================= PRIVATE =======================
    void* hubptr;
    ParseHook parse_h;
    FetchHook fetch_h;
    RequestHook request_h;
    UploadHook upload_h;
    void* modules;
    gh::Reboot* reason;
    bool handled = false;
    bool* safe_upl;

    void gzip_h() {
        server.sendHeader(F("Content-Encoding"), F("gzip"));
    }
    void cache_h() {
        server.sendHeader(F("Cache-Control"), F(GH_CACHE_PRD));
    }

#if !defined(GH_NO_HTTP_FETCH) && !defined(GH_NO_HTTP_TRANSFER) && !defined(GH_NO_FS)
    void _handleFetch(String& path) {
#ifndef GH_NO_MODULES
        if (!((gh::core::Modules*)modules)->read(gh::Module::ModFetch)) return _err(gh::Error::Disabled);
#endif
        String client_id = server.arg(F("client_id"));
        gh::Client client(gh::Connection::HTTP, client_id.c_str(), gh::Source::ESP);
#ifndef GH_NO_REQUEST
        gh::Request req(&client, gh::CMD::Fetch, path.c_str());
        if (!request_h(hubptr, &req)) return _err(gh::Error::Forbidden);
#endif
        gh::Fetcher fetch(hubptr, client, path.c_str(), fetch_h);

        if (fetch.begin()) {
            if (fetch.bytes) {
                server.setContentLength(fetch.len);
                server.send(200, gh::core::getMime(path).c_str(), "");
                if (fetch.pgm) server.sendContent_P((PGM_P)fetch.bytes, fetch.len);
                else server.sendContent((PGM_P)fetch.bytes, fetch.len);
            } else if (fetch.file) {
                server.streamFile(fetch.file, gh::core::getMime(path).c_str());
            }
            fetch.end();
        } else {
            _err(fetch.error);
        }
    }
#endif

    void _err(gh::Error error) {
        char b[3];
        itoa((uint8_t)error, b, DEC);
        server.send(400, F("text/plain"), b);
    }
    void _ok() {
        server.send(200, F("text/plain"), "0");
    }
    void _send_portal() {
        server.send(200, F("text/html"), GH_upload_portal);
    }

#if !defined(GH_NO_HTTP_OTA) && !defined(GH_NO_HTTP_TRANSFER)
    gh::core::Updater* _ota_p = nullptr;
#endif

#ifndef GH_NO_FS

#if !defined(GH_NO_HTTP_UPLOAD) && !defined(GH_NO_HTTP_TRANSFER)
    gh::core::Uploader* _upl_p = nullptr;
#endif

#ifndef GH_NO_HTTP_UPLOAD_PORTAL
    File file_upl;
#endif

#endif  // GH_NO_FS

#ifndef GH_NO_HTTP_DNS
    bool dns_f = false;
    DNSServer dns;
#endif
};
}  // namespace gh::core
#endif