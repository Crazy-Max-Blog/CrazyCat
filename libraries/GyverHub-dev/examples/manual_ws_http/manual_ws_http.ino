// пример работы через Websocket + HTTP с ESP8266WebServer
#include <Arduino.h>

#define AP_SSID ""
#define AP_PASS ""

// отключаем встроенную реализацию HTTP+WS
#define GH_NO_STREAM
// #define GH_NO_HTTP
// #define GH_NO_WS
#define GH_NO_MQTT

// указываем что трансфер файлов будет по вебсокет, 
// потому что работу с файлами силами сервера мы не написали
#define GH_NO_HTTP_TRANSFER 

#include <GyverHub.h>
GyverHub hub;

#include <WebSocketsServer.h>
// порт задаётся в конфиге библиотеки. Если нужно изменить на другой - менять нужно там!
WebSocketsServer ws(GH_WS_PORT, "", "hub");

// будем хранить ID для ответов клиенту
uint8_t ws_id = 0;

#include <ESP8266WebServer.h>
ESP8266WebServer server;

// билдер
void build(gh::Builder& b) {
    static int val;
    b.Title(F("Hello!"));
    b.Slider(&val);
}

// обработчик ответа
void manual(gh::Manual manual) {
    switch (manual.connection) {
        case gh::Connection::HTTP:
            // отвечаем клиенту
            if (manual.text.pgm) server.sendContent_P(manual.text.str, manual.text.len);
            else server.sendContent(manual.text.str);
            break;

        case gh::Connection::WS:
            if (manual.ws_all) ws.broadcastTXT(manual.text.str, manual.text.len);
            else ws.sendTXT(ws_id, manual.text.str, manual.text.len);
            break;

        default:
            break;
    }
}

// обработчик вебсокета
void ws_event(uint8_t num, WStype_t type, uint8_t* data, GH_UNUSED size_t len) {
    if (type == WStype_TEXT) {
        ws_id = num;
        hub.parse((char*)data, gh::Connection::WS);
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_SSID, AP_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println(WiFi.localIP());

    hub.config(F("MyDevices2"), F("ESP"), F(""));
    hub.onBuild(build);
    hub.onManual(manual);
    hub.begin();

    ws.onEvent(ws_event);
    ws.begin();

    server.onNotFound([]() {
        if (server.uri().startsWith(F("/hub/"))) {
            server.setContentLength(CONTENT_LENGTH_UNKNOWN);
            server.send(200, "text/plain");
            // нас интересует часть url без /hub/
            hub.parse(((char*)server.uri().c_str()) + 5, gh::Connection::HTTP);  // +5 == "/hub/"
            server.sendContent("");
            server.client().stop();
        }
    });
    server.begin(GH_HTTP_PORT);
    server.enableCORS(true);    // нужно для работы с приложением и внешним сайтом
}

void loop() {
    hub.tick();
    server.handleClient();
    ws.loop();
}