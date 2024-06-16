#include <SPI.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <PubSubClient.h>

#include "./utils.h"
#include "./configure.h"

// DHT sensor configuration
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Constants for soil moisture sensor
const int soilMoisturePin = A0;
const int dry = 595;
const int wet = 239;

// Variables for timing
unsigned long previousMillis = 0;
const long interval = 30000;  // 1 minute interval

bool wifiConnected = false;
bool mqttConnected = false;

WiFiSSLClient wifiClient;
PubSubClient *mqtt_client = NULL;

int requestId = 0;

#define TELEMETRY_SEND_INTERVAL 30000  // telemetry data sent every 5 seconds

long lastTelemetryMillis = 0;

// telemetry data values
float tempValue = 0.0;
float humidityValue = 0.0;
int soilMoistureValue = 0;

// grab the current time from internet time service
unsigned long getNow()
{
    IPAddress address(129, 6, 15, 28); // time.nist.gov NTP server
    const int NTP_PACKET_SIZE = 48;
    byte packetBuffer[NTP_PACKET_SIZE];
    WiFiUDP Udp;
    Udp.begin(2390);

    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;     // LI, Version, Mode
    packetBuffer[1] = 0;              // Stratum, or type of clock
    packetBuffer[2] = 6;              // Polling Interval
    packetBuffer[3] = 0xEC;           // Peer Clock Precision
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    Udp.beginPacket(address, 123);
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();

    // wait to see if a reply is available
    int waitCount = 0;
    while (waitCount < 20) {
        delay(500);
        waitCount++;
        if (Udp.parsePacket() ) {
            Udp.read(packetBuffer, NTP_PACKET_SIZE);
            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
            unsigned long secsSince1900 = highWord << 16 | lowWord;

            Udp.stop();
            return (secsSince1900 - 2208988800UL);
        }
    }
    return 0;
}

// IoT Hub MQTT publish topics
static const char IOT_EVENT_TOPIC[] = "devices/" IOT_CONFIG_DEVICE_ID "/messages/events/";

// reads the value from the DHT22 and soil moisture sensors
void readSensors() {
    tempValue = dht.readTemperature();
    humidityValue = dht.readHumidity();
    int sensorValue = analogRead(soilMoisturePin);
    soilMoistureValue = map(sensorValue, dry, wet, 0, 100);
}

// arduino setup function called once at device startup
void setup()
{
    Serial.begin(115200);
    delay(1000); // Small delay to ensure Serial Monitor catches all initial prints

    dht.begin();

    // Attempt to connect to Wifi network:
    Serial.print("WiFi Firmware version is ");
    Serial.println(WiFi.firmwareVersion());
    int status = WL_IDLE_STATUS;
    Serial_printf("Attempting to connect to Wi-Fi SSID: %s\n", IOT_CONFIG_WIFI_SSID);
    status = WiFi.begin(IOT_CONFIG_WIFI_SSID, IOT_CONFIG_WIFI_PASSWORD);
    while (status != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to Wi-Fi");

    String sasToken = IOT_CONFIG_SAS_TOKEN;
    String username = String(IOT_CONFIG_IOTHUB_FQDN) + "/" + String(IOT_CONFIG_DEVICE_ID) + "/?api-version=2021-04-12";

    // Debugging prints
    Serial.print("Connecting to MQTT broker at: ");
    Serial.print(IOT_CONFIG_IOTHUB_FQDN);
    Serial.print(":");
    Serial.println(AZ_IOT_DEFAULT_MQTT_CONNECT_PORT);
    Serial.print("Device ID: ");
    Serial.println(IOT_CONFIG_DEVICE_ID);
    Serial.print("Username: ");
    Serial.println(username);
    Serial.print("SAS Token: ");
    Serial.println(sasToken);

    // Connect to the IoT Hub MQTT broker
    mqtt_client = new PubSubClient(IOT_CONFIG_IOTHUB_FQDN, AZ_IOT_DEFAULT_MQTT_CONNECT_PORT, wifiClient);
    
    int retry = 0;
    while (retry < 10 && !mqtt_client->connected()) {     
        if (mqtt_client->connect(IOT_CONFIG_DEVICE_ID, username.c_str(), sasToken.c_str())) {
            Serial.println("===> mqtt connected");
            mqttConnected = true;
        } else {
            Serial.print("---> mqtt failed, rc=");
            Serial.println(mqtt_client->state());
            delay(2000);
            retry++;
        }
    }

    if (!mqttConnected) {
        Serial.println("Failed to connect to MQTT broker.");
    } else {
        Serial.println("Successfully connected to MQTT broker.");
    }

    // Initialize timers
    lastTelemetryMillis = millis();
}

// arduino message loop - do not do anything in here that will block the loop
void loop()
{
    if (mqtt_client->connected()) {
        // give the MQTT handler time to do its thing
        mqtt_client->loop();

        // read the sensor values
        if (millis() - lastTelemetryMillis > TELEMETRY_SEND_INTERVAL) {
            readSensors();
            
            // send telemetry values every 5 seconds
            Serial.println("Sending telemetry ...");
            String topic = (String)IOT_EVENT_TOPIC;
            topic.replace("{device_id}", IOT_CONFIG_DEVICE_ID);
            String payload = "{\"temperature\": {temp}, \"humidity\": {humidity}, \"soilMoisture\": {soilMoisture}}";
            payload.replace("{temp}", String(tempValue));
            payload.replace("{humidity}", String(humidityValue));
            payload.replace("{soilMoisture}", String(soilMoistureValue));
            Serial_printf("\t%s\n", payload.c_str());
            mqtt_client->publish(topic.c_str(), payload.c_str());

            lastTelemetryMillis = millis();
        }
    }
}
