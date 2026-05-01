#define BLYNK_TEMPLATE_ID "TMPL3KCZTd743" // Replace if different
#define BLYNK_TEMPLATE_NAME "industrial"
#define BLYNK_AUTH_TOKEN "SOOg4s-DFhGwabw_LnpbSXwFPCxHar_Y"

#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <Wire.h>

// --- PIN DEFINITIONS ---
#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 4

// Motor Driver (L298N)
#define ENA 13
#define IN1 14
#define IN2 27
#define IN3 26
#define IN4 25
#define ENB 33

// PWM Settings
const int freq = 5000;
const int ledc_resolution = 8;

// Relay for Mist Maker
#define RELAY_MIST 23

// ESP32-CAM Serial Connection
#define CAM_RX 16
#define CAM_TX 17

// --- WiFi & Server Details ---
const char *ssid = "KAVIN";
const char *password = "123456789";
String serverName = "http://10.171.245.229:5000/get_result";

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// --- STATE VARIABLES ---
bool isScanning = false;
bool manualMode = false; // Controlled by V4 switch
int motorSpeed = 150;    // Default speed (0-255)

// --- BLYNK CALLBACKS ---

// Mode Switch (V4)
BLYNK_WRITE(V4) {
  manualMode = param.asInt();
  stopMotors();
  lcd.clear();
  if (manualMode) {
    lcd.print("Manual Mode");
    lcd.setCursor(0, 1);
    lcd.print("Ready to Drive");
  } else {
    lcd.print("Auto Mode");
    isScanning = false;
  }
}

// Direction Controls (V0-V3)
BLYNK_WRITE(V0) {
  if (manualMode && param.asInt())
    moveForward();
  else if (manualMode)
    stopMotors();
}
BLYNK_WRITE(V1) {
  if (manualMode && param.asInt())
    moveBackward();
  else if (manualMode)
    stopMotors();
}
BLYNK_WRITE(V2) {
  if (manualMode && param.asInt())
    turnLeft();
  else if (manualMode)
    stopMotors();
}
BLYNK_WRITE(V3) {
  if (manualMode && param.asInt())
    turnRight();
  else if (manualMode)
    stopMotors();
}

// Speed Control (V6)
BLYNK_WRITE(V6) {
  motorSpeed = param.asInt();
}

// Manual Mist Test (V7) - Hold to Spray, Release to Stop (Silent)
BLYNK_WRITE(V7) {
  if (param.asInt()) {
    digitalWrite(RELAY_MIST, LOW);   // Relay ON
  } else {
    digitalWrite(RELAY_MIST, HIGH);  // Relay OFF
  }
}

// Periodically send distance to Blynk (V5)
void sendSensorData() {
  long distance = getDistance();
  Blynk.virtualWrite(V5, distance);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, CAM_RX, CAM_TX);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is OFF at start
  pinMode(RELAY_MIST, OUTPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, freq, ledc_resolution);
  ledcAttach(ENB, freq, ledc_resolution);

  digitalWrite(RELAY_MIST, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin();
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("Connecting...");

  // Connect to Blynk and WiFi
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  lcd.clear();
  lcd.print("Rover Ready!");

  // Set timer to update distance every 500ms
  timer.setInterval(500L, sendSensorData);

  // Try to get Camera IP
  lcd.clear();
  lcd.print("Waiting for Cam");
  lcd.setCursor(0, 1);
  lcd.print("IP Address...");

  unsigned long startWait = millis();
  String camIP = "";
  while (millis() - startWait < 10000) {
    Blynk.run(); // Keep Blynk alive
    if (Serial2.available()) {
      String line = Serial2.readStringUntil('\n');
      line.trim();
      if (line.startsWith("IP:")) {
        camIP = line.substring(3);
        break;
      }
    }
    delay(10);
  }

  lcd.clear();
  if (camIP != "") {
    lcd.print("Cam IP:");
    lcd.setCursor(0, 1);
    lcd.print(camIP);
  } else {
    lcd.print("Cam IP Timeout");
  }
  blynkSafeDelay(2000);
}

