#include <lora-handler.h>
#include <Arduino.h>
#include <battery.h>

#define LORA_ADDRESS 0x92
#define LORA_MASTER_ADDRESS 0x93
#define LORA_BANDWIDTH_INDEX 5   // 0-9 mayor numero es menor tiempo de transmision. 4 bits
#define LORA_SPREADING_FACTOR 8 // 6-12 mayor numero es mayor tiempo de transmision. 3 bits
#define LORA_CODING_RATE 6       // 5-8 mayor numero es mayor tiempo de transmision. 2 bits
#define LORA_TRANSMIT_POWER 2    // 0-14 elegir valor bajo para corto alcance. 5 bits

#define USB_BAUD_RATE 9600

#define MAX_ERRORS 0
#define MAX_MESSAGE_COUNT 5

volatile byte errors = 0;
volatile byte lastMessageCount = 0;
volatile byte receivedMessageCount = 0;

volatile float timeOut_ms = INFINITY;
volatile bool optimized = false;
volatile bool shouldChangeConfig = false;
volatile unsigned long lastReceiveMessageTime_ms = 0;

void receiveMessage(byte* payload, byte type);
void changeLoraConfig(byte firstByte, byte secondByte);
void handleConfirmation(byte confirmation);
void checkTimeout();

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
    if (!optimized) checkTimeout();
    if (shouldChangeConfig) {
        delay(LoraHandler.calculateTransmissionTime(LastValidConfig) * 2000);
        LoraHandler.setLoraConfig(LORA_CONFIG);
        shouldChangeConfig = false;
    }
}

void checkTimeout() {
    if (millis() - lastReceiveMessageTime_ms > timeOut_ms) {
        LORA_CONFIG = LastValidConfig;
        optimized = true;
        SerialUSB.println("Rollback to last valid config");
        LoraHandler.setLoraConfig(LORA_CONFIG);
    }
}

void receiveMessage(byte* payload, byte type)
{
    lastReceiveMessageTime_ms = millis();
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


    shouldChangeConfig = true;

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

    timeOut_ms = LoraHandler.calculateTransmissionTime(LastValidConfig) * 1000 * 200;
    SerialUSB.print("Estimated transmission time (s): ");
    SerialUSB.println(LoraHandler.calculateTransmissionTime(LastValidConfig));
    SerialUSB.print("Timeout (s): ");
    SerialUSB.println(timeOut_ms / 1000);
    LoraHandler.sendMessage(LORA_MASTER_ADDRESS, 0);
}

void handleConfirmation(byte content)
{
    SerialUSB.print("Message count: ");
    SerialUSB.println(content);
    SerialUSB.println("Sending OK");
    LoraHandler.sendMessage(LORA_MASTER_ADDRESS, content);
}