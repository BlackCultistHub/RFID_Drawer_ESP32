#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#include "Wiegand_monkeyboard/Wiegand_monkeyboard.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <EEPROM.h>

//https://github.com/fhessel/esp32_https_server

// Inlcudes for setting up the server
#include <HTTPSServer.hpp>
// Define the certificate data for the server (Certificate and private key)
#include <SSLCert.hpp>
// Includes to define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
// Required do define ResourceNodes
#include <ResourceNode.hpp>

// Include certificate data
#include "cert.h"
#include "private_key.h"

/** Check if we have multiple cores */
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define DEBUG
//#define EXTENDED_DEBUG

//******[Object inits]******
WIEGAND wg;
httpsserver::SSLCert certificate = httpsserver::SSLCert(example_crt_DER, example_crt_DER_len, example_key_DER, example_key_DER_len);
httpsserver::HTTPSServer HTTPSServer = httpsserver::HTTPSServer(&certificate);

//******[Defines]******
#define EEPROM_SIZE 512
#define PIN_D0 23
#define PIN_D1 19
//#define MAX_DATA_ARGUMENT_AMOUNT 32
#define led 22

//******[Network Settings]******
#define SSID_AP "ESP32_TEST_AP"
#define PASSW_AP "1234567890"
#define SSID_CL "RFIDLAB"
#define PASSW_CL "iubCGgtds715"

//******[Predefined Values]******
bool flagAddCard = false,
    flagDeleteCard = false,
    flagServiceMode = false,
    APMode;

//******[Prototypes]******
//----Funcs----
void unsetFlags();

//----TASKS----
void serverTask(void *params);

//----EEPROM----
void EEPROMwipe();
int EEPROMGetFreeAddress();
void EEPROMAddSerialNumber(unsigned long serialNumber);
void EEPROMDeleteSerialNumber(unsigned long serialNumber);
bool EEPROMCheckSerialNumberValidation(unsigned long serialNumber);
void EEPROMReadSerialNumberBase(unsigned long* serialNumberBase);

//----Handlers----
void handlerIndex(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res);
void handler404(httpsserver::HTTPRequest * req, httpsserver::HTTPResponse * res);
// void handlerAddCard();
// void handlerDeleteCard();
// void handlerGetLogs();
// void handlerServiceMode();
//void handlerData();
//void handlerGetEEPROM();

IPAddress local_IP_AP(109,108,112,114);
IPAddress gatewayAP(1,1,1,1);
IPAddress subnetAP(255,255,255,0);

void setup() 
{
  esp_log_level_set("*", ESP_LOG_ERROR);        // set all components to ERROR level
  esp_log_level_set("wifi", ESP_LOG_WARN);      // enable WARN logs from WiFi stack
  esp_log_level_set("dhcpc", ESP_LOG_INFO);     // enable INFO logs from DHCP client

  #ifdef DEBUG
    Serial.begin(9600);  
    delay(10);
    Serial.println("\nSerial begin\n");
  #endif // DEBUG 

  // init RFID Reader
	wg.begin(PIN_D0, PIN_D1);

  // init WIFI connection

  switch ((int)(EEPROM.readByte(0)))
  {
  case 0:
    APMode = true;
    //WiFi.mode(WIFI_STA);
    WiFi.softAP(SSID_AP, PASSW_AP);
    WiFi.begin();
    //WiFi.softAPConfig(local_IP_AP, gatewayAP, subnetAP);
    #ifdef DEBUG
      Serial.print("AP Started, local IP: ");
      Serial.println(WiFi.softAPIP());
    #endif // DEBUG
    break;
  case 1:
    APMode = false;
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.begin(SSID_CL, PASSW_CL);
    #ifdef DEBUG
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("\nConnected.");
    #endif // DEBUG
    break;
  default:
    #ifdef DEBUG
      Serial.println("SOMETHING BAD HAPPENED!");
      Serial.print("EEPROM read: ");
      Serial.println(EEPROM.readByte(0));
    #endif // DEBUG
    break;
  }

  #ifdef DEBUG
  #ifdef EXTENDED_DEBUG
  //EEPROM OUTPUT
    Serial.println("BEGIN EEPROM OUTPUT");
    Serial.println("-------------------");
    Serial.println("| CELL № |  VALUE |");
    for (int i = 0; i < 256; i++)
    {
      Serial.printf("|   %3d  |   %3d  |\n", i, EEPROM.readByte(i));
    }
    Serial.println("-------------------");
  #endif
  #endif // DEBUG

  // TASKS
  //xTaskCreatePinnedToCore(serverTask, "https443", 6144, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  
  // other stuff
  pinMode(led, OUTPUT);
}

