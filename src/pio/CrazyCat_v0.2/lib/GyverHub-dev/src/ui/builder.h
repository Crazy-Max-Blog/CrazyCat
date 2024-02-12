#pragma once
#include <Arduino.h>

#ifndef GH_NO_PAIRS
#include <Pairs.h>
#endif

#include "core/build.h"
#include "core/client.h"
#include "core/core_class.h"
#include "core/hooks.h"
#include "core/hub_class.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "ui/builder_class.h"
#include "ui/builder_macro.h"
#include "ui/button.h"
#include "ui/canvas.h"
#include "ui/flags.h"
#include "ui/log.h"
#include "utils/datatypes.h"
#include "utils/packet.h"
#include "widget.h"

namespace gh {
namespace core {
typedef void (*BuildCallback)(gh::Builder& builder);
}

using gh::Type;
using gh::core::AnyPtr;
using gh::core::AnyText;
using gh::core::Packet;
using gh::core::Tag;
using gh::core::Widget;

class Builder {
    friend class gh::core::HubCore;
    friend class ::GyverHub;

   public:
    gh::Build& build;
    Widget widget;

    Builder(gh::Build* build,
            Packet* gs = nullptr,
            uint32_t* count = nullptr) : build(*build),
                                         widget(gs, build, count),
                                         gs(gs) {}

    // ========================== SYSTEM ==========================

    // обновить панель управления (по действию с виджета)
    void refresh() {
        _refresh = true;
    }

    // был ли запрос на обновление панели управления
    bool isRefresh() {
        return _refresh;
    }

    // включить/выключить вывод виджетов (только для запроса виджетов)
    bool show(bool en = true) {
        widget._enabled = en;
        return true;
    }

    // значение хотя бы одного виджета было изменено (сигнал на сохранение)
    bool wasSet() {
        return _changed;
    }

    // добавить виджеты из JSON строки
    void addJSON(gh::core::AnyText text) {
        if (gs && gs->hasHook() && build.action == gh::Action::UI) {
            if (_checkFirst()) {
                gs->closeComma();
                gs->comma();
            }
            gs->sendNow();
            gs->sendHook(text);
            gs->comma();
            gs->open();
        }
    }

    // добавить виджеты из JSON строки PROGMEM
    void addJSON_P(PGM_P text) {
        addJSON((FSTR)text);
    }

    // добавить виджеты из JSON из файла
    void addJSON_File(gh::core::AnyText path) {
        AnyText name;
        _checkName(name);
        switch (build.action) {
            case gh::Action::Count:
            case gh::Action::UI:
                _beginName(name);
                _tag(Tag::ui_file);
                widget.valueStr(path);
                break;

            default:
                break;
        }
    }

    // прочитать текущий пункт меню
    uint8_t menu() {
        return _menu ? *_menu : 0;
    }

    // ======================== STRUCTURE ========================

    // горизонтальный контейнер
    GH_BUILD_CONTAINER(Row, Tag::row);

    // вертикальный контейнер
    GH_BUILD_CONTAINER(Col, Tag::col);

    // Пустой виджет. Параметры: size, square
    Widget& Space() {
        AnyText name;
        return _varAndType(Tag::space, name, AnyPtr());
    }

    // ========================= WIDGETS =========================

    // параметры виджета: noTab, label, noLabel, size, square, hint, suffix

    // подключить переменную к виджету, созданному из JSON. Можно навесить attach и click на взаимодействие. Update соответственно виджету
    Widget& Hook_(gh::core::AnyText name, AnyPtr data, const AnyText& func = AnyText()) {
        _checkName(name);
        switch (build.action) {
            case gh::Action::Count:
            case gh::Action::UI:
                if (_checkFirst()) {
                    gs->closeComma();
                    gs->comma();
                }
                gs->open();
                gs->addStr(Tag::type, Tag::hook);
                gs->addStr(Tag::id, name);
                _func(func);
                _uiHelper(name, data);
                break;

            case gh::Action::Get:
                _getHelper(name, data);
                break;

            case gh::Action::Read:
                if (_readName(name) && data.ptr) _writeValue(name, data);
                break;

            case gh::Action::Set:
                _setHelper(name, data);
                break;

            default:
                break;
        }
        return widget;
    }

    // =========== ВВОД ============

    // Поле ввода текста. Параметры: value (текст), color, regex, maxLen, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Input, Tag::input);

