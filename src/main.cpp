#include "Wiegand_monkeyboard/Wiegand_monkeyboard.h"
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

//#define DEBUG

//******[Object inits]******
WIEGAND wg;
WebServer server(80);

//******[Defines]******
#define EEPROM_SIZE 512
#define PIN_D0 23
#define PIN_D1 19
#define MAX_DATA_ARGUMENT_AMOUNT 32
#define led 22

//******[Predefined Values]******
const char* ssid = "RFIDLAB";
const char* password = "iubCGgtds715";
String args[MAX_DATA_ARGUMENT_AMOUNT];
String argVals[MAX_DATA_ARGUMENT_AMOUNT];
int argLen = 0;
bool flagAddCard = false,
    flagDeleteCard = false,
    flagServiceMode = false;

//******[Prototypes]******
void unsetFlags();

//----EEPROM----
void wipeEEPROM();
int EEPROMGetFreeAddress();
void EEPROMAddSerialNumber(unsigned long serialNumber);
void EEPROMDeleteSerialNumber(unsigned long serialNumber);
bool EEPROMCheckSerialNumberValidation(unsigned long serialNumber);
void EEPROMReadSerialNumberBase(unsigned long* serialNumberBase);

//----Handlers----
void handler404();
void handlerIndex();
// void handlerAddCard();
// void handlerDeleteCard();
// void handlerGetLogs();
// void handlerServiceMode();
void handlerData();
void handlerGetEEPROM();

void setup() 
{
  #ifdef DEBUG
	Serial.begin(9600);  
  delay(10);
	Serial.println("\nSerial begin\n");
  #endif // DEBUG 

  // init EEPROM
  if(EEPROM.begin(EEPROM_SIZE))
  {
    #ifdef DEBUG
    Serial.println("EEPROM initialized.");
    #endif // DEBUG
    delay(1);
  }
  else
  {
    #ifdef DEBUG
    Serial.println("EEPROM initialize ERROR.");
    #endif // DEBUG
    delay(1);
  }

  // init RFID Reader
	wg.begin(PIN_D0, PIN_D1);

  // init WIFI connection
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);
  #ifdef DEBUG
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected.");
  #endif // DEBUG

  //init HTTP Server
  server.begin();
  #ifdef DEBUG
  Serial.print("WebServer started.\nLocal ip is:\t");
  Serial.println(WiFi.localIP().toString().c_str());
  #endif // DEBUG

  // handlers processing
  server.onNotFound(handler404);
  server.on("/", handlerIndex);
  server.on("/data", handlerData);
  server.on("/geteeprom", handlerGetEEPROM);

  // other stuff
  pinMode(led, OUTPUT);
}

void loop() 
{
  server.handleClient();
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
//----EEPROM----
void EEPROMwipe()
{
  for (int i = 0; i < 256; i++)
    EEPROM.write(i, 0);
}
int EEPROMGetFreeAddress()
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
void EEPROMDeleteSerialNumber(unsigned long serialNumber)
{
  unsigned long temp = 0;
  for (int i = 0;;i += 4)
  {
    temp = EEPROM.readLong(i);
    if (temp == serialNumber)
      EEPROM.writeULong(i, 0);
  }
}
bool EEPROMCheckSerialNumberValidation(unsigned long serialNumber)
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
void EEPROMReadSerialNumberBase(unsigned long* serialNumberBase)
{
  int i = 0;
  for (int j = 0; j < 256; j += 4)
  {
    serialNumberBase[i] = EEPROM.readULong(j);
    i++;
  }
}

//******[Handlers]******
void handler404()
{
  String msg = String("<h1>404 Not Found.</h1><br>")+
                      "URI: "+server.uri()+"<br>"+
                      "Method: "+((server.method() == HTTP_GET)?"GET":"POST")+"<br>"
                      "Arguments: "+server.args()+"<br>";
  for (uint8_t i = 0; i < server.args(); i++)
    msg += " "+server.argName(i)+": "+server.arg(i)+"<br>";
  server.send(404, "text/html", msg);
}

void handlerIndex()
{
  String msg = String("<h1>Hello! ESP32.</h1><br>");
  server.send(200, "text/html", msg);
}

void handlerData()  //temp
{
  for (uint8_t i = 0; i < MAX_DATA_ARGUMENT_AMOUNT; i++)
  {
    args[i] = "0";
    argVals[i] = "0";
  }
  for(uint8_t i = 0; i < server.args(); i++)
  {
    args[i] = server.argName(i);
    argVals[i] = server.arg(i);
  }
  argLen = server.args();
  String msg = String("<h1>Hello! ESP32.<h1><br>")+
                      "Data recieve completed.<br>"+
                      "Recieved "+server.args()+" arguments.<br>"+
                      "Argument list:<br>";
  for(uint8_t i = 0; i < server.args(); i++)
    msg += (i+1)+". "+server.argName(i)+": "+server.arg(i)+"<br>";
  server.send(200, "text/html", msg);
}

void handlerGetEEPROM()
{

}