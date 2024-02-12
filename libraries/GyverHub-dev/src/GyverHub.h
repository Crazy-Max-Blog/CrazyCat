#ifndef _GyverHub_h
#define _GyverHub_h

#define GH_LIB_VERSION "v0.1b"  // версия библиотеки
#define GH_API_VERSION "1"      // версия API

// =================================
// таймаут соединения, с
#ifndef GH_CONN_TOUT
#define GH_CONN_TOUT 4
#endif

// размер чанка при скачивании с платы
#ifndef GH_FETCH_CHUNK_SIZE
#define GH_FETCH_CHUNK_SIZE 512
#endif

// размер чанка при загрузке на плату
#ifndef GH_UPL_CHUNK_SIZE
#define GH_UPL_CHUNK_SIZE 512
#endif

// глубина сканирования файловой системы (esp32)
#ifndef GH_FS_DEPTH
#define GH_FS_DEPTH 5
#endif

// ws порт
#ifndef GH_WS_PORT
#define GH_WS_PORT 81
#endif

// период переподключения MQTT
#ifndef GH_MQTT_RECONNECT
#define GH_MQTT_RECONNECT 10000
#endif

// путь к папке с файлами с HTTP доступом
#ifndef GH_PUBLIC_PATH
#define GH_PUBLIC_PATH "/www"
#endif

// http порт
#ifndef GH_HTTP_PORT
#define GH_HTTP_PORT 80
#endif

// период кеширования файлов для портала
#ifndef GH_CACHE_PRD
#define GH_CACHE_PRD "max-age=604800"
#endif

// include
#include "core/hub.h"
#include "hub_macro.hpp"
#include "ui/build_helper.h"
#include "ui/button.h"
#include "ui/canvas.h"
#include "ui/canvas_upd.h"
#include "ui/color.h"
#include "ui/csv.h"
#include "ui/csv_file.h"
#include "ui/flags.h"
#include "ui/log.h"
#include "ui/timer.h"
#include "ui/update.h"

// TODO
namespace GH = gh;

#endif