    // Поле ввода текста. Параметры: value (текст), rows, maxLen, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(InputArea, Tag::area);

    // Поле ввода пароля. Параметры: value (текст), color, regex, maxLen, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Pass, Tag::pass);

    // Всплывающее окно подтверждения. Параметры: text (подпись), attach, click. Для активации отправь обновление action()
    GH_BUILD_VAR(Confirm, Tag::confirm);

    // Всплывающее окно ввода. Параметры: value (значение), text (подпись), attach, click. Для активации отправь обновление action()
    GH_BUILD_VAR(Prompt, Tag::prompt);

    // Дата. Параметры: value(unix), color, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Date, Tag::date);

    // Время. Параметры: value(unix), color, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Time, Tag::time);

    // Дата и время. Параметры: value(unix), color, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(DateTime, Tag::datetime);

    // Слайдер. Параметры: value (значение), color, range, unit, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Slider, Tag::slider);

    // Спиннер. Параметры: value (значение), range, unit, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Spinner, Tag::spinner);

    // Выпадающий список. Параметры: value (номер пункта), text (список), color, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Select, Tag::select);

    // Цвет. Параметры: value (цвет), disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Color, Tag::color);

    // ========= ДЕЙСТВИЕ =========

    // Кнопка. Параметры: icon, color, fontSize, disabled, attach, click + параметры виджета
    GH_BUILD_VAR_TYPE(Button, gh::Button*, Tag::button);

    // Выключатель. Параметры: value (состояние), color, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(Switch, Tag::switch_t);

    // Выключатель. Параметры: value (состояние), icon, color, fontSize, disabled, attach, click + параметры виджета
    GH_BUILD_VAR(SwitchIcon, Tag::switch_i);

    // Вкладки. Параметры: value (выбранная), text (список), color, disabled, attach, click + параметры виджета. text не обновляется
    GH_BUILD_VAR(Tabs, Tag::tabs);

    // Кнопки выбора. Параметры: value (флаги), text (список), color, disabled, attach, click + параметры виджета
    GH_BUILD_VAR_TYPE(Flags, gh::Flags*, Tag::flags);

    // Джойстик. keep - возвращать в центр, exp - экспоненциальные значения. Параметры: color + параметры виджета
    Widget& Joystick_(AnyText name = "", gh::Pos* pos = nullptr, bool keep = 0, bool exp = 0) {
        return _joy_pad(Tag::joy, name, pos, keep, exp);
    }
    Widget& Joystick(gh::Pos* pos = nullptr, bool keep = 0, bool exp = 0) {
        return Joystick_(AnyText(), pos, keep, exp);
    }

    // Крестовина. Параметры: color + параметры виджета
    Widget& Dpad_(AnyText name = "", gh::Pos* pos = nullptr) {
        return _joy_pad(Tag::dpad, name, pos);
    }
    Widget& Dpad(gh::Pos* pos = nullptr) {
        return Dpad_(AnyText(), pos);
    }

    // ======== ПАССИВНЫЕ ========

    // Заголовок. Параметры: value (текст), icon, color, align, fontSize + size виджета
    GH_BUILD_TEXT(Title, Tag::title);

    // Надпись. Параметры: value (текст), icon, color, align, fontSize + параметры виджета
    GH_BUILD_TEXT(Label, Tag::label);

    // Надпись. Параметры: value (переменная), icon, color, align, fontSize + параметры виджета
    GH_BUILD_VAR(LabelVar, Tag::label);

    // Окно с текстом. Параметры: value (текст), rows + параметры виджета
    GH_BUILD_TEXT(Text, Tag::text);

    // Окно с текстом. Параметры: value (путь), rows + параметры виджета
    GH_BUILD_TEXT(TextFile, Tag::text_f);

    // Дисплей. Параметры: value (текст), color, fontSize, rows + параметры виджета
    GH_BUILD_TEXT(Display, Tag::display);

    // Дисплей. Параметры: value (переменная), color, fontSize, rows + параметры виджета
    GH_BUILD_VAR(DisplayVar, Tag::text);

    // Картинка. Параметры: value (путь) + параметры виджета
    GH_BUILD_TEXT(Image, Tag::image);

    // Лог. value(текст), rows + параметры виджета
    GH_BUILD_VAR_TYPE(Log, gh::Log*, Tag::log);

