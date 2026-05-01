#include "esp_camera.h"
#include <HTTPClient.h>
#include <WiFi.h>

void startCameraServer();

// ===== AI THINKER CAMERA PINS =====
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5

#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// FLASH LED
#define FLASH_LED 4

// WIFI
const char *ssid = "KAVIN";
const char *password = "123456789";

// SERVER URL
const char *serverName = "http://10.171.245.229:5000/upload"; // 🔥 CHANGE THIS

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  ledcAttach(FLASH_LED, 5000, 8);
  ledcWrite(FLASH_LED, 0);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA; // stable
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 14;
    config.fb_count = 1;
  }

  // CAMERA INIT
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("CAMERA FAIL");
    return;
  }

  Serial.println("CAMERA OK");

  // WIFI CONNECT
  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  startCameraServer();
  Serial.print("IP:");
  Serial.println(WiFi.localIP());
}

// ===== LOOP =====
void loop() {

  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "GET_IP") {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP:");
        Serial.println(WiFi.localIP());
      }
    } else if (cmd == "CAPTURE") {

      Serial.println("CMD RECEIVED");

      // FLASH ON (Low brightness to prevent Brownout crashes)
      ledcWrite(FLASH_LED, 20);
      delay(200);

      // CAPTURE IMAGE
      camera_fb_t *fb = esp_camera_fb_get();

      // FLASH OFF
      ledcWrite(FLASH_LED, 0);

      if (!fb) {
        Serial.println("UPLOAD_FAILED");
        return;
      }

      Serial.println("CAPTURED");

      // SEND TO SERVER
      if (WiFi.status() == WL_CONNECTED) {

        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "image/jpeg");

        int httpResponseCode = http.POST(fb->buf, fb->len);

        if (httpResponseCode > 0) {

          String response = http.getString();
          response.trim();

          if (response == "OK") {
            Serial.println("UPLOAD_SUCCESS");
          } else {
            Serial.println("UPLOAD_FAILED");
          }

        } else {
          Serial.println("UPLOAD_FAILED");
        }

        http.end();

      } else {
        Serial.println("UPLOAD_FAILED");
      }

      esp_camera_fb_return(fb);
    }
  }
}