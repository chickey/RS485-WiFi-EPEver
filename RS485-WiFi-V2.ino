#include <DNSServer.h>
#include <ESPUI.h>
#include <ModbusMaster.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include "settings.h"
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

//#include <ModbusSlaveTCP.h>
#include <ESPAsyncWebServer.h>     //Local WebServer used to serve the configuration portal
#include <ESPAsyncWiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <Updater.h>

const char* OTA_INDEX PROGMEM
    = R"=====(<!DOCTYPE html><html><head><meta charset=utf-8><title>OTA</title></head><body><div class="upload"><form method="POST" action="/ota" enctype="multipart/form-data"><input type="file" name="data" /><input type="submit" name="upload" value="Upload" title="Upload Files"></form></div></body></html>)=====";


#include "config.h"
#include "mqtt.h"
#include "influxdb.h"

AsyncWebServer server(80);
DNSServer dns;
WiFiServer localServer(23);

bool debug = false;

uint16_t button1;
uint16_t Model;
uint16_t CCModel;
uint16_t StatusLabel;
uint16_t BatteryStateOC;
uint16_t ChargingStatus;
uint16_t SolarVoltage;
uint16_t SolarAmps;
uint16_t SolarWattage;
uint16_t BatteryVoltage;
uint16_t BatteryAmps;
uint16_t BatteryWattage;
uint16_t BatteryStatus;
uint16_t LoadVoltage;
uint16_t LoadAmps;
uint16_t LoadWattage;
uint16_t LoadStatus;
uint16_t LoadSwitchstate;
uint16_t Status;
uint16_t TPPassthrough;
uint16_t DeviceTemp;
uint16_t MQTTEN;
uint16_t MQTTIP;
uint16_t MQTTPORT;
uint16_t MQTTUSER;
uint16_t MQTTPASS;
uint16_t MQTTTOPIC;

uint16_t INFLUXDBIP;
uint16_t INFLUXDBPORT;
uint16_t INFLUXDBDB;
uint16_t INFLUXDBUSER;
uint16_t INFLUXDBPASS;
uint16_t INFLUXDBEN;

uint16_t OverVoltDist;
uint16_t OverVoltRecon;
uint16_t EQChargeVolt;
uint16_t BoostChargeVolt;
uint16_t FloatChargeVolt;
uint16_t BoostReconChargeVolt;
uint16_t BatteryChargePercent;
uint16_t ChargeLimitVolt;
uint16_t DischargeLimitVolt;
uint16_t LowVoltDisconnect;
uint16_t LowVoltReconnect;
uint16_t UnderVoltWarningVolt;
uint16_t UnderVoltReconnectVolt;
uint16_t BatteryDischargePercent;
uint16_t BoostDuration;
uint16_t EQDuration;
uint16_t BatteryCapactity;
uint16_t BatteryType;

uint16_t Maxinputvolttoday;
uint16_t Mininputvolttoday;
uint16_t MaxBatteryvolttoday;
uint16_t MinBatteryvolttoday;
uint16_t ConsumedEnergyToday;
uint16_t ConsumedEnergyMonth;
uint16_t ConsumedEngeryYear;
uint16_t TotalConsumedEnergy;
uint16_t GeneratedEnergyToday;
uint16_t GeneratedEnergyMonth;
uint16_t GeneratedEnergyYear;
uint16_t TotalGeneratedEnergy;
uint16_t Co2Reduction;
uint16_t NetBatteryCurrent;
uint16_t BatteryTemp;
uint16_t AmbientTemp;

uint16_t EQChargeVoltValue;

int period = 60000;
unsigned long time_now = 0;

void handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
    if (!index)
    {
        Serial.printf("UploadStart: %s\n", filename.c_str());
         // calculate sketch space required for the update, for ESP32 use the max constant
#if defined(ESP32)
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
#else
        const uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace))
#endif
        {
            // start with max available size
            Update.printError(Serial);
        }
#if defined(ESP8266)
        Update.runAsync(true);
#endif
    }

    if (len)
    {
        Update.write(data, len);
    }

    // if the final flag is set then this is the last frame of data
    if (final)
    {
        if (Update.end(true))
        {
            // true to set the size to the current progress
            Serial.printf("Update Success: %ub written\nRebooting...\n", index + len);
            ESP.restart();
        }
        else
        {
            Update.printError(Serial);
        }
    }
}

