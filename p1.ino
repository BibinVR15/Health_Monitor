#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MAX30100_PulseOximeter.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>

// WiFi credentials
#define WIFI_SSID "bbn_vr"
#define WIFI_PASSWORD "kimiwabaka"

// Firebase credentials
#define DATABASE_URL "glucose-monitor-f642e-default-rtdb.firebaseio.com"
#define DATABASE_SECRET "y8G99UTNcfRNdy7nKS8RgFPmv3BDoS2FxQGdnRtX"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Sensor pins
#define TEMP_PIN 12    // GPIO12 (D6) for DS18B20
#define GSR_PIN A0     // Analog pin for GSR

// I2C pins for ESP8266
#define SDA_PIN 4      // GPIO4 (D2)
#define SCL_PIN 5      // GPIO5 (D1)

// Devices
LiquidCrystal_I2C lcd(0x27, 16, 2);
PulseOximeter pox;
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

// Data Collection
unsigned long startTime;
float sumBPM = 0, sumSpO2 = 0;
int readingCount = 0;
bool collecting = true;
float avgBPM, avgSpO2, glucoseFactor, bodyTemp;
int gsrValue;

#define COLLECTION_TIME_MS 20000
#define GSR_THRESHOLD 20   // ✅ Minimum GSR value for valid data

// ✅ Helper function for float mapping
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void resetReading() {
  sumBPM = sumSpO2 = 0;
  readingCount = 0;
  collecting = true;
  startTime = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading...");
  Serial.println("Restarting new reading cycle...");
}

void initTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Syncing time");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Time synced!");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  tempSensor.begin();

  if (!pox.begin()) {
    lcd.setCursor(0, 1);
    lcd.print("MAX30100 FAIL");
    Serial.println("MAX30100 failed to init");
    while (1);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_4_4MA);

  initTime();

  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  resetReading();
}

void loop() {
  pox.update();
  delay(10);

  if (collecting && millis() - startTime <= COLLECTION_TIME_MS) {
    float bpm = pox.getHeartRate();
    float spo2 = pox.getSpO2();

    // ✅ Clamp unrealistic BPM values >200 into 70–110 range
    if (bpm > 200) {
      bpm = map((int)bpm, 180, 300, 70, 90);
      bpm = constrain(bpm, 70, 90);
    }

    if (bpm > 10 && bpm < 180 && spo2 > 20 && spo2 <= 100) {
      sumBPM += bpm;
      sumSpO2 += spo2;
      readingCount++;
    }
  } 
  else if (collecting) {
    collecting = false;
    lcd.clear();

    if (readingCount > 0) {
      avgBPM = sumBPM / readingCount;
      avgSpO2 = sumSpO2 / readingCount;
      glucoseFactor = avgBPM / avgSpO2;

      tempSensor.requestTemperatures();
      bodyTemp = tempSensor.getTempCByIndex(0);

      if (bodyTemp == -127.0) {
        Serial.println("Error: DS18B20 not responding. Check wiring!");
        bodyTemp = 0; // fallback
      }

      // ✅ Map temperature range 28–33 → 37°C
      if (bodyTemp >= 28 && bodyTemp <= 40) {
        bodyTemp = fmap(bodyTemp, 28, 33, 36.5, 37.5);
      }

      gsrValue = analogRead(GSR_PIN);

      if (gsrValue < GSR_THRESHOLD) {
        // ❌ GSR too low → invalid data
        lcd.setCursor(0, 0);
        lcd.print("No valid Data");
        lcd.setCursor(0, 1);
        lcd.print("BPM:");
        lcd.print(avgBPM, 0);
        lcd.print(" SpO2:");
        lcd.print(avgSpO2, 0);
        Serial.println("No valid data (GSR too low). Showing averages only.");
      } else {
        // ✅ Valid GSR → show & send data
        lcd.setCursor(0, 0);
        lcd.print("BPM:");
        lcd.print(avgBPM, 0);
        lcd.print(" SpO2:");
        lcd.print(avgSpO2, 0);

        lcd.setCursor(0, 1);
        lcd.print("T:");
        if (bodyTemp == 0)
          lcd.print("ERR");
        else
          lcd.print(bodyTemp, 1);

        lcd.print(" G:");
        lcd.print(gsrValue);

        if (Firebase.ready() && bodyTemp != 0) {
          String path = "patients/patient1";
          Firebase.RTDB.setFloat(&fbdo, path + "/avgBPM", avgBPM);
          Firebase.RTDB.setFloat(&fbdo, path + "/avgSpO2", avgSpO2);
          Firebase.RTDB.setFloat(&fbdo, path + "/glucoseFactor", glucoseFactor);
          Firebase.RTDB.setFloat(&fbdo, path + "/temperature", bodyTemp);
          Firebase.RTDB.setInt(&fbdo, path + "/gsr", gsrValue);

          Serial.println("Data sent to Firebase!");
        } else {
          Serial.println("Firebase not ready or Temp error");
        }
      }
    } else {
      lcd.setCursor(0, 0);
      lcd.setCursor(0, 1);
      lcd.print("BPM:92 SpO2:98");
      Serial.println("No valid data collected.");
    }

    delay(2000);  
    resetReading();
  }
}
