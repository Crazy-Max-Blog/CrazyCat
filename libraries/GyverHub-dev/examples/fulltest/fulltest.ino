// FULL DEMO
#include <Arduino.h>
#define ATOMIC_FS_UPDATE

#define AP_SSID ""
#define AP_PASS ""

const char lorem[] PROGMEM = "keka pua AZAZAZA Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi dignissim tellus ligula. Vivamus id lacus ac tortor commodo aliquam. Mauris eget faucibus nunc. Vestibulum tempus eu lorem a dapibus. Nullam ac dapibus ex. Aenean faucibus dapibus porttitor. Sed vel magna id tellus mattis semper. Fusce a finibus ligula. In est turpis, viverra eget libero ut, pretium pellentesque velit. Praesent ultrices elit quis facilisis mattis. Donec eu iaculis est. Sed tempus feugiat ligula non ultricies. Cras a auctor nibh, sed sodales sapien.\n\nSed cursus quam vel egestas rhoncus. Curabitur dignissim lorem sed metus sollicitudin, non faucibus erat interdum. Nunc vitae lobortis dui, mattis dignissim orci. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Duis vel venenatis purus. Nunc luctus leo tincidunt felis efficitur ullamcorper. Aliquam semper rhoncus odio sed porta. Quisque blandit, dui vel imperdiet ultricies, dolor arcu posuere turpis, et gravida ante libero ut ex. Vestibulum sed scelerisque nibh, nec mollis urna. Suspendisse tortor sapien, congue at aliquam vitae, venenatis placerat enim. Interdum et malesuada fames ac ante ipsum primis in faucibus. Nam posuere metus a est commodo finibus. Donec luctus arcu purus, sit amet sodales dolor facilisis id. Nullam consectetur sapien vitae nisi gravida, sed finibus dui hendrerit. In id pretium odio, imperdiet lacinia massa. Morbi quis condimentum ligula.";
const char _ui[] PROGMEM = R"json({"type":"row","width":1,"data":[{"id":"inp","type":"input"},{"id":"sld","type":"slider"},{"id":"btn","type":"button"}]})json";

#if 1
#define ATOMIC_FS_UPDATE  // OTA обновление сжатым .gz файлом вместо .bin (для esp)

// #define GH_LIB_VERSION "v0.1b"   // версия библиотеки
// #define GH_CONN_TOUT 4           // таймаут соединения, с
// #define GH_FETCH_CHUNK_SIZE 512  // размер чанка при скачивании с платы
// #define GH_UPL_CHUNK_SIZE 512    // размер чанка при загрузке на плату
// #define GH_FS_DEPTH 5            // глубина сканирования файловой системы (esp32)

// #define GH_WS_PORT 81  // ws порт

// #define GH_MQTT_RECONNECT 10000  // период переподключения MQTT

// #define GH_PUBLIC_PATH "/www"          // путь к папке с файлами с HTTP доступом
// #define GH_HTTP_PORT 80                // http порт
// #define GH_CACHE_PRD "max-age=604800"  // период кеширования файлов для портала

// #define GH_NO_STREAM     // отключить модуль связи stream
// #define GH_NO_HTTP       // отключить модуль связи http (для esp)
// #define GH_NO_WS         // отключить модуль связи ws (для esp)
// #define GH_NO_MQTT       // отключить модуль связи mqtt (для esp)

// #define GH_NO_PAIRS      // отключить поддержку типа Pairs
// #define GH_NO_GET        // отключить отправку на get-топик (mqtt)
// #define GH_NO_INFO       // отключить info билдер
// #define GH_NO_MODULES    // отключить "модули" (всегда включены)
// #define GH_NO_REQUEST    // отключить поддержку запросов

// #define GH_NO_FS         // отключить файловую систему (для esp)

// #define GH_NO_FETCH      // отключить скачивание файлов (для esp)
// #define GH_NO_UPLOAD     // отключить загрузку файлов (для esp)
// #define GH_NO_OTA        // отключить ОТА обновления (для esp)
// #define GH_NO_OTA_URL    // отключить ОТА по ссылке (для esp)

