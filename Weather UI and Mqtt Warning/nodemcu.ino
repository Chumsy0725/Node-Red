/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Chumssy";
const char* password = "Chumsy@01";


//details about mqtt broker and the topics you are going to use
const char* mqtt_server = "broker.mqttdashboard.com"; //using the mosquito broker
const char* outTopic = "ENTC/EN2560/out/000001"; //we have to upload these to the mqtt client in the computer
const char* inTopic = "EN2560/in/G14";

WiFiClient espClient; //WifiClient Object to handle the wifi connection.(protocol stack)
PubSubClient client(espClient); //PubSubClient Object (MQTT protocl stack) runs on the WiFi Client object.

//Buffer to store the outgoing msg
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
//  starts a pseudo rando generator, causing it
//  to start at an arbitrary point in its random
//  sequence. This sequence, while very long, and
//  random, is always the same. 
//  Later in the code wee need a random sequence

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) { //we use a callback function to do tasks after an event occurs. This function runs 
// when you recieve a msg to the subscribed topic
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH); 
  } 
//  else {
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//  }

}

void reconnect() { 
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
//    we need a unique client id to connect to a
//    MQTT broker. To avoid collisions client ID is
//    randomized each time connections reset
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      subscribing to the inbound topic and
//      publishing to the outbound topic
      client.publish(outTopic, "hello world");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi(); //setting up wifi connection
  client.setServer(mqtt_server, 1883); //setting up mqtt function
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 
//  this function handles incoming msges from the subscribed topic and
//  calls the callback function. If no data is recieved after a timeout,
//  function returns

  unsigned long now = millis();
  String income = "";
  if (now - lastMsg > 2000 && Serial.available() > 0) { //delay of 2 seconds
    income = Serial.readString();
    char * msg = strcpy(new char[income.length() + 1], income.c_str());
    lastMsg = now;
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(outTopic, msg);
  }
}
