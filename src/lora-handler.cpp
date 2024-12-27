#include <lora-handler.h>
#include <Arduino.h>
#include <LoRa.h>

#define LORA_MASTER_ADDRESS 0x93

byte localAddress;
LoraHandlerClass LoraHandler;
const double bandwidth_kHz[10] = {7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,
                            41.7E3, 62.5E3, 125E3, 250E3, 500E3 };
volatile bool isSending = false;
void (*receiveCallback)(byte firstByte, byte secondByte) = nullptr;

void LoraHandlerClass::begin(byte deviceAddress, LoraTransmitConfig config)
{
    localAddress = deviceAddress;
    if (!LoRa.begin(868E6)) {
        digitalWrite(LED_BUILTIN, HIGH);
        while (true);
    }
    setLoraConfig(config);
    LoRa.setSyncWord(0x12);
    LoRa.setPreambleLength(8);
    LoRa.onTxDone(finishedSending);
    LoRa.onReceive(receiveMessage);
    LoRa.receive();
}

void LoraHandlerClass::sendMessage(byte toAddress, byte content)
{
    isSending = true;
    while (!LoRa.beginPacket()) {
        delay(10);
    }
    LoRa.write(toAddress);
    LoRa.write(localAddress);
    LoRa.write(content);
    LoRa.endPacket(true);
}

void LoraHandlerClass::receiveMessage(int packetSize)
{
    if (isSending) return;
    if (packetSize == 0) return;
    if (receiveCallback == NULL) return;
    const byte recipient = LoRa.read();
    const byte sender = LoRa.read();

    if (recipient != localAddress) return;
    if (sender != LORA_MASTER_ADDRESS) return;
    const byte firstByte = LoRa.read();
    const byte secondByte = LoRa.read();
    receiveCallback(firstByte, secondByte);
}

void LoraHandlerClass::onReceive(void (*callback)(byte firstByte, byte secondByte))
{
    receiveCallback = callback;
}

void LoraHandlerClass::setLoraConfig(LoraTransmitConfig config)
{
    LoRa.setSignalBandwidth(long(bandwidth_kHz[config.bandwidthIndex]));
    LoRa.setSpreadingFactor(config.spreadingFactor);
    LoRa.setCodingRate4(config.codingRate);
    LoRa.setTxPower(config.transmitPower, PA_OUTPUT_PA_BOOST_PIN);
}

void LoraHandlerClass::finishedSending()
{
    isSending = false;
    LoRa.receive();
}