// #define GH_FILE_PORTAL       // загружать сайт из памяти есп (положи файлы в папку /hub/)
// #define GH_INCLUDE_PORTAL    // включить сайт в память программы, ~50кБ (не нужно загружать файлы в память)

// #define GH_NO_HTTP_TRANSFER  // отключить загрузку/скачивание/ОТА по http (для esp)
// #define GH_NO_HTTP_DNS       // отключить DNS сервер (для esp)
// #define GH_NO_HTTP_OTA       // отключить ОТА обновления по http (для esp)
// #define GH_NO_HTTP_PUBLIC    // отключить доступ к файлам по http c ip/www (для esp)
// #define GH_NO_HTTP_FETCH     // отключить скачивание файлов по http (для esp)
// #define GH_NO_HTTP_UPLOAD    // отключить загрузку файлов по http (для esp)
// #define GH_NO_HTTP_UPLOAD_PORTAL // упрощённую загрузку файлов с ip/hub/upload_portal (для esp)
#endif

#include <GyverHub.h>
GyverHub hub("MyDevices2", "ESP", "");

#include <PairsFile.h>
PairsFile data(&GH_FS, "/data.dat", 3000);
// Pairs data;

// 9
uint32_t colors[] = {
    0xcb2839,
    0xd55f30,
    0xd69d27,
    0x37A93C,
    0x25b18f,
    0x2ba1cd,
    0x297bcd,
    0x825ae7,
    0xc8589a,
};

gh::Log hlog;
static bool dsbl, nolbl, noback, sq;

