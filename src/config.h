uint8_t i, result;

// Pins
//
#define MAX485_DE         D2  // data or
#define MAX485_RE         D1  // recv enable

// ModBus Register Locations
//
#define LIVE_DATA       0x3100     // start of live-data 
#define LIVE_DATA_CNT   16         // 16 regs

// just for reference, not used in code
#define PANEL_VOLTS     0x00       
#define PANEL_AMPS      0x01
#define PANEL_POWER_L   0x02
#define PANEL_POWER_H   0x03

#define BATT_VOLTS      0x04
#define BATT_AMPS       0x05
#define BATT_POWER_L    0x06
#define BATT_POWER_H    0x07

#define LOAD_VOLTS      0x0C
#define LOAD_AMPS       0x0D
#define LOAD_POWER_L    0x0E
#define LOAD_POWER_H    0x0F

#define CCMODEL           0x2b

#define RTC_CLOCK           0x9013  // D7-0 Sec, D15-8 Min  :   D7-0 Hour, D15-8 Day  :  D7-0 Month, D15-8 Year
#define RTC_CLOCK_CNT       3       // 3 regs

#define BATTERY_SOC         0x311A  // State of Charge in percent, 1 reg

#define BATTERY_CURRENT_L   0x331B  // Battery current L
#define BATTERY_CURRENT_H   0x331C  // Battery current H


#define STATISTICS      0x3300 // start of statistical data
#define STATISTICS_CNT  22     // 22 regs

// just for reference, not used in code
#define PV_MAX     0x00 // Maximum input volt (PV) today  
#define PV_MIN     0x01 // Minimum input volt (PV) today
#define BATT_MAX   0x02 // Maximum battery volt today
#define BATT_MIN   0x03 // Minimum battery volt today

#define CONS_ENERGY_DAY_L   0x04 // Consumed energy today L
#define CONS_ENGERY_DAY_H   0x05 // Consumed energy today H
#define CONS_ENGERY_MON_L   0x06 // Consumed energy this month L 
#define CONS_ENGERY_MON_H   0x07 // Consumed energy this month H
#define CONS_ENGERY_YEAR_L  0x08 // Consumed energy this year L
#define CONS_ENGERY_YEAR_H  0x09 // Consumed energy this year H
#define CONS_ENGERY_TOT_L   0x0A // Total consumed energy L
#define CONS_ENGERY_TOT_H   0x0B // Total consumed energy  H

#define GEN_ENERGY_DAY_L   0x0C // Generated energy today L
#define GEN_ENERGY_DAY_H   0x0D // Generated energy today H
#define GEN_ENERGY_MON_L   0x0E // Generated energy this month L
#define GEN_ENERGY_MON_H   0x0F // Generated energy this month H
#define GEN_ENERGY_YEAR_L  0x10 // Generated energy this year L
#define GEN_ENERGY_YEAR_H  0x11 // Generated energy this year H
#define GEN_ENERGY_TOT_L   0x12 // Total generated energy L
#define GEN_ENERGY_TOT_H   0x13 // Total Generated energy  H

#define CO2_REDUCTION_L    0x14 // Carbon dioxide reduction L  
#define CO2_REDUCTION_H    0x15 // Carbon dioxide reduction H 

#define LOAD_STATE         0x02 // r/w load switch state

#define STATUS_FLAGS    0x3200
#define STATUS_BATTERY    0x00  // Battery status register
#define STATUS_CHARGER    0x01  // Charging equipment status register
#define BATTERY_TYPE      0x9000  // Battery Type 0001H- Sealed , 0002H- GEL, 0003H- Flooded, 0000H- User defined
#define BATTERY_CAPACITY  0x9001  // Rated capacity of the battery in Ah
#define HIGH_VOLTAGE_DISCONNECT 0x9003  //
#define CHARGING_LIMIT_VOLT     0x9004  //
#define EQ_CHARGE_VOLT          0x9006

uint16_t ErrorCounter=0;
uint16_t savestatustxt;
uint16_t Abouttxt;
uint16_t SaveButton;
uint16_t RebootButton;
uint16_t Model;
String CCModel;
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

uint16_t DEVICEID;
uint16_t DEVICEBAUD;

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

  // datastructures, also for buffer to values conversion
  //
  // clock
  union {
    struct {
    uint8_t  s;
    uint8_t  m;
    uint8_t  h;
    uint8_t  d;
    uint8_t  M;
    uint8_t  y;  
   } r;
    uint16_t buf[3];
  } rtc ;

  // live data
  union {
    struct {

      int16_t  pV;
      int16_t  pI;
      int32_t  pP;
   
      int16_t  bV;
      int16_t  bI;
      int32_t  bP;
      
      
      uint16_t  dummy[4];
      
      int16_t  lV;
      int16_t  lI;
      int32_t  lP; 

    } l;
    uint16_t  buf[16];
  } live;


  // statistics
  union {
    struct {
  
      // 4*1 = 4
      uint16_t  pVmax;
      uint16_t  pVmin;
      uint16_t  bVmax;
      uint16_t  bVmin;
  
      // 4*2 = 8
      uint32_t  consEnerDay;
      uint32_t  consEnerMon;
      uint32_t  consEnerYear;
      uint32_t  consEnerTotal;
  
      // 4*2 = 8
      uint32_t  genEnerDay;
      uint32_t  genEnerMon;
      uint32_t  genEnerYear;
      uint32_t  genEnerTotal;
  
      // 1*2 = 2
      uint32_t  c02Reduction;
     
    } s;
    uint16_t  buf[22];  
  } stats;

  // these are too far away for the union conversion trick
  uint16_t batterySOC = 0;
  int32_t batteryCurrent = 0;
    
  // battery status
  struct {
    uint8_t volt;        // d3-d0  Voltage:     00H Normal, 01H Overvolt, 02H UnderVolt, 03H Low Volt Disconnect, 04H Fault
    uint8_t temp;        // d7-d4  Temperature: 00H Normal, 01H Over warning settings, 02H Lower than the warning settings
    uint8_t resistance;  // d8     abnormal 1, normal 0
    uint8_t rated_volt;  // d15    1-Wrong identification for rated voltage
  } status_batt ;

  char batt_volt_status[][20] = {
    "Normal",
    "Overvolt",
    "Low Volt Disconnect",
    "Fault"
  };
  
  char batt_temp_status[][16] = {
    "Normal",
    "Over WarnTemp",
    "Below WarnTemp"
  };

  uint8_t charger_mode  = 0;
    
  char charger_charging_status[][12] = {
    "Off",
    "Float",
    "Boost",
    "Equlization"
  };

  //

  