void setupGUI()
{
    ESPUI.begin("RS485-WiFi"); // It is important that ESPUI.begin(...) is called first so that ESPUI.server is initalized

    ESPUI.server->on("/ota", 
        HTTP_POST, 
        [](AsyncWebServerRequest* request) { request->send(200); }, 
        handleOTAUpload);

    ESPUI.server->on("/ota", 
        HTTP_GET, 
        [](AsyncWebServerRequest* request) {
            AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", OTA_INDEX);
            request->send(response);
        }
    );
}

void buttonCallback(Control *sender, int type) {
  switch (type) {
  case B_DOWN:
    Serial.println("Saving");
    WriteConfigToEEPROM();
    break;
  }
}

void OverVoltDisttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void OverVoltRecontxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void EQChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BoostChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void FloatChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BoostReconChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BatteryChargePercenttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void ChargeLimitVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void DischargeLimitVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void LowVoltDisconnecttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void LowVoltReconnecttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void UnderVoltWarningVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void UnderVoltReconnectVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BatteryDischargePercenttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BoostDurationtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void EQDurationtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BatteryCapactitytxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void MQTTIPtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_server,(sender->value).c_str());
}

void MQTTPorttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  myConfig.mqtt_port = atoi ( (sender->value).c_str() );
}

void MQTTUsertxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_username,(sender->value).c_str());
}

void MQTTPasstxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_password,(sender->value).c_str());
}

void MQTTTopictxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_topic,(sender->value).c_str());
}

void InfluxDBIPtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_host,(sender->value).c_str());
}

void InfluxDBPorttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  myConfig.influxdb_httpPort = atoi ( (sender->value).c_str() );
}

void InfluxDBtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_database,(sender->value).c_str());
}

void InfluxDBUsertxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_user,(sender->value).c_str());
}

void InfluxDBPasstxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_password,(sender->value).c_str());
}

void BatteryTypeList(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void ChargingModeList(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void RatedVoltagelvlList(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}


// Pins
//
#define MAX485_DE         D2  // data or
#define MAX485_RE         D1  // recv enable


ModbusMaster node;   // instantiate ModbusMaster object

void LoadSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    Serial.print("Active:");
    loadState = true;
    do_update = 1;
    switch_load = 1;
    break;

  case S_INACTIVE:
    Serial.print("Inactive");
    loadState = false;
    do_update = 1;
    switch_load = 1;
    break;
  }
  Serial.print(" ");
  Serial.println(sender->id);
}

void InfluxDBEnSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    Serial.print("Active:");
    myConfig.influxdb_enabled = 1;
    break;

  case S_INACTIVE:
    Serial.print("Inactive");
    myConfig.influxdb_enabled = 0;
    break;
  }

  Serial.print(" ");
  Serial.println(sender->id);
}

void MQTTEnSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    Serial.print("Active:");
    myConfig.MQTT_Enable = 1;
    break;

  case S_INACTIVE:
    Serial.print("Inactive");
    myConfig.MQTT_Enable = 0;
    break;
  }

  Serial.print(" ");
  Serial.println(sender->id);
}

