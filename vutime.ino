#include <Arduino.h>

#define DEBUGGING true

class VuDefs {
public:
  VuDefs() {}
  ~VuDefs() {}
  /* Pinmap order:
  0-sw0, sw1, sw2;
  3-resv;
  4-colon;
  5-btw0, btw1, btw2, btw3, btw4, btw5 (halfseconds);
  11-Amm0, Amm1, Amm2, Amm3;
  15-transistor/buzzer;
  16-pot1, pot2;
  18-resv;
  19-resv (-1) signifies vector terminates
  */
  // PWM pins are 3,5*,6*,9,10,11 marked with hex
  // Bitwise output uses analog pins as digital pins
  const short pinmap[20] = {2,  3,   4,   0,   0x5, A0,  A1, A2, A3, A4,
                            A5, 0x3, 0x9, 0xA, 0xB, 0x6, A6, A7, 0,  -1};
  double meter_R_m = 100.0;
  double meter_I_fs = 0.001;
  double Vfs = 5.00;
  double R_n;
  double meter_power;

  int offhour = 12;
  int offminute = 34;
  int almhour = 12;
  int almminute = 0;

  void testmeters() {
    for (int k = 0; k < 255; k += 25) {
      analogWrite(11, k);
      analogWrite(12, k);
      analogWrite(13, k);
      analogWrite(14, k);
      delay(50);
    }
    delay(250);
    for (int k = 255; k > 0; k -= 30) {
      analogWrite(11, k);
      analogWrite(12, k);
      analogWrite(13, k);
      analogWrite(14, k);
      delay(50);
    }
    delay(500);
  }

  void timingRoutine() {
    static long hourComponent, minuteComponent, minute_true;
    unsigned long sec = millis() / 1000;
    minute_true = sec / 60 + offminute;
    minuteComponent = minuteComponent % 60;
    hourComponent = (minute_true / 60 + offhour) % 24;
    unsigned timenumber = 100 * hourComponent + minuteComponent;

    int timehour = timenumber / 100;
    if (timehour == 0) {
      timenumber += 1200;
    }
    if (timehour > 12) {
      timenumber -= 1200;
    }

    static double meter_scales[4] = {1.0, 9.0, 5.0, 9.0};

    for (int k = 0; k <= 3; k++) {
    }
    analogWrite(pinmap[11], (int)((timenumber / 1000) % 10000) * (255.0 / 1.0));
    analogWrite(pinmap[12], (int)((timenumber / 100) % 1000) * (255.0 / 9.0));
    analogWrite(pinmap[13], (int)((timenumber / 10) % 100) * (255.0 / 5.0));
    analogWrite(pinmap[14], (int)((timenumber / 1) % 10) * (255.0 / 9.0));

    digitalWrite(pinmap[4], millis() % 1000 > 500);
    for (int j = 0; j <= 5; j++) {
      digitalWrite(pinmap[5 + j], (sec % 60) & (0b1 << j));
    }
  }

  void alarmRoutine() { analogWrite(pinmap[15], LOW); }

  void buttonsRoutine() { unsigned precomputedthresholds[12]; }
};
VuDefs c;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(c.pinmap[0], INPUT);
  pinMode(c.pinmap[1], INPUT);
  pinMode(c.pinmap[2], INPUT);
  pinMode(c.pinmap[3], INPUT);
  pinMode(c.pinmap[4], OUTPUT);
  pinMode(c.pinmap[5], OUTPUT);
  pinMode(c.pinmap[6], OUTPUT);
  pinMode(c.pinmap[7], OUTPUT);
  pinMode(c.pinmap[8], OUTPUT);
  pinMode(c.pinmap[9], OUTPUT);
  pinMode(c.pinmap[10], OUTPUT);
  pinMode(c.pinmap[11], OUTPUT);
  pinMode(c.pinmap[12], OUTPUT);
  pinMode(c.pinmap[13], OUTPUT);
  pinMode(c.pinmap[14], OUTPUT);
  pinMode(c.pinmap[15], OUTPUT);
  pinMode(c.pinmap[16], INPUT);
  pinMode(c.pinmap[17], INPUT);
  pinMode(c.pinmap[18], INPUT);

  c.R_n = c.Vfs / c.meter_I_fs - c.meter_R_m;
  c.meter_power = pow(c.Vfs, 2.0) / c.meter_R_m;
  if (DEBUGGING) {
    Serial.println(String("Computed Rn value: ") + c.R_n +
                   " ohms.\nComputed Power: " + c.meter_power * 1000 +
                   " milliwatts.\n");
    c.testmeters();
  }
}
void loop() {
  c.timingRoutine();
  c.alarmRoutine();
  c.buttonsRoutine();
}
