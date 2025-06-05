/*
#define SOFTSPI

#define SOFT_SPI_MISO_PIN   12
#define SOFT_SPI_MOSI_PIN   11
#define SOFT_SPI_SCK_PIN    13

*/
#include <SPI.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

// Definicje pinów do silników L293D
#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 7

// Buzzer i diody
#define BUZZER_PIN A0

#define LED_YELLOW_LEFT A2
#define LED_YELLOW_RIGHT A1

#define LED_RED_LEFT A3
#define LED_RED_RIGHT A4

RF24 radio(CE_PIN, CSN_PIN);

const byte address[6] = "1Node";

enum
{
    ANALOG_JOY1_Y,
    ANALOG_JOY1_X,
    ANALOG_JOY2_Y,
    ANALOG_JOY2_X,
    ANALOG_POT1,
    ANALOG_POT2,
    ANALOG_COUNT
};

typedef struct
{
    int8_t analog[ANALOG_COUNT];

    union
    {
        uint8_t buttons;

        struct {
            uint8_t button_0    :   1;
            uint8_t button_1    :   1;
            uint8_t button_2    :   1;
            uint8_t button_3    :   1;
            uint8_t button_4    :   1;
            uint8_t button_5    :   1;
            uint8_t button_6    :   1;
        };
    };
} data_to_send_t;

data_to_send_t data_received;

const int8_t deadZone = 30;

unsigned long buzzerTimer = 0;
//bool buzzerOn = false;


void setup() {
    Serial.begin(9600);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    pinMode(LED_YELLOW_LEFT, OUTPUT);
    pinMode(LED_YELLOW_RIGHT, OUTPUT);
    pinMode(LED_RED_LEFT, OUTPUT);
    pinMode(LED_RED_RIGHT, OUTPUT);

    radio.begin();
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_HIGH);
    radio.startListening();
}

void stopMotors() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void turnLeft() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void driveBackward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void driveForward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void buzzerBeep() {
    // Pip, pauza, pip, pauza...
    unsigned long now = millis();
    unsigned long interval = 300; // czas trwania i przerwy

    if ((now / interval) % 2 == 0) {
        tone(BUZZER_PIN, 1175); // dźwięk 2500 Hz
    } else {
        noTone(BUZZER_PIN);
    }
}

void buzzerOn() {
        tone(BUZZER_PIN, 2050); // dźwięk 1300 Hz
}

void buzzerStop() {
    noTone(BUZZER_PIN);
}
/*
void turnSignalBlink(bool state) {
    digitalWrite(LED_YELLOW_LEFT, state);
    digitalWrite(LED_YELLOW_RIGHT, state);
}

void redLedsOn() {
    digitalWrite(LED_RED_LEFT, HIGH);
    digitalWrite(LED_RED_RIGHT, HIGH);
}

void redLedsOff() {
    digitalWrite(LED_RED_LEFT, LOW);
    digitalWrite(LED_RED_RIGHT, LOW);
}
*/
enum
{
    buzzer_off,
    buzzer_beep,
    buzzer_on
};

enum
{
    led_front_off,
    led_front_blink_left,
    led_front_blink_right,
    led_front_blink_left_right_on,
    led_front_blink_right_left_on,
    led_front_blink_both,
    led_front_back_blink_both,
    led_front_on,
};

enum
{
    led_back_off,
    led_back_on,
};

#define light_on  (data_received.button_6)

