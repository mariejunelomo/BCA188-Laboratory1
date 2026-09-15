#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===============================
// BLE UUIDs
// ===============================
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// ===============================
// BLE Server Callbacks
// ===============================
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("BLE device connected!");
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("BLE device disconnected!");

    // Start advertising again
    pServer->getAdvertising()->start();
    Serial.println("Waiting for BLE connection...");
  }
};

// ===============================
// BLE Receive Callbacks
// ===============================
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

    String message = pCharacteristic->getValue();

    if (message.length() > 0) {

      Serial.print("Received from phone: ");
      Serial.println(message);

      // Send reply back to phone
      if (deviceConnected) {
        String reply = "ESP32-S3: Message received - " + message;

        pCharacteristic->setValue(reply.c_str());
        pCharacteristic->notify();

        Serial.print("Sent to phone: ");
        Serial.println(reply);
      }
    }
  }
};

// ===============================
// SETUP
// ===============================
void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP32-S3 BLE Communication");
  Serial.println("==============================");

  // Initialize BLE
  BLEDevice::init("ESP32-S3");

  // Create BLE server
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE service
  BLEService *pService =
      pServer->createService(SERVICE_UUID);

  // Create BLE characteristic
  pCharacteristic =
      pService->createCharacteristic(
          CHARACTERISTIC_UUID,
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY
      );

  // Enable notifications
  pCharacteristic->addDescriptor(
      new BLE2902()
  );

  // Set receive callback
  pCharacteristic->setCallbacks(
      new MyCallbacks()
  );

  // Start BLE service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising =
      BLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(
      SERVICE_UUID
  );

  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("BLE is ready!");
  Serial.println("Device name: ESP32-S3");
  Serial.println("Waiting for phone connection...");
}

// ===============================
// LOOP
// ===============================
void loop() {

  if (deviceConnected) {
    // BLE communication is handled
    // by the callbacks.
  }

  delay(100);
}