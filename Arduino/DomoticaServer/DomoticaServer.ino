// Arduino Domotica server with Klik-Aan-Klik-Uit-controller
//
// By Sibbele Oosterhaven, Computer Science NHL, Leeuwarden
// V1.2, 16/12/2016, published on BB. Works with Xamarin (App: Domotica)
//
// Hardware: Arduino Uno, Ethernet shield W5100; RF transmitter on RFpin; debug LED for serverconnection on ledPin
// The Ethernet shield uses pin 10, 11, 12 and 13
// Use Ethernet2.h libary with the (new) Ethernet board, model 2
// IP address of server is based on DHCP. No fallback to static IP; use a wireless router
// Arduino server and smartphone should be in the same network segment (192.168.1.x)
//
// Supported kaku-devices
// https://eeo.tweakblogs.net/blog/11058/action-klik-aan-klik-uit-modulen (model left)
// kaku Action device, old model (with dipswitches); system code = 31, device = 'A'
// system code = 31, device = 'A' true/false
// system code = 31, device = 'B' true/false
//
// // https://eeo.tweakblogs.net/blog/11058/action-klik-aan-klik-uit-modulen (model right)
// Based on https://github.com/evothings/evothings-examples/blob/master/resources/arduino/arduinoethernet/arduinoethernet.ino.
// kaku, Action, new model, codes based on Arduino -> Voorbeelden -> RCsw-2-> ReceiveDemo_Simple
//   on      off
// 1 2210415 2210414   replace with your own codes
// 2 2210413 2210412
// 3 2210411 2210410
// 4 2210407 2210406
//
// https://github.com/hjgode/homewatch/blob/master/arduino/libraries/NewRemoteSwitch/README.TXT
// kaku, Gamma, APA3, codes based on Arduino -> Voorbeelden -> NewRemoteSwitch -> ShowReceivedCode
// 1 Addr 21177114 unit 0 on/off, period: 270us   replace with your own code
// 2 Addr 21177114 unit 1 on/off, period: 270us
// 3 Addr 21177114 unit 2 on/off, period: 270us

// Supported KaKu devices -> find, download en install corresponding libraries
#define unitCodeApa3      2030592  // replace with your own code
#define unitCodeActionOld 31        // replace with your own code
#define unitCodeActionNew 2210406   // replace with your own code

// Include files.
#include <SPI.h>                  // Ethernet shield uses SPI-interface
#include <Ethernet.h>             // Ethernet library (use Ethernet2.h for new ethernet shield v2)
#include <NewRemoteTransmitter.h> // Remote Control, Gamma, APA3
#include <RemoteTransmitter.h>    // Remote Control, Action, old model
//#include <RCSwitch.h>           // Remote Control, Action, new model

// Set Ethernet Shield MAC address  (check yours)
byte mac[] = { 0x40, 0x6c, 0x8f, 0x36, 0x84, 0x8a }; // Ethernet adapter shield S. Oosterhaven
int ethPort = 3300;                                  // Take a free port (check your router)

#define RFPin        3  // output, pin to control the RF-sender (and Click-On Click-Off-device)
#define buzzer       6  // output, always HIGH
#define switchPin    7  // input, connected to some kind of inputswitch
#define ledPin       8  // output, led used for "connect state": blinking = searching; continuously = connected
#define infoPin      9  // output, more information
#define analogPin0   0  // sensor value
#define analogPin1   1

EthernetServer server(ethPort);              // EthernetServer instance (listening on port <ethPort>).
NewRemoteTransmitter apa3Transmitter(unitCodeApa3, RFPin, 260, 3);  // APA3 (Gamma) remote, use pin <RFPin>
ActionTransmitter actionTransmitter(RFPin);  // Remote Control, Action, old model (Impulse), use pin <RFPin>
//RCSwitch mySwitch = RCSwitch();            // Remote Control, Action, new model (on-off), use pin <RFPin>

char actionDevice = 'A';                 // Variable to store Action Device id ('A', 'B', 'C')
bool pinState = false;                   // Variable to store actual pin state
bool switch1 = false;
bool switch2 = false;
bool switch3 = false;
bool pinChange = false;                  // Variable to store actual pin change
int sensor1Value = 0;                    // Variable to store actual sensor value
int sensor2Value = 0;
int changedPin;
int duration;
int distanc;
int oldDistanc;
int trigPin = 4;
int echoPin = 5;