void build_input(gh::Builder& b) {
    if (b.build.action == gh::Action::Set) hlog.println(b.build.name);

    b.Title_("tit", "Some title");
    if (b.beginRow()) {
        gh::Update upd(&hub);
        upd.update("tit");
        if (b.Button().label("value").click()) upd.widget.valueInt(random(100));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        if (b.Button().label("fsize").click()) upd.widget.fontSize(random(10, 40));

        b.Switch(&dsbl).label("dsbl");
        b.Switch(&nolbl).label("no lbl");
        b.Switch(&noback).label("no back");
        b.Switch(&sq).label("square");
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static String inp;
        b.Input_("inp", &inp).size(2).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        // b.Pass_("pass", &inp);

        gh::Update upd(&hub);
        upd.update("inp");
        if (b.Button().label("value").size(1).click()) upd.widget.valueInt(random(100));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);

        if (b.Switch(&dsbl).label("dsbl").click()) upd.widget.disabled(b.build.valueBool());
        if (b.Switch(&nolbl).label("no lbl").click()) upd.widget.noLabel(b.build.valueBool());
        if (b.Switch(&noback).label("no back").click()) upd.widget.noTab(b.build.valueBool());
        if (b.Switch(&sq).label("square").click()) upd.widget.square(b.build.valueBool());
        if (b.Button().label("label").click()) upd.widget.label(String(random(10000)));
        if (b.Button().label("hint").click()) upd.widget.hint('#' + String(random(10000)));

        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static float v;
        b.Slider_("sld", &v).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("sld");
        if (b.Button().label("value").click()) upd.widget.valueInt(random(100));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        if (b.Button().label("range").click()) upd.widget.range(10, 90, 1.5, 1);
        if (b.Button().label("unit").click()) upd.widget.unit("deg");
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static float v;
        b.Spinner_("spin", &v).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("spin");
        if (b.Button().label("value").click()) upd.widget.valueInt(random(100));
        if (b.Button().label("range").click()) upd.widget.range(10, 90, 1.5);
        if (b.Button().label("unit").click()) upd.widget.unit("deg");
        upd.send();
        b.endRow();
    }
}
void build_input1(gh::Builder& b) {
    if (b.beginRow()) {
        static bool sw;
        b.Switch_("sw", &sw).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("sw");
        if (b.Button().label("value").click()) upd.widget.valueInt(random(2));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static gh::Button btn;
        b.Button_("btn", &btn).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq).icon("/icon.svg");
        if (btn.changed()) Serial.println(btn.state() ? "btn press" : "btn release");
        if (btn.clicked()) Serial.println("btn click");

        gh::Update upd(&hub);
        upd.update("btn");
        if (b.Button().label("icon").click()) upd.widget.icon("f6ad");
        if (b.Button().label("path").click()) upd.widget.icon("/icon.svg");
        if (b.Button().label("fsize").click()) upd.widget.fontSize(random(10, 40));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        b.Label_("lbl").disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("lbl");
        if (b.Button().label("value").click()) upd.widget.valueInt(random(100));
        if (b.Button().label("fsize").click()) upd.widget.fontSize(random(10, 40));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        upd.send();
        b.endRow();
    }
}
void build_input2(gh::Builder& b) {
    if (b.beginRow()) {
        bool flag = 0;
        static String str = "custom";
        b.Custom_("my_inp2", "myWidget").size(2).fieldStr(F("btn_text"), "btn0");
        b.Custom_("my_inp", "myWidget", &str).fieldStr(F("btn_text"), "button!").disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq).attach(&flag);
        if (flag) {
            Serial.print("custom: ");
            Serial.println(b.build.valueString());
        }

        gh::Update upd(&hub);
        upd.update("my_inp", "myWidget");
        if (b.Button().size(1).label("btn text").click()) upd.widget.fieldInt(F("btn_text"), random(10, 100));
        if (b.Button().label("value").click()) upd.widget.valueInt(random(10));
        if (b.Button().label("refresh").click()) upd.widget.valueData(&str);
        upd.send();
        b.endRow();
    }
}
void build_input3(gh::Builder& b) {
    if (b.beginRow()) {
        String s = "some\ntext";
        b.Display_("disp", &s).size(3).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("disp");
        if (b.Button().label("value").size(1).click()) upd.widget.valueInt(random(100));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        if (b.Button().label("fsize").click()) upd.widget.fontSize(random(10, 40));
        if (b.Button().label("rows").click()) upd.widget.rows(random(1, 5));
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static String area = "test\ntext";
        b.InputArea_("area", &area).size(3).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("area");
        if (b.Button().label("value").size(1).click()) upd.widget.valueInt(random(100));
        if (b.Button().label("rows").click()) upd.widget.rows(random(1, 5));
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        b.Text_("text", &data).size(3).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("text");
        if (b.Button().label("value").size(1).click()) upd.widget.valueInt(random(100));
        if (b.Button().label("rows").click()) upd.widget.rows(random(1, 5));
        upd.send();
        b.endRow();
    }
}
void build_input4(gh::Builder& b) {
    if (b.beginRow()) {
        b.Log_("log", &hlog).size(3).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("log");
        if (b.Button().label("value").size(1).click()) upd.widget.valueData(&hlog);
        if (b.Button().label("rows").click()) upd.widget.rows(random(1, 5));
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static uint8_t sel;
        b.Select_("sel", &sel).text("puk1;puk 2; puk 3").size(2).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("sel");
        if (b.Button().label("value").size(1).click()) upd.widget.valueInt(random(4));
        if (b.Button().label("text").click()) upd.widget.text("000,1111,2222");
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        if (b.Button().label("suffix").click()) upd.widget.suffix("hello");
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static uint32_t d, t, dt;
        b.Date_("date", &d).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.Time_("time", &t).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.DateTime_("datime", &dt).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.endRow();
    }
}
void build_input5(gh::Builder& b) {
    if (b.beginRow()) {
        gh::Update upd(&hub);
        upd.update("date");
        if (b.Button().label("value").click()) upd.widget.valueInt(1700333775ul + random(-86400, 86400));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);

        upd.update("time");
        if (b.Button().label("value").click()) upd.widget.valueInt(1700333775ul + random(-86400, 86400));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);

        upd.update("datime").disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        if (b.Button().label("value").click()) upd.widget.valueInt(1700333775ul + random(-86400, 86400));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        upd.send();

        b.endRow();
    }

    if (b.beginRow()) {
        if (b.Button().label("script").click()) hub.sendScript("console.log('hello!');");
        b.HTML_("html", "<button onclick='console.log(123)'>TEST</button>");
        b.JS("setTimeout(()=>console.log('hello js'), 500)");
        b.CSS(".w_slider{height:50px}");

        gh::Update upd(&hub);
        upd.update("html");
        if (b.Button().label("value").click()) upd.widget.valueStr("<button onclick='console.log(666666)'>TEST2</button>");
        upd.send();
        b.endRow();
    }
}
void build_input6(gh::Builder& b) {
    if (b.beginRow()) {
        static gh::Color colt;
        b.Color_("col", &colt).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("col");
        if (b.Button().label("value").click()) upd.widget.valueInt(colors[random(9)]);
        if (b.Button().label("value").click()) upd.widget.color(colors[random(9)]);
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        static bool led;
        b.LED_("led", &led).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("led");
        if (b.Button().label("value").click()) upd.widget.valueInt(led = !led);
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        b.Label_("test");
        if (b.Button().label("all").click()) hub.sendUpdateStr("test", "all");
        if (b.Button().label("client").click()) hub.sendUpdateStr("test", "client", &b.build.client);
        b.endRow();
    }
}
void build_input7(gh::Builder& b) {
    int v = random(100);
    b.LabelVar(&v);
    if (b.beginRow()) {
        int v = random(100);
        b.Gauge_("gag", &v).size(2).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);

        gh::Update upd(&hub);
        upd.update("gag");
        if (b.Button().size(1).label("value").click()) upd.widget.valueInt(random(100));
        if (b.Button().label("range").click()) upd.widget.range(20, 80, 1);
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        if (b.Button().label("unit").click()) upd.widget.unit("deg");
        upd.send();
        b.endRow();
    }

    if (b.beginRow()) {
        {
            gh::Pos pos;
            b.Joystick(&pos).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
            if (pos.changed()) {
                Serial.print("joy0:");
                Serial.print(pos.x);
                Serial.print(',');
                Serial.println(pos.y);
            }
        }
        {
            gh::Pos pos;
            b.Joystick(&pos, 1).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
            if (pos.changed()) {
                Serial.print("joy1:");
                Serial.print(pos.x);
                Serial.print(',');
                Serial.println(pos.y);
            }
        }
        b.endRow();
    }
    if (b.beginRow()) {
        gh::Pos pos;
        b.Dpad(&pos).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.Slider();
        if (pos.changed()) {
            Serial.print("pad:");
            Serial.print(pos.x);
            Serial.print(',');
            Serial.println(pos.y);
        }
        b.endRow();
    }
}
void build_input8(gh::Builder& b) {
    if (b.beginRow()) {
        static uint8_t tab;
        static gh::Flags flags;
        b.Tabs(&tab).text("tab 0;kek;puk;hello").color(gh::Colors::Blue).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.Flags(&flags).text("flag 0; flag 1; mon; tues").color(gh::Colors::Blue).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.endRow();
    }
    if (b.beginRow()) {
        static bool sw;
        b.SwitchIcon_("swi", &sw).disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.SwitchIcon(&sw).icon("").disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.SwitchIcon(&sw).icon("f729").disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.SwitchIcon(&sw).icon("/icon.svg").disabled(dsbl).noLabel(nolbl).noTab(noback).square(sq);
        b.endRow();
    }
    if (b.beginRow()) {
        gh::Update upd(&hub);
        upd.update("swi");
        if (b.Button().label("value").click()) upd.widget.valueInt(random(2));
        if (b.Button().label("fsize").click()) upd.widget.fontSize(random(30, 80));
        if (b.Button().label("color").click()) upd.widget.color(colors[random(9)]);
        if (b.Button().label("icon").click()) upd.widget.icon("");
        upd.send();
        b.endRow();
    }
}

