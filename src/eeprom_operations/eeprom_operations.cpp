#include "eeprom_operations.h"

void EEPROMwipe() //wipe everything [0-FF]
{
  for (int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.put(i, 0);
    EEPROM.commit();
  }
}

uint8_t EEPROMGetWiFiMode() //[0]
{
  uint8_t wifiMode = 0;
  EEPROM.get(0, wifiMode);
  return wifiMode;
}

void EEPROMSetWiFiMode(bool mode) //[0]
{
  uint8_t wifiMode = mode;
  EEPROM.put(0, wifiMode);
  EEPROM.commit();
}

String EEPROMGetSSID()  //[4-23]
{
  String ssid = "";
  for (int i = 0; i < 20; i++)
    EEPROM.get(i+4, ssid[i]);
  return ssid;
}

void EEPROMSetSSID(String ssid)  //[4-23]
{
  for (int i = 0; i < 20; i++)
  {
    EEPROM.put(i+4, ssid[i]);
    EEPROM.commit();
  }
}

String EEPROMGetPassword()  //[24-43]
{
  String password = "";
  for (int i = 0; i < 20; i++)
    EEPROM.get(i+24, password[i]);
  return password;
}

void EEPROMSetPassword(String password)  //[24-43]
{
  for (int i = 0; i < 20; i++)
  {
    EEPROM.put(i+24, password[i]);
    EEPROM.commit();
  }
}

IPAddress EEPROMGetIP()  //[44-47]
{
  // int mask[4];
  // mask[0] = 0xFF000000; //FIRST OCTET
  // mask[1] = 0x00FF0000; //SECOND OCTET
  // mask[2] = 0x0000FF00; //THIRD OCTET
  // mask[3] = 0x000000FF; //FORTH OCTET
  uint32_t intIP = 0;
  EEPROM.get(44, intIP);
  return IPAddress(intIP);
}

void EEPROMSetIP(IPAddress IP)  //[44-47]
{
  uint32_t intIP = IP; //double check this
  EEPROM.put(44, intIP);
  EEPROM.commit();
}

int EEPROMGetPort()  //[48-49]
{
  uint8_t portPart = 0;
  EEPROM.get(48, portPart);
  int port = portPart;
  EEPROM.get(49, portPart);
  port <<= 8;
  port += portPart;
  return port;
}

void EEPROMSetPort(int port)  //[48-49]
{
  uint8_t portPart = 0;
  portPart = port;  //from end
  EEPROM.put(49, portPart);
  EEPROM.commit();
  port >>= 8;
  portPart = port;
  EEPROM.put(48, portPart);
  EEPROM.commit();
}

uint64_t EEPROMGetToken()  //[50-113]
{
  uint64_t token = 0;
  EEPROM.get(50, token);
  return token;
}

void EEPROMSetToken(uint64_t token)  //[50-113]
{
  EEPROM.put(50, token);
  EEPROM.commit();
}

//FREE 12 bytes here [114-137]

int EEPROMGetFreeAddress()  //finds free space size of ULong in [138-512]
{
  ulong temp = 0;
  int i = 0;
  for (i = 138;;i += 5)
  {
    EEPROM.get(i, temp);
    if (temp == 0)
      break;
  }
  return i;
}

void EEPROMAddSerialNumber(ulong serialNumber, uint8_t permissions)
{
  ulong freeAddr = EEPROMGetFreeAddress();
  EEPROM.put(freeAddr,serialNumber);  // S/N
  EEPROM.commit();
  EEPROM.put(freeAddr+4, permissions); // permissions byte
  EEPROM.commit();
}

void EEPROMAddSerialNumber(ulong serialNumber)
{
  ulong freeAddr = EEPROMGetFreeAddress();
  uint8_t permissions = 0x00;
  EEPROM.put(freeAddr,serialNumber);  // S/N
  EEPROM.commit();
  EEPROM.put(freeAddr+4, permissions); // permissions byte
  EEPROM.commit();
}

void EEPROMDeleteSerialNumber(ulong serialNumber) //finds matching ULong in [138-512] and sets to 0
{
  ulong temp = 0;
  uint8_t permissions = 0x00;
  for (int i = 138;;i += 5)
  {
    EEPROM.get(i, temp);
    if (temp == serialNumber)
    {
      EEPROM.put(i, ulong(0));  // S/N
      EEPROM.commit();
      EEPROM.put(i+4, permissions); // permissions byte
      EEPROM.commit();
    }
  }
}

bool EEPROMCheckSerialNumberValidation(ulong serialNumber) //finds if any of ULong in EEPROM matches the given in [138-512]
{
  ulong temp = 0;
  for (int i = 138;i < 512;i += 5)
  {
    EEPROM.get(i, temp);
    if (temp == serialNumber)
      return true;
  }
  return false;
}

void EEPROMReadSerialNumberBase(ulong* serialNumberBase, uint8_t* permissions) // copy EEPROM by ULong [138-512]
{
  for (int j = 0; j < 75; j += 5)
  {
    EEPROM.get(j+138, serialNumberBase[j]);
    EEPROM.get(j+138+4, permissions[j]);
  }
}

void EEPROMReadSerialNumberBase(ulong* serialNumberBase) // copy EEPROM by ULong [138-512]
{
  for (int j = 0; j < 75; j += 5)
    EEPROM.get(j+138, serialNumberBase[j]);
}