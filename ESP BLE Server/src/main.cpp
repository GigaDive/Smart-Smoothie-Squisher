/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara
   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.
   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
BLEServer* pServer = NULL;
BLECharacteristic* pDebug = NULL;
BLECharacteristic* pFlush = NULL;
BLECharacteristic* pFruit1 = NULL;
BLECharacteristic* pFruit2 = NULL;
BLECharacteristic* pFruit3 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SERVICE2_UUID  "91a23343-0dc2-4f31-852e-de724658e6f4"
#define SERVICE3_UUID "864dda28-8355-417d-89a1-6b69db5e55ce"
#define SERVICE4_UUID "0a5a0a7b-34a9-401c-a964-893edbc840ef"
#define SERVICE5_UUID "d7ed9b8d-062a-43ee-a0b2-7e0296d9d61b"

#define DEBUG_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define FLUSH_UUID "ae1ed7be-a009-4632-b02b-18675e8d847a"
#define FRUIT1_UUID "1ed6a335-83b5-4d58-9a94-dda45807ae41"
#define FRUIT2_UUID "fd64d496-6aeb-4374-9f69-89260a61c1c9"
#define FRUIT3_UUID "e303dd59-69bf-4027-b64d-080e023c6105"

// Motor A
int motor1Pin1 = 17; 
int motor1Pin2 = 21; 
int enable1Pin = 16; 

// Setting PWM properties
const int freq = 100;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 255;

class Fruit1CB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFruit1) {
      std::string value = pFruit1->getValue();
             if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("FRUIT1: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

class Fruit2CB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFruit2) {
      int val = atoi(pFruit2->getValue().c_str());
            val = map(val,0,4095,80,255);
   digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
      if(val > 80){
      ledcWrite(pwmChannel, val);
      }else{
              ledcWrite(pwmChannel, 0);

      }
   /*    */


      Serial.println(val);
/*map(std::strtol(pFruit2->getValue().c_str(),pFruit2->getValue().c_str()),0,4200,80,255)*/
      
    
    }
};

class Fruit3CB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFruit3) {
      std::string value = pFruit3->getValue();
             if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("FRUIT3: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

class FlushCB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pFlush) {
      std::string value = pFlush->getValue();
        int val = atoi(pFlush->getValue().c_str());
 digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

        if (val== 1) {
        Serial.println("*********");
        Serial.print("FLUSH ALL PUMPS TILL SOMETHING ELSE ARRIVES");
        Serial.println();
        Serial.println("*********");
        ledcWrite(pwmChannel, 255);

        while(val==1){

        }
        
      }
        //STOP PUMPS
        Serial.print("STOP FLUSHING");
        digitalWrite(motor1Pin1, LOW);
        digitalWrite(motor1Pin2, LOW);

      
    }
};

class DebugCB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pDEBUG) {
      std::string value = pDEBUG->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("DEBUG: ");
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
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
   // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLEService *pService2 = pServer->createService(SERVICE2_UUID);
  BLEService *pService3 = pServer->createService(SERVICE3_UUID);
  BLEService *pService4 = pServer->createService(SERVICE4_UUID);
  BLEService *pService5 = pServer->createService(SERVICE5_UUID);


  // Create a BLE Characteristic
  pDebug = pService->createCharacteristic(
                      DEBUG_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pFlush = pService->createCharacteristic(
                     FLUSH_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
 pFruit1 = pService2->createCharacteristic(
                     FRUIT1_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
pFruit2 = pService2->createCharacteristic(
                     FRUIT2_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
pFruit3 = pService2->createCharacteristic(
                     FRUIT3_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pDebug->addDescriptor(new BLE2902());
  pDebug->setCallbacks(new DebugCB());
  pDebug->setValue("Hello World");

  pFlush->addDescriptor(new BLE2902());
  pFlush->setCallbacks(new FlushCB());
  pFlush->setValue("Flush");

  pFruit1->addDescriptor(new BLE2902());
  pFruit1->setCallbacks(new Fruit1CB());
  pFruit1->setValue("F1");

  pFruit2->addDescriptor(new BLE2902());
  pFruit2->setCallbacks(new Fruit2CB());
  pFruit2->setValue("F2");

  pFruit3->addDescriptor(new BLE2902());
  pFruit3->setCallbacks(new Fruit3CB());
  pFruit3->setValue("F3");
  // Start the service
  pService->start();
  pService2->start();
  pService3->start();
  pService4->start();
  pService5->start();


  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->addServiceUUID(SERVICE2_UUID);
        pAdvertising->addServiceUUID(SERVICE3_UUID);
    pAdvertising->addServiceUUID(SERVICE4_UUID);

    pAdvertising->addServiceUUID(SERVICE5_UUID);


  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
   digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
}

void loop() {
    // notify changed value
    if (deviceConnected) {
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