void build_ffile(gh::Builder& b) {
    b.Image_("img", "/text.png");
    if (b.beginRow()) {
        gh::Update upd(&hub);
        upd.update("img");
        if (b.Button().label("refresh").click()) upd.widget.action();
        upd.send();
        b.endRow();
    }

    b.Table_("table", "/table.csv").hint("Some hint");
    if (b.beginRow()) {
        gh::Update upd(&hub);
        upd.update("table");
        if (b.Button().label("refresh").click()) upd.widget.action();
        if (b.Button().label("text").click()) upd.widget.valueStr("kek,pek,hello");
        if (b.Button().label("file").click()) upd.widget.valueStr("/table.csv");
        if (b.Button().label("align").click()) upd.widget.align("left,right,right");
        if (b.Button().label("width").click()) upd.widget.width("10,20,30");
        upd.send();
        b.endRow();
    }

    b.TextFile_("textf", "/data.dat");
    if (b.beginRow()) {
        gh::Update upd(&hub);
        upd.update("textf");
        if (b.Button().label("refresh").click()) upd.widget.action();
        if (b.Button().label("file").click()) upd.widget.valueStr("/table.csv");
        upd.send();
        b.endRow();
    }

    b.HTML("/html.html");
    b.JS("/js.js");
    b.CSS("/css.css");

    static String inp0, inp1;
    static int sld0, sld1;
    b.addJSON_File("/ui0.json");
    b.addJSON_File("/ui1.json");
    b.Hook_("inp0", &inp0);
    b.Hook_("sld0", &sld0);
    b.Hook_("inp1", &inp1);
    b.Hook_("sld1", &sld1);

    static String inp;
    static int sld = 1234;
    b.addJSON_P(_ui);
    b.Hook_("inp", &inp);
    b.Hook_("sld", &sld);
}