void loop() 
{
  if (flagAddCard)
  {

  }
  if (flagDeleteCard)
  {

  }
  if (flagServiceMode)
  {

  }
  #ifdef DEBUG
	if(wg.available())
	{
		Serial.print("Wiegand HEX = ");
		Serial.print(wg.getCode(),HEX);
		Serial.print(", DECIMAL = ");
		Serial.print(wg.getCode());
		Serial.print(", Type W");
		Serial.println(wg.getWiegandType()); 
	}
  #endif // DEBUG
  delay(1);
}

//******[Functions]******
void unsetFlags()
{
  flagAddCard = false;
  flagDeleteCard = false;
  flagServiceMode = false;
}

//----EEPROM----  NEEDS CORRECTION IN [0-23?]
void EEPROMwipe() //wipe everything [0-FF]
{
  //save imei
  for (int i = 0; i < EEPROM.length(); i++)
    EEPROM.write(i, 0);
}

int EEPROMGetFreeAddress()  //finds free space size of ULong in [0-FF]
{
  unsigned long temp = 0;
  int i = 0;
  for (;;i += 4)
  {
    temp = EEPROM.readULong(i);
    if (temp == 0)
      break;
  }
  return i;
}

void EEPROMAddSerialNumber(unsigned long serialNumber)
{
  EEPROM.writeULong(EEPROMGetFreeAddress(),serialNumber);
}

void EEPROMDeleteSerialNumber(unsigned long serialNumber) //finds matching ULong in [0-FF] and sets to 0
{
  unsigned long temp = 0;
  for (int i = 0;;i += 4)
  {
    temp = EEPROM.readLong(i);
    if (temp == serialNumber)
      EEPROM.writeULong(i, 0);
  }
}

bool EEPROMCheckSerialNumberValidation(unsigned long serialNumber) //finds if any of ULong in EEPROM matches the given in [0-FF]
{
  unsigned long temp = 0;
  for (int i = 0;i < 256;i += 4)
  {
    temp = EEPROM.readLong(i);
    if (temp == serialNumber)
      return true;
  }
  return false;
}

void EEPROMReadSerialNumberBase(unsigned long* serialNumberBase) // copy EEPROM by ULong [0-FF]
{
  int i = 0;
  for (int j = 0; j < 256; j += 4)
  {
    serialNumberBase[i] = EEPROM.readULong(j);
    i++;
  }
}

//******[TASKS]******
void serverTask(void *params)
{
  //init HTTPS Server

  // handlers processing
  httpsserver::ResourceNode *nodeRoot = new httpsserver::ResourceNode("/", "GET", &handlerIndex);
  httpsserver::ResourceNode *node404 = new httpsserver::ResourceNode("", "GET", &handler404);
  //server.on("/data", handlerData);
  //server.on("/geteeprom", handlerGetEEPROM);

  //Link nodes
  HTTPSServer.registerNode(nodeRoot);
  HTTPSServer.setDefaultNode(node404);

  HTTPSServer.start();
  if (HTTPSServer.isRunning())
  {
    #ifdef DEBUG
    Serial.print("WebServer started. Local ip is:\t");
    Serial.print(WiFi.localIP().toString().c_str());
    Serial.println(" on port 443.");
    #endif // DEBUG
    for (;;)
    {
      HTTPSServer.loop();
      delay(1);
    }
  }
}

//******[Handlers]******
void handlerIndex(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res)
{
  res->setHeader("Content-Type", "text/html");
  String msg = String("<!DOCTYPE html>")+
                "<html><br>"+
                "<head><title>Root page</title></head><br>"+
                "<body><br>"+
                "<h1>Hello! ESP32.</h1><br>"+
                "</body><br>"+
                "</html><br>";
  res->print(msg);
}

void handler404(httpsserver::HTTPRequest * req, httpsserver::HTTPResponse * res) 
{
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();
  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  // Set content type of the response
  res->setHeader("Content-Type", "text/html");
  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}

// void handlerData()  //temp
// {
//   for (uint8_t i = 0; i < MAX_DATA_ARGUMENT_AMOUNT; i++)
//   {
//     args[i] = "0";
//     argVals[i] = "0";
//   }
//   for(uint8_t i = 0; i < server.args(); i++)
//   {
//     args[i] = server.argName(i);
//     argVals[i] = server.arg(i);
//   }
//   argLen = server.args();
//   String msg = String("<h1>Hello! ESP32.<h1><br>")+
//                       "Data recieve completed.<br>"+
//                       "Recieved "+server.args()+" arguments.<br>"+
//                       "Argument list:<br>";
//   for(uint8_t i = 0; i < server.args(); i++)
//     msg += (i+1)+". "+server.argName(i)+": "+server.arg(i)+"<br>";
//   server.send(200, "text/html", msg);
// }

// void handlerGetEEPROM()
// {

// }