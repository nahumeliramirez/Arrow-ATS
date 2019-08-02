/*
  Arduino7 Yún 
  Ubicacion carro +E parte izquierda
  MAC: C4:93:00:04:32:59
  IP: 192.168.0.89

  This example for the YunShield/Yún shows how 
  to use the Bridge library to access the digital and
  analog pins on the board through REST calls.
  It demonstrates how you can create your own API when
  using REST style calls through the browser.

  Possible commands created in this shetch:

  "/arduino/digital/13"     -> digitalRead(13)
  "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
  "/arduino/analog/2/123"   -> analogWrite(2, 123)
  "/arduino/analog/2"       -> analogRead(2)
  "/arduino/mode/13/input"  -> pinMode(13, INPUT)
  "/arduino/mode/13/output" -> pinMode(13, OUTPUT)

*/

#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <HttpClient.h>


// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
BridgeServer server;
int i=0;
    String url ="http://192.168.0.98/api/arduino7/";
    bool O32;
bool O33;
bool O40;
bool O41;
bool I98;
bool I99;
bool I100;
bool I101;
bool I102;
bool I103;
bool I104;
bool I105;
bool O97;
bool O98;
bool O99;
bool O100;
bool O101;
bool O102;
bool O106;
bool I160;
bool I161;
bool I164;
bool O160;
bool O161;
void setup() {
    

  // Bridge startup        Rele    Cable
  pinMode(24, INPUT); //<-      <- CN115
  pinMode(25, INPUT); //<-      <- CU115
  pinMode(26, INPUT); //<-      <- CU114
  pinMode(10, INPUT); //<-      <- SA124
  pinMode(11, INPUT); //<-      <- SA125
  pinMode(12, INPUT); //<-      <- SA135
  pinMode(14, INPUT); //<-      <- SA136
  pinMode(15, INPUT); //<-      <- SC124
  pinMode(16, INPUT); //<-      <- SC125
  pinMode(17, INPUT); //<-      <- SC126
  pinMode(18, INPUT); //<-      <- CN114
  pinMode(28, INPUT); //<- 6R38 <- CN121
  pinMode(23, INPUT); //<- 4R38 <- SA122
  pinMode(27,OUtPUT); //-> 7R38 -> CN122    
  pinMode(22, INPUT); //<- 5R38 <- SA121
  pinMode(2,  INPUT); //<- 8R40 <- CU112
  pinMode(19, INPUT); //<- 8R38 <- CU111
  pinMode(3, OUTPUT); //->      -> SA132
  pinMode(4, OUTPUT); //->      -> SC122
  pinMode(5, OUTPUT); //->      -> SC121
  pinMode(6, OUTPUT); //->      -> ST19
  pinMode(7, OUTPUT); //->      -> CN111
  pinMode(8, OUTPUT); //->      -> SA131
  pinMode(9, OUTPUT); //->      -> SA134
  Bridge.begin();
  

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}
void loop() {
    url ="http://192.168.0.98/api/arduino7/";
    O32=digitalRead(22);
O33=digitalRead(27);
O40=digitalRead(19);
O41=digitalRead(28);
I98=digitalRead(10);
I99=digitalRead(11);
I100=digitalRead(12);
I101=digitalRead(14);
I102=digitalRead(26);
I103=digitalRead(25);
I104=digitalRead(18);
I105=digitalRead(24);
O97=digitalRead(8);
O98=digitalRead(3);
O99=digitalRead(6);
O100=digitalRead(7);
O101=digitalRead(2);
O102=digitalRead(23);
O106=digitalRead(9);
I160=digitalRead(15);
I161=digitalRead(16);
I164=digitalRead(17);
O160=digitalRead(5);
O161=digitalRead(4);
    i++;
    if (i>=25){
        url = url+O32+"/"+O33+"/"+O40+"/"+O41+"/"+I98+"/"+I99+"/"+I100+"/"+I101+"/"+I102+"/"+I103+"/"+I104+"/"+I105+"/"+O97+"/"+O98+"/"+O99+"/"+O100+"/"+O101+"/"+O102+"/"+O106+"/"+I160+"/"+I161+"/"+I164+"/"+O160+"/"+O161;
        HttpClient client;
        client.getAsynchronously(url);
        i=0;
    }


  // Get clients coming from server
  BridgeClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

void process(BridgeClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "digital" command?
  if (command == "digital") {
    digitalCommand(client);
  }

  // is "analog" command?
  if (command == "analog") {
    analogCommand(client);
  }

  // is "mode" command?
  if (command == "mode") {
    modeCommand(client);
  }
}

void digitalCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  } else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  } else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(BridgeClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}

