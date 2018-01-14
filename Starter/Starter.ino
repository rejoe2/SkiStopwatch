/**
  Starter Node for Ski race Stopwatch
*/

// Enable debug prints to serial monitor
#define MY_DEBUG
//#define MY_DEBUG_LOCAL

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_PA_LEVEL RF24_PA_MAX
#define MY_RF24_CHANNEL 90 //120
//#define MY_RADIO_RFM69

// Enabled repeater feature for this node
//#define MY_REPEATER_FEATURE
#define MY_NODE_ID 10
#define MY_PARENT_NODE_ID 11
#define MY_PARENT_NODE_IS_STATIC
#define MY_SISTER_NODE_ID 11
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS 1
#include <MySensors.h>
#include <SPI.h>
unsigned long PING_RX_MAX_TIME = 12000; // Max. time between ping signals from finish Node (in milliseconds)
unsigned long PING_TX_TIME = 5000; //send "alive" signal periodically

MyMessage PingMsg(0, V_STATUS);
MyMessage PongMsg(0, V_TRIPPED);

#define DIGITAL_INPUT_SENSOR 3  // The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define SENSOR_INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define CHILD_ID_STARTER 10              // Id of the sensor child

#define ALIVE_PIN  13  // Arduino Digital I/O pin number for signalising  
#define CHILD_ID_STATUS 100   // Id of the sensor child
#define RELAY_ON 1
#define RELAY_OFF 0

boolean Sister_Received = false;
unsigned long lastSend,lastReceive;

void before() {

  // initialize our digital pins internal pullup resistor so one pulse switches from high to low (less distortion)
  pinMode(DIGITAL_INPUT_SENSOR, INPUT_PULLUP);
  digitalWrite(DIGITAL_INPUT_SENSOR, HIGH);
  attachInterrupt(SENSOR_INTERRUPT, onPulse, FALLING);

  // Then set relay pins in output mode
  pinMode(ALIVE_PIN, OUTPUT);
  // Switch off when starting up
  digitalWrite(ALIVE_PIN, RELAY_OFF);
}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Starter Node", "0.01");
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_STATUS, S_LIGHT);
}

void setup() {
  lastSend = lastReceive = millis();
}

void loop()
{
  unsigned long currentTime = millis();
  if (currentTime - lastSend > PING_TX_TIME) {
    send(PingMsg.setDestination(MY_SISTER_NODE_ID).setSensor(CHILD_ID_STATUS).set(1));
    lastSend = currentTime;
  }
  if (digitalRead(ALIVE_PIN) == RELAY_ON && currentTime - lastReceive > PING_RX_MAX_TIME) {
    digitalWrite(ALIVE_PIN,RELAY_OFF);
  }  
}

void receive(const MyMessage & message) {
  if (message.sensor == CHILD_ID_STATUS) {
    //if (message.type == V_LIGHT) {
      // Change relay state
      bool state = message.getBool();
      if(digitalRead(ALIVE_PIN) == RELAY_ON) {
        digitalWrite(ALIVE_PIN, state ? RELAY_ON : RELAY_OFF);
      }
      lastReceive = millis();
#ifdef MY_DEBUG
      // Write some debug info
      Serial.print("Gw change relay:");
      Serial.print(message.sensor);
      Serial.print(", New status: ");
      Serial.println(message.getBool());
#endif
    //}
  }
}

void onPulse()
{
}