void setup()
{
  Serial.begin(9600);
  //while (!Serial) { ; }               // Wait for serial port to connect. Needed for Leonardo only.

  Serial.println("Domotica project, Arduino Domotica Server\n");

  //Init I/O-pins
  pinMode(switchPin, INPUT);            // hardware switch, for changing pin state
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(RFPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(infoPin, OUTPUT);

  //Default states
  digitalWrite(switchPin, HIGH);        // Activate pullup resistors (needed for input pin)
  digitalWrite(RFPin, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(infoPin, LOW);

  digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      oldDistanc = duration*0.034/2; 

  //Try to get an IP address from the DHCP server.
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Could not obtain IP-address from DHCP -> do nothing");
    while (true) {    // no point in carrying on, so do nothing forevermore; check your router
    }
  }

  Serial.print("LED (for connect-state and pin-state) on pin "); Serial.println(ledPin);
  Serial.print("Input switch on pin "); Serial.println(switchPin);
  Serial.println("Ethernetboard connected (pins 10, 11, 12, 13 and SPI)");
  Serial.println("Connect to DHCP source in local network (blinking led -> waiting for connection)");

  //Start the ethernet server.
  server.begin();

  // Print IP-address and led indication of server state
  Serial.print("Listening address: ");
  Serial.print(Ethernet.localIP());

  // for hardware debug: LED indication of server state: blinking = waiting for connection
  int IPnr = getIPComputerNumber(Ethernet.localIP());   // Get computernumber in local network 192.168.1.3 -> 3)
  Serial.print(" ["); Serial.print(IPnr); Serial.print("] ");
  Serial.print("  [Testcase: telnet "); Serial.print(Ethernet.localIP()); Serial.print(" "); Serial.print(ethPort); Serial.println("]");
  signalNumber(ledPin, IPnr);
}

void loop()
{
  // Listen for incomming connection (app)
  EthernetClient ethernetClient = server.available();
  if (!ethernetClient) {
    blink(ledPin);
    return; // wait for connection and blink LED
  }

  Serial.println("Application connected");
  digitalWrite(ledPin, LOW);
  switchDefault(false, 1);
  delay(100);
  switchDefault(false, 2);
  delay(100);
  switchDefault(false, 3);
  delay(100);
  // Do what needs to be done while the socket is connected.
  while (ethernetClient.connected())
  {
    checkEvent(switchPin, pinState);          // update pin state
    sensor1Value = readSensor(0, 100);         // update sensor value
    delay(10);
    sensor1Value = readSensor(0, 100);
    delay(100);
    sensor2Value = readSensor(2, 100);
    delay(10);
    sensor2Value = readSensor(2, 100);

    // Activate pin based op pinState
    if (pinChange) {
      if (pinState) {
        digitalWrite(ledPin, HIGH);
        switchDefault(true, changedPin);
      }
      else {
        switchDefault(false, changedPin);
        digitalWrite(ledPin, LOW);
      }
      pinChange = false;
    }

    // Execute when byte is received.
    while (ethernetClient.available())
    {
      char inByte = ethernetClient.read();   // Get byte from the client.
      executeCommand(inByte);
      inByte = NULL;                         // Reset the read byte.
    }
  }
  Serial.println("Application disonnected");
}

// Choose and switch your Kaku device, state is true/false (HIGH/LOW)
void switchDefault(bool state, int unit)
{
  apa3Transmitter.sendUnit(unit, state);          // APA3 Kaku (Gamma)
  //delay(100);
  //actionTransmitter.sendSignal(unitCodeActionOld, actionDevice, state);  // Action Kaku, old model
  //delay(100);
  //mySwitch.send(2210410 + state, 24);  // tricky, false = 0, true = 1  // Action Kaku, new model
  //delay(100);
}


int DistanceChanged(int trig, int echo){
      digitalWrite(trig, LOW);
      delayMicroseconds(2);
      digitalWrite(trig, HIGH);
      delayMicroseconds(10);
      digitalWrite(trig, LOW);
      duration = pulseIn(echo, HIGH);
      distanc= duration*0.034/2; 
      if(oldDistanc - distanc > 5){
        return 1;
      }
      else{
        return 0;
      }

      
}

// Implementation of (simple) protocol between app and Arduino
// Request (from app) is single char ('a', 's', 't', 'i' etc.)
// Response (to app) is 4 chars  (not all commands demand a response)
void executeCommand(char cmd)
{
  char buf[4] = {'\0', '\0', '\0', '\0'};

  // Command protocol
  Serial.print("["); Serial.print(cmd); Serial.print("] -> ");
  switch (cmd) {
    case 'a': // Report sensor value to the app
      tone(buzzer, 8000, 1000);
      break;
    case 'b': // Report sensor value to the app
      Serial.print("Distance changed?");
      if(DistanceChanged(trigPin, echoPin)){
        server.write("bTR\n");
        tone(buzzer, 8000, 1000);
        
        Serial.println("yes");
              Serial.println(oldDistanc);
      Serial.println(distanc);
      Serial.println(oldDistanc - distanc);
      }
      if(!DistanceChanged(trigPin, echoPin)){
        server.write("bFL\n");
        Serial.println("no");
              Serial.println(oldDistanc);
      Serial.println(distanc);
      Serial.println(oldDistanc - distanc);
      }
      break;
    case 's': // Report switch state to the app
      if (pinState) {
        server.write(" ON\n");  // always send 4 chars
        Serial.println("Pin state is ON");
      }
      else {
        server.write("OFF\n");
        Serial.println("Pin state is OFF");
      }
      break;
    case 't': // Toggle state; If state is already ON then turn it OFF
      if (pinState) {
        pinState = false;
        Serial.println("Set pin state to \"OFF\"");
      }
      else {
        pinState = true;
        Serial.println("Set pin state to \"ON\"");
      }
      pinChange = true;
      break;
     case 'r':
      server.write("  1\n");
      break;
    case '1': // Toggle state; If state is already ON then turn it OFF
      changedPin = 1;
      if (switch1) {
        switch1 = false;
        switchDefault(false, 1);
        Serial.println("Switch #1 = \"OFF\"");
      }
      else {
        switch1 = true;
        switchDefault(true, 1);
        Serial.println("Switch #1 = \"ON\"");
      }
      //pinChange = true;
      break;
    case '2': // Toggle state; If state is already ON then turn it OFF
      changedPin = 2;
      if (switch2) {
        switch2 = false;
        switchDefault(false, 2);
        Serial.println("Switch #2 = \"OFF\"");
      }
      else {
        switch2 = true;
        switchDefault(true, 2);
        Serial.println("Switch #2 = \"ON\"");
      }
      //pinChange = true;
      break;
    case '3': // Toggle state; If state is already ON then turn it OFF
      changedPin = 3;
      if (switch3) {
        switch3 = false;
        switchDefault(false, 3);
        Serial.println("Switch #3 = \"OFF\"");
      }
      else {
        switch3 = true;
        switchDefault(true, 3);
        Serial.println("Switch #3 = \"ON\"");
      }
      //pinChange = true;
      break;
    case 'i':
      digitalWrite(infoPin, HIGH);
      break;
    default:
      digitalWrite(infoPin, LOW);
  }
}


// read value from pin pn, return value is mapped between 0 and mx-1
int readSensor(int pn, int mx)
{
  return map(analogRead(pn), 0, 1023, 0, mx - 1);
}

// Convert int <val> char buffer with length <len>
void intToCharBuf(int val, char buf[], int len)
{
  String s;
  s = String(val);                        // convert tot string
  if (s.length() == 1) s = "0" + s;       // prefix redundant "0"
  if (s.length() == 2) s = "0" + s;
  s = s + "\n";                           // add newline
  s.toCharArray(buf, len);                // convert string to char-buffer
}

// Check switch level and determine if an event has happend
// event: low -> high or high -> low
void checkEvent(int p, bool &state)
{
  static bool swLevel = false;       // Variable to store the switch level (Low or High)
  static bool prevswLevel = false;   // Variable to store the previous switch level

  swLevel = digitalRead(p);
  if (swLevel)
    if (prevswLevel) delay(1);
    else {
      prevswLevel = true;   // Low -> High transition
      state = true;
      pinChange = true;
    }
  else // swLevel == Low
    if (!prevswLevel) delay(1);
    else {
      prevswLevel = false;  // High -> Low transition
      state = false;
      pinChange = true;
    }
}

// blink led on pin <pn>
void blink(int pn)
{
  digitalWrite(pn, HIGH);
  delay(100);
  digitalWrite(pn, LOW);
  delay(100);
}

// Visual feedback on pin, based on IP number, used for debug only
// Blink ledpin for a short burst, then blink N times, where N is (related to) IP-number
void signalNumber(int pin, int n)
{
  int i;
  for (i = 0; i < 30; i++)
  {
    digitalWrite(pin, HIGH);
    delay(20);
    digitalWrite(pin, LOW);
    delay(20);
  }
  delay(1000);
  for (i = 0; i < n; i++)
  {
    digitalWrite(pin, HIGH);
    delay(300);
    digitalWrite(pin, LOW);
    delay(300);
  }
  delay(1000);
}

// Convert IPAddress tot String (e.g. "192.168.1.105")
String IPAddressToString(IPAddress address)
{
  return String(address[0]) + "." +
         String(address[1]) + "." +
         String(address[2]) + "." +
         String(address[3]);
}

// Returns B-class network-id: 192.168.1.3 -> 1)
int getIPClassB(IPAddress address)
{
  return address[2];
}

// Returns computernumber in local network: 192.168.1.3 -> 3)
int getIPComputerNumber(IPAddress address)
{
  return address[3];
}

// Returns computernumber in local network: 192.168.1.105 -> 5)
int getIPComputerNumberOffset(IPAddress address, int offset)
{
  return getIPComputerNumber(address) - offset;
}


