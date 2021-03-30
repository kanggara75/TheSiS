//Deklarasi Nama Task
TaskHandle_t Task1; //GPS
TaskHandle_t Task2; //ACC

//Import Library
#include <WiFi.h>             //Library Wifi
#include <Wire.h>             //Librari Pengkabelan
#include <TinyGPS++.h>        //Library GPS
#include <HTTPClient.h>       //Library Http
#include <Arduino_JSON.h>     //Librari JSON [Arduino]
#include <SoftwareSerial.h>   //Library SoftwareSerial [ESP32 Version]
#include <Adafruit_Sensor.h>  //Library Tambahan Untuk MPU6050
#include <Adafruit_MPU6050.h> //Library Accelerometer MPU6050

#define TIME_TO_SLEEP 30          /* Time ESP32 will go to sleep (in seconds) */
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

static const uint32_t GPSBaud = 9600;    //Baudrate GPS
static const int RXPin = 13, TXPin = 14; //Deklasasi Pin GPS
static const int SIMrx = 16, SIMtx = 17; //Deklasasi Pin GSM

//Deklarasi Url Server
//const char *serverGet = "http://192.168.43.15/thesis/api/kontrol";
const char *serverGet = "https://thesis.kanggara.net/api/kontrol";
//const char *serverUpd = "http://192.168.43.15/thesis/update";
const char* serverUpd = "https://thesis.kanggara.net/update";

String Slat, Slon; //Koordinat as String
String kontrolReadings;

//Deklarasi Pinout
const int AlarmPin = 25;
const int ListrikPin = 2;
const int MesinPin = 13;

// Pitch, Roll and Yaw values
float Gy = 0, Gx = 0, Gz = 0, Ax = 0, Ay = 0, Az = 0, temp = 0;

// WiFi network info.
const char *ssid = "KAnggara75";
const char *password = "Lolipop.1";

// Keep this API Key value to be compatible with the PHP code provided in the project page.
String accKeyValue = "tPmAT5Ab3j7F9";

TinyGPSPlus gps;                        // The TinyGPS++ object
HTTPClient http;                        //HTTP Object
Adafruit_MPU6050 mpu;                   //Accelerometwr Object
SoftwareSerial gpsSerial(RXPin, TXPin); // The serial connection to the GPS device

void setup()
{
  Serial.begin(115200);

  pinMode(AlarmPin, OUTPUT);
  pinMode(MesinPin, OUTPUT);
  pinMode(ListrikPin, OUTPUT);
  wifi_connect();
  delay(500);
  
  //Deep Sleep [Timer]
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Will be Sleep on " + String(TIME_TO_SLEEP) + " Seconds");

  //Menjalankan menggunakan Core 1
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */
  delay(500);

  //Menjalankan menggunakan Core 0
  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    0,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
  delay(500);

}

void Task1code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  gpsSerial.begin(GPSBaud);

  for (;;)
  {
    cekgps();
  }
}

void Task2code(void *pvParameters)
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  Serial.println("Initialize MPU6050");
  testMpu();

  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      getdata();
      delay(500);
      bacaMPU();
      delay(500);
      kirimAcc();
      delay(1500);
    }
    else
    {
      Serial.println("WiFi Disconnected");
      Serial.println("Trying to Reconnect in 15 Second");
      delay(14000);
      wifi_connect();
    }
    delay(5000);
    Serial.flush();
    sleep();
  }
}

void wifi_connect()
{
  int i = 3;
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print("=> ");
    Serial.println(i);
    i--;
    if (i <= 0)
    {
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
}

void sleep()
{
  gpio_hold_en((gpio_num_t)AlarmPin);
  gpio_hold_en((gpio_num_t)ListrikPin);
  gpio_hold_en((gpio_num_t)MesinPin);
  gpio_deep_sleep_hold_en();

  int i = 3;
  while (i > 0)
  {
    delay(1000);
    Serial.print("Sleep in => ");
    Serial.println(i);
    i--;
    if (i <= 0)
    {
      delay(10);
      Serial.println("Deep Sleep");
      esp_deep_sleep_start();
    }
  }
  Serial.println("This will never be printed");
}

void testMpu()
{
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }

  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G); //Set Acc Range
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);       //Set Gyro Range
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);    //Set MPU Filter Bandwidth
  delay(100);
}

void bacaMPU()
{
  sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);

  Gx = g.gyro.x;
  Gy = g.gyro.y;
  Gz = g.gyro.z;
  Ax = a.acceleration.x;
  Ay = a.acceleration.y;
  Az = a.acceleration.z;
  temp = tmp.temperature;

  /* Print out the values */
  Serial.println("Acceleration X: " + String(Ax) + ", Y: " + String(Ax) + ", Z: " + String(Ax) + " m/s^2");
  Serial.println("Rotation X: " + String(Gx) + ", Y: " + String(Gy) + ", Z: " + String(Gz) + " rad/s");
  Serial.println("Temperature: " + String(temp) + " Celcius");
}

void cekgps()
{
  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated())
    {
      displayInfo();
    }
  }
}

void displayInfo()
{
  Slat = String(gps.location.lat(), 6);
  Slon = String(gps.location.lng(), 6);

  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(Slat);
    Serial.print(F(","));
    Serial.print(Slon);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  Serial.println();
}

void getdata()
{
  gpio_hold_dis((gpio_num_t)AlarmPin); //Disable GPIO Hold
  gpio_hold_dis((gpio_num_t)ListrikPin);
  gpio_hold_dis((gpio_num_t)MesinPin);
  kontrolReadings = httpGETRequest(serverGet);
  JSONVar myObject = JSON.parse(kontrolReadings);
  if (myObject.hasOwnProperty("Alarm"))
  {
    int alarm = myObject["Alarm"];
    if (alarm == 1)
    {
      digitalWrite(AlarmPin, HIGH);
    }
    else
    {
      digitalWrite(AlarmPin, LOW);
    }
  }
  if (myObject.hasOwnProperty("Listrik"))
  {
    int listrik = myObject["Listrik"];
    if (listrik == 1)
    {
      digitalWrite(ListrikPin, HIGH);
    }
    else
    {
      digitalWrite(ListrikPin, LOW);
    }
  }
  if (myObject.hasOwnProperty("Mesin"))
  {
    int mesin = myObject["Mesin"];
    if (mesin == 1)
    {
      digitalWrite(MesinPin, HIGH);
    }
    else
    {
      digitalWrite(MesinPin, LOW);
    }
  }
}

String httpGETRequest(const char *serverGet)
{
  http.begin(serverGet);
  int httpResponseCode = http.GET(); // Send HTTP GET request
  String payload = "{}";
  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}

void kirimAcc()
{
  http.begin(serverUpd);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

  // Prepare your HTTP POST request data
  String httpRequestAcc = "api_key=" + accKeyValue + "&acx=" + String(Ax) + "&acy=" + String(Ay) + "&acz=" + String(Az) + "&grx=" + String(Gx) + "&gry=" + String(Gy) + "&grz=" + String(Gz) + "&temp=" + String(temp) + "&lat=" + String(Slat) + "&lon=" + String(Slon) + "";
  int httpResponseAcc = http.POST(httpRequestAcc);
  if (httpResponseAcc > 0)
  {
    Serial.print("HTTP Response code: \t   : ");
    Serial.println(httpResponseAcc);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseAcc);
    delay(100);
  }
  http.end();
}
