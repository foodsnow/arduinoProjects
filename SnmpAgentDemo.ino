// Before compiling its necessary to have Agentuino, Flash libraries

//Due to its slimness it comes with these limitations:
//  1.SnmpAgent only responds to SNMP Get Requests (no traps or walks)
//  2.Only one OID per request is allowed
//  3.It is compleatly read-only, so no SNMP Set Requests

#include "SnmpAgent.h" // SNMP agent library: github.com/CrientClash/Arduino-Snmp-Agent
#include <UIPEthernet.h>
#include <DHT.h> // DHT sensors library : github.com/adafruit/DHT-sensor-library

SNMPAgent SnmpAgent; // SNMP agent object

#define DHTTYPE DHT11 // type of DHT
#define DHTPIN 3 // pin for input of DHT
DHT dht(DHTPIN, DHTTYPE);

int DEV = 2 ;//DHT 11's deviation is +/- 2 C
int DEV_HUM = 0 ;//DHT 11's humidity deviation is +/- 5% 

// mac address of device (important to be unique in LAN)
static byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF};

static byte ip[] = { 10, 12, 0, 248 };
float temperature = 0; // temporary variable for temperature
float humidity = 0; // temporary variable for humidity
uint32_t sendTemp = 0; // Temperature that will be send 
uint32_t sendHum = 0; // Humidity that will be send
uint32_t lastMillis = 0;

void setup() {

  // put your setup code here, to run once:
  //initialize ethernet with mac address - 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF 
  //and ip address - 10.12.0.248 
  Serial.begin(115200);
  Ethernet.begin(mac, ip);
  
  Serial.print("SNMP Agent IP: "); Serial.println(Ethernet.localIP());

  // Start UDP Server at port 161
  SnmpAgent.begin();
  dht.begin();

  // Setup default SNMP Agent information with OIDS
  SnmpAgent.SetCommunity(PSTR("public"));             // Password/Community (Read-Only)
  SnmpAgent.SetDescription(PSTR("Temperature Sensor"));     // 1.3.6.1.2.1.1.1.0 
  SnmpAgent.SetContact(PSTR("arduinoNano@kaztc.kz"));  // 1.3.6.1.2.1.1.4.0
  SnmpAgent.SetLocation(PSTR("Kaztranscom"));          // 1.3.6.1.2.1.1.6.0
  SnmpAgent.SetSystemName(PSTR("arduino"));           // 1.3.6.1.2.1.1.5.0

  sendTemp = (uint32_t)temperature;
  sendHum = (uint32_t)humidity;
  
  // Setup custom SNMP values (1.3.6.1.4.1.49701.1.X.0),
  // where X is a value between 1 and 5 (defined by MAX_SNMP_VALUES in SnmpAgent.h)
  SnmpAgent.SetValue(1, &sendTemp); // 1.3.6.1.4.1.49701.1.1.0 for temperature
  SnmpAgent.SetValue(2, &sendHum); // 1.3.6.1.4.1.49701.1.2.0 for humidity
}

void loop() {
  // put your main code here, to run repeatedly:
  SnmpAgent.listen();

  // Update the temperature variable every 5 seconds
  if (millis() - lastMillis > 5000) {
    //read DHT measures
    temperature = dht.readTemperature() + DEV; 
    humidity = dht.readHumidity() + DEV_HUM;
    //if dht is not responding send message to serial
    if ( isnan(temperature)) {
      Serial.println(F("Failed to read from DHT sensor!"));
    }
    Serial.println(temperature);
    Serial.println(humidity);
    //assign measured temperature and humidity to send
    sendTemp = (uint32_t)temperature;
    sendHum = (uint32_t)humidity;
    lastMillis = millis();
  }
}
