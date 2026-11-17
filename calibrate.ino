const int   CHARGE_PIN = 21;
const int   ADC_PIN    = 1;
const int   BUTTON_PIN = 4;

const float VCC = 3.3;
const float R1  = 1000000.0;
const unsigned long T_CH_US = 25;
const int   N_AVG = 256;

// ===== ตารางคาลิเบรต (raw, ค่าจริง) เรียงจากน้อย -> มาก =====
const int CAL_N = 4;
float CAL_RAW[CAL_N] = {14.2, 16.8, 19.5, 21.6};   // raw ที่วัดได้
float CAL_C  [CAL_N] = { 5.0, 10.0, 15.0, 20.0};   // ค่าจริง
// ==========================================================

bool prevPressed = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(CHARGE_PIN, OUTPUT);
  digitalWrite(CHARGE_PIN, LOW);
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
  Serial.println("พร้อมวัด — กดสวิตช์ค้างเพื่อวัด Cx");
}

float measureRawCx(float &vcOut) {
  double sumVc = 0;
  for (int i = 0; i < N_AVG; i++) {
    pinMode(CHARGE_PIN, OUTPUT);
    digitalWrite(CHARGE_PIN, LOW);
    delayMicroseconds(300);
    noInterrupts();
    digitalWrite(CHARGE_PIN, HIGH);
    delayMicroseconds(T_CH_US);
    pinMode(CHARGE_PIN, INPUT);
    interrupts();
    sumVc += analogReadMilliVolts(ADC_PIN) / 1000.0;
  }
  float vc = sumVc / N_AVG;
  float ratio = 1.0 - vc / VCC;
  if (ratio < 1e-4) ratio = 1e-4;
  float cx_F = -((double)T_CH_US * 1e-6) / (R1 * log(ratio));
  vcOut = vc;
  return cx_F * 1e12;
}

// แปลง raw -> ค่าจริง ด้วย interpolate เชิงเส้นทีละช่วง
float applyCal(float raw) {
  if (raw <= CAL_RAW[0])
    return CAL_C[0] + (raw - CAL_RAW[0]) * (CAL_C[1]-CAL_C[0]) / (CAL_RAW[1]-CAL_RAW[0]);
  for (int i = 0; i < CAL_N-1; i++) {
    if (raw <= CAL_RAW[i+1]) {
      float t = (raw - CAL_RAW[i]) / (CAL_RAW[i+1] - CAL_RAW[i]);
      return CAL_C[i] + t * (CAL_C[i+1] - CAL_C[i]);
    }
  }
  int i = CAL_N - 2;
  return CAL_C[i] + (raw - CAL_RAW[i]) * (CAL_C[i+1]-CAL_C[i]) / (CAL_RAW[i+1]-CAL_RAW[i]);
}

void loop() {
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);
  if (pressed && !prevPressed) Serial.println("\"TGR-41\"");
  else if (!pressed && prevPressed) {
    Serial.println("");
    pinMode(CHARGE_PIN, OUTPUT); digitalWrite(CHARGE_PIN, LOW);
  }
  prevPressed = pressed;

  if (pressed) {
    float vc;
    float raw = measureRawCx(vc);
    float cx  = applyCal(raw);
    Serial.printf("Capacitance = %.2f pF\n", cx);
    delay(200);
  }
}