void loop() {
  if (radio.available()) {
    radio.read(&data_received, sizeof(data_received));

    int8_t joyX = data_received.analog[ANALOG_JOY1_X];
    int8_t joyY = data_received.analog[ANALOG_JOY1_Y];

    uint8_t buzzerState = buzzer_off;
    uint8_t ledFrontState = led_front_off;
    uint8_t ledBackState = led_back_on;
    uint8_t ledBlinkAllState = led_front_off;

    if (light_on) {
      ledBlinkAllState = led_front_back_blink_both;
    }
    // Use joystick 1 Y for forward/backward, joystick 2 X for left/right
    int8_t joy2X = data_received.analog[ANALOG_JOY2_X];

    if (joyY > deadZone) {
      driveForward();
      ledBackState = led_back_off;
    } else if (joyY < -deadZone) {
      driveBackward();
      ledBackState = led_back_off;
      buzzerState = buzzer_beep;
    } else if (joy2X > deadZone) {
      turnRight();
      ledBackState = led_back_off;
      ledFrontState = light_on ? led_front_blink_right_left_on : led_front_blink_right;
    } else if (joy2X < -deadZone) {
      turnLeft();
      ledBackState = led_back_off;
      ledFrontState = light_on ? led_front_blink_left_right_on : led_front_blink_left;
    } else {
      stopMotors();
    }

    if (data_received.button_2) {
      buzzerState = buzzer_on;
    }
    // If button_6 (light switch) is ON and car is not moving, turn off all lights and blink all lights
    if (data_received.button_6 && abs(joyY) <= deadZone && abs(joy2X) <= deadZone) {
      // Turn off all lights
      digitalWrite(LED_RED_LEFT, LOW);
      digitalWrite(LED_RED_RIGHT, LOW);
      digitalWrite(LED_YELLOW_LEFT, LOW);
      digitalWrite(LED_YELLOW_RIGHT, LOW);
      // Blink all lights
      bool blink = (millis() / 500) % 2;
      digitalWrite(LED_RED_LEFT, blink);
      digitalWrite(LED_RED_RIGHT, blink);
      digitalWrite(LED_YELLOW_LEFT, blink);
      digitalWrite(LED_YELLOW_RIGHT, blink);
      // Skip further LED logic for this loop
      return;
    }
    // Back LEDs
    if (ledBackState == led_back_on) {
      digitalWrite(LED_RED_LEFT, HIGH);
      digitalWrite(LED_RED_RIGHT, HIGH);
    } else {
      digitalWrite(LED_RED_LEFT, LOW);
      digitalWrite(LED_RED_RIGHT, LOW);
    }

    // Blink all LEDs if needed
    if (ledBlinkAllState == led_front_back_blink_both) {
      bool blink = (millis() / 500) % 2;
      digitalWrite(LED_RED_LEFT, blink);
      digitalWrite(LED_RED_RIGHT, blink);
      digitalWrite(LED_YELLOW_LEFT, blink);
      digitalWrite(LED_YELLOW_RIGHT, blink);
    } else {
      // Front LEDs
      switch (ledFrontState) {
        case led_front_off:
          digitalWrite(LED_YELLOW_LEFT, LOW);
          digitalWrite(LED_YELLOW_RIGHT, LOW);
          break;
        case led_front_blink_left:
          digitalWrite(LED_YELLOW_LEFT, (millis() / 500) % 2);
          digitalWrite(LED_YELLOW_RIGHT, LOW);
          break;
        case led_front_blink_right:
          digitalWrite(LED_YELLOW_LEFT, LOW);
          digitalWrite(LED_YELLOW_RIGHT, (millis() / 500) % 2);
          break;
        case led_front_blink_left_right_on:
          digitalWrite(LED_YELLOW_LEFT, (millis() / 500) % 2);
          digitalWrite(LED_YELLOW_RIGHT, HIGH);
          break;
        case led_front_blink_right_left_on:
          digitalWrite(LED_YELLOW_LEFT, HIGH);
          digitalWrite(LED_YELLOW_RIGHT, (millis() / 500) % 2);
          break;
        case led_front_blink_both:
          digitalWrite(LED_YELLOW_LEFT, (millis() / 500) % 2);
          digitalWrite(LED_YELLOW_RIGHT, (millis() / 500) % 2);
          break;
        case led_front_on:
          digitalWrite(LED_YELLOW_LEFT, HIGH);
          digitalWrite(LED_YELLOW_RIGHT, HIGH);
          break;
      }
    }

    // Buzzer control
    switch (buzzerState) {
      case buzzer_beep:
        buzzerBeep();
        break;
      case buzzer_on:
        buzzerOn();
        break;
      default:
        buzzerStop();
        break;
    }
  }
}