    // Светодиод. Параметры: value (состояние 1/0), color + параметры виджета
    GH_BUILD_VAR(LED, Tag::led);

    // Светодиод-иконка. Параметры: value (состояние 1/0), icon, fontSize, color + параметры виджета
    GH_BUILD_VAR(Icon, Tag::icon);

    // Индикаторная шкала. Параметры: value (значение), range, unit, color + параметры виджета
    GH_BUILD_VAR(Gauge, Tag::gauge);

    // Индикаторная шкала круглая. Параметры: value (значение), range, unit, color + параметры виджета
    GH_BUILD_VAR(GaugeRound, Tag::gauge_r);

    // Индикаторная шкала линейная. Параметры: value (значение), icon, range, unit, color + параметры виджета
    GH_BUILD_VAR(GaugeLinear, Tag::gauge_l);

    // Таблица. Параметры: value (текст или путь) + параметры виджета
    // text: таблица в формате CSV - разделитель столбцов ; разделитель строк \n
    // width: ширина, список чисел в процентах (например "30;30;50")
    // align: выравнивание, список из left | center | right (например "left;right")
    Widget& Table_(AnyText name, AnyText text, AnyText width = AnyText(), AnyText align = AnyText()) {
        _varAndType(Tag::table, name, &text);
        if (width.str) widget.fieldStr(Tag::width, width);
        if (align.str) widget.fieldStr(Tag::align, align);
        return widget;
    }
    Widget& Table(AnyText text, AnyText width = AnyText(), AnyText align = AnyText()) {
        return Table_(AnyText(), text, width, align);
    }

    // httpd стрим, без параметров
    void Stream(uint16_t port = 82) {
        AnyText name;
        _checkName(name);
        switch (build.action) {
            case gh::Action::Count:
            case gh::Action::UI:
                if (!widget._enabled) break;
                _beginName(name);
                _tag(Tag::stream);
                widget.fieldInt(Tag::port, port);
                break;

            default:
                break;
        }
    }

    // ========================= CANVAS =========================
    // Холст, рисование. Ширина, длина. Только араметры виджета
    Widget& BeginCanvas(uint16_t width = 400, uint16_t height = 300, gh::Canvas* cv = nullptr, gh::Pos* pos = nullptr) {
        return BeginCanvas_(AnyText(), width, height, cv, pos);
    }
    Widget& BeginCanvas_(AnyText name = "", uint16_t width = 400, uint16_t height = 300, gh::Canvas* cv = nullptr, gh::Pos* pos = nullptr) {
        return Canvas_(name, width, height, cv, pos, true);
    }

    // Холст. Ширина, длина. Только араметры виджета
    Widget& Canvas(uint16_t width = 400, uint16_t height = 300, gh::Canvas* cv = nullptr, gh::Pos* pos = nullptr) {
        return Canvas_(AnyText(), width, height, cv, pos);
    }
    Widget& Canvas_(AnyText name = "", uint16_t width = 400, uint16_t height = 300, gh::Canvas* cv = nullptr, gh::Pos* pos = nullptr, bool begin = false) {
        AnyPtr data(pos);
        _checkName(name);
        if (cv && !(build.action == gh::Action::Count || build.action == gh::Action::UI)) cv->setBuffer(nullptr);  // запретить запись

        switch (build.action) {
            case gh::Action::Count:
            case gh::Action::UI:
                if (!widget._enabled) break;
                _beginName(name);
                _tag(Tag::canvas);
                widget.fieldInt(Tag::width, width);
                widget.fieldInt(Tag::height, height);
                if (pos) widget.fieldInt(Tag::active, 1);
                gs->addKey(Tag::data);
                gs->openSq();
                if (begin && cv) cv->setBuffer(gs);
                else EndCanvas();
                break;

            case gh::Action::Set:
                widget._click = _parseName(name, data);
                if (widget._click) _needs_update = false;
                break;

            default:
                break;
        }
        return widget;
    }

    // завершить холст
    void EndCanvas() {
        if (build.action == gh::Action::Count || build.action == gh::Action::UI) {
            gs->closeSq();
            gs->comma();
        }
    }

    // ========= СИСТЕМА =========

    // Меню. Передать пункты меню списком "пункт1;пункт2" + attach, click
    Widget& Menu(AnyText text) {
        widget._click = 0;
        if (_menu) {
            AnyText name(F("_menu"));
            if (_varAndType(Tag::menu, name, _menu).text(text).click()) {
                refresh();
                _needs_update = false;
            }
        }
        return widget;
    }

