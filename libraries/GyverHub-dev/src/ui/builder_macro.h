#pragma once

#ifndef GH_NO_PAIRS
#endif

#define GH_ROW(b, w, code) \
    do {                   \
        b.beginRow(w);     \
        code;              \
        b.endRow();        \
    } while (0);

#define GH_COL(b, w, code) \
    do {                   \
        b.beginCol(w);     \
        code;              \
        b.endCol();        \
    } while (0);

#define GH_BUILD_TEXT(_fname, _tag)                                   \
    Widget& _fname(AnyText text = AnyText()) {                        \
        AnyText name;                                                 \
        return _varAndType(_tag, name, &text);                        \
    }                                                                 \
    Widget& _fname##_(AnyText name, AnyText text = AnyText()) {       \
        return _varAndType(_tag, name, &text);                        \
    }                                                                 \
    Widget& _fname##_(AnyText name, void* pairs) {                    \
        return _varAndType(_tag, name, AnyPtr(pairs, Type::PAIRS_T)); \
    }

#define GH_BUILD_TEXT_NONAME(_fname, _tag)     \
    Widget& _fname(AnyText text = AnyText()) { \
        AnyText name;                          \
        return _varAndType(_tag, name, &text); \
    }

#define GH_BUILD_VAR(_fname, _tag)                           \
    Widget& _fname(AnyPtr ptr = AnyPtr()) {                  \
        AnyText name;                                        \
        return _varAndType(_tag, name, ptr);                 \
    }                                                        \
    Widget& _fname##_(AnyText name, AnyPtr ptr = AnyPtr()) { \
        return _varAndType(_tag, name, ptr);                 \
    }

#define GH_BUILD_VAR_TYPE(_fname, _type, _tag)             \
    Widget& _fname(_type ptr = nullptr) {                  \
        AnyText name;                                      \
        return _varAndType(_tag, name, ptr);               \
    }                                                      \
    Widget& _fname##_(AnyText name, _type ptr = nullptr) { \
        return _varAndType(_tag, name, ptr);               \
    }

#define GH_BUILD_CUSTOM(_fname, _tag)                                         \
    Widget& _fname(AnyText func, AnyPtr ptr) {                                \
        AnyText name;                                                         \
        return _varAndType(_tag, name, ptr, func);                            \
    }                                                                         \
    Widget& _fname(AnyText func, AnyText text = AnyText()) {                  \
        AnyText name;                                                         \
        return _varAndType(_tag, name, &text, func);                          \
    }                                                                         \
    Widget& _fname##_(AnyText name, AnyText func, AnyPtr ptr) {               \
        return _varAndType(_tag, name, ptr, func);                            \
    }                                                                         \
    Widget& _fname##_(AnyText name, AnyText func, AnyText text = AnyText()) { \
        return _varAndType(_tag, name, &text, func);                          \
    }

#define GH_BUILD_CONTAINER(_fname, _tag)     \
    bool begin##_fname(uint16_t width = 1) { \
        return _beginContainer(width, _tag); \
    }                                        \
    void end##_fname() {                     \
        _endContainer();                     \
    }                                        \
    bool next##_fname(uint16_t width) {      \
        _endContainer();                     \
        return _beginContainer(width, _tag); \
    }

#define _GH_NEWLINE_DUMMY