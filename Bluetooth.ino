#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "ArduinoJson.h"

#define SERVICE_UUID            (BLEUUID((uint16_t)0xFFE0)).toString()
#define CHARACTERISTIC_UUID_RX  (BLEUUID((uint16_t)0xFFE1)).toString()
#define CHARACTERISTIC_UUID_TX  (BLEUUID((uint16_t)0xFFE2)).toString()

#define BtLED_Pin 32

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;

      for(int i=0; i<8; i++) {
        ServoSetup(i);
      }

      BuzzerTone(523.25, 150); // C5
      BuzzerTone(659.25, 150); // E5
      BuzzerTone(783.99, 250); // G5

      digitalWrite(BtLED_Pin, HIGH);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      pServer->getAdvertising()->start();

      for(int i=0; i<8; i++) {
        ServoInhibit(i);
      }

      BuzzerTone(783.99, 150); // G5
      BuzzerTone(659.25, 150); // E5
      BuzzerTone(523.25, 250); // C5

      digitalWrite(BtLED_Pin, LOW);
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      for (int i = 0; i < rxValue.length(); i++) {
        if(rxValue[i]=='#') {
          if((rxValue[i+1]<='8')&&(rxValue[i+1]>='1')) {
            uint8_t ServoNumber = rxValue[i+1]-'1';
            Serial.print("ServoNum: ");
            Serial.print(ServoNumber);
            char speed[4]={0,0,0};
            if(rxValue[i+2]==',') {
              if((rxValue[i+3]!='.')&&(rxValue[i+3]!='#')) {
                speed[0]=rxValue[i+3];
                if((rxValue[i+4]!='.')&&(rxValue[i+4]!='#')) {
                  if(rxValue[i+4]=='-') {
                    speed[0]=rxValue[i+5];
                    if((rxValue[i+6]!='.')&&(rxValue[i+6]!='#')) {
                      speed[1]=rxValue[i+6];
                    }
                  }else {
                    speed[1]=rxValue[i+4];
                    if((rxValue[i+5]!='.')&&(rxValue[i+5]!='#')) {
                      speed[2]=rxValue[i+5];
                    }
                  }
                }
              }
              int16_t speedint=atoi(speed);
              Serial.print(" Speed: ");
              Serial.println(speedint);
              ServoUpdate(ServoNumber, speedint);
            }
          }
        }
      }
    }
  }
};

void BluetoothSetup(void) {
  BLEDevice::init(BLUETOOTHNAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY  |
                    BLECharacteristic::PROPERTY_READ
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();

  pinMode(BtLED_Pin, OUTPUT);
}

void txData(void) {
  StaticJsonDocument<512> jsonData;
  char output[512];
  if(SR04_ConnectFlag==1) {
    jsonData["Distance"]=distance;
  }
  if(BH1750_ConnectFlag==1) {
    jsonData["Light"]=lux;
  }
  if(AHT10_ConnectFlag==1) {
    jsonData["Temp_AHT10"]=temp1.temperature;
    jsonData["Humidity"]=humidity.relative_humidity;
  }
  if(MPU6050_ConnectFlag==1) {
    jsonData["Acc_X"]=a.acceleration.x;
    jsonData["Acc_Y"]=a.acceleration.y;
    jsonData["Acc_Z"]=a.acceleration.z;
    jsonData["Gyro_X"]=g.gyro.x;
    jsonData["Gyro_Y"]=g.gyro.y;
    jsonData["Gyro_Z"]=g.gyro.z;
    jsonData["Temp_MPU6050"]=temp2.temperature;
  }
  jsonData["Touch0"]=Touch0_Flag;
  jsonData["Touch1"]=Touch1_Flag;

  serializeJson(jsonData, output);
  Serial.println((char*)output);
  pTxCharacteristic->setValue((char*)output);
  pTxCharacteristic->notify();
}
