#define BLYNK_TEMPLATE_ID "TMPL6zcTuO3yF"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_AUTH_TOKEN "GaDAwzegfm1Dc068uXWmmBdX6J3j7Ao2"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <string.h>
#include <ESP32Servo.h>
#include "DHT.h"

// Button define
#define button1 15
#define button2 13
boolean button1State = HIGH;
boolean button2State = HIGH;
bool btn;

// DHT define
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 18
DHT dht(DHTPIN, DHTTYPE);

// Sensor khi gas va canh bao
#define sensor 34
#define buzzer 21   
boolean canhbao = 0;
int canhbaoState = 0;

// Define LED
#define LED 5
int button = 0;
  
// Servo
Servo myServo;
#define SERVO_PIN 14
boolean servoPosition = 0; // trạng thái cửa
boolean manualOpen = 0; // trạng thái mở cửa thủ công
boolean gasTriggered = false; // trạng thái mở cửa do gas

//Fan dc
#define FAN_PIN 32 

// Connect wifi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Huyen Trang";
char pass[] = "motdenchin";

void setup() {
  // Debug console
  Serial.begin(115200); 
  
  // Pin modes
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(FAN_PIN, OUTPUT);
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize Blynk
  Blynk.begin(auth, ssid, pass);

  // Initialize Servo
  myServo.attach(SERVO_PIN);
}

BLYNK_WRITE(V3) {
  canhbao = param.asInt();
  warning();
}

void warning() {
  if (canhbao == 1) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(LED, HIGH);
    canhbaoState = 1;
  } else {
    digitalWrite(buzzer, LOW);
    digitalWrite(LED, LOW);
    canhbaoState = 0;
  }
}

BLYNK_WRITE(V2) {  // V2 là nơi bạn đặt điều khiển servo trên ứng dụng Blynk
  int servoPosition = param.asInt(); // Đọc giá trị từ Blynk (0 - 1)
  manualOpen = (servoPosition == 1);
  gasTriggered = false;
  controlDoor(); 
}

// Hàm tắt bật servo
void controlDoor() {
  if (servoPosition == 1) {
    for (int pos = 0; pos <= 90; pos += 5) {
      myServo.write(pos);
      delay(15);
    }
    servoPosition = 0;
    manualOpen = 0;

  } else {
    for (int pos = 90; pos >= 0; pos -= 5) {
      myServo.write(pos);
      delay(15);
    }
    servoPosition = 1;
    manualOpen = 1;
  }
}

void GASLevel() {
  int value = analogRead(sensor);
  float t = dht.readTemperature();
  value = map(value, 0, 4095, 0, 100);
  Blynk.virtualWrite(V4, value);
  Serial.println(value);

  if (canhbaoState == 0) {
    if (value >= 60 || t >= 50) {
      digitalWrite(buzzer, HIGH);
      digitalWrite(LED, HIGH);
      digitalWrite(FAN_PIN, HIGH);
      if (servoPosition == 0) {
        for (int pos = 90; pos >= 0; pos -= 5) {
          myServo.write(pos);
          delay(15);
        }
        servoPosition = 1;
        
      }
      Blynk.virtualWrite(V2, servoPosition);  
    } else if(manualOpen != 1) {
      digitalWrite(buzzer, LOW);
      digitalWrite(LED, LOW);
      digitalWrite(FAN_PIN, LOW);  
      if (servoPosition == 1) {
        for (int pos = 0; pos <= 90; pos += 5) {
          myServo.write(pos);
          delay(15);
        }
        servoPosition = 0;
        Blynk.virtualWrite(V2, servoPosition);
         // Cửa đóng do gas đã giảm
      }
    }
  }
}

void loop() {
  GASLevel();
  Blynk.run();
  
  // Read Temp
  float t = dht.readTemperature();
  // Read Humi
  float h = dht.readHumidity();
  // Check isRead ?
  if (isnan(h) || isnan(t)) {
    delay(500);
    Serial.println("Failed to read from DHT sensor!\n");
    return;
  }
  
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
 
  Serial.print("\n");
  Serial.print("Humidity: " + String(h) + "%");
  Serial.print("\t");
  Serial.print("Temperature:" + String(t) + " C");
  Serial.print("\t");

  // Button điều khiển cửa
  if (digitalRead(button1) == LOW) {
    if (button1State == HIGH) {
      controlDoor();
      Blynk.virtualWrite(V2, servoPosition);
      button1State = LOW; // Cập nhật trạng thái button1
      delay(100);
    }
  } else {
    button1State = HIGH;
  }

  // Button điều khiển cảnh báo
    if (digitalRead(button2) == LOW) {
      if (button2State == HIGH) {
        canhbao = !canhbao; // Đảo trạng thái cảnh báo
        warning();
        Blynk.virtualWrite(V3, canhbao);
        button2State = LOW; // Cập nhật trạng thái button2
        delay(100);
      }
    } else {
      button2State = HIGH;
    }

  delay(200);
}
