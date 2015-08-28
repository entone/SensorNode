// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "libraries/AES/AES.h"
#include "libraries/AnalogSensor/AnalogSensor.h"
#include "libraries/DHT/DHT.h"
#include "libraries/HttpClient/HttpClient.h"

#define LOGGING;

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

unsigned char KEY[16] = "111111111111111";

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

char l_out[10];
char p_out[10];
char h_out[10];
char t_out[10];
char json_body[128];
unsigned int nextTime = 0;
void loop() {
    light.read();
    pot.read();
    if (nextTime > millis()) return;
    if(WiFi.ready()){
        sprintf(json_body, "{\"light\":%d , \"pot\":%d, \"temperature\":%.2f, \"humidity\":%.2f}", light.read(), pot.read(), dht.readTemperature(false), dht.readHumidity());
        Serial.println("Body: ");
        Serial.println(json_body);
        request.body = json_body;
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
