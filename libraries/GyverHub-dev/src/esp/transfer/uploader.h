#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/fs.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "timer.h"
#include "utils/b64.h"
#include "utils/crc32.h"

namespace gh::core {

using gh::Error;

class Uploader {
   public:
    Uploader(gh::Client& client, bool safe = 0) : client(client), safe(safe) {}

    void setCRC(const char* crc) {
        upl_crc32 = (uint32_t)atoll(crc);
        crc32 = 0;
    }

    bool begin(const char* fpath, uint32_t size) {
        uint32_t space = gh::FS.freeSpace();
        if (!safe) {
            File f = gh::FS.openRead(fpath);
            if (f) {
                space += f.size();  // overwrite
                f.close();
            }
        }

        if (size < space) {
            tmr.start();
            upl_size = size;
            path = fpath;
            gh::FS.mkdir(fpath);
            if (safe) file = gh::FS.openWrite(path + ".tmp");
            else file = gh::FS.openWrite(path);

            if (!file) setError(Error::OpenFile);
        } else {
            setError(Error::FreeSpace);
        }
        return !hasError();
    }

    bool write64(const char* data) {
        if (file) {
            uint16_t len = strlen(data);
            uint16_t declen = gh::b64::decodedLen(data, len);
            uint8_t buf[declen];
            gh::b64::decode(buf, data, len);
            write(buf, declen);
        }
        return !hasError();
    }

    bool write(uint8_t* data, uint32_t len) {
        if (file) {
            tmr.start();
            crc32 = gh::core::crc32(crc32, data, len);
            upl_size -= len;
            if (file.write(data, len) != len) {
                setError(Error::Write);
            }
        }
        return !hasError();
    }

    bool end() {
        if (file) {
            if (upl_size == 0 && upl_crc32 == crc32) {
                file.close();
                if (safe) {
                    gh::FS.remove(path);
                    gh::FS.rename(path + ".tmp", path);
                }
            } else {
                if (upl_crc32 != crc32) setError(Error::CrcMiss);
                else setError(Error::SizeMiss);
                cleanup();
            }
        }
        return !hasError();
    }

    bool timeout() {
        if (tmr.tick(GH_CONN_TOUT * 1000)) {
            setError(Error::Timeout);
            cleanup();
        }
        return hasError();
    }

    void abort() {
        setError(Error::Abort);
        cleanup();
    }

    gh::Error error = gh::Error::None;

    bool hasError() {
        return error != gh::Error::None;
    }

    void setError(gh::Error err) {
        error = err;
    }

    gh::Client client;
    String path;
    uint32_t upl_crc32, crc32;

   private:
    bool safe;
    uint32_t upl_size;
    Timer tmr;
    File file;

    void cleanup() {
        if (file) {
            file.close();
            if (safe) gh::FS.remove(path + ".tmp");
            else gh::FS.remove(path);
        }
    }
};
}  // namespace gh::core