    // ========= КАСТОМ =========

    // кастомный невидимый компонент. attach, click
    GH_BUILD_VAR(Dummy, Tag::dummy);

    // HTML код в виджет. Параметры: (код или /путь.html) + параметры виджета
    GH_BUILD_TEXT(HTML, Tag::html);

    // Вставка JS кода. Параметры: (код или /путь.js)
    GH_BUILD_TEXT_NONAME(JS, Tag::js);

    // Вставка CSS кода. Параметры: (код или /путь.css)
    GH_BUILD_TEXT_NONAME(CSS, Tag::css);

    // кастомный виджет, имя функции из плагина. Можно использовать любые параметры виджета
    GH_BUILD_CUSTOM(Custom, Tag::func);

    // свой набор скриптов и стилей. Вызывать один раз на билдер. Принимает код в виде текста или путь к файлу .js / .css
    void Plugin(const AnyText& js, const AnyText& css = "") {
        AnyText name;
        _checkName(name);
        switch (build.action) {
            case gh::Action::Count:
            case gh::Action::UI:
                if (!widget._enabled) break;
                _beginName(name);
                _tag(Tag::plugin);
                widget.fieldStr(Tag::js, js);
                widget.fieldStr(Tag::css, css);
                break;
            default:
                break;
        }
    }

    // =========================== CORE ===========================
   private:
    Widget& _varAndType(Tag tag, AnyText& name, AnyPtr data, const AnyText& func = AnyText()) {
        _checkName(name);
        switch (build.action) {
            case gh::Action::Count:
            case gh::Action::UI:
                if (!widget._enabled) break;
                _beginName(name);
                _tag(tag);
                _func(func);
                _uiHelper(name, data);
                break;

            case gh::Action::Get:
                _getHelper(name, data);
                break;

            case gh::Action::Read:
                if (_readName(name) && data.ptr) _writeValue(name, data);
                break;

            case gh::Action::Set:
                _setHelper(name, data);
                break;
            default:
                break;
        }
        return widget;
    }
    Widget& _joy_pad(Tag tag, AnyText& name, gh::Pos* pos = nullptr, bool keep = 0, bool exp = 0) {
        AnyPtr data(pos);
        _checkName(name);
        switch (build.action) {
            case gh::Action::Count:
            case gh::Action::UI:
                if (!widget._enabled) break;
                _beginName(name);
                _tag(tag);
                if (keep) widget.fieldInt(Tag::keep, 1);
                if (exp) widget.fieldInt(Tag::exp, 1);
                break;

            case gh::Action::Set:
                widget._click = _parseName(name, data);
                if (widget._click) {
                    _needs_update = false;
                    if (pos) {
                        pos->x -= 255;
                        pos->y -= 255;
                    }
                }
                break;
            default:
                break;
        }
        return widget;
    }
    // =========================== SYS ===========================
    Packet* gs;
    uint16_t _index = 0;
    bool _first = true;
    bool _stop = false;
    bool _refresh = false;
    bool _changed = false;
    bool _needs_update = true;
    uint8_t* _menu = nullptr;

    // system
    void _tag(Tag tag) {
        gs->addStr(Tag::type, tag);
        widget._process();
    }
    void _func(const AnyText& func) {
        if (!func.str) return;
        gs->addStr(Tag::func, func);
        widget._process();
    }
    bool _checkFirst() {
        if (_first) return _first = false;
        return 1;
    }
    void _writeValue(AnyText& name, AnyPtr& data) {
        switch (data.type) {
            case gh::Type::BTN_T:
            case gh::Type::NULL_T:
                break;
#ifndef GH_NO_PAIRS
            case gh::Type::PAIRS_T: {
                Pair_t pair = ((Pairs*)data.ptr)->get(PairsUtils::AnyText(name.str, name.pgm));
                if (!pair.val) pair = ((Pairs*)data.ptr)->add(PairsUtils::AnyText(name.str, name.pgm), "");
                if (pair.val) gh::core::varToStr(gs, AnyPtr(&pair));
            } break;
#endif
            default:
                gh::core::varToStr(gs, data);
                break;
        }
    }

