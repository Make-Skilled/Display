#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "tsr";
const char* password = "1234567890";

// API URL
const char* apiUrl = "http://62.72.58.116/fetchMessages.php";

// Message management
const int MAX_MESSAGES = 5;
String messages[MAX_MESSAGES];
volatile int messageCount = 0;
volatile int currentMessage = 0;

// Mutex for thread safety
SemaphoreHandle_t xMutex = NULL;

void wifiTask(void * parameter) {
  while (1) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(apiUrl);
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error && doc["success"].as<bool>()) {
          if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            JsonArray msgArray = doc["messages"];
            messageCount = min((int)msgArray.size(), MAX_MESSAGES);
            for (int i = 0; i < messageCount; i++) {
              messages[i] = msgArray[i]["message"].as<String>();
            }
            xSemaphoreGive(xMutex);
          }
        }
      }

      http.end();
    }

    vTaskDelay(pdMS_TO_TICKS(30000)); // Fetch every 30 seconds
  }
}

void senderTask(void * parameter) {
  while (1) {
    if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (messageCount > 0) {
        String msg = messages[currentMessage];
        Serial.println(msg); // Send to Arduino
        currentMessage = (currentMessage + 1) % messageCount;
      } else {
        Serial.println("Waiting for messages...");
      }
      xSemaphoreGive(xMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(5000)); // Send every 5 seconds
  }
}

void setup() {
  Serial.begin(115200);  // Serial to Arduino
  delay(1000);

  xMutex = xSemaphoreCreateMutex();
  if (xMutex == NULL) return;

  WiFi.mode(WIFI_STA);

  xTaskCreatePinnedToCore(wifiTask, "WiFiTask", 8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(senderTask, "SenderTask", 4096, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelay(1);  // Let FreeRTOS manage tasks
}
