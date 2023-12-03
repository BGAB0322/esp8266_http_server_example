#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#define PORT_NUMBER 80

const char* SSID   = "your wifi ssid"; 
const char* PASSWD = "your wifi password";

IPAddress IP_ADDRESS(192, 168, 0, 0);
IPAddress GATE_WAY(192, 168, 0, 1);
IPAddress SUBNET_MASK(255, 255, 255, 0);

ESP8266WebServer http_server(PORT_NUMBER);

String TOTAL_RESOURCES   = "/projects/http_client/data_types/total_data";
String INTEGER_RESOURCES = "/projects/http_client/data_types/integer";
String ARRAY_RESOURCES   = "/projects/http_client/data_types/array";

uint8_t retries;

struct server_parameter
{
    uint8_t one_data;
    uint8_t array_data[15];
} server_param;

void setup()
{
    Serial.begin(115200);
    Serial.println("Connecting to Wi-Fi AP...");

    WiFi.mode(WIFI_STA);    
    WiFi.config(IP_ADDRESS, GATE_WAY, SUBNET_MASK);
    WiFi.begin(SSID, PASSWD);

    initParameter();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);

        Serial.print(".");
        retries++;

        if (retries > 50)
        {
            Serial.print("[ERROR] Failed to Connect ");
            Serial.println(SSID);

            EspClass esp_module;
            esp_module.restart();
        }       
        else if (WiFi.status() == WL_CONNECTED)
        {
            Serial.print("Connected to ");
            Serial.print(SSID);
            Serial.print("--- IP: ");
            Serial.println(WiFi.localIP());
            retries = 0;
        }
    }

    http_server.on(TOTAL_RESOURCES,    HTTP_GET,   getMethodTotalDataHandle);  
    http_server.on(INTEGER_RESOURCES,  HTTP_POST,  postMethodIntegerHandle);
    http_server.on(ARRAY_RESOURCES,    HTTP_POST,  postMethodArrayHandle);

    http_server.begin();
    Serial.println("HTTP SERVER ON");
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        http_server.handleClient();
    }
    else
    {
        WiFi.reconnect();

        Serial.println("Reconnecting to Wi-Fi AP...");

        while(WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
    
            delay(500);
            retries++;

            if (retries > 50)
            {
                Serial.print("[ERROR] Failed to Connect ");
                Serial.println(SSID);

                EspClass esp_module;
                esp_module.restart();
            }
            else if (WiFi.status() == WL_CONNECTED)
            {
                Serial.print("Reconnected to ");
                Serial.print(SSID);
                Serial.print("--- IP: ");
                Serial.println(WiFi.localIP());
                retries = 0;
            }
        }
    }
}

void getMethodTotalDataHandle()
{
    StaticJsonDocument<500> doc;
    char server_parameter_data[500];

    doc["Integer"] = server_param.one_data;

    JsonArray IntegerArray = doc.createNestedArray("IntegerArray");
    
    for (uint8_t i = 0; i < 15; i++)
    {
        IntegerArray.add(server_param.array_data[i]);
    }

    serializeJsonPretty(doc, server_parameter_data, sizeof(server_parameter_data));
    http_server.send(200, "application/json", server_parameter_data);
}

void postMethodIntegerHandle()
{
    StaticJsonDocument<300> doc;
    String post_body = http_server.arg("plain");

    Serial.print("HTTP Method: ");
    Serial.println(http_server.method());

    DeserializationError error_number = deserializeJson(doc, post_body);

    if (error_number)
    {
        Serial.println("[ERROR] Failed Parsing JSON Body");
        http_server.send(400);
    }
    else
    {
        if (http_server.method() == HTTP_POST)
        {
            server_param.one_data = doc["value"];

            http_server.send(201);
        }
    }
}

void postMethodArrayHandle()
{
    StaticJsonDocument<300> doc;
    String post_body = http_server.arg("plain");

    Serial.print("HTTP Method: ");
    Serial.println(http_server.method());

    DeserializationError error_number = deserializeJson(doc, post_body);

    if (error_number)
    {
        Serial.println("[ERROR] Failed Parsing JSON Body");
        http_server.send(400);
    }
    else
    {
        if (http_server.method() == HTTP_POST)
        {
            for (uint8_t i = 0; i < 15; i++)
            {
                server_param.array_data[i] = doc["value"][i];
            }

            http_server.send(202);
        }
    }
}

void initParameter()
{
    server_param.one_data = 0;

    for (uint8_t i = 0; i < 15; i++)
    {
        server_param.array_data[i] = 0;
    }
}