void loop() {
  Blynk.run();
  timer.run();

  // Check for Serial commands for testing
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "MIST") {
      triggerMist("Manual Mist (Serial)");
    }
  }

  if (!manualMode) {
    autonomousLoop();
  }
}

void autonomousLoop() {
  long distance = getDistance();

  if (distance > 0 && distance < 15) {
    isScanning = false;
    stopMotors();

    lcd.clear();
    lcd.print("Plant Detected");
    lcd.setCursor(0, 1);
    lcd.print("Analyzing...");

    Serial2.println("CAPTURE");

    bool uploadSuccess = false;
    unsigned long startUploadWait = millis();
    while (millis() - startUploadWait < 15000) {
      Blynk.run(); // Essential for preventing disconnection
      if (Serial2.available()) {
        String camResponse = Serial2.readStringUntil('\n');
        camResponse.trim();
        if (camResponse == "UPLOAD_SUCCESS") {
          uploadSuccess = true;
          break;
        } else if (camResponse == "UPLOAD_FAILED") {
          break;
        }
      }
      delay(10);
    }

    if (uploadSuccess) {
      checkResultFromServer();
    } else {
      lcd.clear();
      lcd.print("Cam/Upload Error");
      blynkSafeDelay(2000);
    }

    // Resume movement
    moveBackward();
    blynkSafeDelay(600);
    stopMotors();
    blynkSafeDelay(200);
    turnRight();
    blynkSafeDelay(800);
    stopMotors();

  } else {
    moveForward();
    if (!isScanning) {
      lcd.clear();
      lcd.print("Scanning...");
      isScanning = true;
    }
    blynkSafeDelay(50);
  }
}

// --- HELPER FUNCTIONS ---

// Custom delay that keeps Blynk alive
void blynkSafeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    Blynk.run();
    timer.run();
    delay(1);
  }
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duration == 0)
    return 100;
  return duration * 0.034 / 2;
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ENA, motorSpeed);
  ledcWrite(ENB, motorSpeed);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, motorSpeed);
  ledcWrite(ENB, motorSpeed);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, motorSpeed);
  ledcWrite(ENB, motorSpeed);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ENA, motorSpeed);
  ledcWrite(ENB, motorSpeed);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

void checkResultFromServer() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  http.begin(serverName);
  http.setTimeout(5000);

  int retryCount = 0;
  bool success = false;

  while (retryCount < 3 && !success) {
    Blynk.run();
    int responseCode = http.GET();

    if (responseCode == 200) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        String status = doc["status"];
        if (status == "complete") {
          success = true;
          handlePlantResponse(doc);
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Processing...");
          blynkSafeDelay(3000);
          retryCount++;
        }
      }
    } else {
      retryCount++;
      blynkSafeDelay(1000);
    }
  }
  http.end();

  if (!success) {
    lcd.clear();
    lcd.print("Server Error");
    blynkSafeDelay(2000);
  }
}

void handlePlantResponse(JsonDocument &doc) {
  String condition = doc["condition"];
  String disease = doc["disease_name"];

  lcd.clear();
  if (condition == "disease") {
    lcd.print("D: ");
    lcd.print(disease.substring(0, 13));
    lcd.setCursor(0, 1);
    lcd.print("Spraying Mist...");

    triggerMist(disease);
  } else {
    lcd.print("Healthy Plant!");
    blynkSafeDelay(3000);
  }
}

// Reusable Mist Trigger Function
void triggerMist(String reason) {
  Serial.print(">>> TRIGGERING MIST. Reason: ");
  Serial.println(reason);

  // 1. Warning Buzzer (2 seconds)
  digitalWrite(BUZZER_PIN, HIGH);
  blynkSafeDelay(2000);
  digitalWrite(BUZZER_PIN, LOW);

  // 2. Spraying (10 seconds)
  digitalWrite(RELAY_MIST, LOW);  // Relay ON
  blynkSafeDelay(10000);
  digitalWrite(RELAY_MIST, HIGH); // Relay OFF

  Serial.println(">>> Mist Cycle Complete.");
}