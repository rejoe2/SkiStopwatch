/**
  Finish Node for Ski race Stopwatch
*/

// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_SPLASH_SCREEN_DISABLED
//#define MY_DEBUG_LOCAL

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_PA_LEVEL RF24_PA_MAX
#define MY_RF24_CHANNEL 90  //120
//#define MY_RADIO_RFM69

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE
#define MY_NODE_ID 11
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
#define MY_SISTER_NODE_ID 10
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED 
#define MY_TRANSPORT_WAIT_READY_MS 1
#include <MySensors.h>
#include <SPI.h>
unsigned long PING_RX_MAX_TIME = 12000; // Max. time between ping signals from finish Node (in milliseconds)
unsigned long PING_TX_TIME = 5000; //send "alive" signal periodically

MyMessage PingMsg(0, V_TRIPPED);
MyMessage ResetMsg(0, V_TRIPPED);

#define DIGITAL_INPUT_SENSOR 3  // The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define SENSOR_INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define CHILD_ID_STARTER 10              // Id of the sensor child

#define CONNECTION_LED  A0  // Arduino Digital I/O pin number for signalising  
#define CHILD_ID_STATUS 100   // Id of the sensor child
#define LED_ON 1
#define LED_OFF 0

#define READY_LED A1
#define RUNNING_LED A2

#define FIRST_BUTTON_ID 2
#define MAX_BUTTON 3
const uint8_t buttonPin[] = {4, 5, 6};   //  switch around pins to your desire
Bounce debouncer[MAX_BUTTON];
MyMessage buttonMsg(0, V_TRIPPED);
bool oldButton[MAX_BUTTON] = {false};

#define CHILD_ID_RESET 99   // Id of the sensor child

bool Sister_Received = false;
bool request_Reset = false;
bool is_Running = false;

volatile long startTime = 0;
volatile float RaceTime[4];
volatile bool displayNeedsUpdate = false;

void before() {

  // initialize our digital pins internal pullup resistor so one pulse switches from high to low (less distortion)
  pinMode(DIGITAL_INPUT_SENSOR, INPUT_PULLUP);
  digitalWrite(DIGITAL_INPUT_SENSOR, HIGH);
  attachInterrupt(SENSOR_INTERRUPT, onFinish, FALLING);

  // Then set led pins in output mode
  pinMode(CONNECTION_LED, OUTPUT);
  pinMode(READY_LED, OUTPUT);
  pinMode(RUNNING_LED, OUTPUT);
  
  // Switch off when starting up
  digitalWrite(CONNECTION_LED, LED_OFF);
  digitalWrite(READY_LED, LED_OFF);
  digitalWrite(RUNNING_LED, LED_OFF);
  for (uint8_t i = 0; i < MAX_BUTTON; i++) {
    debouncer[i] = Bounce();                        // initialize debouncer
    debouncer[i].attach(buttonPin[i], INPUT_PULLUP);
    debouncer[i].interval(5);
  }
}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Finish Node", "0.01");
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_STATUS, S_LIGHT);
}

void setup() {
  lastSend = lastReceive = millis();
}

void loop()
{
  bool button[MAX_BUTTON];
  bool bounceUpdate[MAX_BUTTON] = {false}; //true, if button pressed
  for (uint8_t i = 0; i < MAX_BUTTON; i++) {
    debouncer[i].update();
    button[i] = debouncer[i].read() == HIGH;
    if (button[i] != oldButton[i] && button[i]) {
      send(buttonMsg.setDestination(MY_SISTER_NODE_ID).setSensor(FIRST_PIR_ID + i).set( pir[i])); // Send tripped value to sister node
      if (i == 0 && button[i]) {
        digitalWrite(READY_LED, LED_OFF);
		digitalWrite(RUNNING_LED;LED_OFF);
		is_Running = false;
		send(buttonMsg.setDestination(MY_SISTER_NODE_ID).setSensor(CHILD_ID_RESET).set( pir[i])); // Send tripped value to sister node
        if (digitalRead(CONNECTION_LED) == LED_ON) {
			digitalWrite(READY_LED, LED_ON);
		}
      }
      oldPir[i] = pir[i];
    }
  }

  
  unsigned long currentTime = millis();
  if (currentTime - lastSend > PING_TX_TIME) {
    send(PingMsg.setDestination(MY_SISTER_NODE_ID).setSensor(CHILD_ID_STATUS).set(1,1));
    lastSend = currentTime;
  }
  if (digitalRead(CONNECTION_LED) == LED_ON && currentTime - lastReceive > PING_RX_MAX_TIME) {
    digitalWrite(CONNECTION_LED,LED_OFF);
  }  
  if (displayNeedsUpdate) {
	  updateDisplay();
  }
}

void receive(const MyMessage & message) {
  
  if (message.sensor == CHILD_ID_STATUS) {
    //if (message.type == V_LIGHT) {
      // Change relay state
      bool state = message.getBool();
      if(digitalRead(CONNECTION_LED) == LED_ON) {
        digitalWrite(CONNECTION_LED, state ? LED_ON : LED_OFF);
      }
      lastReceive = millis();
#ifdef MY_DEBUG
      // Write some debug info
      Serial.print(F("Child: "));
      Serial.print(message.sensor);
      Serial.print(F(", New status: "));
      Serial.println(state);
	  //}
  }
  else if(message.sensor == CHILD_ID_RESET) {
	digitalWrite(READY_LED, LED_OFF);
	digitalWrite(RUNNING_LED;LED_OFF);
	is_Running = false;
	send(buttonMsg.setDestination(MY_SISTER_NODE_ID).setSensor(CHILD_ID_RESET).set(true)); // Send tripped value to sister node
    if (digitalRead(CONNECTION_LED) == LED_ON) {
		digitalWrite(READY_LED, LED_ON);
	}
#ifdef MY_DEBUG
	// Write some debug info
	Serial.println(F("Reset done and sent"));
#endif
  }
  else if(message.sensor == CHILD_ID_STARTER) {
    if (digitalRead(CONNECTION_LED) == LED_ON) {
		startTime = millis();
		digitalWrite(READY_LED, LED_OFF);
		digitalWrite(RUNNING_LED;LED_ON);
		is_Running = true;
	}
#ifdef MY_DEBUG
	// Write some debug info
	Serial.println(F("Reset done and sent"));
#endif
  }
}

void onFinish()
{
	if	(is_Running = true;) {
		now = millis();
		RaceTime = (now - startTime)/1000;
		displayNeedsUpdate = true;
	}
}

void updateDisplay()
{
	Serial.print(F("Last run: "));
	serial.println(RaceTime);
	displayNeedsUpdate = false;
	digitalWrite(READY_LED, LED_OFF);
	digitalWrite(RUNNING_LED;LED_OFF);
	is_Running = false;
	send(buttonMsg.setDestination(MY_SISTER_NODE_ID).setSensor(CHILD_ID_RESET).set(true)); // Send tripped value to sister node
    if (digitalRead(CONNECTION_LED) == LED_ON) {
		digitalWrite(READY_LED, LED_ON);
	}
#ifdef MY_DEBUG
	// Write some debug info
	Serial.println(F("Display update done and starter reset"));
#endif
}


