#pragma once

#ifndef GH_NO_PAIRS
#include <Pairs.h>
#endif

#include <Arduino.h>

#include "anyText.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "ui/button.h"
#include "ui/color.h"
#include "ui/flags.h"
#include "ui/log.h"
#include "ui/pos.h"
#include "utils/datatypes.h"

namespace gh::core {

class AnyPtr {
   public:
    AnyPtr() {}
    AnyPtr(void* ptr, gh::Type type) : ptr(ptr), type(type) {}

    AnyPtr(bool* ptr) : ptr(ptr), type(gh::Type::BOOL_T) {}
    AnyPtr(int8_t* ptr) : ptr(ptr), type(gh::Type::INT8_T) {}
    AnyPtr(uint8_t* ptr) : ptr(ptr), type(gh::Type::UINT8_T) {}
    AnyPtr(int16_t* ptr) : ptr(ptr), type(gh::Type::INT16_T) {}
    AnyPtr(uint16_t* ptr) : ptr(ptr), type(gh::Type::UINT16_T) {}
    AnyPtr(int32_t* ptr) : ptr(ptr), type(gh::Type::INT32_T) {}
    AnyPtr(uint32_t* ptr) : ptr(ptr), type(gh::Type::UINT32_T) {}

    AnyPtr(float* ptr) : ptr(ptr), type(gh::Type::FLOAT_T) {}
    AnyPtr(double* ptr) : ptr(ptr), type(gh::Type::DOUBLE_T) {}

    AnyPtr(char* ptr) : ptr(ptr), type(gh::Type::CSTR_T) {}
    AnyPtr(String* ptr) : ptr(ptr), type(gh::Type::STR_T) {}

    AnyPtr(gh::Color* ptr) : ptr(ptr), type(gh::Type::COLOR_T) {}
    AnyPtr(gh::Pos* ptr) : ptr(ptr), type(gh::Type::POS_T) {}
    AnyPtr(gh::Flags* ptr) : ptr(ptr), type(gh::Type::FLAGS_T) {}
    AnyPtr(gh::Log* ptr) : ptr(ptr), type(gh::Type::LOG_T) {}
    AnyPtr(gh::Button* ptr) : ptr(ptr), type(gh::Type::BTN_T) {}
    AnyPtr(gh::core::AnyText* ptr) {
        if (ptr && ptr->str) {
            this->ptr = ptr;
            type = gh::Type::TEXT_T;
        }
    }

#ifndef GH_NO_PAIRS
    AnyPtr(Pairs* ptr) : ptr(ptr), type(gh::Type::PAIRS_T) {}
    AnyPtr(Pair_t* ptr) : ptr(ptr), type(gh::Type::PAIR_T) {}
#endif

    void* ptr = nullptr;
    gh::Type type = gh::Type::NULL_T;
    bool empty = 0;
};

}  // namespace gh::core