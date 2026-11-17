// /*
//  * วิธีที่ 3 : Fixed Charging Time + อ่านแรงดันด้วย ADC ของ ESP32
//  * วัดค่าความจุ Cx ของตัวเก็บประจุ/เซนเซอร์
//  *
//  * การต่อวงจร (ใช้ ESP32-S3 / ESP32-C3 เพราะ GPIO1 เป็น ADC1):
//  *
//  *   GPIO21 ──[ R1 1MΩ ]──┬── GPIO1 (ADC)
//  *                         │
//  *                        Cx
//  *                         │
//  *                        GND
//  *
//  *   สวิตช์กดติดปล่อยดับ:  GPIO4 ── สวิตช์ ── GND   (ใช้ pull-up ภายใน)
//  *     กดค้าง = วัดต่อเนื่อง, ปล่อย = หยุด
//  */

// const int CHARGE_PIN = 21;   // ขาขับชาร์จ ต่อผ่าน R1 (1MΩ)
// const int ADC_PIN    = 1;    // ขาอ่านแรงดันที่คร่อม Cx
// const int BUTTON_PIN = 4;    // สวิตช์กดติดปล่อยดับ

// const float VCC  = 3.3;              // V
// const float R1   = 1000000.0;        // 1 MΩ
// const float T_CH = 20e-6;           // เวลาชาร์จคงที่ = 100 µs (วินาที)
// const unsigned long T_CH_US = 20;   // = 100 µs สำหรับ delayMicroseconds()

// const unsigned long MEAS_INTERVAL = 200; // ms ระหว่างการวัดแต่ละครั้งตอนกดค้าง
// bool prevPressed = false;

// void setup() {
//   Serial.begin(115200);
//   pinMode(BUTTON_PIN, INPUT);

//   pinMode(CHARGE_PIN, OUTPUT);
//   digitalWrite(CHARGE_PIN, LOW);            // เริ่มต้นให้คายประจุ

//   analogReadResolution(12);                 // 0 - 4095
//   analogSetPinAttenuation(ADC_PIN, ADC_11db); // วัดได้ถึง ~3.3 V

//   Serial.println("พร้อมวัด — กดสวิตช์ค้างเพื่อวัดค่า Cx");
// }

// float measureCx() {
//   // 1) ปล่อย GPIO(Charge)=LOW เพื่อคายประจุ Cx ให้เป็น 0 V
//   pinMode(CHARGE_PIN, OUTPUT);
//   digitalWrite(CHARGE_PIN, LOW);
//   delay(5);                      // รอให้คายประจุจนหมด (>5τ)

//   // 2) สั่ง GPIO(Charge)=HIGH เพื่อชาร์จด้วยเวลาคงที่ t_ch
//   noInterrupts();
//   digitalWrite(CHARGE_PIN, HIGH);
//   delayMicroseconds(T_CH_US);
//   // 3) หยุดชาร์จทันที โดยปล่อยขาเป็น Hi-Z เพื่อ "ค้าง" แรงดันไว้ให้อ่านตรงเวลา
//   pinMode(CHARGE_PIN, INPUT);
//   interrupts();

//   // 4) อ่านแรงดัน Vc ด้วย ADC (ใช้ค่าที่คาลิเบรตแล้วเป็นมิลลิโวลต์)
//   float vc = analogReadMilliVolts(ADC_PIN) / 1000.0;   // เป็นโวลต์

//   // 5) คำนวณค่าความจุ  Cx = -t_ch / ( R1 · ln(1 - Vc/Vcc) )
//   float ratio = 1.0 - (vc / VCC);
//   if (ratio <= 0.0) ratio = 1e-6;          // กันหารศูนย์ / log ของค่าลบ
//   float cx = -T_CH / (R1 * log(ratio));    // log() ใน C คือ ln

//   Serial.printf("Vc = %.3f V  |  Cx = %.2f pF\n", vc, cx * 1e12);
//   return cx;
// }

// void loop() {
//   bool pressed = (digitalRead(BUTTON_PIN) == LOW);  // กด = LOW

//   if (pressed && !prevPressed) {
//     Serial.println("▶ เริ่มวัด");
//   } else if (!pressed && prevPressed) {
//     Serial.println("■ หยุดวัด");
//     pinMode(CHARGE_PIN, OUTPUT);
//     digitalWrite(CHARGE_PIN, LOW);          // พักขาชาร์จไว้ที่ LOW
//   }
//   prevPressed = pressed;

//   if (pressed) {
//     measureCx();
//     delay(MEAS_INTERVAL);
//   }
// }