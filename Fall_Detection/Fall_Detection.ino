#include <Wire.h>
#include <ESP8266WiFi.h>
const int MPU_addr=0x68;  
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;

boolean fall = false; 
boolean trigger1=false; 
boolean trigger2=false;
boolean trigger3=false;
byte trigger1count=0; 
byte trigger2count=0;
byte trigger3count=0;
int angleChange=0;

const char *ssid =  "vivo 1906";     
const char *pass =  "aarya@1234"; 
void send_event(const char *Fall_Detected);
const char *host = "maker.ifttt.com";
const char *privateKey = "mAgonxIHv_t5kepibPeg93s4yQZ6_hJcb2qW85OPrKN";
void setup(){
 Serial.begin(115200);
 Wire.begin();
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x6B);  
 Wire.write(0);     
 Wire.endTransmission(true);
 Serial.println("Wrote to IMU");
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");              
  }
  Serial.println("");
  Serial.println("WiFi connected");
 }
void loop() {
  mpu_read();
  ax = (AcX - 2050) / 16384.00;
  ay = (AcY - 77) / 16384.00;
  az = (AcZ - 1947) / 16384.00;
  gx = (GyX + 270) / 131.07;
  gy = (GyY - 351) / 131.07;
  gz = (GyZ + 136) / 131.07;

  float Raw_Amp = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
  int Amp = Raw_Amp * 10;  
  Serial.println(Amp);

  if (Amp <= 2 && !trigger2){ 
    trigger1 = true;
    Serial.println("TRIGGER 1 ACTIVATED");
  }

  if (trigger1) {
    trigger1count++;
    if (Amp >= 12) { 
      trigger2 = true;
      Serial.println("TRIGGER 2 ACTIVATED");
      trigger1 = false;
      trigger1count = 0;
    }
  }

  if (trigger2) {
    trigger2count++;
    angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
    Serial.println(angleChange);
    if (angleChange >= 30 && angleChange <= 400) { 
      trigger3 = true;
      trigger2 = false;
      trigger2count = 0;
      Serial.println("TRIGGER 3 ACTIVATED");
    }
  }

  if (trigger3) {
    trigger3count++;
    if (trigger3count >= 10) {
      angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
      Serial.println(angleChange);
      if (angleChange >= 0 && angleChange <= 10) { 
        fall = true;
        trigger3 = false;
        trigger3count = 0;
        Serial.println("FALL DETECTED");
        send_event("fall_detect");
      } else {
        trigger3 = false;
        trigger3count = 0;
        Serial.println("TRIGGER 3 DEACTIVATED");
      }
    }
  }

  if (fall) { 
    Serial.println("FALL DETECTED");
    send_event("fall_detect");
    fall = false;
  }

  if (trigger2count >= 6) { 
    trigger2 = false;
    trigger2count = 0;
    Serial.println("TRIGGER 2 DEACTIVATED");
  }

  if (trigger1count >= 6) { 
    trigger1 = false;
    trigger1count = 0;
    Serial.println("TRIGGER 1 DEACTIVATED");
  }

  delay(100);
}

void mpu_read() {
  Wire.beginTransmission((uint8_t)MPU_addr); 
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_addr, (size_t)14, (bool)true); 
  AcX = Wire.read() << 8 | Wire.read();      
  AcY = Wire.read() << 8 | Wire.read();  
  AcZ = Wire.read() << 8 | Wire.read();  
  Tmp = Wire.read() << 8 | Wire.read();  
  GyX = Wire.read() << 8 | Wire.read();  
  GyY = Wire.read() << 8 | Wire.read();  
  GyZ = Wire.read() << 8 | Wire.read();  
}

void send_event(const char *event)
{
  Serial.print("Connecting to "); 
  Serial.println(host);
    
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }
   
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  while(client.connected())
  {
    if(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    } else {
      
      delay(50);
    };
  }
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}
