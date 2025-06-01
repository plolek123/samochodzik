#include <SPI.h>
#include <RF24.h>

#define JOY1_X A0
#define JOY1_Y A1
#define JOY1_BTN 2

#define JOY2_X A2
#define JOY2_Y A3
#define JOY2_BTN 3

#define POT1 A6
#define POT2 A7

#define BTN1 4
#define BTN2 5
#define BTN3 6
#define BTN4 7
#define BTN5 8


// CE i CSN piny nRF24L01
#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);

const byte address[6] = "1Node";

enum
{
    ANALOG_JOY1_X,
    ANALOG_JOY1_Y,
    ANALOG_JOY2_X,
    ANALOG_JOY2_Y,
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

data_to_send_t data_to_send;

void setup() {
    Serial.begin(9600);

    pinMode(JOY1_BTN, INPUT_PULLUP);
    pinMode(JOY2_BTN, INPUT_PULLUP);
    pinMode(BTN1, INPUT_PULLUP);
    pinMode(BTN2, INPUT_PULLUP);
    pinMode(BTN3, INPUT_PULLUP);
    pinMode(BTN4, INPUT_PULLUP);
    pinMode(BTN5, INPUT_PULLUP);

    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_HIGH);
    radio.stopListening();

}



void read_analog_and_buttons(data_to_send_t *data)
{
    data->analog[ANALOG_JOY1_X] = map(analogRead(JOY1_X), 0, 1023, -100, 100);
    data->analog[ANALOG_JOY1_Y] = map(analogRead(JOY1_Y), 0, 1023, -100, 100);
    data->analog[ANALOG_JOY2_X] = map(analogRead(JOY2_X), 0, 1023, -100, 100);
    data->analog[ANALOG_JOY2_Y] = map(analogRead(JOY2_Y), 0, 1023, -100, 100);
    data->analog[ANALOG_POT1]   = map(analogRead(POT1),   0, 1023, 0, 100);
    data->analog[ANALOG_POT2]   = map(analogRead(POT2),   0, 1023, 0, 100);

    data->button_0 = (digitalRead(JOY1_BTN) == LOW ? 1 : 0);
    data->button_1 = (digitalRead(JOY2_BTN) == LOW ? 1 : 0);
    data->button_2 = (digitalRead(BTN1) == LOW ? 1 : 0);
    data->button_3 = (digitalRead(BTN2) == LOW ? 1 : 0);
    data->button_4 = (digitalRead(BTN3) == LOW ? 1 : 0);
    data->button_5 = (digitalRead(BTN4) == LOW ? 1 : 0);
    data->button_6 = (digitalRead(BTN5) == LOW ? 1 : 0);
}

void loop()
{
    read_analog_and_buttons(&data_to_send);

    bool ok = radio.write(&data_to_send, sizeof(data_to_send_t));

    /*radio.writeFast(&data_to_send, sizeof(data_to_send_t));   //Fills the FIFO buffers up
    bool ok = radio.txStandBy();*/

    /*
    const char message[] = "naprzod";
    bool ok = radio.write(&message, sizeof(message));
    */

    if (ok) {
        //Serial.println("Wysłano");
        /*Serial.print("B:");
        Serial.println(data_to_send.buttons);*/
    } else {
        Serial.println("Błąd wysyłania");
    }

    //delay(10);
}
