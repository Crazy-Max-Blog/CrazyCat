#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/request.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "utils/anyText.h"

namespace gh::core {

typedef void (*ParseHook)(void* hubptr, char* url, char* value, gh::Connection conn, gh::Source src);
typedef void (*AnswerHook)(void* hubptr, gh::core::AnyText text, bool stop);
typedef void (*SendHook)(void* hubptr, String* text, Client* client);
#ifdef GH_ESP_BUILD
typedef void (*FocusHook)(void* hubptr, gh::Connection conn, bool focus);
typedef bool (*RequestHook)(void* hubptr, gh::Request* request);
typedef void (*UploadHook)(void* hubptr, String& path);
#endif

}  // namespace gh::core