void setup(void) {
  Serial.begin(115200);
  localServer.begin();
  localServer.setNoDelay(true);
  
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  // init modbus in receive mode
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
    
  // EPEver Device ID 1
  node.begin(1, Serial);
    
  // modbus callbacks
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
    
  //  Create ESPUI interface tabs
  uint16_t tab1 = ESPUI.addControl( ControlType::Tab, "Settings 1", "Live Data" );
  uint16_t tab2 = ESPUI.addControl( ControlType::Tab, "Settings 2", "Historical Data" );
  uint16_t tab3 = ESPUI.addControl( ControlType::Tab, "Settings 3", "Settings" );
  //uint16_t tab4 = ESPUI.addControl( ControlType::Tab, "Settings 4", "Solar Settings" );
  
  //  Add Live Data controls
  //Status = ESPUI.addControl( ControlType::Label, "Device Status", "Charging", ControlColor::Emerald, tab1);
  SolarVoltage = ESPUI.addControl( ControlType::Label, "Solar Voltage", "0", ControlColor::Emerald, tab1);
  SolarAmps = ESPUI.addControl( ControlType::Label, "Solar Amps", "0", ControlColor::Emerald, tab1);
  SolarWattage = ESPUI.addControl( ControlType::Label, "Solar Wattage", "0", ControlColor::Emerald, tab1);
  BatteryVoltage = ESPUI.addControl( ControlType::Label, "Battery Voltage", "0", ControlColor::Emerald, tab1);
  BatteryAmps = ESPUI.addControl( ControlType::Label, "Battery Amps", "0", ControlColor::Emerald, tab1);
  BatteryWattage = ESPUI.addControl( ControlType::Label, "Battery Wattage", "0", ControlColor::Emerald, tab1);
  LoadVoltage = ESPUI.addControl( ControlType::Label, "Load Voltage", "0", ControlColor::Emerald, tab1);
  LoadAmps = ESPUI.addControl( ControlType::Label, "Load Amps", "0", ControlColor::Emerald, tab1);
  LoadWattage = ESPUI.addControl( ControlType::Label, "Load Wattage", "0", ControlColor::Emerald, tab1);
  BatteryStateOC = ESPUI.addControl( ControlType::Label, "Battery SOC", "0", ControlColor::Emerald, tab1);
  ChargingStatus = ESPUI.addControl( ControlType::Label, "Charging Status", "0", ControlColor::Emerald, tab1);
  BatteryStatus = ESPUI.addControl( ControlType::Label, "Battery Status", "4", ControlColor::Emerald, tab1);
  BatteryTemp = ESPUI.addControl( ControlType::Label, "Battery temperature", "0", ControlColor::Emerald, tab1);
  LoadStatus = ESPUI.addControl( ControlType::Label, "Load Status", "Off", ControlColor::Emerald, tab1);
  DeviceTemp = ESPUI.addControl( ControlType::Label, "Device Temp", "0", ControlColor::Emerald, tab1);
  //Model = ESPUI.addControl( ControlType::Label, "Model", "", ControlColor::Emerald, tab1);
  
  // Add Historical Data Controls
  Maxinputvolttoday = ESPUI.addControl( ControlType::Label, "Max input voltage today", "0", ControlColor::Emerald, tab2);
  Mininputvolttoday = ESPUI.addControl( ControlType::Label, "Min input voltage today", "0", ControlColor::Emerald, tab2);
  MaxBatteryvolttoday = ESPUI.addControl( ControlType::Label, "Max battery voltage today", "0", ControlColor::Emerald, tab2);
  MinBatteryvolttoday = ESPUI.addControl( ControlType::Label, "Min battery voltage today", "0", ControlColor::Emerald, tab2);
  ConsumedEnergyToday = ESPUI.addControl( ControlType::Label, "Consumed energy today", "0", ControlColor::Emerald, tab2);
  ConsumedEnergyMonth = ESPUI.addControl( ControlType::Label, "Consumed energy this month", "0", ControlColor::Emerald, tab2);
  ConsumedEngeryYear = ESPUI.addControl( ControlType::Label, "Consumed energy this year", "0", ControlColor::Emerald, tab2);
  TotalConsumedEnergy = ESPUI.addControl( ControlType::Label, "Total consumed energy", "0", ControlColor::Emerald, tab2);
  GeneratedEnergyToday = ESPUI.addControl( ControlType::Label, "Generated energy today", "0", ControlColor::Emerald, tab2);
  GeneratedEnergyMonth = ESPUI.addControl( ControlType::Label, "Generated energy this month", "0", ControlColor::Emerald, tab2);
  GeneratedEnergyYear = ESPUI.addControl( ControlType::Label, "Generated energy this year", "0", ControlColor::Emerald, tab2);
  TotalGeneratedEnergy = ESPUI.addControl( ControlType::Label, "Total generated energy", "0", ControlColor::Emerald, tab2);
  Co2Reduction = ESPUI.addControl( ControlType::Label, "Carbon dioxide reduction", "0", ControlColor::Emerald, tab2);
  //NetBatteryCurrent = ESPUI.addControl( ControlType::Label, "Net battery current", "0", ControlColor::Emerald, tab2);
  //AmbientTemp = ESPUI.addControl( ControlType::Label, "Ambient temperature", "0", ControlColor::Emerald, tab2);

  
  // Add Local Settings controls
  INFLUXDBIP = ESPUI.addControl( ControlType::Text, "InfluxDB IP:", "192.168.0.254", ControlColor::Emerald, tab3 ,&InfluxDBIPtxt);
  INFLUXDBPORT = ESPUI.addControl( ControlType::Text, "InfluxDB Port:", "8080", ControlColor::Emerald, tab3 ,&InfluxDBPorttxt);
  INFLUXDBDB = ESPUI.addControl( ControlType::Text, "InfluxDB Database:", "powewall", ControlColor::Emerald, tab3 ,&InfluxDBtxt);
  INFLUXDBUSER = ESPUI.addControl( ControlType::Text, "InfluxDB Username:", "username", ControlColor::Emerald, tab3 ,&InfluxDBUsertxt);
  INFLUXDBPASS = ESPUI.addControl( ControlType::Text, "InfluxDB Password:", "password", ControlColor::Emerald, tab3 ,&InfluxDBPasstxt);
  INFLUXDBEN = ESPUI.addControl(ControlType::Switcher, "Enable InfluxDB", "", ControlColor::Alizarin,tab3, &InfluxDBEnSwitch);  

  MQTTIP = ESPUI.addControl( ControlType::Text, "MQTT IP:", "192.168.0.254", ControlColor::Emerald, tab3 ,&MQTTIPtxt);
  MQTTPORT = ESPUI.addControl( ControlType::Text, "MQTT Port:", "1883", ControlColor::Emerald, tab3 ,&MQTTPorttxt);
  MQTTUSER = ESPUI.addControl( ControlType::Text, "MQTT Username:", "username", ControlColor::Emerald, tab3 ,&MQTTUsertxt);
  MQTTPASS = ESPUI.addControl( ControlType::Text, "MQTT Password:", "password", ControlColor::Emerald, tab3 ,&MQTTPasstxt);
  MQTTTOPIC = ESPUI.addControl( ControlType::Text, "MQTT Topic:", "solar", ControlColor::Emerald, tab3 ,&MQTTTopictxt);
  MQTTEN = ESPUI.addControl(ControlType::Switcher, "Enable MQTT", "", ControlColor::Alizarin,tab3, &MQTTEnSwitch);
    
  LoadSwitchstate = ESPUI.addControl(ControlType::Switcher, "Load", "", ControlColor::Alizarin,tab3, &LoadSwitch);
    
  // Add Solar Settings controls
  /*uint16_t BatteryTypeControl = ESPUI.addControl( ControlType::Select, "Battery Type", "", ControlColor::Emerald, tab4 ,&BatteryTypeList);
  ESPUI.addControl( ControlType::Option, "Sealed", "Sealed", ControlColor::Alizarin, BatteryTypeControl );
  ESPUI.addControl( ControlType::Option, "Gel", "Gel", ControlColor::Alizarin, BatteryTypeControl );
  ESPUI.addControl( ControlType::Option, "Flooded", "Flooded", ControlColor::Alizarin, BatteryTypeControl );

  uint16_t select2 = ESPUI.addControl( ControlType::Select, "Charging Mode", "", ControlColor::Emerald, tab4 ,&ChargingModeList);
  ESPUI.addControl( ControlType::Option, "VoltComp", "Volt Comp", ControlColor::Alizarin, select2 );
  ESPUI.addControl( ControlType::Option, "SOC", "SOC", ControlColor::Alizarin, select2 );
  
  uint16_t select3 = ESPUI.addControl( ControlType::Select, "Rated Voltage Level", "", ControlColor::Emerald, tab4 ,&RatedVoltagelvlList);
  ESPUI.addControl( ControlType::Option, "Auto", "Auto", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "12v", "12v", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "24v", "24v", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "36v", "36v", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "48v", "48v", ControlColor::Alizarin, select3 );

  ESPUI.addControl( ControlType::Text, "Over Volt Dist", "", ControlColor::Emerald, tab4 ,&OverVoltDisttxt);
  ESPUI.addControl( ControlType::Text, "Over Volt Reconnect", "", ControlColor::Emerald, tab4 ,&OverVoltRecontxt);
  EQChargeVolt = ESPUI.addControl( ControlType::Text, "EQ Charge Volt", "", ControlColor::Emerald, tab4 ,&EQChargeVolttxt);
  ESPUI.addControl( ControlType::Text, "Boost Charge Volt", "", ControlColor::Emerald, tab4 ,&BoostChargeVolttxt);
  ESPUI.addControl( ControlType::Text, "Float Chrage Volt", "", ControlColor::Emerald, tab4 ,&FloatChargeVolttxt);
  ESPUI.addControl( ControlType::Text, "Boost Reconnect Charge Volt", "", ControlColor::Emerald, tab4 ,&BoostReconChargeVolttxt);
  ESPUI.addControl( ControlType::Text, "Battery Charge Percentage", "", ControlColor::Emerald, tab4 ,&BatteryChargePercenttxt);
  ChargeLimitVolt = ESPUI.addControl( ControlType::Text, "Charge Limit Volt", "", ControlColor::Emerald, tab4 ,&ChargeLimitVolttxt);
  ESPUI.addControl( ControlType::Text, "Discharge Limit Volt", "", ControlColor::Emerald, tab4 ,&DischargeLimitVolttxt);
  ESPUI.addControl( ControlType::Text, "Low Volt Disconnect", "", ControlColor::Emerald, tab4 ,&LowVoltDisconnecttxt);
  ESPUI.addControl( ControlType::Text, "Low Volt Reconnect", "", ControlColor::Emerald, tab4 ,&LowVoltReconnecttxt);
  ESPUI.addControl( ControlType::Text, "Under Volt Warning Volt", "", ControlColor::Emerald, tab4 ,&UnderVoltWarningVolttxt);
  ESPUI.addControl( ControlType::Text, "Under Volt Reconnect Volt", "", ControlColor::Emerald, tab4 ,&UnderVoltReconnectVolttxt);
  ESPUI.addControl( ControlType::Text, "Battery Discharge Percentage", "", ControlColor::Emerald, tab4 ,&BatteryDischargePercenttxt);
  ESPUI.addControl( ControlType::Text, "Boost Duration", "", ControlColor::Emerald, tab4 ,&BoostDurationtxt);
  ESPUI.addControl( ControlType::Text, "EQ Duration", "", ControlColor::Emerald, tab4 ,&EQDurationtxt);
  ESPUI.addControl( ControlType::Text, "Battery Capacity", "", ControlColor::Emerald, tab4 ,&BatteryCapactitytxt);
*/
  button1 = ESPUI.addControl( ControlType::Button, "Save Settings", "Save", ControlColor::Peterriver, tab3, &buttonCallback );
  
  //first parameter is name of access point, second is the password
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.autoConnect("RS485-WiFi");
  wifiManager.setConfigPortalTimeout(180);
  ESPUI.jsonInitialDocumentSize = 16000; // This is the default, adjust when you have too many widgets or options
  //ESPUI.begin("RS485-WiFi");
  setupGUI();

  LoadConfigFromEEPROM();
}

