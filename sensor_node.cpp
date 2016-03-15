#include "application.h"
#include "libraries/AES/AES.h"
#include "libraries/AnalogSensor/AnalogSensor.h"
#include "libraries/DigitalOut/DigitalOut.h"
#include "libraries/DHT/DHT.h"
#include "libraries/CO2Monitor/CO2Monitor.h"
#include "libraries/HttpClient/HttpClient.h"
#include "libraries/MQTT/firmware/MQTT.h"
#include "key.h"

#define MQTT_PORT 1883
#define SERVER_NAME "beagle.local"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(MANUAL);

void callback(char* topic, byte* payload, unsigned int length);

DHT dht(D0, DHT22);
AnalogSensor water_temp(A0);
DigitalOut lights(D6);
DigitalOut pump(D5);
DigitalOut fan(D4);
String myIDStr = Particle.deviceID();
String API_VERSION = String("v1.0");
HttpClient http;
byte server[] = { 192,168,1,5 };
char path[64];
char cookie[64];

MQTT mqtt(server, MQTT_PORT, callback);

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
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.subnetMask());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.SSID());
}

void fix_connection(){
    Serial.println("Fixing WiFi");
    WiFi.disconnect();
    delay(1000);
    WiFi.off();
    delay(1000);
    WiFi.on();
    delay(1000);
    connect_wifi();
}

void handle_action(int pin, int action){
    digitalWrite(pin, action);
}

void parse_data(String message, int *pin, int *action){
    int c_pos = message.indexOf(",");
    *pin = message.substring(0, c_pos).toInt();
    *action = message.substring(c_pos+1).toInt();
}

void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    int pin, action;
    parse_data(message, &pin, &action);
    Serial.print("Action: "); Serial.print(pin); Serial.println(action);
    handle_action(pin, action);
    delay(100);
}

void handle_mqtt(){
    if(mqtt.isConnected()){
        mqtt.loop();
    }
}

unsigned int nextTime = 0;
void post_sensors(){
    //slow down readings
    if (nextTime > millis()) return;
    water_temp.read();
    nextTime = millis() + 5000;
    Serial.println(String("Millis: "+String(nextTime)));
    char json_body[128];
    char body_out[128];
    float w = water_temp.read()*.8192;
    float wt = (w-500)/10;
    sprintf(json_body, "{\"temperature\":%.2f, \"humidity\":%.2f, \"water_temp\":%.2f}", dht.readTemperature(false), dht.readHumidity(), wt);
    Serial.println(json_body);
    aes_128_encrypt(json_body, KEY, body_out);
    memcpy(request.body, body_out, 128);
    http.post(request, response, headers);
    Serial.print("Response status: ");
    Serial.println(response.status);
    if(response.status == -1){
        fix_connection();
    }
}

void setup() {
    Serial.begin(9600);
    while(!Serial.available())  delay(10);
    connect_wifi();
    Serial.println("beginning");
    RGB.control(true);
    RGB.color(0, 255, 255);
    RGB.brightness(255);
    Serial.println("running");
    mqtt.connect(myIDStr);
    if (mqtt.isConnected()) {
        mqtt.publish("/broadcast/new", myIDStr);//probably broadcast capabilities
        mqtt.subscribe(String("/"+myIDStr));
    }
    String s_path = String("/api/"+API_VERSION+"/node/"+myIDStr+"/sensors");
    s_path.toCharArray(path, 64);
    String cook = String("lablog="+myIDStr);
    cook.toCharArray(cookie, 64);
    request.hostname = "entropealabs.com";
    request.port = 80;
    request.path = path;
    water_temp.init();
    lights.init();
    pump.init();
    fan.init();
    dht.begin();
}

void loop() {
    handle_mqtt();
    post_sensors();
}
