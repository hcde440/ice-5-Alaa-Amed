// ********************************************* \\
//                                               \\ 
//                    ICE 5                      \\
//               Working with MQTT               \\
//                                               \\
// ********************************************* \\

// This Program receives sensor data from "topics" it's subscribed to. 
// Sender: Willa 
// Receviver: Alaa 

// Integrating Sensing, Publishing and Subscription
// This program allows users to subscribe to a topic on a server and retrieve information from said topic.
//  The data we are collecting is temperature and humidity from a DHT22 sensor published to a channel named _willa/Temp_

// --you can customize the code to suit your needs--


//Requisite Libraries . . .
#include <ESP8266WiFi.h> 
#include <SPI.h>
#include <Wire.h>  // for I2C communications
#include <Adafruit_Sensor.h>  // the generic Adafruit sensor library used with both sensors
#include <DHT.h>   // temperature and humidity sensor library
#include <DHT_U.h> // unified DHT library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPL115A2.h> // Barometric pressure sensor library
#include <PubSubClient.h>   //
#include <ArduinoJson.h>    //
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Wifi
#define wifi_ssid "University of Washington"  
#define wifi_password "" 


#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!
char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array


typedef struct { // here we create a new data type definition, a box to hold other data types
  String tem;
  String hum;

} Temp;            

Temp w;            // we have created a Temp type, but not an instance of that type,
                   // so we create the variable 'w' of type Temp
   
WiFiClient espClient;             //blah blah blah, espClient
PubSubClient mqtt(espClient);     //blah blah blah, tie PubSub (mqtt) client to WiFi client

void setup() {
  // put your setup code here, to run once:
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  // start the serial connection
  Serial.begin(115200);
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback); //register the callback function

  // wait for serial monitor to open
  while(! Serial);
}

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // Start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
} 

/////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("willa/Temp"); // we are subscribing to 'willa/Temp'
      //mqtt.subscribe("willa/Humd");
    } else {                        // If we are unable to connnect 
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////LOOP/////
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop(); //this keeps the mqtt connection 'active'

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //blah blah blah a DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!
  
  // get temp from JSON and save it as a string 
  w.tem = root["temp"].as<String>();    
  // get humd from JSON and save it as a string        
  w.hum = root["humd"].as<String>();  
  Serial.println( w.tem);
  // Crlear display 
  display.clearDisplay();
    // Normal 1:1 pixel scale
  display.setTextSize(1);
    // Start at top-left corner
  display.setCursor(0, 0);
    // Draw white text
  display.setTextColor(WHITE);
  // Display Humidity & Temperatrure on OLED
  display.println("Temp by Willa: " +w.tem);
  display.println("Humd by Willa: " +w.hum);
  display.display();
  // 1 second delay 
  delay(1000);
  
  // Handling errors 
  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }
}
