#include "Arduino.h"

#ifndef config_settings_H_
#define config_settings_H_

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef DEVICE_NAME
#define DEVICE_NAME "RS485-WiFi"
#endif
#ifndef SW_VERSION
#define SW_VERSION 0.60
#endif
#ifndef DEVICE_DESCRIPTION
  #define DEVICE_DESCRIPTION "EpEver Solar Monitor"
#endif

#ifndef DEVICE_MANUFACTURER
  #define DEVICE_MANUFACTURER "EpEver"
#endif


#define DEVICE_FULL_NAME DEVICE_NAME " v" STR(SW_VERSION)

//Where in EEPROM do we store the configuration
#define EEPROM_storageSize 2048
#define EEPROM_WIFI_CHECKSUM_ADDRESS 0
#define EEPROM_WIFI_CONFIG_ADDRESS EEPROM_WIFI_CHECKSUM_ADDRESS+sizeof(uint32_t)

#define EEPROM_CHECKSUM_ADDRESS 512
#define EEPROM_CONFIG_ADDRESS EEPROM_CHECKSUM_ADDRESS+sizeof(uint32_t)

#ifndef DEFAULT_MQTT_SERVER
  #define DEFAULT_MQTT_SERVER   "192.168.0.254"
#endif
#ifndef DEFAULT_MQTT_USERNAME
  #define DEFAULT_MQTT_USERNAME "username"
#endif
#ifndef DEFAULT_MQTT_PASSWORD
  #define DEFAULT_MQTT_PASSWORD "password"
#endif
#ifndef DEFAULT_MQTT_TOPIC
  #define DEFAULT_MQTT_TOPIC    "solar"
#endif
#ifndef DEFAULT_MQTT_PORT
  #define DEFAULT_MQTT_PORT     1883
#endif

#ifndef DEFAULT_INFLUXDB_HOST
  #define DEFAULT_INFLUXDB_HOST      "192.168.0.254"
#endif
#ifndef DEFAULT_INFLUXDB_DATABASE
  #define DEFAULT_INFLUXDB_DATABASE  "powerwall"
#endif
#ifndef DEFAULT_INFLUXDB_USER
  #define DEFAULT_INFLUXDB_USER      "username"
#endif
#ifndef DEFAULT_INFLUXDB_PASSWORD
  #define DEFAULT_INFLUXDB_PASSWORD  "PASSWORD"
#endif
#ifndef DEFAULT_INFLUXDB_PORT
  #define DEFAULT_INFLUXDB_PORT      8086
#endif

#ifndef DEFAULT_DEVICE_ID
  #define DEFAULT_DEVICE_ID      10
#endif
#ifndef DEFAULT_SERIAL_BAUD
  #define DEFAULT_SERIAL_BAUD    115200
#endif

//We have allowed space for 2048-512 bytes of EEPROM for settings (1536 bytes)
struct eeprom_settings { 
  bool MQTT_Enable;
  int  mqtt_port;
  char mqtt_server[64 + 1];
  char mqtt_username[64 + 1];
  char mqtt_password[64 + 1];
  char mqtt_topic[64 + 1];
  
  bool influxdb_enabled;
  char influxdb_host[64 +1 ];
  int  influxdb_httpPort;
  char influxdb_database[32 + 1];
  char influxdb_user[32 + 1];
  char influxdb_password[32 + 1];
  
  bool HADiscovery_Enable;

  int Device_ID;
  int Device_BAUD;
};

extern char baseMacChr[13];
extern eeprom_settings myConfig;
extern bool isWrittingEEPROM;

void WriteConfigToEEPROM();
bool LoadConfigFromEEPROM();
void WriteWIFIConfigToEEPROM();
bool LoadWIFIConfigFromEEPROM();
void FactoryResetSettings();

#endif
