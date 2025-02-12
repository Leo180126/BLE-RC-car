#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// Define pin
#define motorRight_1 25
#define motorRight_2 26

#define motorLeft_1 18
#define motorLeft_2 19

#define motorLeft_Speed 15
#define motorRight_Speed 14

//Speed
int speed = 120 ;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string lenh;




#define SERVICE_UUID        "4a4bcb3c-7dc4-48f0-bfd5-043ea74c8bf6"
#define CHAR1_UUID          "4b28dc88-3d42-4579-9b58-96610b4b8a7e"
#define CHAR2_UUID          "ec6a5220-90b5-4f3c-a8d7-38fb0dff4272"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class CharacteristicCallBack: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override { 
    lenh = pChar->getValue();
  }
};

class Characteristic_2_CallBack: public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *pChar) override {
    speed = stoi(pChar->getValue());
    Serial.println(speed);
  }
};

class Motor{
  public:
    Motor(int pin_1, int pin_2, int speed_pin);
    void tien(int speed);
    void lui(int speed);
    void brake();
    void standby();
  private:
    int pin_1;
    int pin_2;
    int speed_pin;
};


void setup() {
  Serial.begin(115200);
  //Set pin
  pinMode(motorLeft_1, OUTPUT);
  pinMode(motorRight_1, OUTPUT);
  pinMode(motorLeft_2, OUTPUT);
  pinMode(motorRight_2, OUTPUT);
  //Init bluetooth

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHAR1_UUID,
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_READ
                    );                    
  pCharacteristic_2 = pService->createCharacteristic(
                      CHAR2_UUID,
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_READ
                    );
  // Add all Descriptors here
  
  // Set the callback functions
  pCharacteristic->setCallbacks(new CharacteristicCallBack());
  pCharacteristic_2->setCallbacks(new Characteristic_2_CallBack());
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to ...");

}
  Motor motorLeft(motorLeft_1, motorLeft_2, motorLeft_Speed), motorRight(motorRight_1, motorRight_2, motorRight_Speed);
void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); 
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    
    oldDeviceConnected = deviceConnected;
  }
  if(deviceConnected){
    // if(speed_command[0] >= '0' && speed_command[0] <= '9'){
    //   speed = map(lenh[0] - '0', 0, 10, 120, 255); 
    // }
    // else if(lenh[0] == 'q') speed = 255;
    switch (lenh[0])
    {
    case 'F':
      Serial.println("Tien");
      motorRight.tien(speed);
      motorLeft.tien(speed);
      break;
    case 'B':
      Serial.println("Lui");
      motorLeft.lui(speed);
      motorRight.lui(speed);
      break;
    case 'L':
      Serial.println("Quay trai");
      motorLeft.lui(speed);
      motorRight.tien(speed);
      break;
    case 'R':
      Serial.println("Quay phai");
      motorLeft.tien(speed);
      motorRight.lui(speed);
      break;
    // case 'D':
    //   motorLeft.brake();
    //   motorRight.brake();
    //   break;
    default:
      motorLeft.standby();
      motorRight.standby();
      break;
    }
  }
}

Motor::Motor(int pin_1, int pin_2, int speed_pin)
{
  this->pin_1 = pin_1;
  this->pin_2 = pin_2;
  this->speed_pin = speed_pin;
}

void Motor::tien(int speed)
{
  analogWrite(speed_pin, speed);
  digitalWrite(pin_1, HIGH);
  digitalWrite(pin_2, LOW);
}

void Motor::lui(int speed)
{
  analogWrite(speed_pin, speed);
  digitalWrite(pin_2, HIGH);
  digitalWrite(pin_1, LOW);
}

void Motor::brake()
{
  analogWrite(speed_pin, 255);
  digitalWrite(pin_2, HIGH);
  digitalWrite(pin_1, HIGH);
}

void Motor::standby()
{
  digitalWrite(pin_1, LOW);
  digitalWrite(pin_2, LOW);
}
