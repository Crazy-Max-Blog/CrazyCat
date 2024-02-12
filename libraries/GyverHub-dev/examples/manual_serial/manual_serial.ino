// пример работы через Serial полностью вручную

#include <Arduino.h>

// можно выключить остальную связь
#define GH_NO_STREAM
#define GH_NO_HTTP
#define GH_NO_WS
#define GH_NO_MQTT

#include <GyverHub.h>
GyverHub hub;

// билдер
void build(gh::Builder& b) {
    static int val;
    b.Title(F("Hello!"));
    b.Slider(&val);
}

// обработчик ответа
void manual(gh::Manual manual) {
    if (manual.connection == gh::Connection::Serial) {
        // отправляем данные по сериал
        if (manual.text.pgm) Serial.print((FSTR)manual.text.str);
        else Serial.print(manual.text.str);
    }
}

void setup() {
    Serial.begin(115200);

    hub.config(F("MyDevices2"), F("AVR"), F(""));
    hub.onBuild(build);
    hub.onManual(manual);
    hub.begin();
}

void loop() {
    hub.tick();

    // задача - принять строку (оканчивается нулём) и передать её в парсер
    if (Serial.available()) {
        String str = Serial.readStringUntil('\0');
        hub.parse((char*)str.c_str(), gh::Connection::Serial);
        // подключенная функция manual будет вызвана внутри parse!
    }
}