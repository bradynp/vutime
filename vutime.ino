#include <Arduino.h>

#define PINSW0  2
#define PINSW1  0
#define PINCOL  5 // PWM
#define PINB0   A0
#define PINB1   A1
#define PINB2   A2
#define PINB3   A3
#define PINB4   A4
#define PINB5   A5
#define PINAM0  3  // PWM
#define PINAM1  9  // PWM
#define PINAM2  10 // PWM
#define PINAM3  11 // PWM
#define PINBUZ  6  // PWM
#define PINPOT1 A6
#define PINPOT2 A7

class VuDefs {
public:
    VuDefs() {}
    ~VuDefs() {}

    double meter_R_m  = 152.0;
    double meter_I_fs = 0.001;
    double Vfs        = 5.00;
    double R_n;
    double meter_power;

    const int metercalibrated_A[2]  = {5, 240};
    const int metercalibrated_B[10] = {0, 28, 57, 85, 114, 142, 170, 200, 226, 240};
    const int metercalibrated_C[6]  = {5, 51, 102, 153, 204, 240};
    const int metercalibrated_D[10] = {0, 28, 57, 85, 114, 142, 170, 200, 226, 240};

    void testmeters() {
        analogWrite(PINAM0, 0xFF * 0.85);
        analogWrite(PINAM1, 0xFF * 0.85);
        analogWrite(PINAM2, 0xFF * 0.85);
        analogWrite(PINAM3, 0xFF * 0.85);
        delay(500);
    }
    void hardwarehelper() {
        R_n         = Vfs / meter_I_fs - meter_R_m;
        meter_power = pow(Vfs, 2.0) / meter_R_m;
        Serial.println(String("Computed Rn value: ") + R_n +
                       " ohms.\r\nComputed Power: " + meter_power * 1000 +
                       " milliwatts.\r\n");
    }

    // Initial time values on startup
    int offhour   = 12;
    int offminute = 57;
    int almhour   = 12;
    int almminute = 0;
    int timenumber;

    void timingalmRoutine() {
        // Find time components
        static long hourComponent, minuteComponent, minute_true;
        unsigned long sec = millis() / 1000;
        minute_true       = sec / 60 + offminute;
        minuteComponent   = minute_true % 60;
        hourComponent     = (minute_true / 60 + offhour) % 24;
        timenumber        = 100 * hourComponent + minuteComponent;

        // Check alarm
        analogWrite(PINBUZ, LOW);
        if (!digitalRead(PINSW0) && (hourComponent == almhour) && (minuteComponent == almminute) && (millis() % 1000 > 500)) {
            analogWrite(PINBUZ, HIGH);
        }

        // Adjust values and output to meters
        if (hourComponent == 0) {
            hourComponent += 12;
        }
        if (hourComponent > 12) {
            hourComponent -= 12;
        }

        analogWrite(PINAM0, metercalibrated_A[hourComponent / 10]);
        analogWrite(PINAM1, metercalibrated_B[hourComponent % 10]);
        analogWrite(PINAM2, metercalibrated_C[minuteComponent / 10]);
        analogWrite(PINAM3, metercalibrated_D[minuteComponent % 10]);

        // Binary seconds ticker
        digitalWrite(PINCOL, millis() % 1000 > 500);
        for (int j = 0; j <= 5; j++) {
            digitalWrite(PINB0 + j, (sec % 60) & (0b1 << j));
        }
    }

    void buttonsRoutine() {
        //    if (!digitalRead(PINSW1) {
        //        almhour   = (int) floor(analogRead(pinmap[16]) * 24.0 / 4096.0 + 0.49);
        //        almminute = (int) floor(analogRead(pinmap[17]) * 60.0 / 4096.0 + 0.49);
        //    }
        //    else if (!digitalRead(PINSW0)) {
        //        offhour   = (int) floor(analogRead(pinmap[16]) * 24.0 / 4096.0 + 0.49);
        //        offminute = (int) floor(analogRead(pinmap[17]) * 60.0 / 4096.0 + 0.49);
        //    }
    }
};
VuDefs c;

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Set pin modes
    pinMode(PINSW0, INPUT);
    pinMode(PINSW1, INPUT);
    pinMode(PINCOL, OUTPUT);
    pinMode(PINB0, OUTPUT);
    pinMode(PINB1, OUTPUT);
    pinMode(PINB2, OUTPUT);
    pinMode(PINB3, OUTPUT);
    pinMode(PINB4, OUTPUT);
    pinMode(PINB5, OUTPUT);
    pinMode(PINAM0, OUTPUT);
    pinMode(PINAM1, OUTPUT);
    pinMode(PINAM2, OUTPUT);
    pinMode(PINAM3, OUTPUT);
    pinMode(PINBUZ, OUTPUT);
    pinMode(PINPOT1, INPUT);
    pinMode(PINPOT2, INPUT);

    c.testmeters();
}
void loop() {
    c.timingalmRoutine();
    c.buttonsRoutine();
}
