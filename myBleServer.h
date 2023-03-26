#ifndef myBle_h
#define myBle_h

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

#define SERVICE_UUID           "bd5bfe6c-acea-11ec-b909-0242ac120002" // UART service UUID
#define CHARACTERISTIC_UUID_RX "c3832180-acea-11ec-b909-0242ac120002"
#define CHARACTERISTIC_UUID_TX "1d9bdc66-aceb-11ec-b909-0242ac120002"


//////////////////////////////////////////////////////////////


class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

std::vector<std::string> split(std::string s, std::string divid) {
  std::vector<std::string> v;
  char* c = strtok((char*)s.c_str(), divid.c_str());
  while (c) {
    v.push_back(c);
    c = strtok(NULL, divid.c_str());
  }
  return v;
}

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    char buff[1024] = {0, };
    for (int i = 0; i < rxValue.length(); i++)
      buff[i] = rxValue[i];

    if ( (rxValue.length() > 10) /*&& (rxValue.compare(DEF_GET) == 0)*/ ) {
      Serial.println("*********");
      Serial.println(rxValue.c_str());

      std::string _rstring = std::string(buff);
      std::vector<std::string> vecValue = split(_rstring, ",");

      pTxCharacteristic->setValue("Changed Base Values ... ");
      pTxCharacteristic->notify();
    }
    else {
      Serial.println("Send signal Switch on off");
      std::string _rval = "off";
      pTxCharacteristic->setValue(_rval);
      pTxCharacteristic->notify();    
      pTxCharacteristic->setValue("\n");
      pTxCharacteristic->notify();      
    }
  }
};

class myBleServer {
  public:

  public:
    myBleServer() {

    }

    void NotifySensorValues(String & _sensor_values)
    {
        pTxCharacteristic->setValue(std::string(_sensor_values.c_str()));
        pTxCharacteristic->notify();
        
        pTxCharacteristic->setValue("\n");
        pTxCharacteristic->notify();   
    }

    void BLECheck(){
      // disconnecting
      if (!deviceConnected && oldDeviceConnected) {
          delay(500); // give the bluetooth stack the chance to get things ready
          pServer->startAdvertising(); // restart advertising
          Serial.println("start advertising");
          oldDeviceConnected = deviceConnected;
      }
      // connecting
      if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
          oldDeviceConnected = deviceConnected;
      }
    }


    void BLEStart() {
      // Create the BLE Device
      BLEDevice::init("Home Control");
      pServer = BLEDevice::createServer();
      pServer->setCallbacks(new MyServerCallbacks());

      BLEService *pService = pServer->createService(SERVICE_UUID);
      pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
      pTxCharacteristic->addDescriptor(new BLE2902());

      BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                          CHARACTERISTIC_UUID_RX,
                          BLECharacteristic::PROPERTY_WRITE
                        );
      pRxCharacteristic->setCallbacks(new MyCallbacks());

      pService->start();

      pServer->getAdvertising()->start();
      Serial.println("Waiting a client connection to notify...");
    };


}

#endif