#include <AntaresESPMQTT.h>
#include <WiFi.h>
#include <PubSubClient.h> 
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// =====================================================
// 1. KREDENSIAL & KONFIGURASI
// =====================================================
#define ACCESSKEY "6a407b911fd207ec:ece4b7092300a72f"
#define WIFISSID "yn's"
#define PASSWORD "0987654321"

#define projectName "UAS_IOT"
#define deviceName "SmartWaterUsage"

const char* emqx_broker = "broker.emqx.io";
const int emqx_port = 1883;
const char* topicControl = "smartwater/control"; 

const char* botToken = "8785146989:AAHYf_uRO-t_K1y2q3qENdIBlvnph9LGFMo";
const char* chatId = "5193349682";

// =====================================================
// 2. PIN & KONSTANTA
// =====================================================
const int trigPin = 5;
const int echoPin = 18;
const int waterLevelPin = 34;
const int ledPin = 2;
const int servoPin = 13;

const float TINGGI_TANDON = 168.0;
const float JARI_JARI = 58.0;
const float PI_VAL = 3.14159265;

// =====================================================
// 3. OBJEK & VARIABEL
// =====================================================
AntaresESPMQTT antares(ACCESSKEY);
WiFiClient netEMQX; 
PubSubClient emqxClient(netEMQX); 
WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 28800);
Servo kranServo;

float volumeSekarang = 0, volumeSebelumnya = 0, penggunaanAir = 0;
bool notifSent = false, modeManual = false;
int targetServoPos = 90;
String statusPenuh = "", statusServo = "AUTO", statusEfisiensi = "HEMAT", formattedTime = "";
int hariSebelumnya = -1;

unsigned long lastAntares = 0;
unsigned long lastSerial = 0;
const long intervalAntares = 15000; 
const long intervalSerial = 2000;  

// =====================================================
// 4. CALLBACK EMQX (LOGIKA SUBSCRIBE)
// =====================================================
void emqxCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) { message += (char)payload[i]; }

  Serial.println("\n--- PERINTAH DITERIMA (EMQX) ---");
  if (message == "1") {
    modeManual = true;
    targetServoPos = 0;
    statusServo = "BUKA (MANUAL)";
  } 
  else if (message == "0") {
    modeManual = true;
    targetServoPos = 90;
    statusServo = "TUTUP (MANUAL)";
  } 
  else if (message == "auto") {
    modeManual = false;
    statusServo = "AUTO";
  }
}

// =====================================================
// 5. RECONNECT EMQX
// =====================================================
void reconnectEMQX() {
  while (!emqxClient.connected()) {
    Serial.print("Menghubungkan ke EMQX...");
    String clientId = "ESP32_SmartWater_" + String(random(0xffff), HEX);
    if (emqxClient.connect(clientId.c_str())) {
      Serial.println("TERKONEKSI!");
      emqxClient.subscribe(topicControl);
    } else {
      Serial.print("GAGAL, rc=");
      Serial.print(emqxClient.state());
      delay(5000);
    }
  }
}

// =====================================================
// 6. SENSOR & HITUNG
// =====================================================
float bacaJarak() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long durasi = pulseIn(echoPin, HIGH, 30000);
  float hasil = (durasi == 0) ? TINGGI_TANDON : (durasi * 0.034 / 2);
  return (hasil > TINGGI_TANDON) ? TINGGI_TANDON : hasil;
}

float hitungVolume(float jarakSensor) {
  float tinggiAir = TINGGI_TANDON - jarakSensor;
  if (tinggiAir < 0) tinggiAir = 0;
  return (PI_VAL * JARI_JARI * JARI_JARI * tinggiAir) / 1000.0;
}

// =====================================================
// 7. SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(waterLevelPin, INPUT);
  pinMode(ledPin, OUTPUT);

  antares.wifiConnection(WIFISSID, PASSWORD);
  antares.setMqttServer();

  emqxClient.setServer(emqx_broker, emqx_port);
  emqxClient.setCallback(emqxCallback);

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  timeClient.begin();
  
  kranServo.attach(servoPin);
  kranServo.write(90); 
  
  volumeSebelumnya = hitungVolume(bacaJarak());
  Serial.println("\n>>> SISTEM SMART WATER SIAP <<<");
}

