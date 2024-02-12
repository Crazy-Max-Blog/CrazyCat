#include "types_p.h"

GH_PGM(_GH_STA0, "UI");
GH_PGM(_GH_STA1, "Ping");
GH_PGM(_GH_STA2, "Unfocus");
GH_PGM(_GH_STA3, "Info");
GH_PGM(_GH_STA4, "Files");
GH_PGM(_GH_STA5, "Format");
GH_PGM(_GH_STA6, "Reboot");
GH_PGM(_GH_STA7, "FetchNext");
GH_PGM(_GH_STA8, "Data");
GH_PGM(_GH_STA9, "Set");
GH_PGM(_GH_STA10, "Get");
GH_PGM(_GH_STA11, "Read");
GH_PGM(_GH_STA12, "CLI");
GH_PGM(_GH_STA13, "Delete");
GH_PGM(_GH_STA14, "Rename");
GH_PGM(_GH_STA15, "Create");
GH_PGM(_GH_STA16, "FsAbort");
GH_PGM(_GH_STA17, "Fetch");
GH_PGM(_GH_STA18, "Upload");
GH_PGM(_GH_STA19, "UploadChunk");
GH_PGM(_GH_STA20, "Ota");
GH_PGM(_GH_STA21, "OtaChunk");
GH_PGM(_GH_STA22, "OtaUrl");
GH_PGM(_GH_STA23, "Search");
GH_PGM(_GH_STA24, "Discover");
GH_PGM(_GH_STA25, "Unknown");
GH_PGM_LIST(_GH_cmd_list, _GH_STA0, _GH_STA1, _GH_STA2, _GH_STA3, _GH_STA4, _GH_STA5, _GH_STA6, _GH_STA7, _GH_STA8, _GH_STA9, _GH_STA10, _GH_STA11, _GH_STA12, _GH_STA13, _GH_STA14, _GH_STA15, _GH_STA16, _GH_STA17, _GH_STA18, _GH_STA19, _GH_STA20, _GH_STA21, _GH_STA22, _GH_STA23, _GH_STA24, _GH_STA25);

FSTR gh::readCMD(gh::CMD n) {
    return (FSTR)pgm_read_ptr(_GH_cmd_list + (uint8_t)n);
}

GH_PGM(_GH_CON0, "Serial");
GH_PGM(_GH_CON1, "Bluetooth");
GH_PGM(_GH_CON2, "WS");
GH_PGM(_GH_CON3, "MQTT");
GH_PGM(_GH_CON4, "HTTP");
GH_PGM(_GH_CON5, "Telegram");
GH_PGM(_GH_CON6, "System");
GH_PGM_LIST(_GH_con_list, _GH_CON0, _GH_CON1, _GH_CON2, _GH_CON3, _GH_CON4, _GH_CON5, _GH_CON6);

FSTR gh::readConnection(gh::Connection n) {
    return (FSTR)pgm_read_ptr(_GH_con_list + (uint8_t)n);
}

GH_PGM(_GH_BLD0, "Count");
GH_PGM(_GH_BLD1, "UI");
GH_PGM(_GH_BLD2, "Read");
GH_PGM(_GH_BLD3, "Set");
GH_PGM(_GH_BLD4, "Get");
GH_PGM(_GH_BLD5, "None");
GH_PGM_LIST(_GH_bld_list, _GH_BLD0, _GH_BLD1, _GH_BLD2, _GH_BLD3, _GH_BLD4, _GH_BLD5);

FSTR gh::readAction(gh::Action n) {
    return (FSTR)pgm_read_ptr(_GH_bld_list + (uint8_t)n);
}

GH_PGM(_GH_RES0, "None");
GH_PGM(_GH_RES1, "Button");
GH_PGM(_GH_RES2, "Ota");
GH_PGM(_GH_RES3, "OtaUrl");
GH_PGM_LIST(_GH_res_list, _GH_RES0, _GH_RES1, _GH_RES2, _GH_RES3);

FSTR gh::readReboot(gh::Reboot n) {
    return (FSTR)pgm_read_ptr(_GH_res_list + (uint8_t)n);
}

GH_PGM(_GH_SRC0, "ESP");
GH_PGM(_GH_SRC1, "Manual");
GH_PGM(_GH_SRC2, "Stream");
GH_PGM(_GH_SRC3, "System");
GH_PGM_LIST(_GH_src_list, _GH_SRC0, _GH_SRC1, _GH_SRC2, _GH_SRC3);

FSTR gh::readSource(gh::Source n) {
    return (FSTR)pgm_read_ptr(_GH_src_list + (uint8_t)n);
}