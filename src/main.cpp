#include <lora-handler.h>
#include <Arduino.h>
#include <battery.h>

#define LORA_ADDRESS 0x92
#define LORA_MASTER_ADDRESS 0x93
#define LORA_BANDWIDTH_INDEX 6   // 0-9 mayor numero es menor tiempo de transmision. 4 bits
#define LORA_SPREADING_FACTOR 12 // 6-12 mayor numero es mayor tiempo de transmision. 3 bits
#define LORA_CODING_RATE 8       // 5-8 mayor numero es mayor tiempo de transmision. 2 bits
#define LORA_TRANSMIT_POWER 2    // 0-14 elegir valor bajo para corto alcance. 5 bits

#define USB_BAUD_RATE 9600

#define MAX_ERRORS 0
#define MAX_MESSAGE_COUNT 5

volatile byte errors = 0;
volatile byte lastMessageCount = 0;
volatile byte receivedMessageCount = 0;

volatile long checkConfigTimer = 0;
volatile bool checkingNewConfig = false;

void receiveMessage(byte* payload, byte type);
void changeLoraConfig(byte firstByte, byte secondByte);
void handleConfirmation(byte confirmation);

LoraTransmitConfig LORA_CONFIG = {
    .bandwidthIndex = LORA_BANDWIDTH_INDEX,
    .spreadingFactor = LORA_SPREADING_FACTOR,
    .codingRate = LORA_CODING_RATE,
    .transmitPower = LORA_TRANSMIT_POWER};

LoraTransmitConfig LastValidConfig = LORA_CONFIG;

void setup()
{
    Serial.begin(USB_BAUD_RATE);
    while (!Serial);
    init_PMIC();
    LoraHandler.begin(LORA_ADDRESS, LORA_CONFIG);
    LoraHandler.onReceive(receiveMessage);
    SerialUSB.println("Starting");
}

void loop() {
    if (checkingNewConfig) {
        if (millis() - checkConfigTimer > 8000 && receivedMessageCount < 4) {
            LORA_CONFIG = LastValidConfig;
            checkingNewConfig = false;
            SerialUSB.println("Rollback to last valid config");
            LoraHandler.setLoraConfig(LORA_CONFIG);
        }
    }
}

void receiveMessage(byte* payload, byte type)
{
    if (type == 0) {
        changeLoraConfig(payload[0], payload[1]);
    } else if (type == 1) {
        handleConfirmation(payload[0] & 0b01111111);
    }
}

void changeLoraConfig(byte firstByte, byte secondByte)
{
    const byte bandwidthIndex = (firstByte & 0b01111000) >> 3;
    const byte spreadingFactor = (firstByte & 0b00000111) + 6;
    const byte codingRate = ((secondByte & 0b01100000) >> 5) + 5;
    const byte transmitPower = (secondByte & 0b00011111);
    const LoraTransmitConfig config = {
        .bandwidthIndex = bandwidthIndex,
        .spreadingFactor = spreadingFactor,
        .codingRate = codingRate,
        .transmitPower = transmitPower};
    LastValidConfig = LORA_CONFIG;
    LORA_CONFIG = config;
    LoraHandler.setLoraConfig(config);

    SerialUSB.println("Setting Lora config to:");
    SerialUSB.print("Bandwidth index: ");
    SerialUSB.println(config.bandwidthIndex);
    SerialUSB.print("Spreading factor: ");
    SerialUSB.println(config.spreadingFactor);
    SerialUSB.print("Coding rate: ");
    SerialUSB.println(config.codingRate);
    SerialUSB.print("Transmit power: ");
    SerialUSB.println(config.transmitPower);
    SerialUSB.println();
    checkingNewConfig = true;
    checkConfigTimer = millis();
}

void handleConfirmation(byte messageCount)
{
    SerialUSB.println("Received confirmation");
    SerialUSB.print("Message count: ");
    SerialUSB.println(messageCount);

    if (MAX_MESSAGE_COUNT == messageCount) {
        SerialUSB.println("Sending OK\n");
        LoraHandler.sendMessage(LORA_MASTER_ADDRESS, 0);
        checkingNewConfig = false;
    }
}