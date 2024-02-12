#pragma once
#include <Arduino.h>

#include "core/build.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "ui/builder_class.h"
#include "ui/flag.h"
#include "update_class.h"
#include "utils/anyText.h"
#include "utils/anyptr.h"
#include "utils/datatypes.h"
#include "utils/packet.h"

namespace gh::core {
using gh::core::Tag;

class Widget {
    friend class gh::Builder;
    friend class gh::Update;

   public:
    Widget(gh::core::Packet* str,
           gh::Build* build = nullptr,
           uint32_t* count = nullptr) : sp(str),
                                        _build(build),
                                        _count(count) {
        // update/count/ui
        _allowed = !_build || (build->action == gh::Action::Count || build->action == gh::Action::UI);
    }

    // ===================== CUSTOM ====================
    /**
     * @brief Строковое поле
     *
     * @param key ключ
     * @param str значение
     * @return Widget&
     */
    Widget& fieldStr(const AnyText& key, const AnyText& str) {
        if (_en()) sp->addStrEsc(key, str);
        _process();
        return *this;
    }

    // целочисленное/bool поле
    template <typename T>
    Widget& fieldInt(const AnyText& key, T val) {
        if (_en()) sp->addInt(key, val);
        _process();
        return *this;
    }

    // float поле
    Widget& fieldFloat(const AnyText& key, float val, uint8_t dec = 2) {
        if (_en()) sp->addFloat(key, val, 2);
        _process();
        return *this;
    }

    // ===================== VALUE =====================
    // привязать переменную как значение
    Widget& valueVar(const gh::core::AnyPtr& data) {
        if (_en() && data.ptr) {
            sp->addKey(Tag::value);
            sp->dquot();
            gh::core::varToStr(sp, data);
            sp->dquot();
            sp->comma();
        }
        _process();
        return *this;
    }

    // целочисленное/bool значение
    template <typename T>
    Widget& valueInt(T val) {
        return fieldInt(Tag::value, val);
    }

    // float значение
    Widget& valueFloat(float val, uint8_t dec = 2) {
        return fieldFloat(Tag::value, val, dec);
    }

    // строковое (любой тип) значение
    Widget& valueStr(AnyText str = "") {
        return fieldStr(Tag::value, str);
    }

    // ===================== TEXT =====================
    // текст (любой тип)
    Widget& text(AnyText str) {
        return fieldStr(Tag::text, str);
    }

    // иконка (glyph или unicode) https://fontawesome.com/v5/search?o=r&m=free&s=solid
    Widget& icon(AnyText str) {
        return fieldStr(Tag::icon, str);
    }

    // максимальная длина текста
    Widget& maxLen(uint16_t len) {
        return fieldInt(Tag::maxlen, len);
    }

    // ===================== MISC =====================
    // количество строк поля текста
    Widget& rows(uint16_t rows) {
        return fieldInt(Tag::rows, rows);
    }

    // regex для Input и Pass
    Widget& regex(AnyText str) {
        return fieldStr(Tag::regex, str);
    }

    // выравнивание текста для label/title
    Widget& align(gh::Align align) {
        return fieldInt(Tag::align, (uint8_t)align);
    }

    // минимум, максимум и шаг изменения значения (float)
    Widget& range(float min, float max, float step, uint8_t dec = 2) {
        if (_en()) {
            sp->addFloat(Tag::min, min, dec);
            sp->addFloat(Tag::max, max, dec);
            sp->addFloat(Tag::step, step, dec);
            sp->addInt(Tag::dec, dec);
        }
        _process();
        return *this;
    }

    // минимум, максимум и шаг изменения значения (целые числа)
    template <typename T>
    Widget& range(T min, T max, T step) {
        if (_en()) {
            sp->addInt(Tag::min, min);
            sp->addInt(Tag::max, max);
            sp->addInt(Tag::step, step);
            sp->addInt(Tag::dec, 0);
        }
        _process();
        return *this;
    }

    // единицы измерения
    Widget& unit(AnyText str) {
        return fieldStr(Tag::unit, str);
    }

    // размер шрифта/кнопки
    Widget& fontSize(uint16_t size) {
        return fieldInt(Tag::fsize, size);
    }

