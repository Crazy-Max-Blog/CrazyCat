// пример работы через MQTT через PubSubClient
// работа с PubSubClient взята из примера mqtt_esp8266.ino

#include <Arduino.h>

#define AP_SSID ""
#define AP_PASS ""

// отключаем встроенную реализацию MQTT (для esp)
#define GH_NO_MQTT

#include <GyverHub.h>
GyverHub hub;

// MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

// билдер
void build(gh::Builder& b) {
    static int val;
    b.Title(F("Hello!"));
    b.Slider(&val);
}

// обработчик ответа
void manual(gh::Manual m) {
    switch (m.connection) {
        case gh::Connection::MQTT:
            client.beginPublish(m.topic, m.text.len, 0);
            if (m.text.pgm) {
                uint8_t buf[m.text.len];
                memcpy_P(buf, m.text.str, m.text.len);
                client.write(buf, m.text.len);
            } else {
                client.write((uint8_t*)m.text.str, m.text.len);
            }
            client.endPublish();
            break;

        default:
            break;
    }
}

// MQTT
void callback(char* topic, byte* data, unsigned int len) {
    char data_buf[len + 1];
    memcpy(data_buf, data, len);
    data_buf[len] = 0;

    uint16_t tlen = strlen(topic);
    char topic_buf[tlen + 1];
    memcpy(topic_buf, topic, tlen);
    topic_buf[tlen] = 0;

    hub.parse(topic_buf, data_buf, gh::Connection::MQTT);
}

// MQTT
void reconnect() {
    while (!client.connected()) {
        String clientId = "hub-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.subscribe(hub.topicDiscover().c_str());
            client.subscribe(hub.topicHub().c_str());
        } else {
            delay(1000);
        }
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

    // MQTT
    client.setServer("test.mosquitto.org", 1883);
    client.setCallback(callback);
}

void loop() {
    hub.tick();

    // MQTT
    if (!client.connected()) reconnect();
    client.loop();
}