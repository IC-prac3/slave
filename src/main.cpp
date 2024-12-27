#include <lora-handler.h>
#include <Arduino.h>
#include <battery.h>

#define LORA_ADDRESS 0x92
#define LORA_BANDWIDTH_INDEX 6   // 0-9 mayor numero es menor tiempo de transmision. 4 bits
#define LORA_SPREADING_FACTOR 10 // 6-12 mayor numero es mayor tiempo de transmision. 3 bits
#define LORA_CODING_RATE 5       // 5-8 mayor numero es mayor tiempo de transmision. 2 bits
#define LORA_TRANSMIT_POWER 2    // 2-20 elegir valor bajo para corto alcance. 5 bits

#define USB_BAUD_RATE 9600

void receiveMessage(byte firstByte, byte secondByte);

const LoraTransmitConfig LORA_CONFIG = {
    .bandwidthIndex = LORA_BANDWIDTH_INDEX,
    .spreadingFactor = LORA_SPREADING_FACTOR,
    .codingRate = LORA_CODING_RATE,
    .transmitPower = LORA_TRANSMIT_POWER};

void setup()
{
    Serial.begin(9600);
    while (!Serial);
    init_PMIC();
    LoraHandler.begin(LORA_ADDRESS, LORA_CONFIG);
    LoraHandler.onReceive(receiveMessage);
}

void loop() {}

void receiveMessage(byte firstByte, byte secondByte)
{
    const byte bandwidthIndex = (firstByte & 0b01111000) >> 3;
    const byte spreadingFactor = (firstByte & 0b00000111);
    const byte codingRate = (secondByte & 0b01100000) >> 5;
    const byte transmitPower = (secondByte & 0b00011111);
    const LoraTransmitConfig config = {
        .bandwidthIndex = bandwidthIndex,
        .spreadingFactor = spreadingFactor,
        .codingRate = codingRate,
        .transmitPower = transmitPower};
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
}