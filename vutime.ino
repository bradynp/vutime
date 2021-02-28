#include <Arduino.h>

#define DEBUGGING false

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

    const int metercalibrated_A[2]  = {35, 215};
    const int metercalibrated_B[10] = {1, 21, 42, 73, 95, 130, 161, 184, 207, 235};
    const int metercalibrated_C[6]  = {17, 58, 104, 151, 198, 245};
    const int metercalibrated_D[10] = {9, 34, 56, 85, 115, 150, 170, 200, 218, 242};
    const double ADCMax             = pow(2.0, 10.0);

    void testmeters() {
        analogWrite(PINAM0, 0xFF * 0.93);
        analogWrite(PINAM1, 0xFF * 0.93);
        analogWrite(PINAM2, 0xFF * 0.93);
        analogWrite(PINAM3, 0xFF * 0.93);
        delay(500);
    }

    // Initial time values on startup
    int offhour   = 12;
    int offminute = 00;
    int almhour   = 12;
    int almminute = 00;

    long hourComponent, minuteComponent, minute_true;
    unsigned long sec;

    void timingRoutine() {
        // Find time components
        sec             = millis() / 1000;
        minute_true     = sec / 60 + offminute;
        minuteComponent = minute_true % 60;
        hourComponent   = (minute_true / 60 + offhour) % 24;

        // Check alarm
        //analogWrite(PINBUZ, LOW);
        //if (!digitalRead(PINSW0) && (hourComponent == almhour) && (minuteComponent == almminute) && (millis() % 1000 > 500)) {
        //    analogWrite(PINBUZ, HIGH);
        //}
        // Adjust values before output to meters
        if (hourComponent == 0) {
            hourComponent += 12;
        }
        if (hourComponent > 12) {
            hourComponent -= 12;
        }
        // Binary seconds ticker
        digitalWrite(PINCOL, millis() % 1000 > 500);
        for (int j = 0; j <= 5; j++) {
            digitalWrite(PINB0 + j, (sec % 60) & (0b1 << j));
        }
        if (digitalRead(PINSW1)) { // alarm-set is HIGH, normal op
            analogWrite(PINAM0, metercalibrated_A[hourComponent / 10]);
            analogWrite(PINAM1, metercalibrated_B[hourComponent % 10]);
            analogWrite(PINAM2, metercalibrated_C[minuteComponent / 10]);
            analogWrite(PINAM3, metercalibrated_D[minuteComponent % 10]);
        }
        else { // setting alarm:
            almhour   = floor(analogRead(PINPOT1) * 24.0 / ADCMax + 0.3);
            almminute = floor(analogRead(PINPOT2) * 60.0 / ADCMax + 0.3);
            analogWrite(PINAM0, metercalibrated_A[almhour / 10]);
            analogWrite(PINAM1, metercalibrated_B[almhour % 10]);
            analogWrite(PINAM2, metercalibrated_C[almminute / 10]);
            analogWrite(PINAM3, metercalibrated_D[almminute % 10]);
        }
        if (!digitalRead(PINSW0)) { // last case, setting time
            unsigned long minspassed = (sec / 60) % 60;
            unsigned long hrspassed  = (minspassed / 60) % 24;
            offhour                  = floor(analogRead(PINPOT1) * 24.0 / ADCMax + 0.3) - minspassed;
            offminute                = floor(analogRead(PINPOT2) * 60.0 / ADCMax + 0.3) - hrspassed;
        }
    }
};
VuDefs clk;

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Set pin modes
    pinMode(PINSW0, INPUT_PULLUP);
    pinMode(PINSW1, INPUT_PULLUP);
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

    clk.testmeters();

    if (DEBUGGING) {
        Serial.begin(9600);
    }
}
void loop() {
    while (DEBUGGING) {

        if (Serial.available()) {
            int b = Serial.readStringUntil('\n').toInt();
            analogWrite(PINAM0, b);
            analogWrite(PINAM1, b);
            analogWrite(PINAM2, b);
            analogWrite(PINAM3, b);
            Serial.println(b);
        }
    }
    clk.timingRoutine();
}
