#include "application.h"
#include "libraries/AES/AES.h"
#include "libraries/AnalogSensor/AnalogSensor.h"
#include "libraries/DHT/DHT.h"
#include "libraries/CO2Monitor/CO2Monitor.h"
#include "libraries/HttpClient/HttpClient.h"
#include "libraries/DustSensor/DustSensor.h"
#include "key.h"

SYSTEM_MODE(MANUAL);

DHT dht(D2, DHT22);
DustSensor dust(D3);
AnalogSensor light(A2);
CO2Monitor co2;
String myIDStr = Particle.deviceID();
String API_VERSION = String("v1.0");
HttpClient http;
char path[64];
char cookie[64];

http_request_t request;
http_response_t response;
http_header_t headers[] = {
    { "Content-Type", "application/json" },
    { "Accept" , "application/json" },
    { "Cookie", "lablog=2983749283749287349827349" },
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

void connect_wifi(){
    while(!WiFi.ready()){
        WiFi.connect();
        Serial.println("Connecting to Wifi...");
        delay(500);
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("beginning");
    light.init();
    co2.init();
    dht.begin();
    dust.begin();
    RGB.control(true);
    RGB.brightness(5);
    connect_wifi();
    Serial.println("running");
    Serial.println(request.path);
    String s_path = String("/api/"+API_VERSION+"/node/"+myIDStr+"/sensors");
    s_path.toCharArray(path, 64);
    String cook = String("lablog="+myIDStr);
    cook.toCharArray(cookie, 64);
    request.hostname = "crtlabsdev.realtors.org";
    request.port = 80;
    request.path = path;
}

unsigned int nextTime = 0;
unsigned int counter = 0;
void loop() {
    light.read();
    dust.read();
    if (nextTime > millis()) return;
    if(WiFi.ready()){
        char json_body[128];
        char body_out[128];
        sprintf(json_body, "{\"temperature\":%.2f, \"humidity\":%.2f, \"light\":%d , \"co2\":%d, \"dust\":%.2f}", dht.readTemperature(false), dht.readHumidity(), light.read(), co2.read(), dust.read());
        aes_128_encrypt(json_body, KEY, body_out);
        memcpy(request.body, body_out, 128);
        http.post(request, response, headers);
        nextTime = millis() + 1000;
        counter++;
        Serial.print("Count: ");
        Serial.println(counter);
        Serial.print("Response status: ");
        Serial.println(response.status);
        Serial.print("HTTP Response Body: ");
        Serial.println(response.body);
    }else{
        connect_wifi();
    }
}
