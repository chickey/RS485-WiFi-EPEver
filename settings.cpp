#include <EEPROM.h>

#include "settings.h"
eeprom_settings myConfig;

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

  Serial.println(checksum, HEX);
  Serial.println(existingChecksum, HEX);

  if (checksum == existingChecksum) {
    //Clone the config into our global variable and return all OK
    memcpy(&myConfig, &restoredConfig, sizeof(eeprom_settings));   
    return true;
  }

  
  //Config is not configured or gone bad, return FALSE
  Serial.println("Config is not configured or gone bad");
  return false;
}

void FactoryResetSettings() {
  const char mqtt_server[] = "192.168.0.254";
  const char mqtt_username[]="username";
  const char mqtt_password[]="password";
  const char mqtt_topic[]="topic";
  
  const char influxdb_host[] = "192.168.0.254";
  const char influxdb_database[] = "powerwall";
  const char influxdb_user[] = "username";
  const char influxdb_password[] = "password";

  strcpy(myConfig.influxdb_host, influxdb_host );
  strcpy(myConfig.influxdb_database, influxdb_database );
  strcpy(myConfig.influxdb_user, influxdb_user );
  strcpy(myConfig.influxdb_password, influxdb_password );

  myConfig.influxdb_enabled=false;
  myConfig.influxdb_httpPort=8086;

  strcpy(myConfig.mqtt_server, mqtt_server );
  strcpy(myConfig.mqtt_username, mqtt_username);
  strcpy(myConfig.mqtt_password, mqtt_password);
  strcpy(myConfig.mqtt_topic, mqtt_topic);
  
  myConfig.MQTT_Enable=false;
  myConfig.mqtt_port = 1883;

  WriteConfigToEEPROM();
}