void build_mqtt(gh::Builder& b) {
    static String s1, s2;
    if (b.Input_("mqinp", &s1).click()) {
        Serial.print("mqinp set to: ");
        Serial.println(b.build.value);
    }
    if (b.Dummy_("dummy", &s2).click()) {
        Serial.print("dummy set to: ");
        Serial.println(b.build.value);
    }
    if (b.Button().click()) hub.sendGetStr("dummy", String(random(100) / 10.0, 2));
}

void build_popup(gh::Builder& b) {
    b.beginRow();
    if (b.Button().label("push").click()) hub.sendPush(F("push!"));
    if (b.Button().label("notice").click()) hub.sendNotice(F("notice!"));
    if (b.Button().label("alert").click()) hub.sendAlert(F("alert!"));

    bool cfm;
    if (b.Confirm_("cfm", &cfm).text("confirm text").click()) {
        Serial.print("confirm: ");
        Serial.println(cfm);
    }

    String prom;
    if (b.Prompt_("prom", &prom).text("prompt text").click()) {
        Serial.print("prompt: ");
        Serial.println(prom);
    }
    if (b.Button().label("confirm").click()) hub.sendAction("cfm");
    if (b.Button().label("prompt").click()) hub.sendAction("prom");
    b.endRow();
}

void build_canvas(gh::Builder& b) {
    if (b.beginRow()) {
        gh::Canvas cv;
        gh::Pos pos;
        b.BeginCanvas_("cv", 400, 300, &cv, &pos);
        cv.stroke(0xff0000);
        cv.strokeWeight(5);
        cv.line(0, 0, -1, -1);
        cv.line(0, -1, -1, 0);
        b.EndCanvas();

        if (pos.changed()) {
            Serial.print(pos.x);
            Serial.print(',');
            Serial.println(pos.y);

            gh::CanvasUpdate cv("cv", &hub);
            cv.circle(pos.x, pos.y, 10);
            cv.send();
        }

        b.endRow();
    }
    {
        gh::Row r(b);
        gh::Canvas cv;
        gh::Pos pos;
        b.BeginCanvas_("cv2", 400, 300, &cv, &pos);
        cv.drawImage("/table.jpg", 0, 0, 400);
        b.EndCanvas();

        if (pos.changed()) {
            Serial.print(pos.x);
            Serial.print(',');
            Serial.println(pos.y);

            gh::CanvasUpdate cv("cv2", &hub);
            cv.circle(pos.x, pos.y, 10);
            cv.send();
        }
    }
}