// =====================================================
// 8. LOOP
// =====================================================
void loop() {
  antares.checkMqttConnection();
  if (!emqxClient.connected()) { reconnectEMQX(); }
  emqxClient.loop(); 

  timeClient.update();
  formattedTime = timeClient.getFormattedTime();

  int hariSekarang = timeClient.getDay();
  if (hariSebelumnya != -1 && hariSekarang != hariSebelumnya) { penggunaanAir = 0; }
  hariSebelumnya = hariSekarang;

  float jarak = bacaJarak();
  volumeSekarang = hitungVolume(jarak);

  int waterLevelPhysical = digitalRead(waterLevelPin);

  // --- LOGIKA KONTROL SERVO ---
  if (waterLevelPhysical == HIGH) { 
    // PRIORITAS 1: Jika air menyentuh sensor water level (PENUH)
    statusPenuh = "PENUH";
    statusServo = "TUTUP (PROTEKSI)";
    kranServo.write(90);
    modeManual = false;
  } 
  else {
    // JIKA TIDAK PENUH
    statusPenuh = "TIDAK PENUH";
    
    if (modeManual) {
      // PRIORITAS 2: Kendali Manual dari EMQX/Kodular
      kranServo.write(targetServoPos);
    } 
    else {
      // PRIORITAS 3: Kendali Otomatis berdasarkan Volume (Ultrasonik)
      if (volumeSekarang < 500.0) {
        kranServo.write(0);
        statusServo = "BUKA (OTOMATIS)";
      } else {
        kranServo.write(90);
        statusServo = "TUTUP (OTOMATIS)";
      }
    }
  }

  if (volumeSebelumnya > volumeSekarang) { 
    penggunaanAir += (volumeSebelumnya - volumeSekarang); 
  }
  volumeSebelumnya = volumeSekarang;

  if (penggunaanAir > 120.0) statusEfisiensi = "BOROS";
  else if (penggunaanAir < 70.0) statusEfisiensi = "HEMAT";
  else statusEfisiensi = "NORMAL";

  if (volumeSekarang < 450.0) {
    digitalWrite(ledPin, HIGH);
    if (!notifSent) {
      bot.sendMessage(chatId, "⚠️ PERINGATAN AIR RENDAH\nVolume: " + String(volumeSekarang, 1) + " L", "");
      notifSent = true;
    }
  } else {
    digitalWrite(ledPin, LOW);
    notifSent = false;
  }

  // --- PUBLISH KE ANTARES ---
  if (millis() - lastAntares > intervalAntares) {
    antares.add("waktu", formattedTime);
    antares.add("volume", volumeSekarang);
    antares.add("penggunaan", penggunaanAir);
    antares.add("efisiensi", statusEfisiensi);
    antares.add("servo_status", statusServo);
    antares.add("water_level", statusPenuh);
    
    antares.publish(projectName, deviceName);
    Serial.println(">>> DATA + WAKTU TERKIRIM KE ANTARES <<<");
    lastAntares = millis();
  }

  // --- TAMPILAN SERIAL ---
  if (millis() - lastSerial > intervalSerial) {
    Serial.println("------------------------------------");
    Serial.print("Waktu       : "); Serial.println(formattedTime);
    Serial.print("Status Bak  : "); Serial.println(statusPenuh);
    Serial.print("Volume      : "); Serial.print(volumeSekarang, 2); Serial.println(" L");
    Serial.print("Penggunaan  : "); Serial.print(penggunaanAir, 2); Serial.println(" L");
    Serial.print("Efisiensi   : "); Serial.println(statusEfisiensi);
    Serial.print("Status Servo: "); Serial.println(statusServo);
    Serial.println("------------------------------------");
    lastSerial = millis();
  }
}