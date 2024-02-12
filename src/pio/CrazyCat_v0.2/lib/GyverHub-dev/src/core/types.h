#pragma once

namespace gh {

// ошибка
enum class Error : uint8_t {
    None,  // нет ошибки

    OpenFile,   // ошибка открытия файла
    FreeSpace,  // нет свободного места
    CrcMiss,    // ошибка контрольной суммы
    SizeMiss,   // не совпал размер

    Start,  // ошибка старта
    Write,  // ошибка записи
    End,    // ошибка завершения

    Abort,
    Timeout,      // таймаут соединения
    Busy,         // загрузчик занят другим клиентом
    Memory,       // невозможно выделить память
    WrongClient,  // не тот клиент
    Forbidden,    // запрещено в request
    Disabled,     // модуль отключен
};

enum class Align : uint8_t {
    Left,
    Center,
    Right,
};

// тип данных
enum class Type : uint8_t {
    NULL_T,
    STR_T,     // String
    CSTR_T,    // char[]
    BOOL_T,    // bool
    INT8_T,    // int8_t
    UINT8_T,   // uint8_t
    INT16_T,   // int16_t
    UINT16_T,  // uint16_t
    INT32_T,   // int32_t
    UINT32_T,  // uint32_t
    FLOAT_T,   // float
    DOUBLE_T,  // double
    COLOR_T,   // gh::Colors
    FLAGS_T,   // gh::Flags
    POS_T,     // gh::Pos
    BTN_T,     // gh::Button
    PAIRS_T,   // Pairs
    PAIR_T,    // Pair_t
    LOG_T,     // gh::Log
    TEXT_T,    // gh::core::AnyText
};

// тип билда
enum class Action : uint8_t {
    Count,
    UI,
    Read,
    Set,
    Get,
    None,
};

// источник данных
enum class Source : uint8_t {
    ESP,
    Manual,
    Stream,
    System,
};

// причина перезагрузки
enum class Reboot : uint8_t {
    None,
    Button,
    Ota,
    OtaUrl,
};

#define GH_CONN_AMOUNT 6

// тип подключения
enum class Connection : uint8_t {
    Serial,
    Bluetooth,
    WS,
    MQTT,
    HTTP,
    Telegram,
    System,
};

// системные события
enum class CMD : uint8_t {
    UI,
    Ping,
    Unfocus,
    Info,
    Files,
    Format,
    Reboot,
    FetchNext,
    Data,
    Set,
    Get,
    Read,
    CLI,
    Delete,
    Rename,
    Create,
    FsAbort,
    Fetch,
    Upload,
    UploadChunk,
    Ota,
    OtaChunk,
    OtaUrl,
    Unix,
    Search,
    Discover,
    Unknown,
};

}  // namespace gh