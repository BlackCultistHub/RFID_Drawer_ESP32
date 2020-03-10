#ifndef EEPROM_OPERATIONS_H
#define EEPROM_OPERATIONS_H

#include <EEPROM.h>

//------------------ [STRUCTURE] -------------
// [ADDRESSES]        "PURPOSE"         {SIZE}
//--------------------------------------------
// [0]             "wifi-mode-flag"      {1b}
// [1]                  "free"           {1b}
// [2]                  "free"           {1b}
// [3]                  "free"           {1b}
// [4-23]             "wifi-ssid"        {20b}
// [24-43]          "wifi-password"      {20b}
// [44-47]             "ip-addr"         {4b}
// [48-49]             "ip-port"         {2b}
// [50-137]             "token"          {64b}
// [114-137]            "free"           {12b}
// [138-512]            "S/N"            {375b}
//--------------------------------------------
// S/N record{5b}: Serial/number{4b} + permissions{1b}
//--------------------------------------------

void EEPROMwipe();
uint8_t EEPROMGetWiFiMode(); //[0]
void EEPROMSetWiFiMode(bool mode);
String EEPROMGetSSID();  //[4-23]
void EEPROMSetSSID(String ssid);
String EEPROMGetPassword();  //[24-43]
void EEPROMSetPassword(String password);
IPAddress EEPROMGetIP();  //[44-47]
void EEPROMSetIP(IPAddress IP);
int EEPROMGetPort();  //[48-49]
void EEPROMSetPort(int port);
uint64_t EEPROMGetToken();  //[50-113]
void EEPROMSetToken(uint64_t token);
int EEPROMGetFreeAddress();
void EEPROMAddSerialNumber(ulong serialNumber, uint8_t permissions);
void EEPROMAddSerialNumber(ulong serialNumber);
void EEPROMDeleteSerialNumber(ulong serialNumber);
bool EEPROMCheckSerialNumberValidation(ulong serialNumber);
void EEPROMReadSerialNumberBase(ulong* serialNumberBase, uint8_t* permissions);
void EEPROMReadSerialNumberBase(ulong* serialNumberBase);

#endif // EEPROM_OPERATIONS_H_