    // цвет uint32_t 24 бит
    Widget& color(uint32_t color) {
        if (color != 0xffffffff) return fieldInt(Tag::color, color);
        return *this;
    }

    // цвет gh::Colors
    Widget& color(gh::Colors col) {
        return color((uint32_t)col);
    }

    /**
     * @brief Действие (обновить файл, вызвать Confirm/Prompt)
     * @param bool true - вызвать [умолч. true]
     */
    Widget& action(bool act = 1) {
        return fieldInt(Tag::action, act);
    }

    // ===================== WIDGET =====================
    /**
     * @brief Ширина и высота виджета
     * @param uint16_t ширина, относительная величина
     * @param uint16_t минимальная высота, px [умолч. 0 - авто]
     */
    Widget& size(uint16_t width, uint16_t height = 0) {
        if (_en()) {
            sp->addInt(Tag::wwidth, width);
            if (height) sp->addInt(Tag::wheight, height);
        }
        _process();
        return *this;
    }

    /**
     * @brief Заголовок виджета
     * @param AnyText текст (String, "string", F("string"))
     */
    Widget& label(AnyText str) {
        return fieldStr(Tag::label, str);
    }

    /**
     * @brief Убрать заголовок виджета
     * @param bool true - убрать, false - вернуть [умолч. true]
     */
    Widget& noLabel(bool nolabel = true) {
        return fieldInt(Tag::nolabel, nolabel);
    }

    /**
     * @brief Дополнительный заголовок виджета справа
     * @param AnyText текст (String, "string", F("string"))
     */
    Widget& suffix(AnyText str) {
        return fieldStr(Tag::suffix, str);
    }

    /**
     * @brief Убрать задний фон виджета
     * @param bool true - убрать, false - вернуть [умолч. true]
     */
    Widget& noTab(bool notab = true) {
        return fieldInt(Tag::notab, notab);
    }

    /**
     * @brief Сделать виджет квадратным
     * @param bool true - квадратный, false - обычный [умолч. true]
     */
    Widget& square(bool square = true) {
        return fieldInt(Tag::square, square);
    }

    /**
     * @brief Отключить виджет
     * @param bool true - отключить, false - включить [умолч. true]
     */
    Widget& disabled(bool disable = true) {
        return fieldInt(Tag::disable, disable);
    }

    /**
     * @brief Подсказка виджета. Пустая строка - убрать подсказку
     * @param AnyText текст (String, "string", F("string"))
     */
    Widget& hint(AnyText str) {
        return fieldStr(Tag::hint, str);
    }

    // ===================== ACTION =====================
    /**
     * @brief Проверка на клик по виджету
     * @warning Вызывать последним!
     * @return bool
     */
    bool click() {
        return _click;
    }

    // ===================== ATTACH =====================
    // Подключить функцию вида void f()
    Widget& attach(void (*cb)()) {
        if (_click && cb) cb();
        return *this;
    }

    // Подключить функцию вида void f(gh::Build& build)
    Widget& attach(void (*cb)(gh::Build& build)) {
        if (_click && cb && _build) cb(*_build);
        return *this;
    }

    // Подключить функцию вида void f(gh::Builder& build)
    Widget& attach(void (*cb)(gh::Builder& build), gh::Builder& b) {
        if (_click && cb && _build) cb(b);
        return *this;
    }

    // Подключить gh::Flag* флаг
    Widget& attach(gh::Flag* flag) {
        if (_click) flag->_changed = true;
        return *this;
    }

    // Подключить bool* флаг
    Widget& attach(bool* flag) {
        if (_click) *flag = true;
        return *this;
    }

   protected:
    gh::core::Packet* sp;
    gh::Build* _build;
    uint32_t* _count;
    bool _enabled = 1;
    bool _allowed = 0;
    bool _click = false;
    bool _empty = true;

    void _process() {
        _empty = false;
        if (_build && _build->action == gh::Action::Count) {
            *_count += sp->length();
            sp->clear();
        }
    }
    bool _en() {
        return _enabled && _allowed;
    }
};

}  // namespace gh::core