uint16_t ReadTegister(uint16_t Register) {
  // Read register at the address passed in
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(Register, 1);
  if (result == node.ku8MBSuccess)  {
    
    EQChargeVoltValue = node.getResponseBuffer(0);
    Serial.println(String(node.getResponseBuffer(0)));
  } else  {
    Serial.print("Miss read - "); 
    Serial.print(Register);
    Serial.print(", ret val:");
    Serial.println(result, HEX);
  }
  return result;
}

void ReadValues() {  
  // clear old data
  //
  memset(rtc.buf,0,sizeof(rtc.buf));
  memset(live.buf,0,sizeof(live.buf));
  memset(stats.buf,0,sizeof(stats.buf));

  // Read registers for clock
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readHoldingRegisters(RTC_CLOCK, RTC_CLOCK_CNT);
  if (result == node.ku8MBSuccess)  {

    rtc.buf[0]  = node.getResponseBuffer(0);
    rtc.buf[1]  = node.getResponseBuffer(1);
    rtc.buf[2]  = node.getResponseBuffer(2);
    
  } else {
    Serial.print("Miss read rtc-data, ret val:");
    Serial.println(result, HEX);
  } 

  // read LIVE-Data
  // 
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(LIVE_DATA, LIVE_DATA_CNT);

  if (result == node.ku8MBSuccess)  {

    for(i=0; i< LIVE_DATA_CNT ;i++) live.buf[i] = node.getResponseBuffer(i);
       
  } else {
    Serial.print("Miss read liva-data, ret val:");
    Serial.println(result, HEX);
  } 

 

  // Statistical Data
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(STATISTICS, STATISTICS_CNT);

  if (result == node.ku8MBSuccess)  {
    
    for(i=0; i< STATISTICS_CNT ;i++)  stats.buf[i] = node.getResponseBuffer(i);
    
  } else  {
    Serial.print("Miss read statistics, ret val:");
    Serial.println(result, HEX);
  } 

  // BATTERY_TYPE
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(BATTERY_TYPE, 1);
  if (result == node.ku8MBSuccess)  {
    
    BatteryType = node.getResponseBuffer(0);
    Serial.println(String(node.getResponseBuffer(0)));
  } else  {
    Serial.print("Miss read BATTERY_TYPE, ret val:");
    Serial.println(result, HEX);
  }

  // EQ_CHARGE_VOLT
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(EQ_CHARGE_VOLT, 1);
  if (result == node.ku8MBSuccess)  {
    
    EQChargeVoltValue = node.getResponseBuffer(0);
    Serial.println(String(node.getResponseBuffer(0)));
  } else  {
    Serial.print("Miss read EQ_CHARGE_VOLT, ret val:");
    Serial.println(result, HEX);
  }

  // CHARGING_LIMIT_VOLT
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(CHARGING_LIMIT_VOLT, 1);
  if (result == node.ku8MBSuccess)  {
    
    ChargeLimitVolt = node.getResponseBuffer(0);
    Serial.println(String(node.getResponseBuffer(0)));
  } else  {
    Serial.print("Miss read CHARGING_LIMIT_VOLT, ret val:");
    Serial.println(result, HEX);
  }
  

  
  // Capacity
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(BATTERY_CAPACITY, 1);
  if (result == node.ku8MBSuccess)  {
    
    BatteryCapactity = node.getResponseBuffer(0);
    Serial.println(String(node.getResponseBuffer(0)));
  } else  {
    Serial.print("Miss read BATTERY_CAPACITY, ret val:");
    Serial.println(result, HEX);
  }
  
  
  // Battery SOC
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(BATTERY_SOC, 1);
  if (result == node.ku8MBSuccess)  {
    
    batterySOC = node.getResponseBuffer(0);
    
  } else  {
    Serial.print("Miss read batterySOC, ret val:");
    Serial.println(result, HEX);
  }

  // Battery Net Current = Icharge - Iload
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(  BATTERY_CURRENT_L, 2);
  if (result == node.ku8MBSuccess)  {
    
    batteryCurrent = node.getResponseBuffer(0);
    batteryCurrent |= node.getResponseBuffer(1) << 16;
    
  } else  {
    Serial.print("Miss read batteryCurrent, ret val:");
    Serial.println(result, HEX);
  }
 
  if (!switch_load) {
  // State of the Load Switch
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readCoils(  LOAD_STATE, 1 );
  if (result == node.ku8MBSuccess)  {
    
    loadState = node.getResponseBuffer(0);
        
  } else  {
    Serial.print("Miss read loadState, ret val:");
    Serial.println(result, HEX);
  }
  }

  // Read Model
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(CCMODEL, 1);
  if (result == node.ku8MBSuccess)  {
    
    CCModel = node.getResponseBuffer(0);
    
  } else  {
    Serial.print("Miss read Model, ret val:");
    Serial.println(result, HEX);
  }
    
  // Read Status Flags
  //
  delay(200);
  node.clearResponseBuffer();
  result = node.readInputRegisters(  0x3200, 2 );
  if (result == node.ku8MBSuccess)  {

    uint16_t temp = node.getResponseBuffer(0);
    if (debug) Serial.print( "Batt Flags : "); Serial.println(temp);
    
    status_batt.volt = temp & 0b1111;
    status_batt.temp = (temp  >> 4 ) & 0b1111;
    status_batt.resistance = (temp  >>  8 ) & 0b1;
    status_batt.rated_volt = (temp  >> 15 ) & 0b1;
    
    temp = node.getResponseBuffer(1);
    if (debug) Serial.print( "Chrg Flags : "); Serial.println(temp, HEX);
   
    //charger_input     = ( temp & 0b0000000000000000 ) >> 15 ;
    charger_mode        = ( temp & 0b0000000000001100 ) >> 2 ;
    //charger_input     = ( temp & 0b0000000000000000 ) >> 12 ;
    //charger_operation = ( temp & 0b0000000000000000 ) >> 0 ;
    
    if (debug) Serial.print( "charger_mode  : "); Serial.println( charger_mode );
    
  } else  {
    Serial.print("Miss read ChargeState, ret val:");
    Serial.println(result, HEX);
  }
}

