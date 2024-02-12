#include "datatypes.h"

void gh::core::strToVar(const char* str, gh::core::AnyPtr& data) {
    if (!data.ptr || data.type == gh::Type::NULL_T) return;

    switch (data.type) {
        case gh::Type::STR_T:
            *(String*)data.ptr = str;
            break;
        case gh::Type::CSTR_T:
            strcpy((char*)data.ptr, str);
            break;

        case gh::Type::BOOL_T:
            *(bool*)data.ptr = (str[0] == '1');
            break;

        case gh::Type::INT8_T:
            *(int8_t*)data.ptr = atoi(str);
            break;

        case gh::Type::UINT8_T:
            *(uint8_t*)data.ptr = atoi(str);
            break;

        case gh::Type::INT16_T:
            *(int16_t*)data.ptr = atoi(str);
            break;
        case gh::Type::UINT16_T:
            *(uint16_t*)data.ptr = atoi(str);
            break;

        case gh::Type::INT32_T:
            *(int32_t*)data.ptr = atol(str);
            break;
        case gh::Type::UINT32_T:
            *(uint32_t*)data.ptr = atol(str);
            break;

        case gh::Type::FLOAT_T:
            *(float*)data.ptr = atof(str);
            break;
        case gh::Type::DOUBLE_T:
            *(double*)data.ptr = atof(str);
            break;

        case gh::Type::COLOR_T:
            ((gh::Color*)data.ptr)->setHEX(atol(str));
            break;
        case gh::Type::FLAGS_T:
            ((gh::Flags*)data.ptr)->flags = atoi(str);
            break;
        case gh::Type::POS_T: {
            uint32_t xy = atol(str);
            ((gh::Pos*)data.ptr)->_changed = true;
            ((gh::Pos*)data.ptr)->x = xy >> 16;
            ((gh::Pos*)data.ptr)->y = xy & 0xffff;
        } break;
        case gh::Type::BTN_T:
            if (str[0] == '2') {
                ((gh::Button*)data.ptr)->_changed = 0;
                ((gh::Button*)data.ptr)->_clicked = 1;
            } else {
                ((gh::Button*)data.ptr)->_changed = 1;
                ((gh::Button*)data.ptr)->_clicked = 0;
                ((gh::Button*)data.ptr)->_state = str[0] & 0xF;
            }
            break;

        default:
            break;
    }
}

void gh::core::varToStr(gh::core::Packet* s, const gh::core::AnyPtr& data) {
    if (!data.ptr || data.type == gh::Type::NULL_T) return;

    switch (data.type) {
        case gh::Type::STR_T:
            s->addEscape(((String*)data.ptr)->c_str());
            break;
        case gh::Type::CSTR_T:
            s->addEscape((const char*)data.ptr);
            break;

        case gh::Type::BOOL_T:
            *s += *(bool*)data.ptr;
            break;

        case gh::Type::INT8_T:
            *s += *(int8_t*)data.ptr;
            break;

        case gh::Type::UINT8_T:
            *s += *(uint8_t*)data.ptr;
            break;

        case gh::Type::INT16_T:
            *s += *(int16_t*)data.ptr;
            break;
        case gh::Type::UINT16_T:
            *s += *(uint16_t*)data.ptr;
            break;

        case gh::Type::INT32_T:
            *s += *(int32_t*)data.ptr;
            break;
        case gh::Type::UINT32_T:
            *s += *(uint32_t*)data.ptr;
            break;

        case gh::Type::FLOAT_T:
            if (isnan(*(float*)data.ptr)) *s += '0';
            else *s += *(float*)data.ptr;
            break;
        case gh::Type::DOUBLE_T:
            if (isnan(*(double*)data.ptr)) *s += '0';
            else *s += *(double*)data.ptr;
            break;

        case gh::Type::COLOR_T:
            *s += ((gh::Color*)data.ptr)->getHEX();
            break;
        case gh::Type::FLAGS_T:
            *s += ((gh::Flags*)data.ptr)->flags;
            break;

        case gh::Type::LOG_T:
            ((gh::Log*)data.ptr)->read(s, true);
            break;

        case gh::Type::TEXT_T:
            s->addEscape(*(gh::core::AnyText*)data.ptr);
            break;

#ifndef GH_NO_PAIRS
        case gh::Type::PAIR_T: {
            Pair_t* p = (Pair_t*)data.ptr;
            char* end = (char*)(p->val) + p->val_len;
            char buf = *end;
            *end = 0;
            *s += p->val;
            *end = buf;
        } break;
#endif

        default:
            break;
    }
}