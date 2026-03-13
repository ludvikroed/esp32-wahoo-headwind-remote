#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

#define BUTTON_UP 13 // button pin number 
#define BUTTON_DOWN 14 // button pin number 

const char* headwindAddress = "Headwind mac address"; // MAC til Headwind (lowercase)
BLEScan* pBLEScan = nullptr;
BLEAdvertisedDevice* foundDevice = nullptr;
BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pChar = nullptr;
bool doConnect = false;
bool connected = false;
bool notifyActive = false;

uint8_t fanSpeed = 20;
const unsigned long debounceDelay = 250; // ms
unsigned long lastUpPress = 0;
unsigned long lastDownPress = 0;
const unsigned long notifyTimeout = 5000; // ms
unsigned long notifyRequestTime = 0;

// Notify callback
void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData, size_t length,
  bool isNotify) {
    Serial.print("Notify received: ");
    for (size_t i = 0; i < length; i++) {
      Serial.print(pData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    notifyActive = true;
}

// BLE scan callback
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.getAddress().equals(BLEAddress(headwindAddress))) {
      Serial.println("Found Headwind!");
      if (foundDevice) delete foundDevice;
      foundDevice = new BLEAdvertisedDevice(advertisedDevice);
      pBLEScan->stop();
      doConnect = true;
    }
  }
};

// Connect og setup notify
bool connectToHeadwind() {
  if (!foundDevice) return false;

  Serial.print("Connecting to Headwind at ");
  Serial.println(foundDevice->getAddress().toString().c_str());

  if (!pClient) pClient = BLEDevice::createClient();
  if (!pClient->connect(foundDevice)) {
    Serial.println("ERROR: Failed to connect to Headwind");
    return false;
  }

  Serial.println("Connected!");
  pClient->getServices();

  std::map<std::string, BLERemoteService*>* services = pClient->getServices();
  pChar = nullptr;
  for (auto const& entry : *services) {
    BLERemoteService* service = entry.second;
    BLERemoteCharacteristic* candidate = service->getCharacteristic(
      BLEUUID("a026e038-0a7d-4ab3-97fa-f1500f9feb8b"));
    if (candidate) {
      pChar = candidate;
      break;
    }
  }

  if (!pChar) {
    Serial.println("ERROR: Characteristic not found");
    pClient->disconnect();
    return false;
  }

  // Sett manual mode
  uint8_t manualCmd[2] = {0x04, 0x04};
  pChar->writeValue(manualCmd, 2);
  Serial.println("Set Headwind to MANUAL mode");

  // Registrer notify
  if (!pChar->canNotify()) {
    Serial.println("Warning: Characteristic cannot notify!");
  } else {
    pChar->registerForNotify(notifyCallback);
    notifyActive = false;
    notifyRequestTime = millis();
    Serial.println("Notify subscription registered, waiting for first notify...");
  }

  connected = true;
  return true;
}

// Send fan speed
void sendFanSpeed() {
  if (!connected || !pClient || !pClient->isConnected() || !pChar) {
    Serial.println("Not connected, trying reconnect...");
    connectToHeadwind();
    return;
  }
  if (!notifyActive) {
    if (millis() - notifyRequestTime > notifyTimeout) {
      Serial.println("Notify not received, reconnecting...");
      connected = false;
      pClient->disconnect();
      return;
    }
    Serial.println("Waiting for notify before sending command...");
    return;
  }

  uint8_t cmd[2] = {0x02, fanSpeed};
  pChar->writeValue(cmd, 2);
  Serial.print("Command sent: 0x"); Serial.print(cmd[0], HEX);
  Serial.print(" 0x"); Serial.print(cmd[1], HEX);
  Serial.print(" | Fan speed: "); Serial.println(fanSpeed);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("Scanning for Headwind...");
  pBLEScan->start(0, nullptr, false); // scan forever
}

void loop() {
  if (doConnect && !connected) {
    if (connectToHeadwind()) {
      Serial.println("Headwind connected!");
    } else {
      Serial.println("Failed to connect, restarting scan...");
      if (foundDevice) delete foundDevice;
      foundDevice = nullptr;
      doConnect = false;
      pBLEScan->start(0, nullptr, false);
    }
    doConnect = false;
  }

  if (connected && (!pClient || !pClient->isConnected())) {
    Serial.println("Lost connection, restarting scan...");
    connected = false;
    notifyActive = false;
    if (foundDevice) delete foundDevice;
    foundDevice = nullptr;
    doConnect = false;
    pBLEScan->start(0, nullptr, false);
    delay(2000);
    return;
  }

  unsigned long now = millis();

  // Button for more speed (10 %)
  if (connected && digitalRead(BUTTON_UP) == LOW && now - lastUpPress > debounceDelay) {
    fanSpeed = min(fanSpeed + 10, 100);
    Serial.println("Button UP pressed");
    sendFanSpeed();
    lastUpPress = now;
  }

  // Button for less speed (10 %)
  if (connected && digitalRead(BUTTON_DOWN) == LOW && now - lastDownPress > debounceDelay) {
    fanSpeed = max(fanSpeed - 10, 0);
    Serial.println("Button DOWN pressed");
    sendFanSpeed();
    lastDownPress = now;
  }
}
