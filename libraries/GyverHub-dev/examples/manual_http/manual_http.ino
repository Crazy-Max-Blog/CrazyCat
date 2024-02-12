// пример работы только через HTTP вручную с WiFiServer
// в этом режиме работает всё, кроме обновлений update и sendXxx функций

#include <Arduino.h>

#define AP_SSID ""
#define AP_PASS ""

// отключаем встроенную реализацию HTTP+WS
// #define GH_NO_STREAM
#define GH_NO_HTTP
#define GH_NO_WS
// #define GH_NO_MQTT

// указываем что трансфер файлов будет не через http url
// потому что работу с файлами силами сервера мы не написали
#define GH_NO_HTTP_TRANSFER 

#include <GyverHub.h>
GyverHub hub;

#include <ESP8266WiFi.h>
// порт задаётся в конфиге библиотеки
WiFiServer server(GH_HTTP_PORT);

// будем хранить указатель на клиента для ответов в обработчике
WiFiClient* client_p = nullptr;

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
            if (client_p) client_p->print(manual.text.str);
            break;

        default:
            break;
    }
}

// парсим HTTP запросы
void http_loop() {
    WiFiClient client = server.accept();
    if (client) {
        String req = client.readStringUntil('/');  // "GET /"
        req = client.readStringUntil('\r');        // первая строка до конца
        while (client.available()) client.read();  // пропускаем остальное

        // нас интересует url /hub/... HTTP/, берём нужный сегмент
        int start = req.indexOf(F("hub/"));
        int end = req.indexOf(F(" HTTP/"));
        if (start < 0 || end < 0) return;
        req = req.substring(start + 4, end);

        // отвечаем клиенту с no-cors хэдером (для работы с внешним сайтом или приложением)
        client.print(F(
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
            "Access-Control-Allow-Origin:*\r\n"
            "Access-Control-Allow-Private-Network: true\r\n"
            "Access-Control-Allow-Methods:*\r\n\r\n"));

        // обработчик manual будет вызван внутри parse, так что указываем клиента
        client_p = &client;
        hub.parse((char*)req.c_str(), gh::Connection::HTTP);
        client_p = nullptr;
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

    server.begin();
}

void loop() {
    hub.tick();
    http_loop();
}