    // named widget
    void _beginName(AnyText& name) {
        if (_checkFirst()) {
            gs->closeComma();
            gs->comma();
        }
        gs->open();
        gs->addKey(Tag::id);
        gs->dquot();
        if (name.str) {
            if (name.pgm) *gs += (FSTR)name.str;
            else *gs += (PGM_P)name.str;
        } else {
            *gs += F("_n");
            *gs += _index;
        }
        gs->dquot();
        gs->comma();
        widget._process();
    }
    void _checkName(AnyText& name) {
        bool f;
        if (!name.str) f = 1;                                       // nullptr
        else if (name.pgm) f = !pgm_read_byte((uint8_t*)name.str);  // name[0] == '\0'
        else f = !(*(uint8_t*)name.str);                            // name[0] == '\0'
        if (f) {
            _index++;
            name.str = nullptr;
        }
    }
    bool _nameEq(AnyText& name) {
        if (name.str) {
            if (name.pgm) return !strcmp_P(build.name, name.str);
            else return !strcmp(build.name, name.str);
        } else {
            return _autoNameEq();
        }
    }
    bool _autoNameEq() {
        return build.name[0] == '_' && build.name[1] == 'n' && (uint16_t)atoi(build.name + 2) == _index;
    }
    bool _readName(AnyText& name) {
        if (_stop) return false;
        if (name.str) {
            if (name.pgm ? (!strcmp_P(build.name, name.str)) : (!strcmp(build.name, name.str))) {
                _stop = true;
                return true;
            }
        } else {
            if (_autoNameEq()) {
                _stop = true;
                return true;
            }
        }
        return false;
    }
    bool _parseName(AnyText& name, AnyPtr& data) {
        if (!_stop && _nameEq(name)) {
            _stop = true;
            if (!data.ptr) return 1;
            if (data.type == gh::Type::PAIRS_T) {
#ifndef GH_NO_PAIRS
                ((Pairs*)data.ptr)->set(PairsUtils::AnyText(name.str, name.pgm), build.value);
#endif
            } else {
                gh::core::strToVar(build.value, data);
            }
            return 1;
        }
        return 0;
    }

    // container
    bool _beginContainer(uint16_t width, Tag tag) {
        if (!gs || !_allowContainer()) return true;
        if (_checkFirst()) {
            gs->closeComma();
            gs->comma();
        }
        gs->open();
        gs->addStr(Tag::type, tag);
        gs->addInt(Tag::wwidth, width);
        gs->addKey(Tag::data);
        gs->openSq();
        _first = true;
        return true;
    }
    void _endContainer() {
        if (!gs || !_allowContainer()) return;
        if (_checkFirst()) {
            gs->closeComma();
        }
        gs->closeSqComma();
        _first = false;
    }
    bool _nextContainer(uint16_t width, Tag tag) {
        if (!gs || !_allowContainer()) return true;
        _endContainer();
        _beginContainer(width, tag);
        return true;
    }
    bool _allowContainer() {
        return (build.action == gh::Action::Count || build.action == gh::Action::UI) && widget._enabled;
    }

    // helper
    void _uiHelper(AnyText& name, AnyPtr& data) {
        if (data.ptr) {
            switch (data.type) {
                case gh::Type::NULL_T:
                case gh::Type::BTN_T:
                    break;
#ifndef GH_NO_PAIRS
                case gh::Type::PAIRS_T: {
                    Pair_t pair = ((Pairs*)data.ptr)->get(PairsUtils::AnyText(name.str, name.pgm));
                    if (pair.val) widget.valueVar(AnyPtr(&pair));
                } break;
#endif
                default:
                    widget.valueVar(data);
                    break;
            }
        }
    }
    void _getHelper(AnyText& name, AnyPtr& data) {
        if (name.str && data.ptr) {
            gs->addKey(name);
            gs->dquot();
            _writeValue(name, data);
            gs->dquot();
            gs->comma();
        }
    }
    void _setHelper(AnyText& name, AnyPtr& data) {
        switch (data.type) {
            // case gh::Type::NULL_T:   // TODO?
            case gh::Type::TEXT_T:
                break;
            case gh::Type::BTN_T:
                widget._click = 0;
                if (_parseName(name, data)) {
                    widget._click = (build.value[0] == '2');
                    _needs_update = false;
                }
                break;
            default:
                widget._click = _parseName(name, data);
                if (widget._click) _changed = true;
                break;
        }
    }
};

}  // namespace gh