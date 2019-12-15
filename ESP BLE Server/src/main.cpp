#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pDEBUG = NULL;
BLECharacteristic* pFlush = NULL;
BLECharacteristic* pFRUIT1 = NULL;
BLECharacteristic* pFRUIT2 = NULL;
BLECharacteristic* pFRUIT3 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DEBUG_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define FLUSH_UUID "ae1ed7be-a009-4632-b02b-18675e8d847a"
#define FRUIT1_UUID "1ed6a335-83b5-4d58-9a94-dda45807ae41"
#define FRUIT2_UUID "fd64d496-6aeb-4374-9f69-89260a61c1c9"
#define FRUIT3_UUID "e303dd59-69bf-4027-b64d-080e023c6105"

class Fruit1CB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFruit1) {
      std::string value = pFruit1->getValue();
       
    }
};

class Fruit2CB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFruit2) {
      std::string value = pFruit2->getValue();
       
    }
};

class Fruit3CB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFruit3) {
      std::string value = pFruit3->getValue();
       
    }
};

class FlushCB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFlush) {
      std::string value = pFlush->getValue();
        if(value == "1"){
          //SET EVERY PUMP TO MAX
        }
    }
};

class DebugCB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pDEBUG) {
      std::string value = pDEBUG->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};




void setup() {
  Serial.begin(9600);

  // Create the BLE Device
  BLEDevice::init("Smart-Smoothie-Squisher");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pDEBUG = pService->createCharacteristic(
                      DEBUG_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pFlush = pService->createCharacteristic(
                      FLUSH_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pFRUIT1 = pService->createCharacteristic(
                      FRUIT1_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pFRUIT2 = pService->createCharacteristic(
                    FRUIT2_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE
                  );
  pFRUIT3 = pService->createCharacteristic(
                      FRUIT3_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pDEBUG->addDescriptor(new BLE2902());
  pDEBUG->setCallbacks(new DebugCB());
  pDEBUG->setValue("DEBUG VALUE");

  pFlush->addDescriptor(new BLE2902());
  pFlush->setCallbacks(new FlushCB());
  pFlush->setValue("FLUSH ALL");

  pFRUIT1->addDescriptor(new BLE2902());
  pFRUIT1->setCallbacks(new Fruit1CB());
  pFRUIT1->setValue("FRUIT1 VALUE");

  pFRUIT2->addDescriptor(new BLE2902());
  pFRUIT2->setCallbacks(new Fruit2CB());
  pFRUIT2->setValue("FRUIT2 VALUE");

  pFRUIT3->addDescriptor(new BLE2902());
  pFRUIT3->setCallbacks(new Fruit3CB());
  pFRUIT3->setValue("FRUIT3 VALUE");

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
        pDEBUG->setValue((uint8_t*)&value, 4);
        pDEBUG->notify();
        value++;
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
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