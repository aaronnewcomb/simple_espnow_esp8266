//master/sender ESP-8266

//#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <Ticker.h>
extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}

typedef struct esp_now_peer_info {
    u8 peer_addr[6];    /**< ESPNOW peer MAC address that is also the MAC address of station or softap */
    uint8_t channel;                        /**< Wi-Fi channel that peer uses to send/receive ESPNOW data. If the value is 0,
                                                 use the current channel which station or softap is on. Otherwise, it must be
                                                 set as the channel that station or softap is on. */
} esp_now_peer_info_t;

#define CHANNEL 0

uint8_t remoteMac[] = {0xA0, 0x20, 0xA6, 0x00, 0xF1, 0xCC}; // Replace with the AP MAC address of the slave/receiver
uint8_t data = 1;
bool retry = true;

void printMacAddress(uint8_t* macaddr) {
  Serial.print("{");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    Serial.print(macaddr[i], HEX);
    if (i < 5) Serial.print(',');
  }
  Serial.println("};");
}

// Init ESP Now with fallback
void InitESPNow() {
  if (esp_now_init() == 0) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
}

void sendData() {
  data++;
  int result = esp_now_send(remoteMac, &data, sizeof(data));
  Serial.print("Send Command: ");
  if (result ==0) {
    Serial.println("Success " + String(result));
  } else {
    Serial.println("Failed " + String(result));
  }
  delay(100);
}

// callback when data is sent from Master to Slave
esp_now_send_cb_t OnDataSent(const uint8_t *mac_addr, u8 status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == 0 ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  Serial.println("ESPNow Basic Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  // esp_now_register_send_cb(OnDataSent);
  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    printMacAddress(macaddr);
    static uint32_t ok = 0;
    static uint32_t fail = 0;
    if (status == 0) {
      Serial.println("ESPNOW: ACK_OK");
      ok++;
      retry = false;
    }
    else {
      Serial.println("ESPNOW: SEND_FAILED");
      fail++;
    }
    Serial.printf("[SUCCESS] = %lu/%lu \r\n", ok, ok+fail);
  });
  int addStatus = esp_now_add_peer((u8*)remoteMac, ESP_NOW_ROLE_CONTROLLER, CHANNEL, NULL, 0);
  if (addStatus == 0) {
    // Pair success
    Serial.println("Pair success");
  } else {
    Serial.println("Pair failed");
  }
}

void loop() {
  while (retry) {
    sendData();
    delay(100);
  }
  retry = true;
  delay(10000);
}
