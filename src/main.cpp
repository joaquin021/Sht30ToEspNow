#include <Arduino.h>
#include <WEMOS_SHT3X.h>

#include "EspNowService.hpp"
#include "RequestUtils.hpp"
#include "Secrets.h"
#include "WifiUtils.hpp"

SHT3X sht30(0x45);
ADC_MODE(ADC_VCC);

EspNowService espNowService;
RequestUtils requestUtils = RequestUtils::getInstance();

void buildTemperatureRequest(request *request) {
    char temperature[6];
    dtostrf(sht30.cTemp, 5, 2, temperature);
    requestUtils.buildSendOperation(request, temperature, "temperature");
}

void buildHumidityRequest(request *request) {
    char humidity[6];
    dtostrf(sht30.humidity, 5, 2, humidity);
    requestUtils.buildSendOperation(request, humidity, "humidity");
}

void buildBatteryRequest(request *request) {
    char batteryVoltage[5];
    dtostrf(ESP.getVcc() / 1000.0, 4, 3, batteryVoltage);
    char batteryLevel[5];
    itoa(map(ESP.getVcc(), 2200, 3200, 0, 100), batteryLevel, 10);
    requestUtils.buildSendOperation(request, batteryVoltage, "batteryVol");
    requestUtils.buildSendOperation(request, batteryLevel, "batteryLev");
}

void sendRequest(request *request) {
    uint8_t serializedBuffer[ESPNOW_BUFFERSIZE];
    int serializedLen = requestUtils.serialize(serializedBuffer, request);
    espNowService.send(gatewayAddress, serializedBuffer, serializedLen);
}

void setup() {
    Serial.begin(115200);
    debugln();
    setupWiFiForEspNow();
    espNowService.setup(espNowSendCallBackDummy, espNowRecvCallBackDummy);
}

void loop() {
    if (sht30.get() == 0) {
        request request = requestUtils.createRequest(clientName, clientAdress, ESP.getChipId());
        buildTemperatureRequest(&request);
        buildHumidityRequest(&request);
        buildBatteryRequest(&request);
        sendRequest(&request);
    } else {
        debugln("Error getting sht30 sensor");
    }
    ESP.deepSleep(900000000);
}