void preTransmission()
{
  digitalWrite(MAX485_RE, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
}

void debug_output(){
  //Output values to serial
  Serial.printf("\n\nTime:  20%02d-%02d-%02d   %02d:%02d:%02d   \n",  rtc.r.y , rtc.r.M , rtc.r.d , rtc.r.h , rtc.r.m , rtc.r.s  );
  
  Serial.print(  "\nLive-Data:           Volt        Amp       Watt  ");
      
  Serial.printf( "\n  Panel:            %7.3f    %7.3f    %7.3f ",  live.l.pV/100.f ,  live.l.pI/100.f ,  live.l.pP/100.0f );
  Serial.printf( "\n  Batt:             %7.3f    %7.3f    %7.3f ",  live.l.bV/100.f ,  live.l.bI/100.f ,  live.l.bP/100.0f );
  Serial.printf( "\n  Load:             %7.3f    %7.3f    %7.3f ",  live.l.lV/100.f ,  live.l.lI/100.f ,  live.l.lP/100.0f );
  Serial.println();
  Serial.printf( "\n  Battery Current:  %7.3f  A ",      batteryCurrent/100.f  );
  Serial.printf( "\n  Battery SOC:      %7.0f  %% ",     batterySOC/1.0f  );
  Serial.printf( "\n  Load Switch:          %s   ",     (loadState==1?" On":"Off") );


  Serial.print(  "\n\nStatistics:  ");
    
  Serial.printf( "\n  Panel:       min: %7.3f   max: %7.3f   V", stats.s.pVmin/100.f  , stats.s.pVmax/100.f  );
  Serial.printf( "\n  Battery:     min: %7.3f   max: %7.3f   V", stats.s.bVmin /100.f , stats.s.bVmax/100.f);
  Serial.println();
  Serial.printf( "\n  Consumed:    day: %7.3f   mon: %7.3f   year: %7.3f  total: %7.3f   kWh",
      stats.s.consEnerDay/100.f  ,stats.s.consEnerMon/100.f  ,stats.s.consEnerYear/100.f  ,stats.s.consEnerTotal/100.f   );
  Serial.printf( "\n  Generated:   day: %7.3f   mon: %7.3f   year: %7.3f  total: %7.3f   kWh",
      stats.s.genEnerDay/100.f   ,stats.s.genEnerMon/100.f   ,stats.s.genEnerYear/100.f   ,stats.s.genEnerTotal/100.f  );
  Serial.printf( "\n  CO2-Reduction:    %7.3f  t ",      stats.s.c02Reduction/100.f  );
  Serial.println();

  Serial.print(  "\nStatus:");
  Serial.printf( "\n    batt.volt:         %s   ",     batt_volt_status[status_batt.volt] );
  Serial.printf( "\n    batt.temp:         %s   ",     batt_temp_status[status_batt.temp] );
  Serial.printf( "\n    charger.charging:  %s   ",     charger_charging_status[ charger_mode] );
  Serial.println();
  Serial.println();
}

void loop(void) {
  // Print out to serial if debug is enabled.
  //
  if (debug) debug_output();

  ESPUI.updateControlValue(MQTTIP , String(myConfig.mqtt_server));
  ESPUI.updateControlValue(MQTTPORT , String(myConfig.mqtt_port));
  ESPUI.updateControlValue(MQTTUSER , String(myConfig.mqtt_username));
  ESPUI.updateControlValue(MQTTPASS , String(myConfig.mqtt_password));
  ESPUI.updateControlValue(MQTTTOPIC , String(myConfig.mqtt_topic));
  ESPUI.updateControlValue(MQTTEN , String(myConfig.MQTT_Enable));

  ESPUI.updateControlValue(INFLUXDBIP , String(myConfig.influxdb_host));
  ESPUI.updateControlValue(INFLUXDBPORT , String(myConfig.influxdb_httpPort));
  ESPUI.updateControlValue(INFLUXDBUSER , String(myConfig.influxdb_user));
  ESPUI.updateControlValue(INFLUXDBPASS , String(myConfig.influxdb_password));
  ESPUI.updateControlValue(INFLUXDBDB , String(myConfig.influxdb_database));
  ESPUI.updateControlValue(INFLUXDBEN , String(myConfig.influxdb_enabled));
  
  // Read Values from Charge Controller
  ReadValues();
  
  //Update ESPUI Live Data components  
  ESPUI.updateControlValue(LoadStatus , String(loadState==1?" On":"Off"));
  ESPUI.updateControlValue(DeviceTemp , String(batt_temp_status[status_batt.temp]));
  ESPUI.updateControlValue(SolarVoltage , String(live.l.pV/100.f)+"V");
  ESPUI.updateControlValue(SolarAmps , String(live.l.pI/100.f)+"A");
  ESPUI.updateControlValue(SolarWattage , String(live.l.pP/100.0f)+"w");
  ESPUI.updateControlValue(BatteryVoltage  , String(live.l.bV/100.f)+"V");
  ESPUI.updateControlValue(BatteryAmps , String(live.l.bI/100.f)+"A");
  ESPUI.updateControlValue(BatteryWattage , String(live.l.bP/100.0f)+"w");
  ESPUI.updateControlValue(LoadVoltage , String(live.l.lV/100.f)+"V");
  ESPUI.updateControlValue(LoadAmps , String(live.l.lI/100.f)+"a");
  ESPUI.updateControlValue(LoadWattage , String(live.l.lP/100.0f)+"w");
  ESPUI.updateControlValue(BatteryStateOC , String(batterySOC/1.0f)+"V");
  ESPUI.updateControlValue(ChargingStatus , String(charger_charging_status[ charger_mode]));
  ESPUI.updateControlValue(BatteryStatus , String(batt_volt_status[status_batt.volt]));
  ESPUI.updateControlValue(BatteryTemp , String(batt_temp_status[status_batt.temp]));
//  ESPUI.updateControlValue(Model, String(CCModel));

  //Update historical values
  ESPUI.updateControlValue(Maxinputvolttoday, String(stats.s.pVmax/100.f)+"V");
  ESPUI.updateControlValue(Mininputvolttoday , String(stats.s.pVmin/100.f)+"V");
  ESPUI.updateControlValue(MaxBatteryvolttoday , String(stats.s.bVmax/100.f)+"V");
  ESPUI.updateControlValue(MinBatteryvolttoday, String(stats.s.bVmin /100.f)+"V");
  ESPUI.updateControlValue(ConsumedEnergyToday , String(stats.s.consEnerDay/100.f)+" kWh");
  ESPUI.updateControlValue(ConsumedEnergyMonth , String(stats.s.consEnerMon/100.f)+" kWh");
  ESPUI.updateControlValue(ConsumedEngeryYear , String(stats.s.consEnerYear/100.f)+" kWh");
  ESPUI.updateControlValue(TotalConsumedEnergy , String(stats.s.consEnerTotal/100.f)+" kWh");
  ESPUI.updateControlValue(GeneratedEnergyToday , String(stats.s.genEnerDay/100.f)+" kWh");
  ESPUI.updateControlValue(GeneratedEnergyMonth , String(stats.s.genEnerMon/100.f)+" kWh");
  ESPUI.updateControlValue(GeneratedEnergyYear , String(stats.s.genEnerYear/100.f)+" kWh");
  ESPUI.updateControlValue(TotalGeneratedEnergy , String(stats.s.genEnerTotal/100.f)+" kWh");
  ESPUI.updateControlValue(Co2Reduction , String(stats.s.c02Reduction/100.f)+"t");
//  ESPUI.updateControlValue(NetBatteryCurrent , String(batterySOC/1.0f)+"v");
//  ESPUI.updateControlValue(AmbientTemp , String(batterySOC/1.0f)+"v");

  //Update controller settings tab
  //ESPUI.updateControlValue(EQChargeVolt , String(EQChargeVoltValue/100.0f)+"v");  
  //ESPUI.updateControlValue(ChargeLimitVolt , String(ChargeLimitVolt/100.0f)+"v");  
   
  //Serial.print("EQChargeVolt : ");
  //Serial.println(String(EQChargeVolt));
  
  //ESPUI.updateControlValue(BatteryTypeControl, String(batteryTYPE)); 
  if(millis() >= time_now + period){
  time_now += period;      
      
  if (myConfig.MQTT_Enable == 1) {
    // establish/keep mqtt connection
    //
    if (!mqtt_client.connected())
    { 
      mqtt_client.setServer(myConfig.mqtt_server, myConfig.mqtt_port);
      mqtt_client.setCallback(mqtt_callback);  
      mqtt_reconnect(); 
    }
  
    mqtt_publish();
    mqtt_client.loop();
  }

    // establish/keep influxdb connection
  if(myConfig.influxdb_enabled == 1) {
    Influxdb_postData(); 
  }

// Do the Switching of the Load here
//
  if( switch_load == 1 ){
    switch_load = 0;  
    Serial.print("Switching Load ");
    Serial.println( (loadState?"On":"Off") );

    delay(200);
    result = node.writeSingleCoil(0x0002, loadState);
    if (result != node.ku8MBSuccess)  {
      Serial.print("Miss write loadState, ret val:");
      Serial.println(result, HEX);
    } 
    
  }
  }
  // power down MAX485_DE
  digitalWrite(MAX485_RE, 0); // low active
  digitalWrite(MAX485_DE, 0);
}
