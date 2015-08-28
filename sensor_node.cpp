// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "libraries/AES/AES.h"
#include "libraries/AnalogSensor/AnalogSensor.h"
#include "libraries/DHT/DHT.h"
#include "libraries/HttpClient/HttpClient.h"
#include "key.h"

SYSTEM_MODE(MANUAL);

IPAddress server(192,168,1,174);

DHT dht(D0, DHT22);
AnalogSensor light(A2, 1);
AnalogSensor pot(A4, 2);
String myIDStr = Spark.deviceID();
HttpClient http;
char path[64];

http_request_t request;
http_response_t response;
http_header_t headers[] = {
    { "Content-Type", "application/json" },
    { "Accept" , "application/json" },
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
    pot.init();
    dht.begin();
    RGB.control(true);
    RGB.brightness(5);
    connect_wifi();
    Serial.println("running");
    Serial.println(request.path);
    String s_path = String("/node/"+myIDStr+"/sensors");
    s_path.toCharArray(path, 64);
    //request.ip = server;
    request.hostname = "crtlabsdev.realtors.org";
    request.port = 80;
    request.path = path;
}

unsigned int nextTime = 0;
void loop() {
    light.read();
    pot.read();
    if (nextTime > millis()) return;
    if(WiFi.ready()){
        char json_body[128];
        char body_out[128];
        sprintf(json_body, "{\"light\":%d , \"pot\":%d, \"temperature\":%.2f, \"humidity\":%.2f}", light.read(), pot.read(), dht.readTemperature(false), dht.readHumidity());
        aes_128_encrypt(json_body, KEY, body_out);
        Serial.println("Body: ");
        Serial.println(json_body);
        Serial.println(sizeof(body_out));
        memcpy(request.body, body_out, 128);
        http.post(request, response, headers);
        Serial.print("Application>\tResponse status: ");
        Serial.println(response.status);
        Serial.print("Application>\tHTTP Response Body: ");
        Serial.println(response.body);
        nextTime = millis() + 100;
    }else{
        connect_wifi();
    }
}
