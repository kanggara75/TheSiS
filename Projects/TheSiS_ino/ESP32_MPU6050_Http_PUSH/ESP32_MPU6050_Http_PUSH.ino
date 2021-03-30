//Refrensi From
//https://randomnerdtutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
//https://randomnerdtutorials.com/esp32-esp8266-mysql-database-php/

#include <WiFi.h>
#include <HTTPClient.h>


#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;


// REPLACE with your Domain name with URL path or IP address with path
//const char* serverAcc = "http://192.168.43.15/thesis/kontrol/acc";
const char* serverAcc = "https://thesis.kanggara.net/kontrol/acc";


// WiFi network info.
//const char* ssid = "ZTE_2.4Ghz_dfpfyp";
//const char* password = "masihyanglama";
const char* ssid = "KAnggara75";
const char* password = "Lolipop.1";

// Pitch, Roll and Yaw values
float Gy = 0;
float Gx = 0;
float Gz = 0;
float Ax = 0;
float Ay = 0;
float Az = 0;
float temp = 0;

// Keep this API Key value to be compatible with the PHP code provided in the project page.
String accKeyValue = "tPmAT5Ab3j7F9";
HTTPClient http;

void setup()
{
  Serial.begin(115200);
  wifi_connect();
  Serial.println("Initialize MPU6050");
  testMpu();
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
    bacaMPU();
    kirimAcc();
    delay(500);
  }
  else
  {
    Serial.println("WiFi Disconnected");
    Serial.println("Trying to Reconnect in 30 Second");
    delay(29000);
    wifi_connect();
  }
    delay(1500);
}

void testMpu()
{
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }

  Serial.println("");
  delay(100);
}

void bacaMPU() {
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
  Serial.print("Acceleration X: ");
  Serial.print(Ax);
  Serial.print(", Y: ");
  Serial.print(Ay);
  Serial.print(", Z: ");
  Serial.print(Az);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(Gx);
  Serial.print(", Y: ");
  Serial.print(Gy);
  Serial.print(", Z: ");
  Serial.print(Gz);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" degC");

}

void kirimAcc()
{
  // Your Domain name with URL path or IP address with path
  http.begin(serverAcc);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Prepare your HTTP POST request data
    String httpRequestAcc = "api_key=" + accKeyValue + "&acx=" + String(Ax) + "&acy=" + String(Ay) + "&acz=" + String(Az) + "&grx=" + String(Gx) + "&gry=" + String(Gy) + "&grz=" + String(Gz) + "&temp=" + String(temp) + "";
//  String httpRequestAcc = "api_key=tPmAT5Ab3j7F9&acx=10.31&acy=-0.20&acz=-0.44&grx=-0.05&gry=-0.02&grz=0.02&temp=31.04";
  int httpResponseAcc = http.POST(httpRequestAcc);
  Serial.print("Kirim Ke: \t   : ");
  Serial.println(serverAcc);
  Serial.print("HTTP Request: \t   : ");
  Serial.println(httpRequestAcc);
  Serial.print("HTTP Response code: \t   : ");
  Serial.println(httpResponseAcc);
  Serial.println("\n");
  http.end();
}