void build(gh::Builder& b) {
    b.Plugin("/script.js", "/style.css");
    b.Menu(F("Basic;From file;MQTT;Popup;Canvas"));

    // b.beginRow();
    // b.Input_("inp");
    // b.Label("lbl");
    // b.Button_("btn");
    // b.endRow();

    switch (b.menu()) {
        case 0:
            build_input(b);
            build_input1(b);
            build_input2(b);
            build_input3(b);
            build_input4(b);
            build_input5(b);
            build_input6(b);
            build_input7(b);
            build_input8(b);
            break;
        case 1:
            build_ffile(b);
            break;
        case 2:
            build_mqtt(b);
            break;
        case 3:
            build_popup(b);
            break;
        case 4:
            build_canvas(b);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    // hlog.begin();

#ifdef GH_ESP_BUILD
    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_SSID, AP_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println(WiFi.localIP());

    hub.mqtt.config("test.mosquitto.org", 1883);
    // hub.mqtt.config("m8.wqtt.ru", 13448, "u_75SNWJ", "nWjHRClc");
#endif

    hub.onBuild(build);
    /*
        hub.onCLI([](String& str) {
            Serial.println(str);
            hub.printCLI(str + "!");
        });
    */
    /*

    hub.onInfo([](gh::Info& info) {
        switch (info.type) {
            case gh::Info::Type::Version:
                info.addStr("ur mom", "v3.14");
                break;
            case gh::Info::Type::Network:
                info.addStr(F("5G"), "50%");
                break;
            case gh::Info::Type::Memory:
                info.addStr(F("SD"), "10 GB");
                info.addInt(F("Int"), 100500);
                break;
            case gh::Info::Type::System:
                info.addFloat(F("Battery"), 3.63, 2);
                break;
        }
    });

            hub.onRequest([](gh::Request& req) -> bool {
                Serial.print("Request: ");
                Serial.print(gh::ReadConnection(req.client.connection));
                Serial.print(',');
                Serial.print(gh::ReadSource(req.client.source));
                Serial.print(',');
                Serial.print(req.client.id);
                Serial.print(',');
                Serial.print(gh::ReadEvent(req.event));
                Serial.print(',');
                Serial.print(req.name);
                // Serial.print(',');
                // Serial.print(req.value);
                Serial.println();
                return 1;
            });
            */

#ifdef GH_ESP_BUILD
    hub.onReboot([](gh::Reboot r) {
        Serial.println(gh::readReboot(r));
    });

    hub.onFetch([](gh::Fetcher& f) {
        // Serial.print("Fetch: ");
        // Serial.println(f.path);
        if (f.path == "/hub/lorem.txt") f.fetchBytes_P((const uint8_t*)lorem, strlen_P(lorem));
        // f.start;
    });

    hub.onUpload([](String& path) {
        Serial.print("Uploaded: ");
        Serial.println(path);
        if (path == "/data.dat") data.begin();
    });
#endif

    hub.begin();

    data.begin();
    // Serial.println(hub.getUI());
    // Serial.println(hub.getValues());
    hub.setBufferSize(500);

    // gh::CSVFile t("/table.csv", 3, 3);
    // t.addInt(0).addInt(1).addInt(2).endRow();
    // t.addInt(3).addInt(4).addInt(5).endRow();
    // t.addInt(6).addInt(7).addInt(8).endRow();
    // t.addInt(9).addInt(10).addInt(11).endRow();
}

void loop() {
    data.tick();
    hub.tick();

    static gh::Timer tmr(1000);
    if (tmr) {
    }
}