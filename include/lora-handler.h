#ifndef LORA_RECEIVER_H
#define LORA_RECEIVER_H

#include <Arduino.h>

struct LoraTransmitConfig {
    byte bandwidthIndex;
    byte spreadingFactor;
    byte codingRate;
    byte transmitPower;
};

class LoraHandlerClass {
public:
    void begin(byte deviceAddress, LoraTransmitConfig config);
    void sendMessage(byte toAddress, byte content);
    void onReceive(void (*callback)(byte* payload, byte type));
    void setLoraConfig(LoraTransmitConfig config);
    float calculateTransmissionTime(LoraTransmitConfig config);
private:
    static void receiveMessage(int packetSize);
    static void finishedSending();
};

extern LoraHandlerClass LoraHandler;

#endif