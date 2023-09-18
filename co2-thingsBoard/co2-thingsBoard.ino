//#include <WiFi101.h>  // MKR1000 
#include <WiFiNINA.h> // MKR1010
#include <ThingsBoard.h> // Needs ArduinoHttpClient & PubSubClient & ArduinoJson
#include <SCD30.h> // CO2 Sensor + Temp + Humidity

/* Configuration options: */
   
//#define THINGSBOARD_SERVER "thingsboard.cloud"
#define THINGSBOARD_SERVER "nova-aps.it"
    
#define THINGSBOARD_ACCESS_TOKEN "..."

// Enable serial monitor prints for debugging
#define DEBUG 1

char ssid[] = "...";    // Nome rete Wi-Fi
char pass[] = "...";   // Password rete Wi-Fi


/* You should not need to edit under this line */

int status = WL_IDLE_STATUS;

WiFiClient client;
ThingsBoard tb(client);


void setup() {
  
  SerialUSB.begin(115200);   

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  delay(10000);
 
  /* Sensor initialization */
  SerialUSB.print("Initialize CO2 sensor... ");
  scd30.initialize();
  SerialUSB.println("Done.");
  
}

void loop() {

  if (DEBUG) SerialUSB.println("--- New loop iteration ---");

  /****************************************
   * WI-FI CONNECTION
   ****************************************/
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);   // Connessione fallita: spengo il led
    SerialUSB.print("Attempting WiFi connection to network: ");
    SerialUSB.print(ssid);
    status = WiFi.begin(ssid, pass);
    
    /* Se non riesco a connettermi, attendo 5 secondi e riprovo */
    if (status == WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, HIGH);  // Connessione ok: accendo il led
      SerialUSB.println(" -> Connected!");
    } else {
      digitalWrite(LED_BUILTIN, LOW);   // Connessione fallita: spengo il led
      SerialUSB.println(" ...");
    }
    
    delay(5000);
  }

  /****************************************
   * MQTT BROKER "CONNCETION"
   ****************************************/
  if(!tb.connected()){
    
    digitalWrite(LED_BUILTIN, LOW);   // Connessione fallita: spengo il led   
    SerialUSB.print("Attempting MQTT connection to: ");
    SerialUSB.print(THINGSBOARD_SERVER);
    //tb.connect("thingsboard.cloud", THINGSBOARD_ACCESS_TOKEN);
    //tb.connect("65.108.244.100", THINGSBOARD_ACCESS_TOKEN, 8080);
    //tb.connect("nova-aps.it", THINGSBOARD_ACCESS_TOKEN);
    tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN);
      
    if(tb.connected()) {
      digitalWrite(LED_BUILTIN, HIGH);  // Connessione ok: accendo il led
      SerialUSB.println(" -> Connected!");
    } else {
      digitalWrite(LED_BUILTIN, LOW);   // Connessione fallita: spengo il led
      SerialUSB.println(" ...");
    }

   delay(5000);
   
  }
  
  /****************************************
   * Reading CO2, Temperature, & Humidity
   ***************************************/
  
  if(tb.connected()) {
    
    float result[3] = {0};
  
    float co2 = 0;
    float humidity = 0;
    float temperature = 0;

    if (DEBUG) SerialUSB.print("Reading sensors... ");
 
    scd30.getCarbonDioxideConcentration(result);

    if (DEBUG) SerialUSB.println(" Done.");

    co2 = result[0];
    temperature = result[1];
    humidity = result[2];

    if (DEBUG) {
      SerialUSB.print("CO2: ");
      SerialUSB.print(co2);
      SerialUSB.println("ppm");
    
      SerialUSB.print("Temp: ");
      SerialUSB.print(temperature);
      SerialUSB.println("C");
    
      SerialUSB.print("Humidity: ");
      SerialUSB.print(humidity);
      SerialUSB.println("%");
    }

  /****************************************
   * Sending telemetry
   ***************************************/

    if (DEBUG) SerialUSB.print("Sending telemetry...");
  
    tb.sendTelemetryData("co2", co2);
    tb.sendTelemetryData("humidity", humidity);
    tb.sendTelemetryData("temp", temperature);

    if (DEBUG) SerialUSB.println(" Done.");

  }

  /***************************************
   * Waiting for next loop iteration
   ***************************************/

  tb.loop();
  
  delay(5000);
  
}
