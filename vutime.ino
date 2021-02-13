#include <Arduino.h>

#define DEBUGGING true

class VuDefs {
public:
    VuDefs() {}
    ~VuDefs() {}
    /*
  Pinmap order:
  0-sw0, sw1, sw2; alarm toggle switch, setting time toggle, setting alarm toggle
  3-resv;
  4-colon;
  5-btw0, btw1, btw2, btw3, btw4, btw5 (halfseconds);
  11-Amm0, Amm1, Amm2, Amm3;
  15-transistor/buzzer;
  16-pot1, pot2;
  18-resv;
  19-resv (-1) signifies vector terminates
  PWM pins are 3,5*,6*,9,10,11 marked with hex
  Binary output uses analog pins as digital pins
  */
    const short pinmap[20] = {2, 3, 4, 0, 0x5, A0, A1, A2, A3, A4,
                              A5, 0x3, 0x9, 0xA, 0xB, 0x6, A6, A7, 0, -1};
    double meter_R_m       = 100.0;
    double meter_I_fs      = 0.001;
    double Vfs             = 5.00;
    double R_n;
    double meter_power;

    // Initial time values on startup
    int offhour   = 7;
    int offminute = 27;
    int almhour   = 12;
    int almminute = 0;

    void testmeters() {
        for (int k = 0; k < 255; k += 25) {
            analogWrite(11, k);
            analogWrite(12, k);
            analogWrite(13, k);
            analogWrite(14, k);
            delay(50);
        }
        delay(500);
        for (int k = 255; k > 0; k -= 25) {
            analogWrite(11, k);
            analogWrite(12, k);
            analogWrite(13, k);
            analogWrite(14, k);
            delay(50);
        }
        delay(500);
    }

    void timingalmRoutine() {
        // Find time components
        static long hourComponent, minuteComponent, minute_true;
        unsigned long sec   = millis() / 1000;
        minute_true         = sec / 60 + offminute;
        minuteComponent     = minuteComponent % 60;
        hourComponent       = (minute_true / 60 + offhour) % 24;
        unsigned timenumber = 100 * hourComponent + minuteComponent;

        // Check alarm
        analogWrite(pinmap[15], LOW);
        if (!digitalRead(pinmap[0]) && (hourComponent == almhour) && (minuteComponent == almminute) && (millis() % 1000 > 500)) {
            analogWrite(pinmap[15], HIGH);
        }

        // Adjust values and output to meters
        if (hourComponent == 0) {
            hourComponent += 12;
        }
        if (hourComponent > 12) {
            hourComponent -= 12;
        }
        static byte metercalibrated_A[2]  = {5, 250};
        static byte metercalibrated_B[10] = {0, 28, 57, 85, 114,
                                             142, 170, 200, 226, 255};
        static byte metercalibrated_C[6]  = {5, 51, 102, 153, 204, 250};
        static byte metercalibrated_D[10] = {0, 28, 57, 85, 114,
                                             142, 170, 200, 226, 255};
        analogWrite(pinmap[11], metercalibrated_A[hourComponent / 10]);
        analogWrite(pinmap[12], metercalibrated_B[hourComponent % 10]);
        analogWrite(pinmap[13], metercalibrated_C[minuteComponent / 10]);
        analogWrite(pinmap[14], metercalibrated_D[minuteComponent % 10]);

        // Binary seconds ticker
        digitalWrite(pinmap[4], millis() % 1000 > 500);
        for (int j = 0; j <= 5; j++) {
            digitalWrite(pinmap[5 + j], (sec % 60) & (0b1 << j));
        }
    }

    //
    void buttonsRoutine() {
        if (!digitalRead(pinmap[2])) {
            almhour   = (int) floor(analogRead(pinmap[16]) * 24.0 / 4096.0 + 0.49);
            almminute = (int) floor(analogRead(pinmap[17]) * 60.0 / 4096.0 + 0.49);
        }
        else if (!digitalRead(pinmap[1])) {
            offhour   = (int) floor(analogRead(pinmap[16]) * 24.0 / 4096.0 + 0.49);
            offminute = (int) floor(analogRead(pinmap[17]) * 60.0 / 4096.0 + 0.49);
        }
    }
};
VuDefs c;

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Set mode of all pins
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

    // Hardware value helper
    c.R_n         = c.Vfs / c.meter_I_fs - c.meter_R_m;
    c.meter_power = pow(c.Vfs, 2.0) / c.meter_R_m;
    if (DEBUGGING) {
        Serial.println(String("Computed Rn value: ") + c.R_n +
                       " ohms.\nComputed Power: " + c.meter_power * 1000 +
                       " milliwatts.\n");
        c.testmeters();
    }
}
void loop() {
    c.timingalmRoutine();
    c.buttonsRoutine();
}
