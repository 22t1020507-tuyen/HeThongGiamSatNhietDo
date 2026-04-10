#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// --- Pin Config ---
#define DHTPIN 14        
#define DHTTYPE DHT22    
#define SOIL_PIN 34      
#define RELAY_PIN 2      
#define BUZZER_PIN 4     
#define LED_VANG_PIN 5   

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- MQTT Config ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";
const char* mqtt_server = "31e313925d0b4a6a965c8c890db0d38f.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883;
const char* mqtt_user = "tuyen2808";
const char* mqtt_pass = "Tuyen28082004@";

WiFiClientSecure espClient; 
PubSubClient client(espClient);

// Các Topic gửi/nhận
const char* topic_nhietdo = "adminpc/tuoicay/nhietdo";
const char* topic_doam = "adminpc/tuoicay/doam";
const char* topic_dat = "adminpc/tuoicay/doamdat"; 
const char* topic_bom_status = "adminpc/tuoicay/trangthaibom"; 
const char* topic_bom = "adminpc/tuoicay/bom"; 
const char* topic_pwm = "adminpc/tuoicay/pwm"; 

unsigned long lastSendTime = 0;
unsigned long lastReadTime = 0;
bool autoMode = true;
float t = 0, h = 0;
int soil = 0;

// Biến hỗ trợ hiển thị tin nhắn "Đã gửi dữ liệu"
unsigned long msgTimer = 0;
bool isShowingMsg = false;

// --- GỬI TRẠNG THÁI LÊN UI TỨC THÌ ---
void publishStatus() {
    String status = (digitalRead(RELAY_PIN) == HIGH) ? "ON" : "OFF";
    client.publish(topic_bom_status, status.c_str());
}

// --- CALLBACK NHẬN LỆNH TỪ NODE-RED ---
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) message += (char)payload[i];
  
  if (String(topic) == topic_bom) {
    isShowingMsg = false; // Ngắt thông báo gửi dữ liệu để ưu tiên hiện lệnh thủ công
    if (message == "ON") {
      autoMode = false; 
      digitalWrite(RELAY_PIN, HIGH); 
      analogWrite(BUZZER_PIN, 200);
      lcd.setCursor(0, 1); lcd.print("MANUAL: PUMP ON ");
    } else if (message == "OFF") {
      autoMode = false; 
      digitalWrite(RELAY_PIN, LOW); 
      analogWrite(BUZZER_PIN, 0);
      lcd.setCursor(0, 1); lcd.print("MANUAL: PUMP OFF");
    } else if (message == "AUTO") {
      autoMode = true; 
      lcd.setCursor(0, 1); lcd.print("Mode: AUTO      ");
    }
    publishStatus(); 
  }
  
  if (String(topic) == topic_pwm) {
    autoMode = false; 
    isShowingMsg = false;
    lcd.setCursor(0, 1); lcd.print("MANUAL: LAMP    ");
    
    int brightness = message.toInt(); 
    analogWrite(LED_VANG_PIN, brightness);
  }
}

void reconnect() {
  while (!client.connected()) {
    lcd.setCursor(0, 0); lcd.print("Ket noi HiveMQ.."); // Tối đa 16 ký tự
    lcd.setCursor(0, 1); lcd.print("Vui long doi... ");
    
    String clientId = "ESP32-" + String(random(0, 9999)); 
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      lcd.setCursor(0, 1); lcd.print("Thanh cong!     ");
      delay(1500); // Dừng 1.5 giây để bạn kịp đọc chữ "Thành công"
      lcd.clear(); // Xóa màn hình để bắt đầu chạy vòng lặp chính
      
      client.subscribe(topic_bom);
      client.subscribe(topic_pwm);
      publishStatus();
    } else {
      delay(3000);
    }
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_VANG_PIN, OUTPUT); 
  
  digitalWrite(RELAY_PIN, LOW); 
  analogWrite(BUZZER_PIN, 0);
  analogWrite(LED_VANG_PIN, 0); 

  Serial.begin(115200);
  dht.begin();
  
  // Khởi tạo LCD và in thông báo kết nối WiFi
  lcd.init(); lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Ket noi WiFi... "); // Tối đa 16 ký tự
  lcd.setCursor(0, 1); lcd.print("                ");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  
  // Khi đã kết nối WiFi
  lcd.setCursor(0, 1); lcd.print("WiFi OK!        ");
  delay(1500); // Dừng 1.5 giây để bạn kịp đọc chữ "WiFi OK!"
  
  espClient.setInsecure(); 
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 1. ĐỌC CẢM BIẾN (Mỗi 2 giây)
  if (millis() - lastReadTime > 2000) {
    lastReadTime = millis();
    float newH = dht.readHumidity();
    float newT = dht.readTemperature();
    if (!isnan(newH) && !isnan(newT)) { h = newH; t = newT; }
    soil = map(analogRead(SOIL_PIN), 0, 4095, 0, 100);

    // Hiển thị dòng 1 trên LCD
    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print((int)t); lcd.print("C H:"); lcd.print((int)h); lcd.print("%   ");
  }

  // 2. GỬI DỮ LIỆU LÊN UI (Mỗi 10 giây)
  if (millis() - lastSendTime > 10000) {
    lastSendTime = millis();
    client.publish(topic_nhietdo, String(t).c_str());
    client.publish(topic_doam, String(h).c_str());
    client.publish(topic_dat, String(soil).c_str()); 
    
    // Bật cờ thông báo đã gửi dữ liệu lên LCD
    isShowingMsg = true;
    msgTimer = millis();
    lcd.setCursor(0, 1);
    lcd.print("Du lieu da gui! ");
  }

  // Tự động tắt dòng chữ "Du lieu da gui!" sau 1.5 giây
  if (isShowingMsg && (millis() - msgTimer > 1500)) {
    isShowingMsg = false;
  }

  // 3. LOGIC TỰ ĐỘNG
  if (autoMode) {
    // Chỉ hiển thị trạng thái AUTO nếu không bận hiển thị chữ "Dữ liệu đã gửi"
    if (!isShowingMsg) {
      lcd.setCursor(0, 1);
      lcd.print("Dat:"); lcd.print(soil); lcd.print("% AUTO     ");
    }
    
    int currentState = digitalRead(RELAY_PIN);

    // --- A. TỰ ĐỘNG BƠM NƯỚC ---
    if (soil < 40 || (t > 35.0 && h < 40.0 && soil < 70)) {
      digitalWrite(RELAY_PIN, HIGH);
      analogWrite(BUZZER_PIN, 200); 
    } else if (soil > 70) {
      digitalWrite(RELAY_PIN, LOW);
      analogWrite(BUZZER_PIN, 0);   
    }

    if (digitalRead(RELAY_PIN) != currentState) {
      publishStatus();
    }

    // --- B. TỰ ĐỘNG BẬT ĐÈN SƯỞI ẤM ---
    if (t < 20.0) {
      analogWrite(LED_VANG_PIN, 255); 
    } 
    else if (t >= 20.0 && t < 25.0) {
      analogWrite(LED_VANG_PIN, 100); 
    } 
    else {
      analogWrite(LED_VANG_PIN, 0);   
    }
  }

  delay(500); 
}