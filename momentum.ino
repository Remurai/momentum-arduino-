#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Delcare wifi connection
#define SSID ""
#define PASS ""

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 //Use 8883 for SSL

#define AIO_USERNAME "Remurai"
#define AIO_KEY "aio_nLgV95CKpjsSuz2UTdqr6yT884AE"

//Declare Ultrasonic Sensor
const int TRIG = D5;
const int ECHO = D6;
long duration;

//Declare baud rate
long BR = 9600;

//Declare variables for momentum
double distance[2];
double velo, momentum;
const double mass = 40;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Publish in MQTT protocol means write operation
Adafruit_MQTT_Publish highMomentum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/momentum.highmomentum");
Adafruit_MQTT_Publish medMomentum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/momentum.medmomentum");
Adafruit_MQTT_Publish lowMomentum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/momentum.lowmomentum");

void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  Serial.begin(BR);
  delay(10);
  
  Serial.println("Adafruit MQTT");
  WiFi.begin(SSID, PASS);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi connected.");
  Serial.println();
}

void loop() { 
  MQTT_connect();
  getMomentum();
  if (!mqtt.ping()) mqtt.disconnect();  
}

//Get Ultrasonic value
void getSonic(){
  //Set a period for an ultrasonic wave.
  digitalWrite(TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  //Time duration of the feedback wave
  duration = pulseIn(ECHO, HIGH);
}

//Get momentum by formula of mass*((initial distance - final distance) per second)
void getMomentum(){
  for (int i=0; i<2; i++){
    getSonic();
    
    //Distance
    distance[i] = duration * 0.00017;
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(distance[i]);
    Serial.print("m");
    Serial.print("\t");

    if (i == 0){
      velo = distance[i+1]-distance[i];
    }
    else{
      velo = distance[i-1]-distance[i];
    }

    //Turn negative value to positive value
    if (velo < 0) velo = velo*-1;

    //Velocity
    Serial.print("Velocity: ");
    Serial.print(velo);
    Serial.print(" m/s");
    Serial.print("\t");

    //Momentum
    momentum = mass * velo;
    Serial.print("Momentum: ");
    Serial.print(momentum);
    Serial.println(" kgm/s");
    
    MQTT_write()
    delay(3000);
  }
}

// Write to Adafruit MQTT server
void MQTT_write() {
  Serial.print("\nSend momentum value: ");
  Serial.print(momentum);

  //Determine which momentum write to low, med or high
  if (momentum <= 3){
    lowMomentum.publish(momentum);
  }
  else if (momentum <= 7){
    medMomentum.publish(momentum);
  }
  else{
    highMomentum.publish(momentum);
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect(){
  int8_t ret;

  //Stop if connected
  if(mqtt.connected()) return;

  Serial.print("Connecting to MQTT...");

  uint8_t retries = 3;
  while((ret = mqtt.connect()) != 0){ // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(10000); // wait 10 seconds
    retries--;
    if(retries == 0)
    while (1); // basically die and wait for WDT to reset me
    }
    Serial.println("MQTT Connected!");
}
