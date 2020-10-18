#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
MPU6050 mpu;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial gpsSerial(RXPin, TXPin);

int16_t lat, lon, sat, hdop;

// Timers
unsigned long timer = 0;
float timeStep = 0.01;

// Pitch, Roll and Yaw values
float GyY = 0;
float GyX = 0;
float GyZ = 0;


// WiFi network info.
const char* ssid = "ZTE_2.4G_dfpfyp";
const char* password = "masihyanglama";
//const char* ssid = "KAnggara75";
//const char* password = "klapankali3";

// REPLACE with your Domain name with URL path or IP address with path
const char* serverAcc = "http://192.168.100.2/thesis/kontrol/acc";
const char* serverGps = "http://192.168.100.2/thesis/kontrol/gps";
//const char* serverName = "https://thesis.kanggara.net/kontrol/acc";

// Keep this API Key value to be compatible with the PHP code provided in the project page.
String accKeyValue = "tPmAT5Ab3j7F9";
String gpsKeyValue = "tPmAT5Ab3j7F9";
HTTPClient http;
void setup()
{
  Serial.begin(115200);
  gpsSerial.begin(GPSBaud);
  Serial.println("Initialize MPU6050");
  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  //  mpu.calibrateGyro();
  wifi_connect();
}

void wifi_connect()
{
  int i = 10;
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print("=> ");
    Serial.println(i);
    i--;
    if (i <= 0) {
      setup();
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.print(ssid);
  Serial.print(" network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    mpu_read();
  }
  else
  {
    Serial.println("WiFi Disconnected");
    Serial.println("Trying to Reconnect in 30 Second");
    delay(29000);
    wifi_connect();
  }
  delay(15000);
}

void mpu_read()
{
  timer = millis();
  // Read normalized values
  Vector normAccel = mpu.readNormalizeAccel();
  // Read normalized values
  Vector norm = mpu.readNormalizeGyro();

  float Tmp = mpu.readTemperature();
  float AcX = (atan2(normAccel.YAxis, normAccel.ZAxis) * 180.0) / M_PI;
  float AcY = 0 + normAccel.YAxis / 100;
  float AcZ = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis * normAccel.YAxis + normAccel.ZAxis * normAccel.ZAxis)) * 180.0) / M_PI;
  // Calculate Pitch, Roll and Yaw
  GyX = GyX + norm.XAxis * timeStep;
  GyY = GyY + norm.YAxis * timeStep;
  GyZ = GyZ + norm.ZAxis * timeStep;

  // Your Domain name with URL path or IP address with path
  http.begin(serverAcc);
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Prepare your HTTP POST request data
  String httpRequestAcc = "api_key=" + accKeyValue + "&acx=" + String(AcX) + "&acy=" + String(AcY) + "&acz=" + String(AcZ) + "&grx=" + String(GyX) + "&gry=" + String(GyY) + "&grz=" + String(GyZ) + "&temp=" + String(Tmp) + "";
  //  Serial.print("httpRequestData: ");
  //  Serial.println(httpRequestAcc);
  Serial.print("Accelerometer Values: "); Serial.print("\tAcX: "); Serial.print(AcX); Serial.print("\t\tAcY: "); Serial.print(AcY); Serial.print("\t\tAcZ: "); Serial.print(AcZ);
  Serial.print("\nGyroscope Values: "); Serial.print("\tGyX: "); Serial.print(GyX); Serial.print("\t\tGyY: "); Serial.print(GyY); Serial.print("\t\tGyZ: "); Serial.print(GyZ);
  Serial.print("\nTemperature: \t\t   : " );  Serial.print(Tmp); Serial.println(" *C");

  // Send HTTP POST request
  int httpResponseAcc = http.POST(httpRequestAcc);
  Serial.print("HTTP Response code: \t   : ");
  Serial.println(httpResponseAcc);
  Serial.println("\n");
  http.end();
}
