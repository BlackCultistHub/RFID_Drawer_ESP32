//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"

//#include "libs/Wiegand_monkeyboard/Wiegand_monkeyboard.h"
#include <Wiegand_monkeyboard.h>
#include <WiFi.h>

//include project-based EEPROM operations
#include "eeprom_operations/eeprom_operations.h"

//http server for AP mode
#include <WebServer.h>

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

//Include html pages in c-style
#include "pages/pages.h"

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
WebServer server(80);
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
#define PASSW_AP "000987654321000"
#define SSID_CL "InterZet610_2.4"
#define PASSW_CL "0987654321000"
#define SSID_CL2 "RFID_LAB"
#define PASSW_CL2 "iubCGgtds715"

//******[Predefined Values]******
bool flagAddCard = false,
    flagDeleteCard = false,
    flagServiceMode = false,
    APMode;
int timer0 = 0;

//******[Prototypes]******
//----Funcs----
void unsetFlags();

//----TASKS----
void serverTask(void *params);
void httpsTask(void *params);

//----EEPROM----
void EEPROMwipe();
int EEPROMGetFreeAddress();
void EEPROMAddSerialNumber(unsigned long serialNumber);
void EEPROMDeleteSerialNumber(unsigned long serialNumber);
bool EEPROMCheckSerialNumberValidation(unsigned long serialNumber);
void EEPROMReadSerialNumberBase(unsigned long* serialNumberBase);

//----Handlers----
//http
void handlerAP404();
void handlerAPIndex();
void handlerAPCloudPage();
void handlerAPCustomPage();
void handlerAPLocalSetup();
void handlerAPChangeConfig();
void handlerAPResetDevice();

//https
void handlerIndex(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res);
void handler404(httpsserver::HTTPRequest * req, httpsserver::HTTPResponse * res);
// void handlerAddCard();
// void handlerDeleteCard();
// void handlerGetLogs();
// void handlerServiceMode();
//void handlerData();
//void handlerGetEEPROM();

void setup() 
{
  #ifdef DEBUG
    Serial.begin(9600);  
    delay(10);
    Serial.println("\nSerial begin\n");
  #endif // DEBUG 

  //init EEPROM
  if(!EEPROM.begin(EEPROM_SIZE))
  {
    #ifdef DEBUG
      Serial.println("ERROR beginning EEPROM. Rebooting...");
    #endif //DEBUG
    ESP.restart();
  }
  else
  {
    #ifdef DEBUG
      Serial.println("EEPROM initialized.");
    #endif //DEBUG
  }
  

  // init RFID Reader
	wg.begin(PIN_D0, PIN_D1);

  // init WIFI connection
  switch ((int)(EEPROM.readByte(0)))
  {
  case 0:
    APMode = true;
    WiFi.softAP(SSID_AP, PASSW_AP);
    #ifdef DEBUG
      Serial.print("AP Started, local IP: ");
      Serial.println(WiFi.softAPIP());
    #endif // DEBUG
    // TASK HTTP
    xTaskCreatePinnedToCore(serverTask, "http80", 6144, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
    break;
  case 1:
    APMode = false;
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.begin(SSID_CL, PASSW_CL);
    timer0 = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      #ifdef DEBUG
        Serial.print(".");
      #endif // DEBUG
      if (millis()-timer0 > 15000)
      {
        
        ESP.restart();
      }
    }
    #ifdef DEBUG
      Serial.println("\nConnected.");
    #endif // DEBUG
    // TASK HTTPS
    //xTaskCreatePinnedToCore(httpsTask, "https443", 6144, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
    break;
  default:
    #ifdef DEBUG
      Serial.println("SOMETHING BAD HAPPENED!");
      Serial.print("EEPROM read: ");
      Serial.println(EEPROM.readByte(0));
      Serial.print("Writing 0 to [0] and restarting...");
      EEPROM.put(0, 0);
      EEPROM.commit();
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

  
  
  // other stuff
  pinMode(led, OUTPUT);

  Serial.println(EEPROM.readByte(0));
  Serial.println(EEPROM.readByte(1));
  Serial.println(EEPROM.readByte(2));
  String checkEE = "";
  EEPROM.get(0, checkEE);
  Serial.println(checkEE);

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

//******[TASKS]******
void serverTask(void *params)
{
  // begin
  server.begin();

  // handlers
  server.onNotFound(handlerAP404);
  server.on("/", handlerAPIndex);
  server.on("/cloudSetup", HTTP_POST, handlerAPCloudPage);
  server.on("/localSetup", handlerAPLocalSetup);
  server.on("/customServerSetup", HTTP_POST, handlerAPCustomPage);
  server.on("/changeAPConfig", HTTP_POST, handlerAPChangeConfig);
  server.on("/factoryReset", HTTP_POST, handlerAPResetDevice);

  //
  #ifdef DEBUG
    Serial.print("HTTP server started on local ip: ");
    Serial.print(WiFi.softAPIP());
    Serial.println(" on port 80.");
  #endif // DEBUG
  while (true)
  {
    server.handleClient();
    delay(1);
  }
}

void httpsTask(void *params)
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
    Serial.print(WiFi.localIP());
    Serial.println(" on port 443.");
    #endif // DEBUG
    while (true)
    {
      HTTPSServer.loop();
      delay(1);
    }
  }
}

//******[Handlers]******
//http
void handlerAP404()
{
  server.send(404, "text/html", AP404);
}

void handlerAPIndex()
{
  server.send(200, "text/html", APIndex);
}

void handlerAPCloudPage()
{
  //DO ARGS
  server.send(200, "text/html", APConfirmCloudPage);
}

void handlerAPCustomPage()
{
  //DO ARGS
  server.send(200, "text/html", APConfirmCustomPage);
}

void handlerAPLocalSetup()
{
  server.send(200, "text/html", APLocalSetupPage);
}

void handlerAPChangeConfig()
{
  //DO ARGS
  server.send(200, "text/html", APChangeConfigPage);
}

void handlerAPResetDevice()
{
  EEPROMwipe();
  server.send(200, "text/html", APResetDevicePage);
}

//https
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


