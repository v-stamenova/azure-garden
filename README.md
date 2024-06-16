# AzureGarden - Project Build for CCO Course

AzureGarden is a project built as a PoC for the Cloud Computing course at HZ UAS. The main goal of AzureGarden is to help plant owners take care of their plants. Since AzureGarden is, as of now, just a Proof of Concept (PoC). It includes hardware components, cloud computing services, and even some local development.

![Azure Diagram](/images/azure-diagram.png)

## Arduino UNO R4 Wi-Fi

The system uses this specific Arduino board as central hardware to communicate with Azure IoT Hub. Two sensors are connected to the Arduino - one capacitive soil moisture sensor and a DHT22 (measuring temperature and air humidity). 
![Arduino Circuit](/images/arduino-circuit.png)

Based on the required configurations/variables below, the Arduino first tries to connect to the Wi-Fi network. If the connection is successful, the board then tries to establish a connection to the Azure IoT Hub. The board prepares the necessary details to connect to the Azure IoT Hub via MQTT. If that connection is successful as well, the board reads the sensor data every 30 seconds and sends the telemetry to the Azure IoT Hub. The required variables can be found in `configure.example.h` but are listed here as well:

```cpp
#define IOT_CONFIG_WIFI_SSID // Network's name
#define IOT_CONFIG_WIFI_PASSWORD // Network's passowrd
#define IOT_CONFIG_IOTHUB_FQDN // Azure IoT Hub's host name
#define IOT_CONFIG_DEVICE_ID // The ID of the configured device in Azure IoT Hub
#define IOT_CONFIG_DEVICE_KEY // The primary or secondary key of the configured device in Azure IoT Hub
#define AZ_IOT_DEFAULT_MQTT_CONNECT_PORT 8883 // Default port for MQTT
#define IOT_CONFIG_SAS_TOKEN "SharedAccessSignature sr=" //SAS Token (Can be created via Azure Cloud Shell )
```

## Azure Cloud Services

Three Azure Services were utilized during this project - Azure IoT Hub, Azure Function App, and Azure Storage Account. Azure IoT Hub was used to establish communication between the Arduino and the Cloud. Since the data received from the Arduino is structured and non-relational, a full database would be overkill for a PoC. Therefore, the service chosen for storing the data is Azure Storage Account and more specifically the Table Storage. Azure Function App is used to save the data received from the Azure IoT Hub to the Azure Storage Account. The script for the Azure Function App is provided as well in the `azure-function-app`.

## Local components

The repository also contains scripts for two services. One of the servers is responsible for fetching data from the Azure Storage Account and acting as an API, while the other one is purely front-end, responsible for fetching data from the API and displaying it. The data that can be retrieved from the API includes the last or the last 20 records from the table in the Azure Storage Account. The only requirement is to create an `.env` file in the `azure-sat-api` with the following variables, and most likely to change the columns in the `start.js:`
```js
ACCOUNT_NAME= //The name of the Azure Storage Account
ACCOUNT_KEY= // The access key for the Azure Storage Account
TABLES_URL= // The URL of the Table in the Azure Storage Account that contains the required data
TABLE_NAME= // The name  of the Table in the Azure Storage Account that contains the required data
```

