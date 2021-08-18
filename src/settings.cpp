#include <EEPROM.h>

#include "settings.h"
eeprom_settings myConfig;
char baseMacChr[13] = {0};

uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
  //This calculates a CRC32 the same as used in MPEG2 streams
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

void WriteConfigToEEPROM() {
  uint32_t checksum = calculateCRC32((uint8_t*)&myConfig, sizeof(eeprom_settings));
   
  EEPROM.begin(EEPROM_storageSize);
  EEPROM.put(EEPROM_CONFIG_ADDRESS, myConfig);
  EEPROM.put(EEPROM_CHECKSUM_ADDRESS, checksum);
  EEPROM.end();
}

bool LoadConfigFromEEPROM() {
  eeprom_settings restoredConfig;
  uint32_t existingChecksum;

  EEPROM.begin(EEPROM_storageSize);
  EEPROM.get(EEPROM_CONFIG_ADDRESS, restoredConfig);
  EEPROM.get(EEPROM_CHECKSUM_ADDRESS, existingChecksum);
  EEPROM.end();

  // Calculate the checksum of an entire buffer at once.
  uint32_t checksum = calculateCRC32((uint8_t*)&restoredConfig, sizeof(eeprom_settings));

#ifdef DEBUG
  Serial.println(checksum, HEX);
  Serial.println(existingChecksum, HEX);
#endif

  if (checksum == existingChecksum) {
    //Clone the config into our global variable and return all OK
    memcpy(&myConfig, &restoredConfig, sizeof(eeprom_settings));   
    return true;
  }

#ifdef DEBUG
  //Config is not configured or gone bad, return FALSE
  Serial.println(F("Config is not configured or gone bad"));
#endif

  return false;
}

void FactoryResetSettings() {
  strcpy(myConfig.influxdb_host    , DEFAULT_INFLUXDB_HOST );
  strcpy(myConfig.influxdb_database, DEFAULT_INFLUXDB_DATABASE );
  strcpy(myConfig.influxdb_user    , DEFAULT_INFLUXDB_USER );
  strcpy(myConfig.influxdb_password, DEFAULT_INFLUXDB_PASSWORD );

  myConfig.influxdb_enabled  = false;
  myConfig.influxdb_httpPort = DEFAULT_INFLUXDB_PORT;

  strcpy(myConfig.mqtt_server  , DEFAULT_MQTT_SERVER );
  strcpy(myConfig.mqtt_username, DEFAULT_MQTT_USERNAME);
  strcpy(myConfig.mqtt_password, DEFAULT_MQTT_PASSWORD);
  strcpy(myConfig.mqtt_topic   , DEFAULT_MQTT_TOPIC);
  
  myConfig.MQTT_Enable =false;
  myConfig.mqtt_port   = DEFAULT_MQTT_PORT;

  myConfig.HADiscovery_Enable = false;
  
  myConfig.Device_ID   = DEFAULT_DEVICE_ID;
  myConfig.Device_BAUD = DEFAULT_SERIAL_BAUD;
  
  WriteConfigToEEPROM();
}
