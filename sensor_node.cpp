// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "libraries/AES/AES.h"
#include "libraries/AnalogSensor/AnalogSensor.h"
#include "libraries/CO2Monitor/CO2Monitor.h"
#include "libraries/PietteTech_DHT/PietteTech_DHT.h"
#include "libraries/HttpClient/HttpClient.h"
#include "key.h"

SYSTEM_MODE(MANUAL);

void dht_wrapper();

PietteTech_DHT dht(D2, DHT22, dht_wrapper);
AnalogSensor light(A2, 1);
CO2Monitor co2;
String myIDStr = Particle.deviceID();
String API_VERSION = String("v1.0");
HttpClient http;
char path[64];
char cookie[64];
bool dht_started;


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
    light.init();
    co2.init();
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

void dht_wrapper() {
    dht.isrCallback();
}

unsigned int nextTime = 0;
void loop() {
    light.read();
    if (nextTime > millis()) return;
    if(WiFi.ready()){
        if(!dht_started){
            dht.acquire();
            dht_started = true;
            Serial.println("Acquiring");
        }
        if(!dht.acquiring()){
            Serial.println("Acquired");
            int dht_result = dht.getStatus();
            char json_body[128];
            char body_out[128];
            char h[6];
            char c[6];
            char dp[6];
            String(dht.getHumidity()).toCharArray(h, 6);
            String(dht.getCelsius()).toCharArray(c, 6);
            String(dht.getDewPoint()).toCharArray(dp, 6);
            sprintf(json_body, "{\"temperature\":%s, \"humidity\":%s, \"dew_point\":%s, \"light\":%d , \"co2\":%d}", c, h, dp, light.read(), co2.read());
            aes_128_encrypt(json_body, KEY, body_out);
            Serial.println("Body: ");
            Serial.println(json_body);
            Serial.println(sizeof(body_out));
            memcpy(request.body, body_out, 128);
            http.post(request, response, headers);
            Serial.print("Response status: ");
            Serial.println(response.status);
            Serial.print("HTTP Response Body: ");
            Serial.println(response.body);
            dht_started = false;
            nextTime = millis() + 1000;
        }
    }else{
        connect_wifi();
    }
}
