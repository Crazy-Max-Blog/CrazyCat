#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/core_class.h"
#include "core/fs.h"
#include "core/hooks.h"
#include "core/hub_class.h"
#include "core/types.h"
#include "esp/sync/http_class.h"
#include "esp/transfer/timer.h"
#include "hub_macro.hpp"
#include "utils/b64.h"
#include "utils/crc32.h"

namespace gh {
class Fetcher;

namespace core {
typedef void (*FetchHook)(void* hubptr, Fetcher* fetcher);
}

using gh::Error;
using gh::core::FetchHook;

#if !defined(GH_NO_FS) && (!defined(GH_NO_FETCH) || !defined(GH_NO_HTTP_FETCH))
class Fetcher {
    friend class gh::core::HubCore;
    friend class ::GyverHub;
    friend class gh::core::HTTP;

   public:
    Fetcher(void* hub,
            gh::Client& client,
            const char* path,
            FetchHook fh) : client(client),
                            path(path),
                            hubptr(hub),
                            fetch_h(fh) {}

    // отправить файл по пути
    void fetchFile(const char* path) {
        cleanup();
        file = gh::FS.openRead(path);
        if (file) len = file.size();
    }

    // отправить файл
    template <typename T>
    void fetchFile(T& file) {
        cleanup();
        this->file = file;
        if (file) len = file.size();
    }

    // отправить сырые данные
    void fetchBytes(uint8_t* bytes, uint32_t len) {
        cleanup();
        this->bytes = bytes;
        this->len = len;
        pgm = false;
    }

    // отправить сырые данные из PGM
    void fetchBytes_P(const uint8_t* bytes, uint32_t len) {
        cleanup();
        this->bytes = bytes;
        this->len = len;
        pgm = true;
    }

    bool start;

    // клиент
    gh::Client client;

    // путь к скачиваемому файлу
    const String path;

   private:
    bool begin() {
        tmr.start();
        start = 1;
        fetch_h(hubptr, this);

        if (!file && !bytes) {
            file = gh::FS.openRead(path);
            if (file) len = file.size();
        }

        if (!file && !bytes) setError(Error::Start);
        return !hasError();
    }

    bool writeChunk(gh::core::Packet* gs) {
        tmr.start();
        if (hasError()) return 0;
        if (file) {
            uint8_t buf[GH_FETCH_CHUNK_SIZE * 3 / 4];
            size_t len = file.readBytes((char*)buf, sizeof(buf));
            if (sizeof(buf) != len) lastChunk = 1;
            gh::b64::encode(gs, buf, len);
            crc32 = gh::core::crc32(crc32, buf, len);
        } else if (bytes) {
            uint32_t curlen = GH_FETCH_CHUNK_SIZE * 3 / 4;
            if (len >= curlen) {
                len -= curlen;
                if (!len) lastChunk = 1;
            } else {
                curlen = len;
                lastChunk = 1;
            }
            gh::b64::encode(gs, (uint8_t*)bytes + b_index, curlen, pgm);
            crc32 = gh::core::crc32(crc32, (uint8_t*)bytes + b_index, curlen, pgm);
            b_index += curlen;
        }
        return 1;
    }

    bool isLast() {
        return lastChunk;
    }

    bool end() {
        start = 0;
        fetch_h(hubptr, this);
        cleanup();
        return !hasError();
    }

    bool timeout() {
        if (tmr.tick(GH_CONN_TOUT * 1000)) {
            setError(Error::Timeout);
            start = 0;
            fetch_h(hubptr, this);
            cleanup();
        }
        return hasError();
    }

    void abort() {
        setError(Error::Abort);
        start = 0;
        fetch_h(hubptr, this);
        cleanup();
    }

    gh::Error error = gh::Error::None;

    bool hasError() {
        return error != gh::Error::None;
    }

    void setError(gh::Error err) {
        error = err;
    }

   private:
    void* hubptr;
    FetchHook fetch_h;

    gh::core::Timer tmr;
    File file;
    bool pgm = false;
    const uint8_t* bytes = nullptr;
    uint32_t len = 0;
    uint32_t b_index = 0;
    uint32_t crc32 = 0;
    bool lastChunk = 0;

    void cleanup() {
        if (file) file.close();
        bytes = nullptr;
        len = 0;
        b_index = 0;
    }
};
#endif
}  // namespace gh