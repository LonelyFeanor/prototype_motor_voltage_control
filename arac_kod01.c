/*
 * PROJE: 4x4 Çekiş Kontrol Sistemi - Akım Bazlı Voltaj Regülasyonu
 * DONANIM: Arduino Mega + 4x BTS7960B
 * GÜNCEL PİN YAPILANDIRMASI:
 * Motor 1: 9-8 (PWM), A0-A1 (Akım)
 * Motor 2: 7-6 (PWM), A2-A3 (Akım)
 * Motor 3: 5-4 (PWM), A4-A5 (Akım)
 * Motor 4: 3-2 (PWM), A8-A9 (Akım)
 */

// --- MOTOR YAPILANDIRMASI (STRUCT) ---
struct Motor {
  int r_pwm;    // İleri PWM Pini
  int l_pwm;    // Geri PWM Pini
  int r_is;     // İleri Akım Sensör Pini
  int l_is;     // Geri Akım Sensör Pini
  bool boosted; // Motor şu an 24V modunda mı?
};

// YENİ PİN HARİTASI
Motor motors[4] = {
  {9, 8, A0, A1, false}, // Motor 1
  {7, 6, A2, A3, false}, // Motor 2
  {5, 4, A4, A5, false}, // Motor 3
  {3, 2, A8, A9, false}  // Motor 4
};

// --- AYARLAR (KALİBRASYON) ---
const int PWM_12V = 127;   // 12V için PWM değeri (~%50)
const int PWM_24V = 255;   // 24V için PWM değeri (%100)

// Eşik Değerleri (Seri Porttan izleyerek ince ayar yapmalısın)
const int LOAD_THRESHOLD_ON = 350;  // Akım bu değeri geçerse 24V ver
const int LOAD_THRESHOLD_OFF = 250; // Akım bu değerin altına düşerse 12V'a dön

// Yön Kontrolü (1: İleri, -1: Geri, 0: Dur)
int direction = 1; 

void setup() {
  Serial.begin(9600);
  Serial.println("Sistem Yeni Pinlerle Baslatiliyor...");

  // Pin modlarını ayarla
  for (int i = 0; i < 4; i++) {
    pinMode(motors[i].r_pwm, OUTPUT);
    pinMode(motors[i].l_pwm, OUTPUT);
    // Analog pinler (A0..A9) otomatik INPUT modundadır
  }
  
  delay(1000); // Sistem açılış güvenliği
}

void loop() {
  // Tüm motorları kontrol et
  for (int i = 0; i < 4; i++) {
    controlMotor(i);
  }
  delay(50); // Döngü kararlılığı
}

void controlMotor(int id) {
  // 1. AKIM OKUMA (İki yönü de kontrol et)
  int currentR = analogRead(motors[id].r_is);
  int currentL = analogRead(motors[id].l_is);
  int currentLoad = max(currentR, currentL); // Hangi yön aktifse o akımı al

  // 2. KARAR MEKANİZMASI (Histerezis)
  if (currentLoad > LOAD_THRESHOLD_ON) {
    motors[id].boosted = true; // Yük algılandı -> 24V Modu
  } 
  else if (currentLoad < LOAD_THRESHOLD_OFF) {
    motors[id].boosted = false; // Yük kalktı -> 12V Modu
  }

  // Hızı belirle
  int targetSpeed = motors[id].boosted ? PWM_24V : PWM_12V;

  // 3. MOTORU SÜRME
  if (direction == 1) { // İLERİ
    analogWrite(motors[id].r_pwm, targetSpeed);
    analogWrite(motors[id].l_pwm, 0);
  } 
  else if (direction == -1) { // GERİ
    analogWrite(motors[id].r_pwm, 0);
    analogWrite(motors[id].l_pwm, targetSpeed);
  } 
  else { // DUR
    analogWrite(motors[id].r_pwm, 0);
    analogWrite(motors[id].l_pwm, 0);
  }

  // 4. DEBUG (Seri Porttan Takip)
  // Sadece Motor 1'i yazdırıyoruz, diğerlerini görmek için "id == 0" kısmını değiştir.
  if (id == 0) {
    Serial.print("M1 Akim: ");
    Serial.print(currentLoad);
    Serial.print(" | Gerilim: ");
    Serial.println(motors[id].boosted ? "24V (TAM GUC)" : "12V (NOMINAL)");
  }
}