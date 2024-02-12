#include "event_p.h"

GH_PGM(_GH_CMD0, "ui");
GH_PGM(_GH_CMD1, "ping");
GH_PGM(_GH_CMD2, "unfocus");
GH_PGM(_GH_CMD3, "info");
GH_PGM(_GH_CMD4, "files");
GH_PGM(_GH_CMD5, "format");
GH_PGM(_GH_CMD6, "reboot");
GH_PGM(_GH_CMD7, "fetch_next");
GH_PGM(_GH_CMD8, "data");
GH_PGM(_GH_CMD9, "set");
GH_PGM(_GH_CMD10, "get");
GH_PGM(_GH_CMD11, "read");
GH_PGM(_GH_CMD12, "cli");
GH_PGM(_GH_CMD13, "delete");
GH_PGM(_GH_CMD14, "rename");
GH_PGM(_GH_CMD15, "mkfile");
GH_PGM(_GH_CMD16, "fs_abort");
GH_PGM(_GH_CMD17, "fetch");
GH_PGM(_GH_CMD18, "upload");
GH_PGM(_GH_CMD19, "upload_chunk");
GH_PGM(_GH_CMD20, "ota");
GH_PGM(_GH_CMD21, "ota_chunk");
GH_PGM(_GH_CMD22, "ota_url");
GH_PGM(_GH_CMD23, "unix");

#define GH_CMD_LEN 24
GH_PGM_LIST(_GH_cmd_list, _GH_CMD0, _GH_CMD1, _GH_CMD2, _GH_CMD3, _GH_CMD4, _GH_CMD5, _GH_CMD6, _GH_CMD7, _GH_CMD8, _GH_CMD9, _GH_CMD10, _GH_CMD11, _GH_CMD12, _GH_CMD13, _GH_CMD14, _GH_CMD15, _GH_CMD16, _GH_CMD17, _GH_CMD18, _GH_CMD19, _GH_CMD20, _GH_CMD21, _GH_CMD22, _GH_CMD23);

gh::CMD gh::core::getCMD(const char* str) {
    for (int8_t i = 0; i < GH_CMD_LEN; i++) {
        if (!strcmp_P(str, (PGM_P)pgm_read_ptr(_GH_cmd_list + i))) return (gh::CMD)i;
    }
    return gh::CMD::Unknown;
}