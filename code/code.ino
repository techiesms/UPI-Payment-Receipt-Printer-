/***********************************************************************
  This is the code for printing Payment Receits for RazorPay Payment Gateway

  About this Project:
  __________________


  It uses Pipedream for webhooks URl and later it sends the data to Adafruit IO
  and this code recevies the data from Adafruit IO and Prints it using thermal printer



  Libraries & it's Versions
  _________________________

  > Adafruit MQTT (2.5.4) - https://github.com/adafruit/Adafruit_MQTT_Library
  > Adafruit Thermal Printer (1.4.1) - https://github.com/adafruit/Adafruit-Thermal-Printer-Library


 **********************************************************************/


//********************************* Necesaary Library Decalration
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Adafruit_Thermal.h"

// Adding Thermal Printer to Hardware Serial 2 of ESP32
Adafruit_Thermal printer(&Serial2);


//********************************* Credentials that you need to Change

// WiFi Credentials

#define WLAN_SSID "SSID"
#define WLAN_PASS "PASS"

// Adafriut IO Credentials

#define AIO_SERVER          "io.adafruit.com"
#define AIO_SERVERPORT      1883
#define AIO_USERNAME        "USERNAME"
#define AIO_KEY             "ACTIVE KEY"
#define FeedName            "amount"

/************ Global State (you don't need to change this!) ******************/

WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe amount = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/" FeedName);


/*************************** Sketch Code ************************************/

void setup()
{
  Serial.begin(115200); // Beginning Serial Monitor
  Serial2.begin(9600); // Beginning Thermal Printer


  printer.setFont('A'); // Setting Font Type
  printer.setSize('S'); // Setting Font Size
  printer.justify('L'); // Setting Allignment
  printer.begin();  // Init printer (same regardless of serial type)
  printer.setFont('A');
  printer.setSize('S');
  printer.justify('L');
  printer.println();
  delay(10);

  Serial.println(F("RazorPay Payment Receipt Example Code"));

  // Connect to WiFi access point.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  delay(1000);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(2000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mqtt.subscribe(&amount); // Subscribing to MQTT Topic
}



void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  String final_payment;
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    // Check if its the onoff button feed
    if (subscription == &amount)
    {
      Serial.print(F("amount: "));
      Serial.println((char *)amount.lastread);
      int payment = atoi((char *)amount.lastread);

      // Converting Data recevied in Actual Amount Value ( For Eg. Converting 100 to 1.00 )

      int R = payment / 100;
      int P = payment % 100;
      String myR;
      myR = String(R);  //Converts integer to string
      String myP;
      myP = String(P);  //Converts integer to string
      if (P == 0) {
        final_payment = myR;
      } else {
        final_payment = myR + "." + myP;
      }


      //********************************* Printing Receipt using Thermal Printer

      printer.setFont('B');
      printer.setSize('L');
      printer.justify('C');
      printer.println("--------------");
      //-----------------------------------------
      printer.justify('C');
      printer.setSize('S');
      printer.setFont('A');
      printer.boldOn();
      printer.println(F("*** Payment Received ***"));
      printer.boldOff();
      printer.println();

      printer.setFont('A');
      printer.setSize('S');
      printer.justify('L');
      printer.print(F("Amount : Rs."));
      printer.underlineOn();
      printer.print(final_payment);
      printer.println("/-");
      printer.underlineOff();
      printer.print("Status : ");
      printer.println("success");

      //--------------------------------------------
      printer.setSize('S');
      printer.setFont('A');
      printer.justify('C');      // Set type size, accepts 'S', 'M', 'L'
      printer.println(F("_________________"));// Font options
      //----------------------------------------------
      printer.boldOn();
      printer.println(F("*Thank You*"));
      printer.boldOff();
      //---------------------------------------------
      printer.setSize('S');
      printer.setFont('A');
      printer.justify('C');      // Set type size, accepts 'S', 'M', 'L'
      printer.println(F("_________________"));// Font options
      //-----------------------------------------------
      printer.setSize('S');
      printer.justify('C');
      printer.inverseOn();
      printer.println(F("Techiesms"));
      printer.inverseOff();
      //-----------------------------------------------
      printer.setSize('L');
      printer.setFont('B');
      printer.justify('C');
      printer.println("---------------");
      printer.println();
      printer.println();
    }
  }
}



// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {  // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }

  Serial.println("MQTT Connected!");
}
