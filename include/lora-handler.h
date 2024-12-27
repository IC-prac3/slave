#ifndef LORA_RECEIVER_H
#define LORA_RECEIVER_H

#include <Arduino.h>

struct LoraConfig {
    byte deviceAddress;
    byte bandwidthIndex;
    byte spreadingFactor;
    byte codingRate;
    byte transmitPower;
};

class LoraHandler{
public:
    static void begin(LoraConfig config);
    static void sendMessage(byte toAddress, byte content);
    static void onReceive(void (*callback)(byte fromAddress, byte content));
private:
    static void receiveMessage(int packetSize);
    static void (*receiveCallback)(byte fromAddress, byte content);
    static void finishedSending();
    static byte localAddress;
};

#endif