#include <Arduino.h>

// Program 'Esp32_Viessm_Analog_Meter' Branch Master
#define PROGRAMVERSION "v1.0.0"
// Last updated: 2025_03_08
// Copyright: RoSchmi 2024 License: Apache 2.0
// the App was tested only on ESP32 Dev Board, no attempts were made to run it 
// on variations of ESP32 or ESP8266

// Uses
// platform espressif32@6.7.0 and platform_package framework-arduinoespressif32 @ 3.20017.0
// Switched to built-in Filesystem 'LittleFS' instead of 'LITTLEFS'
//
// In cases of a wrong (old) format of the LittleFS flash storage,
// the flash must be cleared with the command (cmd window)
// python C:\Users\<user>\.platformio\packages\tool-esptoolpy\esptool.py erase_flash

// If you still have problems with formatting the flash for LittleFS, try to use
// https://github.com/RoSchmi/ESP32_LittleFS_Simple_Formatter

// Definition of your TimeZone has to be made in config.h
// Original Timezone_Generic library by Khoih-prog is not used
// (didn't take the time to understand) instead I used a modification
// which is part of this App (lib/RoSchmi/RSTimeZone/src)

// The application doesn't compile without a trick:
// The libraries NTPClient_Generic and Timezone_Generic load the
// dependency:
// {
//      "owner": "khoih-prog",
//      "name": "ESP8266_AT_WebServer",
//      "version": ">=1.5.4",
//      "platforms": ["*"]
//    },
// This dependency doesn't compile. So we delete in folder libdeps/ESP32 
// the dependency in the 'library.json' file and delete the entry 
// 'ESP8266_AT_WebServer' in the folder libdeps/ESP32

// This App is based on the software of my App https://github.com/RoSchmi/Esp32_WiFiManager_HeatingSurvey
// this App for Esp32 monitors the activity of the burner of an oil-heating
// (or any other noisy thing) by measuring the sound of e.g. the heating burner
// For details visit the Github Repo of this App 
//
// The Story:
// As I relocated to another city into a house with a Viessmann Vitodens 333F
// heating. For this heating it was easyly possible to mount a VITOCONNECT 100 OPTO1
// module which frequently sends sensor data of the heating via WiFi and internet
// to the Viessmann Cloud and makes it possible to control and monitor the heating 
// via the 'Vicare' App from Viessmann.
// Using this App one can see actual sensor data and change settings, but it is not
// possible to maintain historic sensor data and to display sensor data graphically 
// as a timeline graph.
// As Viessmann provides access to the actual heating sensor data via an API
// I decided to download the sensor data via the API around every minute 
// (1440 downloads per day are free) and store the values of several sensors
// on Azure Storage Tables. This is not free but very cheap (only several cents
// per month) but requires a Microsoft Azure Account.
// So I made this App which loads the sensor data from the Viessmann Cloud
// and stores the data in Azure Storage Tables in a certain way.
// To display these data graphically on an Apple iPhone I use the iPhone App
// 'Charts4Azure' (Android version and a Microsoft Store App with minor functionality
// exist as well). The App 'Charts4Azure' allways displays the combination of
// 4 analog timeline graphs (e.g. 4 temperature sensors) and 4 On/Off timeline graphs 
// (e.g. for burner or pump activity) on one page.

// For this Esp App Router-WiFi Credentials, Azure Credentials and Viessmann Credentials
// can be entered via a Captive Portal page which is provided for one minute
// by the Esp32 after powering up the device. After having entered the credentials
// one time they stay permanently on the Esp32 and need not to be entered every time.

// This App uses an adaption of the 'Async_ConfigOnStartup.ino' as WiFi Mangager
// from: -https://github.com/khoih-prog/ESPAsync_WiFiManager 
// More infos about the functions and Licenses see:
// -https://github.com/khoih-prog/ESPAsync_WiFiManager/blob/master/examples/Async_ConfigOnDoubleReset_Multi
// -https://github.com/khoih-prog/ESPAsync_WiFiManager/tree/master/examples/Async_ConfigOnStartup
 

// The WiFiManager will open a configuration portal in the WLAN Access-Point for 60 seconds when powered up 
// if the boards has stored WiFi Credentials, otherwise, the Access Point will stay indefinitely in ConfigPortal 
// until the Router WiFi Credentials and other Credentials are entered.
// When the Esp32 starts up it provides an Access Point after about one minute to which you must connect
// with your iPhone. The SSID of this Access point is like e.g. ESP32_xxxxxx, if a password is requested 
// (only the first time), the passwort is "My" + the SSID-Name, e.g. MyESP32_xxxxxx.
// Some settings for the Esp32 App have to be made in the files config.h and config_secret.h.
// The special config_secret.h for your system must be provided by copying and renaming
// of config_secret_template.h to config_secret.h

// To access the Viessmann Api through this Esp32 App you to need a Client-Id
// and a 'Refresh Token' which have to be entered in config_secret.h or
// in the Captive Portal Page. The Client-Id and an Access-Token (valid for
// 1 hour can be retrieved from: https://developer.viessmann.com/start/pricing.html
// (Basic -> Get started)
// To get a Refresh-Token you can use my C# Windows Application:
// https://github.com/RoSchmi/RoSchmiViessmannApiTest
// Valueble info can be found here: 
// https://www.rustimation.eu/index.php/a-viessmann-api-und-node-red/

// How to get an Azure Account and there a Storage Account have a
// look here: https://azureiotcharts.home.blog/getting-started-guide/

 
#include <Arduino.h>
#include <vector>
#include <time.h>
#include "ESPAsyncWebServer.h"
#include "defines.h"
#include "config.h"
#include "config_secret.h"
#include "DateTime.h"
#include <freertos/FreeRTOS.h>
#include "Esp.h"
#include "esp_task_wdt.h"
#include "rom/rtc.h"


#include "CloudStorageAccount.h"
#include "TableClient.h"
#include "TableEntityProperty.h"
#include "TableEntity.h"
#include "AnalogTableEntity.h"
#include "OnOffTableEntity.h"

#include "ViessmannApiAccount.h"
#include "ViessmannClient.h"
#include "ViessmannApiSelection.h"

#include "RestApiAccount.h"
#include "AiOnTheEdgeClient.h"
#include "AiOnTheEdgeApiSelection.h"

#include "NTPClient_Generic.h"
#include "Timezone_Generic.h"
#include "WiFiUdp.h"
#include "WiFiClient.h"
#include "HTTPClient.h"

#include "DataContainerWio.h"
#include "OnOffDataContainerWio.h"
#include "OnOffSwitcherWio.h"
#include "SoundSwitcher.h"
#include "ImuManagerWio.h"
#include "AnalogSensorMgr.h"
#include "OnOffSensor.h"

#include "azure/core/az_platform.h"
#include "azure/core/az_http.h"
#include "azure/core/az_http_transport.h"
#include "azure/core/az_result.h"
#include "azure/core/az_config.h"
#include "azure/core/az_context.h"
#include "azure/core/az_span.h"

#include "Rs_TimeNameHelper.h"

// Now support ArduinoJson 6.0.0+ ( tested with v6.14.1 )
#include "ArduinoJson.h"      // get it from https://arduinojson.org/ or install via Arduino library manager

// Default Esp32 stack size of 8192 byte is not enough for this application.
// --> configure stack size dynamically from code to 16384
// https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/4

SET_LOOP_TASK_STACK_SIZE ( 16*1024 ); // 16KB

//SET_LOOP_TASK_STACK_SIZE ( 32*1024 ); // 32KB  used this

// Allocate memory space in memory segment .dram0.bss, ptr to this memory space is later
// passed to TableClient (is used there as the place for some buffers to preserve stack )

//const uint16_t bufferStoreLength = 4000;
//const uint16_t bufferStoreLength = 20000;
const uint16_t bufferStoreLength = 10000; // Test 06.04.2025)

uint8_t bufferStore[bufferStoreLength] {0};
uint8_t * bufferStorePtr = &bufferStore[0];

char viessmannClientId[50] = VIESSMANN_CLIENT_ID;
char viessmannAccessToken[1120] = VIESSMANN_ACCESS_TOKEN;
char viessmannRefreshToken[60] = VIESSMANN_REFRESH_TOKEN;
char viessmannUserBaseUri[60] = VIESSMANN_USER_BASE_URI;
// RoSchmi delete next 2 lines ?
char viessmannIotBaseUri[60] = VIESSMANN_IOT_BASE_URI;
char viessmannTokenBaseUri[60] = VIESSMANN_TOKEN_BASE_URI;

DateTime AccessTokenRefreshTime = DateTime();
TimeSpan AccessTokenRefreshInterval = TimeSpan(VIESSMANN_TOKEN_REFRESH_INTERVAL_SECONDS);

ViessmannApiAccount myViessmannApiAccount(viessmannClientId, viessmannAccessToken, viessmannIotBaseUri, viessmannUserBaseUri, viessmannTokenBaseUri, true); 
ViessmannApiAccount * myViessmannApiAccountPtr = &myViessmannApiAccount;

ViessmannApiSelection viessmannApiSelection(DateTime(), TimeSpan(VIESSMANN_API_READ_INTERVAL_SECONDS));
ViessmannApiSelection * viessmannApiSelectionPtr = &viessmannApiSelection;

RestApiAccount gasmeterApiAccount("gasmeter", "","gasmeter", false);
//RestApiAccount * gasmeterApiAccountPtr = &gasmeterApiAccount;

AiOnTheEdgeApiSelection gasmeterApiSelection(DateTime(), TimeSpan(GASMETER_AI_API_READ_INTERVAL_SECONDS));
AiOnTheEdgeApiSelection * gasmeterApiSelectionPtr = &gasmeterApiSelection;

bool viessmannUserId_is_read = false;
const uint16_t viessmannUserBufLen = 1000;
uint8_t viessmannApiUser [viessmannUserBufLen] {0};

bool viessmannEquip_is_read = false;
uint32_t Data_0_Id = 0;
const int equipBufLen = 35;
char Data_0_Description[equipBufLen]= {0};
char Data_0_Address_Street[equipBufLen] {0};
char Data_0_Address_HouseNumber[equipBufLen] = {0};
char Gateways_0_Serial[equipBufLen] = {0};
char Gateways_0_Devices_0_Id[equipBufLen] = {0};

#define VI_FEATURES_COUNT 16
ViessmannApiSelection::Feature features[VI_FEATURES_COUNT];

#define AI_FEATURES_COUNT 6
AiOnTheEdgeApiSelection::Feature ai_features[AI_FEATURES_COUNT];

bool isFirstGasmeterRead = true;

#define IS_ACTIVE true
OnOffSensor OnOffBurnerStatus(IS_ACTIVE, false, true, true, DateTime());
OnOffSensor OnOffCirculationPumpStatus(IS_ACTIVE, false, true, true, DateTime());
OnOffSensor OnOffHotWaterCircualtionPumpStatus(IS_ACTIVE);
OnOffSensor OnOffHotWaterPrimaryPumpStatus(IS_ACTIVE);

//void * StackPtrAtStart;
//void * StackPtrEnd;
UBaseType_t * StackPtrAtStart;
UBaseType_t * StackPtrEnd;

UBaseType_t watermarkStart;
TaskHandle_t taskHandle_0 =  xTaskGetCurrentTaskHandleForCPU(0);
TaskHandle_t taskHandle_1 =  xTaskGetCurrentTaskHandleForCPU(1);

bool watchDogEnabled = false;
bool watchDogCommandSuccessful = false;

RESET_REASON resetReason_0;
RESET_REASON resetReason_1;

uint8_t lastResetCause = 0;

#define GPIOPin 0
bool buttonPressed = false;

const char analogTableName[45] = ANALOG_TABLENAME;
const char viessmAnalogTableName_01[45] = VIESSMANN_ANALOG_TABLENAME_01;

const char OnOffTableName_1[45] = ON_OFF_TABLENAME_01;
const char OnOffTableName_2[45] = ON_OFF_TABLENAME_02;
const char OnOffTableName_3[45] = ON_OFF_TABLENAME_03;
const char OnOffTableName_4[45] = ON_OFF_TABLENAME_04;

// The PartitionKey for the analog table may have a prefix to be distinguished, here: "Y2_" 
const char * analogTablePartPrefix = (char *)ANALOG_TABLE_PART_PREFIX;

// The PartitionKey for the On/Off-tables may have a prefix to be distinguished, here: "Y3_" 
const char * onOffTablePartPrefix = (char *)ON_OFF_TABLE_PART_PREFIX;

// The PartitinKey can be augmented with a string representing year and month (recommended)
const bool augmentPartitionKey = true;

// The TableName can be augmented with the actual year (recommended)
const bool augmentTableNameWithYear = true;

typedef const char* X509Certificate;

typedef int t_httpCode;

t_httpCode httpCode = -1;


// https://techcommunity.microsoft.com/t5/azure-storage/azure-storage-tls-critical-changes-are-almost-here-and-why-you/ba-p/2741581
// baltimore_root_ca will expire in 2025, then take digicert_globalroot_g2_ca
//X509Certificate myX509Certificate = baltimore_root_ca;

X509Certificate myX509Certificate = digicert_globalroot_g2_ca;

// Init the Secure client object

#if TRANSPORT_PROTOCOL == 1
    static WiFiClientSecure wifi_client;     
#else
    static WiFiClient wifi_client;
#endif

static WiFiClientSecure secure_wifi_client;
static WiFiClient plain_wifi_client;


  // A UDP instance to let us send and receive packets over UDP
  WiFiUDP ntpUDP;
  static NTPClient timeClient(ntpUDP);
  
  HTTPClient http;
  
  // Ptr to HTTPClient
  static HTTPClient * httpPtr = &http;

// Define Datacontainer with SendInterval and InvalidateInterval as defined in config.h
int sendIntervalSeconds_Vi = (SENDINTERVAL_MINUTES_VI * 60) < 1 ? 1 : (SENDINTERVAL_MINUTES_VI * 60);
int sendIntervalSeconds_Ai = (SENDINTERVAL_MINUTES_AI * 60) < 1 ? 1 : (SENDINTERVAL_MINUTES_AI * 60);

DataContainerWio dataContainer(TimeSpan(sendIntervalSeconds_Ai), TimeSpan(0, 0, INVALIDATEINTERVAL_MINUTES % 60, 0), (float)MIN_DATAVALUE_AI, (float)MAX_DATAVALUE_AI, (float)MAGIC_NUMBER_INVALID);

DataContainerWio dataContainerAnalogViessmann01(TimeSpan(sendIntervalSeconds_Vi), TimeSpan(0, 0, INVALIDATEINTERVAL_MINUTES % 60, 0), (float)MIN_DATAVALUE_VI, (float)MAX_DATAVALUE_VI, (float)MAGIC_NUMBER_INVALID);

AnalogSensorMgr analogSensorMgr(MAGIC_NUMBER_INVALID);

AnalogSensorMgr analogSensorMgr_Api_01(MAGIC_NUMBER_INVALID);

OnOffDataContainerWio onOffDataContainer;
 
OnOffSwitcherWio onOffSwitcherWio;

// Possible configuration for Adafruit Huzzah Esp32
static const i2s_pin_config_t pin_config_Adafruit_Huzzah_Esp32 = {
    .bck_io_num = 14,                   // BCKL
    .ws_io_num = 15,                    // LRCL
    .data_out_num = I2S_PIN_NO_CHANGE,  // not used (only for speakers)
    .data_in_num = 32                   // DOUT
};
// Possible configuration for some Esp32 DevKitC V4
static const i2s_pin_config_t pin_config_Esp32_dev = {
    .bck_io_num = 26,                   // BCKL
    .ws_io_num = 25,                    // LRCL
    .data_out_num = I2S_PIN_NO_CHANGE,  // not used (only for speakers)
    .data_in_num = 22                   // DOUT
};

MicType usedMicType = (USED_MICROPHONE == 0) ? MicType::SPH0645LM4H : MicType::INMP441;

SoundSwitcher soundSwitcher(pin_config_Esp32_dev, usedMicType);
//SoundSwitcher soundSwitcher(pin_config_Adafruit_Huzzah_Esp32, usedMicType);

FeedResponse feedResult;

int soundSwitcherUpdateInterval = SOUNDSWITCHER_UPDATEINTERVAL;
uint32_t soundSwitcherReadDelayTime = SOUNDSWITCHER_READ_DELAYTIME;

uint64_t loopCounter = 0;
int insertCounterAnalogTable = 0;
int insertCounterApiAnalogTable01 = 0;

uint32_t tryUploadCounter = 0;
uint32_t failedUploadCounter = 0;
uint32_t timeNtpUpdateCounter = 0;

uint32_t loadViFeaturesCount = 0;
uint32_t loadViFeaturesResp400Count = 0;
uint32_t loadViFeaturesRespOtherCount = 0;

uint32_t loadGasMeterJsonCount = 0;
uint32_t loadRefreshTokenCount = 0;

// not used on Esp32
int32_t sysTimeNtpDelta = 0;

  bool ledState = false;
  
  const int timeZoneOffset = (int)TIMEZONEOFFSET;
  const int dstOffset = (int)DSTOFFSET;

  Rs_TimeNameHelper timeNameHelper;

  DateTime dateTimeUTCNow;    // Seconds since 2000-01-01 08:00:00
  DateTime localTime;
  TimeSpan timeDiffUtcToLocal;

  Timezone myTimezone;

// Set transport protocol as defined in config.h
static bool UseHttps_State = TRANSPORT_PROTOCOL == 0 ? false : true;


const char * CONFIG_FILE = "/ConfigSW.json";    // Configuration for Azure and threshold
                                                // 'CONFIG_FILENAME' is used for Router Credentials

#define AZURE_CONFIG_ACCOUNT_NAME "AzureStorageAccName"

#define AZURE_CONFIG_ACCOUNT_KEY   "YourStorageAccountKey"

// Parameter for WiFi-Manager
char azureAccountName[20] =  AZURE_CONFIG_ACCOUNT_NAME;
char azureAccountKey[90] =  AZURE_CONFIG_ACCOUNT_KEY;
char sSwiThresholdStr[6] = SOUNDSWITCHER_THRESHOLD;

#define AzureAccountName_Label "azureAccountName"
#define AzureAccountKey_Label "azureAccountKey"
#define ViessmannClientId_Label "viessmannClientId"
#define ViessmannRefreshToken_Label "viessmannRefreshToken"
#define SoundSwitcherThresholdString_Label "sSwiThresholdStr"

// for Ai-on-the-edge-devices
char GasMeterAccountName[20] =  "gasmeter";
char GasMeterHostName[20] = "gasmeter";

char WaterMeterAccountName[20] =  "watermeter";
char WaterMeterHostName[20] = "watermeter";



// Function Prototypes for WiFiManager
bool loadApplConfigData();
bool saveApplConfigData();

// Note: AzureAccountName and azureccountKey will be changed later in setup
CloudStorageAccount myCloudStorageAccount(azureAccountName, azureAccountKey, UseHttps_State);
CloudStorageAccount * myCloudStorageAccountPtr = &myCloudStorageAccount;

RestApiAccount gasMeterAccount(GasMeterAccountName, "", GasMeterHostName, false);
//RestApiAccount * gasMeterAccountPtr = &gasMeterAccount; 


void GPIOPinISR()
{
  buttonPressed = true;
}

// function forward declarations
void trimLeadingSpaces(char * workstr);
ViessmannApiSelection::Feature ReadViessmannApi_Analog_01(int pSensorIndex, const char * pSensorName);
AiOnTheEdgeApiSelection:: Feature ReadAiOnTheEdgeApi_Analog_01(int pSensorIndex, const char * pSensorName, AiOnTheEdgeApiSelection * pAiOnTheEdgeApiSelectionPtr);
t_httpCode refreshAccessTokenFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr, const char * refreshToken); 
t_httpCode readViessmannFeaturesFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr, uint32_t Data_0_Id, const char * p_gateways_0_serial, const char * p_gateways_0_devices_0_id, ViessmannApiSelection * apiSelectionPtr);
t_httpCode readEquipmentFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr, uint32_t * p_data_0_id, const int equipBufLen, char * p_data_0_description, char * p_data_0_address_street, char * p_data_0_address_houseNumber, char * p_gateways_0_serial, char * p_gateways_0_devices_0_id);
t_httpCode readUserFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr);
t_httpCode readJsonFromRestApi(X509Certificate pCaCert, const char * pUrl, int pMaxUrlLength, AiOnTheEdgeApiSelection * apiSelectionPtr);
t_httpCode setAiPreValueViaRestApi(X509Certificate pCaCert, const char * pUrl, int pUrlMaxlength, const char * pPreValue);

void print_reset_reason(RESET_REASON reason);
void scan_WIFI();
String floToStr(float value);
bool isValidFloat(const char* str);
//float ReadAnalogSensor_01(int pSensorIndex);
ValueStruct ReadAnalogSensorStruct_01(int pSensorIndex);
void createSampleTime(DateTime dateTimeUTCNow, int timeZoneOffsetUTC, char * sampleTime);
az_http_status_code  createTable(CloudStorageAccount * myCloudStorageAccountPtr, X509Certificate pCaCert, const char * tableName);
az_http_status_code insertTableEntity(CloudStorageAccount *myCloudStorageAccountPtr,X509Certificate pCaCert, const char * pTableName, TableEntity pTableEntity, char * outInsertETag);
void makePartitionKey(const char * partitionKeyprefix, bool augmentWithYear, DateTime dateTime, az_span outSpan, size_t *outSpanLength);
void makeRowKey(DateTime actDate, az_span outSpan, size_t *outSpanLength);
int getDayNum(const char * day);
int getMonNum(const char * month);
int getWeekOfMonthNum(const char * weekOfMonth);
uint8_t connectMultiWiFi();

// Here: Begin of WiFi Manager definitions
//***************************************************************

#if !( defined(ESP8266) ||  defined(ESP32) )
  #error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#define ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET     "ESPAsync_WiFiManager v1.10.1"

// Definition of Debug Level of EspAsync_WiFiManaer (defined in config.h)
// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#ifndef _ESPASYNC_WIFIMGR_LOGLEVEL_
  #define _ESPASYNC_WIFIMGR_LOGLEVEL_ 0
#endif

//Ported to ESP32
#ifdef ESP32
  #include <esp_wifi.h>
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WiFiMulti.h>

  WiFiMulti wifiMulti;
  
  // Definitions of the Filesystem used on ESP32 (e.g. for storing Router Credentials)
  // LittleFS has higher priority than SPIFFS
  #if ( ARDUINO_ESP32C3_DEV )
    // Currently, ESP32-C3 only supporting SPIFFS and EEPROM. Will fix to support LittleFS
    #define USE_LITTLEFS          false
    #define USE_SPIFFS            true
  #else
    #define USE_LITTLEFS    true
    #define USE_SPIFFS      false
  #endif

  #if USE_LITTLEFS
    // Use LittleFS
    
    //Former: #include <LITTLEFS.h>             // https://github.com/lorol/LITTLEFS
    #include <LittleFS.h>
    
    FS* filesystem =      &LittleFS;
    #define FileFS        LittleFS
    #define FS_Name       "LittleFS"
    
  #elif USE_SPIFFS
    #include <SPIFFS.h>
    FS* filesystem =      &SPIFFS;
    #define FileFS        SPIFFS
    #define FS_Name       "SPIFFS"
  #else
    // Use FFat
    #include <FFat.h>
    FS* filesystem =      &FFat;
    #define FileFS        FFat
    #define FS_Name       "FFat"
  #endif
  //////

  #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

  #define LED_BUILTIN       2
  //#define LED_BUILTIN       14
  #define LED_ON            HIGH
  #define LED_OFF           LOW

#else   // (Not Esp32)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
  //needed for library
  #include <DNSServer.h>

  // From v1.1.0
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;

  #define USE_LITTLEFS      true
  
  #if USE_LITTLEFS
    #include <LittleFS.h>
    FS* filesystem = &LittleFS;
    #define FileFS    LittleFS
    #define FS_Name       "LittleFS"
  #else
    FS* filesystem = &SPIFFS;
    #define FileFS    SPIFFS
    #define FS_Name       "SPIFFS"
  #endif
  //////
  
  #define ESP_getChipId()   (ESP.getChipId())
  
  #define LED_ON      LOW
  #define LED_OFF     HIGH
#endif

// SSID and PW for Config Portal
String ssid = "ESP_" + String(ESP_getChipId(), HEX);
String password;

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM       true
#define FORMAT_FILESYSTEM         false

#define MIN_AP_PASSWORD_SIZE    8

#define SSID_MAX_LEN            32
//WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64

/*
typedef struct
{
  float displayValue;
  float unClippedValue;
}  ValueSet;
*/
typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

#define NUM_WIFI_CREDENTIALS      2 // Esp32 can switch automatically between
                                    // multiple Router SSIDs,

// Assuming max 49 chars
#define TZNAME_MAX_LEN            50
#define TIMEZONE_MAX_LEN          50

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
  char TZ_Name[TZNAME_MAX_LEN];     // "America/Toronto"
  char TZ[TIMEZONE_MAX_LEN];        // "EST5EDT,M3.2.0,M11.1.0"
  uint16_t checksum;
} WM_Config;

WM_Config         WM_config;

WM_Config         Loaded_WM_config;

// Name of file for Router Credentials
#define  CONFIG_FILENAME              F("/wifi_cred.dat")
//////

// Indicates whether ESP has WiFi credentials saved from previous session
// Is set to true if WiFi-Credentials could be read from SPIFFS/LittleFS
bool initialConfig = false;

// Use false if you don't like to display Available Pages in Information Page of Config Portal
// Comment out or use true to display Available Pages in Information Page of Config Portal
// Must be placed before #include <ESP_WiFiManager.h>
#define USE_AVAILABLE_PAGES     true

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal from sketch. Valid only if DHCP is used.
// You'll loose the feature of dynamically changing from DHCP to static IP, or vice versa
// You have to explicitly specify false to disable the feature.
#define USE_STATIC_IP_CONFIG_IN_CP          false

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP     false

// Just use enough to save memory. On ESP8266, can cause blank ConfigPortal screen
// if using too much memory
#define USING_AFRICA        false
#define USING_AMERICA       false
#define USING_ANTARCTICA    false
#define USING_ASIA          false
#define USING_ATLANTIC      false
#define USING_AUSTRALIA     false
#define USING_EUROPE        true
#define USING_INDIAN        false
#define USING_PACIFIC       false
#define USING_ETC_GMT       false

// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP          false

// New in v1.0.11
#define USING_CORS_FEATURE          true
//////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
  #if defined(USE_DHCP_IP)
    #undef USE_DHCP_IP
  #endif
  #define USE_DHCP_IP     true
#else
  // You can select DHCP or Static IP here
  
  #define USE_DHCP_IP     false
#endif

#if ( USE_DHCP_IP )
  // Use DHCP
  //#warning Using DHCP IP
  IPAddress stationIP   = IPAddress(0, 0, 0, 0);
  IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#else
  // Use static IP
  #warning Using static IP
  
  #ifdef ESP32
    IPAddress stationIP   = IPAddress(192, 168, 2, 232);
  #else
    IPAddress stationIP   = IPAddress(192, 168, 2, 186);
  #endif
  
  IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

#define USE_CONFIGURABLE_DNS      true

IPAddress dns1IP      = gatewayIP;
IPAddress dns2IP      = IPAddress(8, 8, 8, 8);

#define USE_CUSTOM_AP_IP          false

IPAddress APStaticIP  = IPAddress(192, 168, 100, 1);
IPAddress APStaticGW  = IPAddress(192, 168, 100, 1);
IPAddress APStaticSN  = IPAddress(255, 255, 255, 0);

#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager

#define HTTP_PORT     80

// Onboard LED I/O pin on NodeMCU board
const int PIN_LED = 2; // D4 on NodeMCU and WeMos. GPIO2/ADC12 of ESP32. Controls the onboard LED.

///////////////////////////////////////////
// New in v1.4.0
/******************************************
 * // Defined in ESPAsync_WiFiManager.h
typedef struct
{
  IPAddress _ap_static_ip;
  IPAddress _ap_static_gw;
  IPAddress _ap_static_sn;

}  WiFi_AP_IPConfig;

typedef struct
{
  IPAddress _sta_static_ip;
  IPAddress _sta_static_gw;
  IPAddress _sta_static_sn;
#if USE_CONFIGURABLE_DNS  
  IPAddress _sta_static_dns1;
  IPAddress _sta_static_dns2;
#endif
}  WiFi_STA_IPConfig;
******************************************/

WiFi_AP_IPConfig  WM_AP_IPconfig;        // For Access Point
WiFi_STA_IPConfig WM_STA_IPconfig;       // For Router STA connection

// Functions

void initAPIPConfigStruct(WiFi_AP_IPConfig &in_WM_AP_IPconfig)
{
  in_WM_AP_IPconfig._ap_static_ip   = APStaticIP;
  in_WM_AP_IPconfig._ap_static_gw   = APStaticGW;
  in_WM_AP_IPconfig._ap_static_sn   = APStaticSN;
}

void initSTAIPConfigStruct(WiFi_STA_IPConfig &in_WM_STA_IPconfig)
{
  in_WM_STA_IPconfig._sta_static_ip   = stationIP;
  in_WM_STA_IPconfig._sta_static_gw   = gatewayIP;
  in_WM_STA_IPconfig._sta_static_sn   = netMask;
#if USE_CONFIGURABLE_DNS  
  in_WM_STA_IPconfig._sta_static_dns1 = dns1IP;
  in_WM_STA_IPconfig._sta_static_dns2 = dns2IP;
#endif
}

void displayIPConfigStruct(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
  LOGERROR3(F("stationIP ="), in_WM_STA_IPconfig._sta_static_ip, ", gatewayIP =", in_WM_STA_IPconfig._sta_static_gw);
  LOGERROR1(F("netMask ="), in_WM_STA_IPconfig._sta_static_sn);
#if USE_CONFIGURABLE_DNS
  LOGERROR3(F("dns1IP ="), in_WM_STA_IPconfig._sta_static_dns1, ", dns2IP =", in_WM_STA_IPconfig._sta_static_dns2);
#endif
}

void configWiFi(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
  #if USE_CONFIGURABLE_DNS  
    // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
    WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn, in_WM_STA_IPconfig._sta_static_dns1, in_WM_STA_IPconfig._sta_static_dns2);  
  #else
    // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
    WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn);
  #endif 
}

///////////////////////////////////////////

void toggleLED()
{
  //toggle state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

#if USE_ESP_WIFIMANAGER_NTP

void printLocalTime()
{
#if ESP8266
  static time_t now;
  
  now = time(nullptr);
  
  if ( now > 1451602800 )
  {
    Serial.print("Local Date/Time: ");
    Serial.print(ctime(&now));
  }
#else
  struct tm timeinfo;

  getLocalTime( &timeinfo );

  // Valid only if year > 2000. 
  // You can get from timeinfo : tm_year, tm_mon, tm_mday, tm_hour, tm_min, tm_sec
  if (timeinfo.tm_year > 100 )
  {
    Serial.print("Local Date/Time: ");
    Serial.print( asctime( &timeinfo ) );
  }
#endif
}

#endif

void heartBeatPrint()
{
#if USE_ESP_WIFIMANAGER_NTP
  printLocalTime();
#else
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
  {
    //Serial.print(F("H"));        // H means connected to WiFi
  }
  else
  {
    //Serial.print(F("F"));        // F means not connected to WiFi
  }

  if (num == 80)
  {
    //Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    //Serial.print(F(" "));
  }
#endif  
}

void check_WiFi()
{
  if ( (WiFi.status() != WL_CONNECTED) )
  {
    Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
    connectMultiWiFi();
  }
}  

// From example Async_ConfigOnDoubleReset_Multi.cpp
void check_status()
{
  static ulong checkstatus_timeout  = 0;
  static ulong checkwifi_timeout    = 0;

  static ulong current_millis;

#define WIFICHECK_INTERVAL    1000L

#if USE_ESP_WIFIMANAGER_NTP
#define HEARTBEAT_INTERVAL    60000L
#else
#define HEARTBEAT_INTERVAL    10000L
#endif

  current_millis = millis();

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }
}


int calcChecksum(uint8_t* address, uint16_t sizeToCalc)
{
  uint16_t checkSum = 0;
  
  for (uint16_t index = 0; index < sizeToCalc; index++)
  {
    checkSum += * ( ( (byte*) address ) + index);
  }

  return checkSum;
}

bool loadWiFiConfigData()    // Load configuration to access Router WiFi Credentials from filesystem
{
  File file = FileFS.open(CONFIG_FILENAME, "r");
  LOGERROR(F("LoadWiFiCfgFile "));
  
  // Set content of the structs to 0
  memset((void *) &WM_config,       0, sizeof(WM_config));
  memset((void *) &WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
  
  if (file)
  {
    file.readBytes((char *) &WM_config,   sizeof(WM_config));

    // RoSchmi 12.07.2024
    /*
    String theSSID = (char *) &WM_config.WiFi_Creds[0].wifi_ssid;
    String theSSID1 = (char *) &WM_config.WiFi_Creds[1].wifi_ssid;
    Serial.println("");
    Serial.println("*******************");
    Serial.println(theSSID);
    Serial.println(theSSID1);
    Serial.println("*******************");
    */

    file.readBytes((char *) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
     
    file.close();
    LOGERROR(F("OK"));

    if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) )
    {
      LOGERROR(F("WM_config checksum wrong"));  
      return false;
    }
    
    displayIPConfigStruct(WM_STA_IPconfig);
   
    return true;
  }
  else
  {
    LOGERROR(F("failed"));

    return false;
  }
}

void saveWiFiConfigData()   // Save Router WiFi-Credentials to file
{
  File file = FileFS.open(CONFIG_FILENAME, "w");
  LOGERROR(F("SaveWiFiCfgFile "));

  if (file)
  {
    WM_config.checksum = calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) );   
    file.write((uint8_t*) &WM_config, sizeof(WM_config));
    displayIPConfigStruct(WM_STA_IPconfig);

    file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    
    file.close();
    LOGERROR(F("OK"));
  }
  else
  {
    LOGERROR(F("failed"));
  }
}   // end saveWiFiConfigData

bool loadApplConfigData()    // Config parameter for Azure credentials, Viessmann Refresh-Token and threshold
{
  // this opens the config file in read-mode
  File f = FileFS.open(CONFIG_FILE, "r");

  if (!f)
  {
    Serial.println(F("Configuration file not found"));
    return false;
  }
  else
  { 
    // we could open the file
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size + 1]);

    // Read and store file contents in buf
    f.readBytes(buf.get(), size);
    // Closing file
    f.close();

    // Using dynamic JSON buffer which is not the recommended memory model, but anyway
    // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model
#if (ARDUINOJSON_VERSION_MAJOR >= 6)
    
    DynamicJsonDocument json(1024);
    auto deserializeError = deserializeJson(json, buf.get());
    if ( deserializeError )
    {
      Serial.println(F("JSON parseObject() failed"));
      return false;
    }
    Serial.println(F("Here we could print the custom parameters like Azure Credentials read from filesystem"));
    //serializeJson(json, Serial);
#else
    DynamicJsonBuffer jsonBuffer;
    // Parse JSON string
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    // Test if parsing succeeds.
    if (!json.success())
    {
      Serial.println(F("JSON parseObject() failed"));
      return false;
    }
    json.printTo(Serial);
#endif

    // Parse all config file parameters, override
    // local config variables with parsed values    
    if (json.containsKey(AzureAccountName_Label))
    {
      strcpy(azureAccountName, json[AzureAccountName_Label]);      
    }
    if (json.containsKey(AzureAccountKey_Label))
    {
      strcpy(azureAccountKey, json[AzureAccountKey_Label]);    
    }
    if (json.containsKey(ViessmannClientId_Label))
    {
      if (strlen(json[ViessmannClientId_Label]) > 2)
      {
        strcpy(viessmannClientId, json[ViessmannClientId_Label]);
      }      
    }
    if (json.containsKey(ViessmannRefreshToken_Label))
    {
      if (strlen(json[ViessmannRefreshToken_Label]) > 2)
      {
        strcpy(viessmannRefreshToken, json[ViessmannRefreshToken_Label]);
      }      
    }
    if (json.containsKey(SoundSwitcherThresholdString_Label))
    {
      strcpy(sSwiThresholdStr, json[SoundSwitcherThresholdString_Label]);      
    }
  }
  Serial.println(F("\nCustom config file was successfully parsed")); 
  return true;
}

bool saveApplConfigData()
{
  LOGERROR(F("Saving additional configuration to file"));
  
#if (ARDUINOJSON_VERSION_MAJOR >= 6)
  DynamicJsonDocument json(1024);
#else
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
#endif

  // Occasionally it happenend that blanks were accidently
  // added at start of the strings through 'copy and paste'
  // so to be safe, leading blanks are removed
  trimLeadingSpaces((char *)azureAccountKey);
  trimLeadingSpaces((char *)viessmannClientId);
  trimLeadingSpaces((char *)viessmannRefreshToken);

  // JSONify local configuration parameters 
  json[AzureAccountName_Label] = azureAccountName;
  json[AzureAccountKey_Label] = strlen(azureAccountKey) > 2 ? azureAccountKey : "";
  json[ViessmannClientId_Label] = strlen(viessmannClientId) > 2 ? viessmannClientId : "";
  json[ViessmannRefreshToken_Label] = strlen(viessmannRefreshToken) > 2 ? viessmannRefreshToken : "";
  
  //json[AzureAccountKey_Label] = azureAccountKey;
  //json[ViessmannClientId_Label] = viessmannClientId;
  //json[ViessmannRefreshToken_Label] = viessmannRefreshToken;
  json[SoundSwitcherThresholdString_Label] = sSwiThresholdStr;
  // Open file for writing
  File f = FileFS.open(CONFIG_FILE, "w");
  
  if (!f)
  {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

#if (ARDUINOJSON_VERSION_MAJOR >= 6)

  Serial.println(F("Here we could print the additional config parameter written to file"));
  //serializeJsonPretty(json, Serial);

  // Write data to file and close it
  serializeJson(json, f);
#else
  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(f);
#endif

  f.close();

  Serial.println(F("\nConfig file was successfully saved"));
  return true;
}

void trimLeadingSpaces(char *str) {
    int index = 0;
    int i = 0;

    // Finde die Position des ersten Nicht-Leerzeichens
    while (str[index] != '\0' && isspace((unsigned char)str[index])) {
        index++;
    }

    // Verschiebe die Zeichen nach vorne
    while (str[index] != '\0') {
        str[i++] = str[index++];
    }
    str[i] = '\0'; // Null-terminieren des Strings
}

void setup()
{
  // At the begin of setup we get some information about
  // stack, heap and watchdog

  // Get Stackptr at start of setup()
  //void* SpStart = NULL;
  UBaseType_t * SpStart = NULL;
  //StackPtrAtStart = (void *)&SpStart;
  StackPtrAtStart = (UBaseType_t *)&SpStart;
  
  // Get StackHighWatermark at start of setup()
  watermarkStart =  uxTaskGetStackHighWaterMark(NULL);
  // Calculate (not exact) end-address of the stack
  StackPtrEnd = StackPtrAtStart - watermarkStart;

  __unused uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
  uint32_t freeHeapSize = esp_get_free_heap_size();
  
  /*
  UBaseType_t watermarkStart_0 = uxTaskGetStackHighWaterMark(taskHandle_0);
  UBaseType_t watermarkStart_1 = uxTaskGetStackHighWaterMark(NULL);
  */

  resetReason_0 = rtc_get_reset_reason(0);
  resetReason_1 = rtc_get_reset_reason(1);
  lastResetCause = resetReason_1;  

  // Disable watchdog
  // volatile int wdt_State = esp_task_wdt_status(NULL);
  // volatile int wdt_deinit_result = esp_task_wdt_deinit();
  // ESP_ERR_INVALID_STATE  -- 259
  // ESP_ERR_NOT_FOUND -- 261

    if ((esp_task_wdt_status(NULL) == ESP_ERR_NOT_FOUND) || (esp_task_wdt_deinit() == ESP_OK))
    {
      watchDogEnabled = false;
      watchDogCommandSuccessful = true;    
    }
    else
    {
      watchDogCommandSuccessful = false;   
    }
  
  // initialize the LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  while (!Serial);

  delay(4000);
  
  // is needed for Button
  // attachInterrupt(GPIOPin, GPIOPinISR, RISING);
  
  Serial.printf("\r\n\r\nAddress of Stackpointer near start is:  %p \r\n",  (void *)StackPtrAtStart);
  Serial.printf("End of Stack is near:                   %p \r\n",  (void *)StackPtrEnd);
  Serial.printf("Free Stack near start is:  %d \r\n",  (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);

  /*
  // Get free stack at actual position (can be used somewhere in program)
  void* SpActual = NULL;
  Serial.printf("\r\nFree Stack at actual position is: %d \r\n", (uint32_t)&SpActual - (uint32_t)StackPtrEnd);
  */

  Serial.print(F("Free Heap: "));
  Serial.println(freeHeapSize);
  Serial.printf("Last Reset Reason: CPU_0 = %u, CPU_1 = %u\r\n", resetReason_0, resetReason_1);
  Serial.println(F("Reason CPU_0: "));
  print_reset_reason(resetReason_0);
  Serial.println(F("Reason CPU_1: "));
  print_reset_reason(resetReason_1);

  if(watchDogCommandSuccessful)
  {
    Serial.println(F("\r\nWatchdog is preliminary disabled"));  
  }
  else
  {
    Serial.println(F("\r\nFailed to disable Watchdog"));
    Serial.println(F("\r\nRestarting"));
    // Wait some time (3000 ms)
    uint32_t start = millis();
    while ((millis() - start) < 3000)
    {
      delay(10);
    }
    ESP.restart();
  }

  Serial.printf("Watchdog mode: %s\r\n", WORK_WITH_WATCHDOG == 0 ? "Disabled" : "Enabled");
  Serial.printf("Transport Protokoll: %s\r\n", TRANSPORT_PROTOCOL == 0 ? "http" : "https");
  Serial.printf("Selected Microphone Type = %s\r\n", (usedMicType == MicType::SPH0645LM4H) ? "SPH0645" : "INMP441");
  
  delay(4000);
  
  // If wanted -> Wait on press/release of boot button
  /*
  Serial.println(F("\r\n\r\nPress Boot Button to continue!"));
  while (!buttonPressed)
  {
    delay(100);
  }
  */

  // Wait some time (3000 ms)
  uint32_t start = millis();
  while ((millis() - start) < 3000)
  {
    delay(10);
  }
  
  Serial.print(F("\nStarting 'ESP32_Viessm_Analog_Meter' Version "));
  Serial.print(PROGRAMVERSION);
  Serial.print(" using");  
  Serial.print(FS_Name);
  Serial.print(F(" on ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);

  if ( String(ESP_ASYNC_WIFIMANAGER_VERSION) < ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET )
  {
    Serial.print("Warning. Must use this example on Version later than : ");
    Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET);
  }
  
  // 
  Serial.setDebugOutput(false);
  
  if (FORMAT_FILESYSTEM)
  {
    #ifdef ESP32
      FileFS.begin(false, "/littlefs", 10, "spiffs");
    #endif
    FileFS.format();
  }
  else
  {
  #ifdef ESP32 
    FileFS.begin(false, "/littlefs", 10, "spiffs");  // First with parameter false
    if (!FileFS.begin(true, "/littlefs", 10, "spiffs"))
    {
      Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));
    }
  #else
    if (!FileFS.begin())
    {
      Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));
    }
  #endif
  }     
    if (!FileFS.begin())
    {     
      // prevents debug info from the library to hide err message.
      delay(100);
      
#if USE_LITTLEFS
      Serial.println(F("LittleFS failed!. Please use SPIFFS or EEPROM. Stay forever"));
#else
      Serial.println(F("SPIFFS failed!. Please use LittleFS or EEPROM. Stay forever"));
#endif

      while (true)
      {
        delay(1);
      }
    }
  

  unsigned long startedAt = millis();

  // New in v1.4.0
  initAPIPConfigStruct(WM_AP_IPconfig);    // For Access Point
  initSTAIPConfigStruct(WM_STA_IPconfig);  // For WiFi-STA Mode, Connect with router
  //////

  if (!loadApplConfigData())   // For Azure Credentials, Viessman Refresh-Token and threshold
  {
    Serial.println(F("Failed to read ConfigFile, using default values"));
  }

  //Local intialization. Once its business is done, there is no need to keep it around
  // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  //ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer);
  // Use this to personalize DHCP hostname (RFC952 conformed)
  AsyncWebServer webServer(HTTP_PORT);

#if ( USING_ESP32_S2 || USING_ESP32_C3 )
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, NULL, "AsyncConfigOnStartup");
#else
  // RoSchmi
  //DNSServer dnsServer;
  AsyncDNSServer dnsServer;
  
  //dnsServer.start();
  
  
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "AsyncConfigOnStartup");
#endif

#if USE_CUSTOM_AP_IP 
  //set custom ip for portal
  // New in v1.4.0
  ESPAsync_wifiManager.setAPStaticIPConfig(WM_AP_IPconfig);
  //////
#endif

  ESPAsync_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESPAsync_wifiManager.setConfigPortalChannel(0);
  //////

#if !USE_DHCP_IP    
    // Set (static IP, Gateway, Subnetmask, DNS1 and DNS2) or (IP, Gateway, Subnetmask). New in v1.0.5
    // New in v1.4.0
    ESPAsync_wifiManager.setSTAStaticIPConfig(WM_STA_IPconfig);
    //////
#endif

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // So there are functions to store and retrieve the credentials in the filesystem
  Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
  Router_Pass = ESPAsync_wifiManager.WiFi_Pass();
  
  
  ssid.toUpperCase();
  password = "My" + ssid;


  // RoSchmi new
  // Extra parameters to be configured
  // After connecting, parameter.getValue() will get you the configured value
  // Format: <ID> <Placeholder text> <default value> <length> <custom HTML> <label placement>
  
  ESPAsync_WMParameter p_azureAccountName(AzureAccountName_Label, "Azure Storage Account Name", azureAccountName, 20);
  ESPAsync_WMParameter p_azureAccountKey(AzureAccountKey_Label, "Azure Storage Account Key", "", 90);
  ESPAsync_WMParameter p_viessmannClientId(ViessmannClientId_Label, "Viessmann Client Id", viessmannClientId, 50);
  ESPAsync_WMParameter p_viessmannRefreshToken(ViessmannRefreshToken_Label, "Viessmann Refresh Token", "", 60);
  ESPAsync_WMParameter p_soundSwitcherThreshold(SoundSwitcherThresholdString_Label, "Noise Threshold", sSwiThresholdStr, 6);
  // Just a quick hint
  ESPAsync_WMParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty. <br/>*Portal Password = MyESP_'hexnumber'</small>");
  ESPAsync_WMParameter p_hint2("<small><br/>*Hint: to enter the Azure Key, Viessmann Client Id and Viessmann Refresh Token send them to your Phone by E-Mail and use copy and paste.<br/></small>");
    
  //add all parameters here
  ESPAsync_wifiManager.addParameter(&p_hint);
  ESPAsync_wifiManager.addParameter(&p_hint2);
  ESPAsync_wifiManager.addParameter(&p_azureAccountName);
  ESPAsync_wifiManager.addParameter(&p_azureAccountKey);
  ESPAsync_wifiManager.addParameter(&p_viessmannClientId);
  ESPAsync_wifiManager.addParameter(&p_viessmannRefreshToken);
  ESPAsync_wifiManager.addParameter(&p_soundSwitcherThreshold);

  // Check if there are stored WiFi router/password credentials.
  // If not found, device will remain in configuration mode until switched off via webserver.
  Serial.println(F("Opening configuration portal."));
  
  bool configWiFiDataLoaded = false;

  // From v1.1.0, Don't permit NULL password
  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("From v1.1.0 * Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
    ESPAsync_wifiManager.getSSID();
    ESPAsync_wifiManager.setConfigPortalTimeout(60); //If no access point name has been previously entered disable timeout.
    Serial.println(F("Here we could print the Self-Stored Router WiFi-Credentials"));
    //Remove this line if you do not want to see WiFi password printed
    LOGERROR3(F("ESP Self-Stored Credentials :  "), Router_SSID, F(", PW = "), Router_Pass);
  }
  else
  {
    LOGERROR3(F("From v1.1.0 * Neglect ESP-Self_Stored SSID = "), Router_SSID, F(", PW = "), Router_Pass);
  }
  
  if (loadWiFiConfigData())   // Load credentials for Router WiFi STA connection from SPIFFS/LittleFS    
  {
    configWiFiDataLoaded = true;
    
    ESPAsync_wifiManager.setConfigPortalTimeout(60); //If no access point name has been previously entered disable timeout.
    Serial.println(F("Got stored Credentials. Timeout 60s for Config Portal"));
    
    // Added by RoSchmi
    // Store credentials read from file in backstore for later use
    memcpy(&Loaded_WM_config, &WM_config, sizeof(Loaded_WM_config));
    
    LOGERROR(F("Could print Credentials, loaded and saved in backstore"));
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
    LOGERROR3(F("SSID: "), Loaded_WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), Loaded_WM_config.WiFi_Creds[i].wifi_pw);
    }
    


#if DISPLAY_STORED_CREDENTIALS_IN_CP    // Cave: This directive is alse used in ESPAsync_WiFiManger
    // New. Update Credentials, got from loadWiFiConfigData(), to display on CP
    
    // Cave:dangerous, the passwords should never be shown
    
    ESPAsync_wifiManager.setCredentials(WM_config.WiFi_Creds[0].wifi_ssid, WM_config.WiFi_Creds[0].wifi_pw,
                                        WM_config.WiFi_Creds[1].wifi_ssid, WM_config.WiFi_Creds[1].wifi_pw); 
#endif
    
#if USE_ESP_WIFIMANAGER_NTP      
    if ( strlen(WM_config.TZ_Name) > 0 )
    {
      LOGERROR3(F("Current TZ_Name ="), WM_config.TZ_Name, F(", TZ = "), WM_config.TZ);

  #if ESP8266
      configTime(WM_config.TZ, "pool.ntp.org"); 
  #else
      //configTzTime(WM_config.TZ, "pool.ntp.org" );
      configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  #endif   
    }
    else
    {
      Serial.println(F("Current Timezone is not set. Enter Config Portal to set."));
    } 
#endif
  }
  else
  {
    // Enter CP only if no stored SSID on flash and file 
    Serial.println(F("Open Config Portal without Timeout: No stored Credentials."));
    initialConfig = true;
  }

  //RoSchmi must always be true in this App 
  initialConfig = true;

  if (initialConfig)
  {
    Serial.print(F("Starting configuration portal @ "));
    
#if USE_CUSTOM_AP_IP    
    Serial.print(APStaticIP);
#else
    Serial.print(F("192.168.4.1"));
#endif

    Serial.print(F(", SSID = "));
    Serial.print(ssid);
    Serial.print(F(", PWD = "));
    Serial.println(password);

    digitalWrite(LED_BUILTIN, LED_ON); // Turn led on as we are in configuration mode.

    //sets timeout in seconds until configuration portal gets turned off.
    //If not specified device will remain in configuration mode until
    //switched off via webserver or device is restarted.
    //ESPAsync_wifiManager.setConfigPortalTimeout(600);

    // Starts an access point, --> open the access point and enter Router WiFi Credentials
    
    if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password.c_str()))
    {
      Serial.println(F("Not connected to WiFi but continuing anyway."));
    }
    else
    {
      Serial.println(F("WiFi connected...yeey :)"));
    }

    // Stored  for later usage, from v1.1.0, but clear first
    // Clear the struct. The former content from file is saved in 'Loaded_WM_config'
    memset(&WM_config, 0, sizeof(WM_config));
    
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      String tempSSID = ESPAsync_wifiManager.getSSID(i);
      String tempPW   = ESPAsync_wifiManager.getPW(i);
      
      if (tempSSID.isEmpty())
      {
        tempSSID = Loaded_WM_config.WiFi_Creds[i].wifi_ssid;       
      }
      
      if (tempPW.isEmpty())
      {
        tempPW = Loaded_WM_config.WiFi_Creds[i].wifi_pw;
      }
      
      // Check if length of SSID and password fit into the place in the struct
      if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);
      if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  

      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("To Multi-List added: SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }     
    }
#if USE_ESP_WIFIMANAGER_NTP      
    String tempTZ   = ESPAsync_wifiManager.getTimezoneName();

    if (strlen(tempTZ.c_str()) < sizeof(WM_config.TZ_Name) - 1)
      strcpy(WM_config.TZ_Name, tempTZ.c_str());
    else
      strncpy(WM_config.TZ_Name, tempTZ.c_str(), sizeof(WM_config.TZ_Name) - 1);

    const char * TZ_Result = ESPAsync_wifiManager.getTZ(WM_config.TZ_Name);
    
    if (strlen(TZ_Result) < sizeof(WM_config.TZ) - 1)
      strcpy(WM_config.TZ, TZ_Result);
    else
      strncpy(WM_config.TZ, TZ_Result, sizeof(WM_config.TZ_Name) - 1);
         
    if ( strlen(WM_config.TZ_Name) > 0 )
    {
      LOGERROR3(F("Saving current TZ_Name ="), WM_config.TZ_Name, F(", TZ = "), WM_config.TZ);

#if ESP8266
      configTime(WM_config.TZ, "pool.ntp.org"); 
#else
      //configTzTime(WM_config.TZ, "pool.ntp.org" );
      configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
#endif
    }
    else
    {
      LOGERROR(F("Current Timezone Name is not set. Enter Config Portal to set."));
    }
#endif

    ESPAsync_wifiManager.getSTAStaticIPConfig(WM_STA_IPconfig);
    LOGERROR(F("Here we could print Router WiFi Credentals saved to file"));
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      LOGERROR3(F("Saved SSID: "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
    }
    saveWiFiConfigData();
  }

  // Getting posted form values and overriding local variables parameters
  // Config file is written regardless the connection state 
  strcpy(azureAccountName, p_azureAccountName.getValue());
  if (strlen(p_azureAccountKey.getValue()) > 1)
  {
    strcpy(azureAccountKey, p_azureAccountKey.getValue());
  }
  if (strlen(p_viessmannClientId.getValue()) > 1)
  {
    strcpy(viessmannClientId, p_viessmannClientId.getValue());
  }
  if (strlen(p_viessmannRefreshToken.getValue()) > 1)
  {
    strcpy(viessmannRefreshToken, p_viessmannRefreshToken.getValue());
  }
  strcpy(sSwiThresholdStr, p_soundSwitcherThreshold.getValue());
    
    // Writing JSON config file to flash for next boot

  saveApplConfigData();

  digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.

  

  startedAt = millis();



  // Azure Acount must be updated here with eventually changed values from WiFi-Manager
  myCloudStorageAccount.ChangeAccountParams((char *)azureAccountName, (char *)azureAccountKey, UseHttps_State);
  
  #if WORK_WITH_WATCHDOG == 1
    // Start watchdog with 20 seconds
    if (esp_task_wdt_init(20, true) == ESP_OK)
    {
      Serial.println(F("\r\nWatchdog enabled with interval of 20 sec\r\n"));
    }
    else
    {
      Serial.println(F("Failed to enable watchdog"));
    }
    esp_task_wdt_add(NULL);

    //https://www.az-delivery.de/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/watchdog-und-heartbeat

    //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/wdts.html
  #endif

   if ( WiFi.status() != WL_CONNECTED )
    {
      Serial.println(F("ConnectMultiWiFi in setup"));

      connectMultiWiFi();
    }

    Serial.print(F("After waiting "));
  Serial.print((float) (millis() - startedAt) / 1000);
  Serial.print(F(" secs more in setup(), connection result is "));

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print(F("connected. Local IP: "));
    Serial.println(WiFi.localIP());
  }
  else
    Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));

  uint8_t status;
  int i = 0;
  status = wifiMulti.run();
             
  delay(800L);  //(WIFI_MULTI_1ST_CONNECT_WAITING_MS) 

  if ( status == WL_CONNECTED )
  {
    Serial.print(F("connected. Local IP: "));
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));
    #if WORK_WITH_WATCHDOG == 1
      while (true)  // Wait to be rebooted by watchdog
      {
        delay(100);
      }
    #endif
  }

  /*
  while (true)
  {
    Serial.print(F("connected. Local IP: "));
    Serial.println(WiFi.localIP());
    delay (1000);
  }
  */

  soundSwitcher.begin(atoi((char *)sSwiThresholdStr), Hysteresis::Percent_10, soundSwitcherUpdateInterval, soundSwitcherReadDelayTime);
  soundSwitcher.SetCalibrationParams(-20.0);  // optional
  soundSwitcher.SetActive();
  //**************************************************************
  onOffDataContainer.begin(DateTime(), OnOffTableName_1, OnOffTableName_2, OnOffTableName_3, OnOffTableName_4);

  // Initialize State of 4 On/Off-sensor representations 
  // and of the inverter flags (Application specific)
  for (int i = 0; i < 4; i++)
  {
    onOffDataContainer.PresetOnOffState(i, false, true);
    onOffDataContainer.Set_OutInverter(i, true);
    onOffDataContainer.Set_InputInverter(i, false);
  }

  //Initialize OnOffSwitcher (for tests and simulation)
  onOffSwitcherWio.begin(TimeSpan(15 * 60));   // Toggle every 30 min
  //onOffSwitcherWio.SetInactive();
  onOffSwitcherWio.SetActive();

// Setting Daylightsavingtime. Enter values for your zone in file include/config.h
  // Program aborts in some cases of invalid values
  
  int dstWeekday = getDayNum(DST_START_WEEKDAY);
  int dstMonth = getMonNum(DST_START_MONTH);
  int dstWeekOfMonth = getWeekOfMonthNum(DST_START_WEEK_OF_MONTH);

  TimeChangeRule dstStart {DST_ON_NAME, (uint8_t)dstWeekOfMonth, (uint8_t)dstWeekday, (uint8_t)dstMonth, DST_START_HOUR, TIMEZONEOFFSET + DSTOFFSET};
  
  bool firstTimeZoneDef_is_Valid = (dstWeekday == -1 || dstMonth == - 1 || dstWeekOfMonth == -1 || DST_START_HOUR > 23 ? true : DST_START_HOUR < 0 ? true : false) ? false : true;
  
  dstWeekday = getDayNum(DST_STOP_WEEKDAY);
  dstMonth = getMonNum(DST_STOP_MONTH);
  dstWeekOfMonth = getWeekOfMonthNum(DST_STOP_WEEK_OF_MONTH);

  TimeChangeRule stdStart {DST_OFF_NAME, (uint8_t)dstWeekOfMonth, (uint8_t)dstWeekday, (uint8_t)dstMonth, (uint8_t)DST_START_HOUR, (int)TIMEZONEOFFSET};

  bool secondTimeZoneDef_is_Valid = (dstWeekday == -1 || dstMonth == - 1 || dstWeekOfMonth == -1 || DST_STOP_HOUR > 23 ? true : DST_STOP_HOUR < 0 ? true : false) ? false : true;
  
  if (firstTimeZoneDef_is_Valid && secondTimeZoneDef_is_Valid)
  {
    myTimezone.setRules(dstStart, stdStart);
  }
  else
  {
    // If timezonesettings are not valid: -> take UTC and wait for ever  
    TimeChangeRule stdStart {DST_OFF_NAME, (uint8_t)dstWeekOfMonth, (uint8_t)dstWeekday, (uint8_t)dstMonth, (uint8_t)DST_START_HOUR, (int)0};
    myTimezone.setRules(stdStart, stdStart);
    while (true)
    {
      Serial.println(F("Invalid DST Timezonesettings"));
      delay(5000);
    }
  }
  
  Serial.println(F("Starting timeClient"));
  timeClient.begin();
  timeClient.setUpdateInterval((NTP_UPDATE_INTERVAL_MINUTES < 1 ? 1 : NTP_UPDATE_INTERVAL_MINUTES) * 60 * 1000);
  // 'setRetryInterval' should not be too short, may be that short intervals lead to malfunction 
  timeClient.setRetryInterval(20000);  // Try to read from NTP Server not more often than every 20 seconds
  Serial.println("Using NTP Server " + timeClient.getPoolServerName());
  
  timeClient.update();
  uint32_t counter = 0;
  uint32_t maxCounter = 10;
  
  while(!timeClient.updated() &&  counter++ <= maxCounter)
  {
    Serial.println(F("NTP FAILED: Trying again"));
    delay(1000);
    #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
    #endif
    timeClient.update();
  }

  if (counter >= maxCounter)
  {
    #if ESP8266      
    ESP.reset();
#else
    ESP.restart();
#endif  
    while(true)
    {
      delay(500); //Wait for ever, could not get NTP time, eventually reboot by Watchdog
    }
  }

  Serial.println("\r\n********UPDATED********");
    
  Serial.println("UTC : " + timeClient.getFormattedUTCTime());
  Serial.println("UTC : " + timeClient.getFormattedUTCDateTime());
  Serial.println("LOC : " + timeClient.getFormattedTime());
  Serial.println("LOC : " + timeClient.getFormattedDateTime());
  Serial.println("UTC EPOCH : " + String(timeClient.getUTCEpochTime()));
  Serial.println("LOC EPOCH : " + String(timeClient.getEpochTime()));

  unsigned long utcTime = timeClient.getUTCEpochTime();  // Seconds since 1. Jan. 1970
  
  dateTimeUTCNow =  utcTime;

  Serial.printf("%s %i %02d %02d %02d %02d", (char *)"UTC-Time is  :", dateTimeUTCNow.year(), 
                                        dateTimeUTCNow.month() , dateTimeUTCNow.day(),
                                        dateTimeUTCNow.hour() , dateTimeUTCNow.minute());
  Serial.println("");
  
  // RoSchmi
  //DateTime localTime = myTimezone.toLocal(dateTimeUTCNow.unixtime());
  localTime = myTimezone.toLocal(dateTimeUTCNow.unixtime());
  timeDiffUtcToLocal = localTime.operator-(dateTimeUTCNow);

  
  Serial.printf("%s %i %02d %02d %02d %02d", (char *)"Local-Time is:", localTime.year(), 
                                        localTime.month() , localTime.day(),
                                        localTime.hour() , localTime.minute());
  Serial.println("\n");
  
  // Set Standard Read Interval
  // Is limited to be not below 2 seconds
  analogSensorMgr.SetReadInterval(ANALOG_SENSOR_READ_INTERVAL_SECONDS < 2 ? 2 : ANALOG_SENSOR_READ_INTERVAL_SECONDS);
  // Set Read Interval for special sensor
  analogSensorMgr.SetReadInterval(0, GASMETER_AI_API_READ_INTERVAL_SECONDS);
  analogSensorMgr.SetReadInterval(1, GASMETER_AI_API_READ_INTERVAL_SECONDS);
  analogSensorMgr.SetReadInterval(2, GASMETER_AI_API_READ_INTERVAL_SECONDS);
  
  analogSensorMgr_Api_01.SetReadInterval(API_ANALOG_SENSOR_READ_INTERVAL_SECONDS);
  
  httpCode = refreshAccessTokenFromApi(myX509Certificate, myViessmannApiAccountPtr, viessmannRefreshToken);
  if (httpCode == t_http_codes::HTTP_CODE_OK)
  {   
    AccessTokenRefreshTime = dateTimeUTCNow;
  }
  else
  {     
    Serial.println(F("Couldn't refresh accessToken from Viessmann Cloud. Error message is:"));
    Serial.println((char*)bufferStorePtr);
    ESP.restart();
    while(true)
    {
      delay(500);
    }
  }
  
  httpCode = readUserFromApi(myX509Certificate, myViessmannApiAccountPtr);
  if (httpCode == t_http_codes::HTTP_CODE_OK)
  {
    Serial.println(F("UserId successfully read from Viessmann Cloud"));
    Serial.println(F(""));
    viessmannUserId_is_read = true;    
  }
  else
  {     
    Serial.println(F("Couldn't read UserId from Viessmann Cloud.\r\nError message is:"));
    Serial.println((char*)bufferStorePtr);
    ESP.restart();
    while(true)
    {
      delay(500);
    }
  }
  
  httpCode = readEquipmentFromApi(myX509Certificate, myViessmannApiAccountPtr, &Data_0_Id, equipBufLen, Data_0_Description, Data_0_Address_Street, Data_0_Address_HouseNumber, Gateways_0_Serial, Gateways_0_Devices_0_Id);
  if (httpCode == t_http_codes::HTTP_CODE_OK)
  {
    Serial.println(F("Equipment successfully read from Viessmann Cloud"));
    viessmannEquip_is_read = true;
  }
  else
  {     
    Serial.println(F("Couldn't read Equipment from Viessmann Cloud.\r\nError message is:"));
    Serial.println((char*)bufferStorePtr);
    // RoSchmi
    
    ESP.restart();
    while(true)
    {
      delay(500);
    }
      
  }
}
  
void loop()
{
  check_status();   // Checks if WiFi is still connected
                    // if not, try other Accesspoint
  // put your main code here, to run repeatedly:
  if (++loopCounter % 100000 == 0)   // Make decisions to send data every 100000 th round and toggle Led to signal that App is running
  {

    #if SERIAL_PRINT == 1
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);    // toggle LED to signal that App is running
    #endif

    #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
    #endif
    
      // Update RTC from Ntp when ntpUpdateInterval has expired, retry when RetryInterval has expired       
      if (timeClient.update())
      {                                                                      
        dateTimeUTCNow = timeClient.getUTCEpochTime();
        
        timeNtpUpdateCounter++;

        #if SERIAL_PRINT == 1
          // Indicate that NTP time was updated
          char buffer[35] = {0};     
          strcpy(buffer, "NTP-Utc: YYYY-MM-DD hh:mm:ss");           
          dateTimeUTCNow.toString(buffer);
          Serial.println(buffer);
        #endif
      }  // End NTP stuff
          
      dateTimeUTCNow = timeClient.getUTCEpochTime();
      
      // Get offset in minutes between UTC and local time with consideration of DST
      int timeZoneOffsetUTC = myTimezone.utcIsDST(dateTimeUTCNow.unixtime()) ? TIMEZONEOFFSET + DSTOFFSET : TIMEZONEOFFSET;
      
      localTime = myTimezone.toLocal(dateTimeUTCNow.unixtime());
      timeDiffUtcToLocal = localTime.operator-(dateTimeUTCNow);
      
      // refresh access token if refresh interval has expired 
      if ((AccessTokenRefreshTime.operator+(AccessTokenRefreshInterval)).operator<(dateTimeUTCNow))
      {
          httpCode = refreshAccessTokenFromApi(myX509Certificate, myViessmannApiAccountPtr, viessmannRefreshToken);
          
          if (httpCode == t_http_codes::HTTP_CODE_OK)
          {           
            AccessTokenRefreshTime = dateTimeUTCNow;
          }
          else
          {
            Serial.println(F("Token Refresh failed\n"));            
            AccessTokenRefreshTime = dateTimeUTCNow.operator-(TimeSpan(300));
          }
      }

      // In the last 15 sec of each day we set a pulse to Off-State when we had On-State before
      bool isLast15SecondsOfDay = (localTime.hour() == 23 && localTime.minute() == 59 &&  localTime.second() > 45) ? true : false;

      // Get readings from 4 different analog sensors stored in the Viessmann Cloud    
      // and store the values in a container
       
      dataContainerAnalogViessmann01.SetNewValue(0, dateTimeUTCNow, atof((ReadViessmannApi_Analog_01(0, (const char *)"_95_heating_temperature_outside")).value)); // Aussen
      dataContainerAnalogViessmann01.SetNewValue(1, dateTimeUTCNow, atof((ReadViessmannApi_Analog_01(1, (const char *)"_3_temperature_main")).value)); // Vorlauf                
      dataContainerAnalogViessmann01.SetNewValue(2, dateTimeUTCNow, atof((ReadViessmannApi_Analog_01(2, (const char *)"_90_heating_dhw_cylinder_temperature")).value)); // Boiler
      dataContainerAnalogViessmann01.SetNewValue(3, dateTimeUTCNow, atof((ReadViessmannApi_Analog_01(3, (const char *)"_7_burner_modulation")).value));  // Modulation
      
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);    // toggle LED to signal that App is running

      // Get readings from 4 different analog sensors, (preferably measured by the Esp 32 device, e.g. noise level)     
      // and store the values in a container
       
      dataContainer.SetNewValueStruct(0, dateTimeUTCNow, ReadAnalogSensorStruct_01(0), true);
      dataContainer.SetNewValueStruct(1, dateTimeUTCNow, ReadAnalogSensorStruct_01(1), true);     
      dataContainer.SetNewValueStruct(2, dateTimeUTCNow, ReadAnalogSensorStruct_01(2), false);      
      dataContainer.SetNewValueStruct(3, dateTimeUTCNow, ReadAnalogSensorStruct_01(3), false); 
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);    // toggle LED to signal that App is running

      // Check if automatic OnOffSwitcher has toggled (used to simulate on/off changes)
      // and accordingly change the state of one representation (here index 3) in onOffDataContainer
      // This can be used for debugging, shows that displaying on/off states is
      // working
      /*
      if (onOffSwitcherWio.hasToggled(dateTimeUTCNow))
      {
        bool state = onOffSwitcherWio.GetState();
        onOffDataContainer.SetNewOnOffValue(3, !state, dateTimeUTCNow, timeZoneOffsetUTC);    
      }
      */
      
      // Check if one of the here listed On/Off states has changed     
      // (here: Burner, Circulation Pump, HotWaterCirculation Pump,
      // HotWaterPimary Pump)
   
      if (OnOffBurnerStatus.HasChangedState())
      {    
        onOffDataContainer.SetNewOnOffValue(0, OnOffBurnerStatus.GetStateAndResetChangedFlag(), dateTimeUTCNow, timeZoneOffsetUTC);
      }

      if (OnOffCirculationPumpStatus.HasChangedState())
      {
        onOffDataContainer.SetNewOnOffValue(1, OnOffCirculationPumpStatus.GetStateAndResetChangedFlag(), dateTimeUTCNow, timeZoneOffsetUTC);
      }

      if (OnOffHotWaterCircualtionPumpStatus.HasChangedState())
      {
        onOffDataContainer.SetNewOnOffValue(2, OnOffHotWaterCircualtionPumpStatus.GetStateAndResetChangedFlag(), dateTimeUTCNow, timeZoneOffsetUTC);
      }

      if (OnOffHotWaterPrimaryPumpStatus.HasChangedState())
      {
        onOffDataContainer.SetNewOnOffValue(3, OnOffHotWaterPrimaryPumpStatus.GetStateAndResetChangedFlag(), dateTimeUTCNow, timeZoneOffsetUTC);
      }
      
      
      // This is for Burner Sound Sensor (need not to be used in this App)    
      /*
      if (feedResult.isValid && (feedResult.hasToggled || feedResult.analogToSend))
      {
          if (feedResult.hasToggled)
          {
            onOffDataContainer.SetNewOnOffValue(0, feedResult.state, dateTimeUTCNow, timeZoneOffsetUTC);
            Serial.print("\r\nHas toggled, new state is: ");
            Serial.println(feedResult.state == true ? "High" : "Low");
            Serial.println();
          }
          // if toogled activate make hasToBeSent flag
          // so that the analog values have to be updated in the cloud
          if (feedResult.analogToSend)
          {
            dataContainer.setHasToBeSentFlag();
            Serial.print("Average is: ");
            Serial.println(feedResult.avValue);             
            Serial.println();
          }           
      }
      */           
      // Check if something is to do: send analog data ? send On/Off-Data ? Handle EndOfDay stuff ?
      if (dataContainerAnalogViessmann01.hasToBeSent() || dataContainer.hasToBeSent() || onOffDataContainer.One_hasToBeBeSent(localTime) || isLast15SecondsOfDay)
      {     
        //Create some buffer
        char sampleTime[25] {0};    // Buffer to hold sampletime        
        char strData[100] {0};          // Buffer to hold display message
        
        char EtagBuffer[50] {0};    // Buffer to hold returned Etag

        // Create az_span to hold partitionkey
        char partKeySpan[25] {0};
        size_t partitionKeyLength = 0;
        az_span partitionKey = AZ_SPAN_FROM_BUFFER(partKeySpan);
        
        // Create az_span to hold rowkey
        char rowKeySpan[25] {0};
        size_t rowKeyLength = 0;
        az_span rowKey = AZ_SPAN_FROM_BUFFER(rowKeySpan);
        
        if (dataContainerAnalogViessmann01.hasToBeSent())   // have to send analog values read from Viessmann ?
        {         
          // Retrieve edited sample values from container
          SampleValueSet sampleValueSet = dataContainerAnalogViessmann01.getCheckedSampleValues(dateTimeUTCNow, true);
                  
          createSampleTime(sampleValueSet.LastUpdateTime, timeZoneOffsetUTC, (char *)sampleTime);
          // Define name of the table (arbitrary name + actual year, like: AnalogTestValues2020)
          String augmentedAnalogTableName = viessmAnalogTableName_01; 
          if (augmentTableNameWithYear)
          {
            // RoSchmi changed 10.07.2024 to resolve issue 1         
            //augmentedAnalogTableName += (dateTimeUTCNow.year());
            augmentedAnalogTableName += (localTime.year()); 
          }

          // Create Azure Storage Table if table doesn't exist
          if (localTime.year() != dataContainerAnalogViessmann01.Year)    // if new year
          {  
            az_http_status_code respCode = createTable(myCloudStorageAccountPtr, myX509Certificate, (char *)augmentedAnalogTableName.c_str());
           
            Serial.printf("\r\nCreate Table: Statuscode: %s\n", ((String)respCode).c_str());

            if ((respCode == AZ_HTTP_STATUS_CODE_CONFLICT) || (respCode == AZ_HTTP_STATUS_CODE_CREATED))
            {
              dataContainerAnalogViessmann01.Set_Year(localTime.year());                   
            }
            else
            {
              // Reset board if not successful
             
             //SCB_AIRCR = 0x05FA0004;             
            }                     
          }   

          // Create an Array of (here) 5 Properties
          // Each Property consists of the Name, the Value and the Type (here only Edm.String is supported)

          // Besides PartitionKey and RowKey we have 5 properties to be stored in a table row
          // (SampleTime and 4 samplevalues)
          size_t analogPropertyCount = 5;
          EntityProperty AnalogPropertiesArray[5];

          #if ANALOG_SENSORS_USE_AVERAGE == 1
            AnalogPropertiesArray[0] = (EntityProperty)TableEntityProperty((char *)"SampleTime", (char *) sampleTime, (char *)"Edm.String");
            AnalogPropertiesArray[1] = (EntityProperty)TableEntityProperty((char *)"T_1", (char *)floToStr(sampleValueSet.SampleValues[0].AverageValue).c_str(), (char *)"Edm.String");
            AnalogPropertiesArray[2] = (EntityProperty)TableEntityProperty((char *)"T_2", (char *)floToStr(sampleValueSet.SampleValues[1].AverageValue).c_str(), (char *)"Edm.String");
            AnalogPropertiesArray[3] = (EntityProperty)TableEntityProperty((char *)"T_3", (char *)floToStr(sampleValueSet.SampleValues[2].AverageValue).c_str(), (char *)"Edm.String");
            AnalogPropertiesArray[4] = (EntityProperty)TableEntityProperty((char *)"T_4", (char *)floToStr(sampleValueSet.SampleValues[3].AverageValue).c_str(), (char *)"Edm.String");         
          #else
            AnalogPropertiesArray[0] = (EntityProperty)TableEntityProperty((char *)"SampleTime", (char *) sampleTime, (char *)"Edm.String");
            AnalogPropertiesArray[1] = (EntityProperty)TableEntityProperty((char *)"T_1", (char *)floToStr(sampleValueSet.SampleValues[0].Value).c_str(), (char *)"Edm.String");
            AnalogPropertiesArray[2] = (EntityProperty)TableEntityProperty((char *)"T_2", (char *)floToStr(sampleValueSet.SampleValues[1].Value).c_str(), (char *)"Edm.String");
            AnalogPropertiesArray[3] = (EntityProperty)TableEntityProperty((char *)"T_3", (char *)floToStr(sampleValueSet.SampleValues[2].Value).c_str(), (char *)"Edm.String");
            AnalogPropertiesArray[4] = (EntityProperty)TableEntityProperty((char *)"T_4", (char *)floToStr(sampleValueSet.SampleValues[3].Value).c_str(), (char *)"Edm.String");
          #endif

          // Create the PartitionKey (special format)
          makePartitionKey(analogTablePartPrefix, augmentPartitionKey, localTime, partitionKey, &partitionKeyLength);
          partitionKey = az_span_slice(partitionKey, 0, partitionKeyLength);

          // Create the RowKey (special format)        
          makeRowKey(localTime, rowKey, &rowKeyLength);
          
          rowKey = az_span_slice(rowKey, 0, rowKeyLength);
  
          // Create TableEntity consisting of PartitionKey, RowKey and the properties named 'SampleTime', 'T_1', 'T_2', 'T_3' and 'T_4'
          AnalogTableEntity analogTableEntity(partitionKey, rowKey, az_span_create_from_str((char *)sampleTime),  AnalogPropertiesArray, analogPropertyCount);
          
          #if SERIAL_PRINT == 1
            Serial.printf("Trying to insert %u \r\n", insertCounterApiAnalogTable01);
            Serial.printf("Analog Table Name: %s \r\n\n", (const char *)augmentedAnalogTableName.c_str()); 
          #endif
             
          // Keep track of tries to insert and check for memory leak
          insertCounterApiAnalogTable01++;

          // RoSchmi, Todo: event. include code to check for memory leaks here

          // Store Entity to Azure Cloud   
          __unused az_http_status_code insertResult =  insertTableEntity(myCloudStorageAccountPtr, myX509Certificate, (char *)augmentedAnalogTableName.c_str(), analogTableEntity, (char *)EtagBuffer);
        }
        
        if (dataContainer.hasToBeSent())   // have to send analog values read by Device (e.g. noise)?
        {
          //printf("\n###### AiOnTheEdge dataContainer has to be sent\n");
          Serial.println(F("\n###### AiOnTheEdge dataContainer has to be sent to Azure\n"));        
          // Retrieve edited sample values from container
          SampleValueSet sampleValueSet = dataContainer.getCheckedSampleValues(dateTimeUTCNow, true);
          
          

          createSampleTime(sampleValueSet.LastUpdateTime, timeZoneOffsetUTC, (char *)sampleTime);

          // Define name of the table (arbitrary name + actual year, like: AnalogTestValues2020)
          String augmentedAnalogTableName = analogTableName; 
          if (augmentTableNameWithYear)
          {
            // RoSchmi changed 10.07.2024 to resolve issue 1         
            //augmentedAnalogTableName += (dateTimeUTCNow.year());
            augmentedAnalogTableName += (localTime.year());  
          }
          
          // Create Azure Storage Table if table doesn't exist
          if (localTime.year() != dataContainer.Year)    // if new year
          {  
            az_http_status_code respCode = createTable(myCloudStorageAccountPtr, myX509Certificate, (char *)augmentedAnalogTableName.c_str());
                     
            if ((respCode == AZ_HTTP_STATUS_CODE_CONFLICT) || (respCode == AZ_HTTP_STATUS_CODE_CREATED))
            {
              dataContainer.Set_Year(localTime.year());                   
            }
            else
            {
              // Reset board if not successful
             
             //SCB_AIRCR = 0x05FA0004;             
            }                     
          }         
          // Create an Array of (here) 5 Properties
          // Each Property consists of the Name, the Value and the Type (here only Edm.String is supported)

          // Besides PartitionKey and RowKey we have 5 properties to be stored in a table row
          // (SampleTime and 4 samplevalues)
          size_t analogPropertyCount = 5;
          EntityProperty AnalogPropertiesArray[5];
          AnalogPropertiesArray[0] = (EntityProperty)TableEntityProperty((char *)"SampleTime", (char *) sampleTime, (char *)"Edm.String");

          #if ANALOG_SENSORS_USE_AVERAGE == 1
          AnalogPropertiesArray[1] = (EntityProperty)TableEntityProperty((char *)"T_1", (char *)floToStr(sampleValueSet.SampleValues[0].AverageValue).c_str(), (char *)"Edm.String");
          AnalogPropertiesArray[2] = (EntityProperty)TableEntityProperty((char *)"T_2", (char *)floToStr(sampleValueSet.SampleValues[1].AverageValue).c_str(), (char *)"Edm.String");
          AnalogPropertiesArray[3] = (EntityProperty)TableEntityProperty((char *)"T_3", (char *)floToStr(sampleValueSet.SampleValues[2].AverageValue).c_str(), (char *)"Edm.String");
          AnalogPropertiesArray[4] = (EntityProperty)TableEntityProperty((char *)"T_4", (char *)floToStr(sampleValueSet.SampleValues[3].AverageValue).c_str(), (char *)"Edm.String");
          #else
          AnalogPropertiesArray[1] = (EntityProperty)TableEntityProperty((char *)"T_1", (char *)floToStr(sampleValueSet.SampleValues[0].Value).c_str(), (char *)"Edm.String");
          AnalogPropertiesArray[2] = (EntityProperty)TableEntityProperty((char *)"T_2", (char *)floToStr(sampleValueSet.SampleValues[1].Value).c_str(), (char *)"Edm.String");
          AnalogPropertiesArray[3] = (EntityProperty)TableEntityProperty((char *)"T_3", (char *)floToStr(sampleValueSet.SampleValues[2].Value).c_str(), (char *)"Edm.String");
          AnalogPropertiesArray[4] = (EntityProperty)TableEntityProperty((char *)"T_4", (char *)floToStr(sampleValueSet.SampleValues[3].Value).c_str(), (char *)"Edm.String");
          #endif

          // Create the PartitionKey (special format)
          makePartitionKey(analogTablePartPrefix, augmentPartitionKey, localTime, partitionKey, &partitionKeyLength);
          partitionKey = az_span_slice(partitionKey, 0, partitionKeyLength);

          // Create the RowKey (special format)        
          makeRowKey(localTime, rowKey, &rowKeyLength);
          
          rowKey = az_span_slice(rowKey, 0, rowKeyLength);
  
          // Create TableEntity consisting of PartitionKey, RowKey and the properties named 'SampleTime', 'T_1', 'T_2', 'T_3' and 'T_4'
          AnalogTableEntity analogTableEntity(partitionKey, rowKey, az_span_create_from_str((char *)sampleTime),  AnalogPropertiesArray, analogPropertyCount);
          
          #if SERIAL_PRINT == 1
            Serial.printf("Trying to insert %u \r\n", insertCounterAnalogTable);
            Serial.printf("Analog Table Name: %s \r\n\n", (const char *)augmentedAnalogTableName.c_str()); 
          #endif
             
          // Keep track of tries to insert and check for memory leak
          insertCounterAnalogTable++;

          // RoSchmi, Todo: event. include code to check for memory leaks here

          // Store Entity to Azure Cloud
             
          __unused az_http_status_code insertResult =  insertTableEntity(myCloudStorageAccountPtr, myX509Certificate, (char *)augmentedAnalogTableName.c_str(), analogTableEntity, (char *)EtagBuffer);
                 
        }
        // Now test if Send On/Off values or End of day stuff?
        if (onOffDataContainer.One_hasToBeBeSent(localTime) || isLast15SecondsOfDay)
        {
          //Serial.println(F("..."));
          #if SERIAL_PRINT == 1
          Serial.println("One On/Off Value has to be sent");
          #endif

          OnOffSampleValueSet onOffValueSet = onOffDataContainer.GetOnOffValueSet();
          
          for (int i = 0; i < 4; i++)    // Do for 4 OnOff-Tables  
          {
            DateTime lastSwitchTimeDate = DateTime(onOffValueSet.OnOffSampleValues[i].LastSwitchTime.year(), 
                                                onOffValueSet.OnOffSampleValues[i].LastSwitchTime.month(), 
                                                onOffValueSet.OnOffSampleValues[i].LastSwitchTime.day());

            DateTime actTimeDate = DateTime(localTime.year(), localTime.month(), localTime.day());
            
            //RoSchmi
            //if (onOffValueSet.OnOffSampleValues[i].hasToBeSent || ((onOffValueSet.OnOffSampleValues[i].actState == true) &&  (lastSwitchTimeDate.operator!=(actTimeDate))))
            
            if (false)
            {
              if (onOffValueSet.OnOffSampleValues[i].hasToBeSent)
              {
                onOffDataContainer.Reset_hasToBeSent(i);     
                EntityProperty OnOffPropertiesArray[5];
               
                TimeSpan  onTime = onOffValueSet.OnOffSampleValues[i].OnTimeDay;
                if (lastSwitchTimeDate.operator!=(actTimeDate))
                {
                  onTime = TimeSpan(0);                 
                  onOffDataContainer.Set_OnTimeDay(i, onTime);

                  if (onOffValueSet.OnOffSampleValues[i].actState == true)
                  {
                    onOffDataContainer.Set_LastSwitchTime(i, actTimeDate);
                  }
              
                }
                          
              char OnTimeDay[15] = {0};
              sprintf(OnTimeDay, "%03i-%02i:%02i:%02i", onTime.days(), onTime.hours(), onTime.minutes(), onTime.seconds());
              createSampleTime(dateTimeUTCNow, timeZoneOffsetUTC, (char *)sampleTime);

              // Tablenames come from the onOffValueSet, here usually the tablename is augmented with the actual year
              String augmentedOnOffTableName = onOffValueSet.OnOffSampleValues[i].tableName;
              if (augmentTableNameWithYear)
              {               
                augmentedOnOffTableName += (localTime.year()); 
              }

              // Create table if table doesn't exist
              if (localTime.year() != onOffValueSet.OnOffSampleValues[i].Year)
              {
                 az_http_status_code respCode = createTable(myCloudStorageAccountPtr, myX509Certificate, (char *)augmentedOnOffTableName.c_str());
                 
                 if ((respCode == AZ_HTTP_STATUS_CODE_CONFLICT) || (respCode == AZ_HTTP_STATUS_CODE_CREATED))
                 {
                    onOffDataContainer.Set_Year(i, localTime.year());
                 }
                 else
                 {
                    delay(3000);
                     //Reset Teensy 4.1
                    //SCB_AIRCR = 0x05FA0004;      
                 }
              }

              //RoSchmi
              Serial.println(F("\nDidn't need to create Tabele\n"));

              
              
              TimeSpan TimeFromLast = onOffValueSet.OnOffSampleValues[i].TimeFromLast;

              char timefromLast[15] = {0};
              sprintf(timefromLast, "%03i-%02i:%02i:%02i", TimeFromLast.days(), TimeFromLast.hours(), TimeFromLast.minutes(), TimeFromLast.seconds());
                         
              size_t onOffPropertyCount = 5;
              OnOffPropertiesArray[0] = (EntityProperty)TableEntityProperty((char *)"ActStatus", onOffValueSet.OnOffSampleValues[i].outInverter ? (char *)(onOffValueSet.OnOffSampleValues[i].actState ? "On" : "Off") : (char *)(onOffValueSet.OnOffSampleValues[i].actState ? "Off" : "On"), (char *)"Edm.String");
              OnOffPropertiesArray[1] = (EntityProperty)TableEntityProperty((char *)"LastStatus", onOffValueSet.OnOffSampleValues[i].outInverter ? (char *)(onOffValueSet.OnOffSampleValues[i].lastState ? "On" : "Off") : (char *)(onOffValueSet.OnOffSampleValues[i].lastState ? "Off" : "On"), (char *)"Edm.String");
              OnOffPropertiesArray[2] = (EntityProperty)TableEntityProperty((char *)"OnTimeDay", (char *) OnTimeDay, (char *)"Edm.String");
              OnOffPropertiesArray[3] = (EntityProperty)TableEntityProperty((char *)"SampleTime", (char *) sampleTime, (char *)"Edm.String");
              OnOffPropertiesArray[4] = (EntityProperty)TableEntityProperty((char *)"TimeFromLast", (char *) timefromLast, (char *)"Edm.String");
          
              // Create the PartitionKey (special format)
              makePartitionKey(onOffTablePartPrefix, augmentPartitionKey, localTime, partitionKey, &partitionKeyLength);
              partitionKey = az_span_slice(partitionKey, 0, partitionKeyLength);
              
              // Create the RowKey (special format)            
              makeRowKey(localTime, rowKey, &rowKeyLength);
              
              rowKey = az_span_slice(rowKey, 0, rowKeyLength);
  
              // Create TableEntity consisting of PartitionKey, RowKey and the properties named 'SampleTime', 'T_1', 'T_2', 'T_3' and 'T_4'
              OnOffTableEntity onOffTableEntity(partitionKey, rowKey, az_span_create_from_str((char *)sampleTime),  OnOffPropertiesArray, onOffPropertyCount);
          
              onOffValueSet.OnOffSampleValues[i].insertCounter++;
                    
              // Serial.printf("OnOff Table Name: %s \r\n\n", (const char *)augmentedOnOffTableName.c_str());
              // Store Entity to Azure Cloud   
             __unused az_http_status_code insertResult =  insertTableEntity(myCloudStorageAccountPtr, myX509Certificate, (char *)augmentedOnOffTableName.c_str(), onOffTableEntity, (char *)EtagBuffer);
              
              delay(1000);     // wait at least 1 sec so that two uploads cannot have the same RowKey

              break;          // Send only one in each round of loop
              }
              else
              {
                if (isLast15SecondsOfDay && !onOffValueSet.OnOffSampleValues[i].dayIsLocked)
                {
                  if (onOffValueSet.OnOffSampleValues[i].actState == true)              
                  {               
                    onOffDataContainer.Set_ResetToOnIsNeededFlag(i, true);                 
                    onOffDataContainer.SetNewOnOffValue(i, onOffValueSet.OnOffSampleValues[i].inputInverter ? true : false, dateTimeUTCNow, timeZoneOffsetUTC);
                    delay(1000);   // because we don't want to send twice in the same second 
                    break;
                  }
                else
                {              
                  if (onOffValueSet.OnOffSampleValues[i].resetToOnIsNeeded)
                  {                  
                    onOffDataContainer.Set_DayIsLockedFlag(i, true);
                    onOffDataContainer.Set_ResetToOnIsNeededFlag(i, false);
                    onOffDataContainer.SetNewOnOffValue(i, onOffValueSet.OnOffSampleValues[i].inputInverter ? false : true, dateTimeUTCNow, timeZoneOffsetUTC);
                    break;
                  }                 
                }              
              }
              } 
            }                                
          }               
        }       
      }          
  } 
}


uint8_t connectMultiWiFi()
{
#if ESP32
  // For ESP32, this better be 0 to shorten the connect time.
  // For ESP32-S2/C3, must be > 500
  #if ( USING_ESP32_S2 || USING_ESP32_C3 )
    #define WIFI_MULTI_1ST_CONNECT_WAITING_MS           500L
  #else
    // For ESP32 core v1.0.6, must be >= 500
    #define WIFI_MULTI_1ST_CONNECT_WAITING_MS           800L
  #endif
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS             2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS                   500L

  uint8_t status;

  //WiFi.mode(WIFI_STA);

  LOGERROR(F("ConnectMultiWiFi with :"));

  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
    LOGERROR3(F("In connectMultiWiFi() * Add Router-SSID = "), Router_SSID, F(", PW = "), Router_Pass );
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
    {
      LOGERROR3(F("In connectMultiWiFi() * Add Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
      // RoSchmi added
      wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
    }
    else
    {
      //RoSchmi added 20.07.2024 
      LOGERROR3(F("In connectMultiWiFi() * Additional SSID neglected = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
    }
      

  LOGERROR(F("Connecting MultiWifi..."));

  //WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
  // New in v1.4.0
  configWiFi(WM_STA_IPconfig);
  //////
#endif

  
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);
  int i2 = 0;
  while ( ( i2++ < 20 ) && ( status != WL_CONNECTED ) )
  {
    status = WiFi.status();

    if ( status == WL_CONNECTED )
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if ( status == WL_CONNECTED )
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
  }
  else
  {
    LOGERROR(F("WiFi not connected"));
 
#if ESP8266      
    ESP.reset();
#else
    ESP.restart();
#endif  
  }

  }

  return status;
}

// To manage daylightsavingstime stuff convert input ("Last", "First", "Second", "Third", "Fourth") to int equivalent
int getWeekOfMonthNum(const char * weekOfMonth)
{
  for (int i = 0; i < 5; i++)
  {  
    if (strcmp((char *)timeNameHelper.weekOfMonth[i], weekOfMonth) == 0)
    {
      return i;
    }   
  }
  return -1;
}

int getMonNum(const char * month)
{
  for (int i = 0; i < 12; i++)
  {  
    if (strcmp((char *)timeNameHelper.monthsOfTheYear[i], month) == 0)
    {
      return i + 1;
    }   
  }
  return -1;
}

int getDayNum(const char * day)
{
  for (int i = 0; i < 7; i++)
  {  
    if (strcmp((char *)timeNameHelper.daysOfTheWeek[i], day) == 0)
    {
      return i + 1;
    }   
  }
  return -1;
}

// Scan for available Wifi networks
// print result als simple list
void scan_WIFI() 
{
      Serial.println("WiFi scan ...");
      // WiFi.scanNetworks returns the number of networks found
      int n = WiFi.scanNetworks();
      if (n == 0) {
          Serial.println("[ERR] no networks found");
      } else {
          
          Serial.printf("[OK] %i networks found:\n", n);       
          for (int i = 0; i < n; ++i) {
              // Print SSID for each network found
              Serial.printf("  %i: ",i+1);
              Serial.println(WiFi.SSID(i));
              delay(10);
          }
      }
}

// establish the connection to an Wifi Access point
// This function (from a former version) is not used in this application but
// gives insight in the process of connecting, so it is left here as an example
// in this application the function 'connectMultiWiFi' is used
/*
boolean connect_Wifi(const char *ssid, const char * password)
{
  // Establish connection to the specified network until success.
  // Important to disconnect in case that there is a valid connection
  WiFi.disconnect();
  Serial.println("Connecting to ");
  Serial.println(ssid);
  delay(500);
  //Start connecting (done by the ESP in the background)
  
  #if USE_WIFI_STATIC_IP == 1
  IPAddress presetIp(192, 168, 1, 83);
  IPAddress presetGateWay(192, 168, 1, 1);
  IPAddress presetSubnet(255, 255, 255, 0);
  IPAddress presetDnsServer1(8,8,8,8);
  IPAddress presetDnsServer2(8,8,4,4);

  WiFi.config(presetIp, presetGateWay, presetDnsServer1, presetDnsServer2);
  #endif

  WiFi.begin(ssid, password, 6);
  // read wifi Status
  wl_status_t wifi_Status = WiFi.status();  
  int n_trials = 0;
  // loop while waiting for Wifi connection
  // run only for 5 trials.
  while (wifi_Status != WL_CONNECTED && n_trials < 5) {
    // Check periodicaly the connection status using WiFi.status()
    // Keep checking until ESP has successfuly connected
    // or maximum number of trials is reached
    wifi_Status = WiFi.status();
    n_trials++;
    switch(wifi_Status){
      case WL_NO_SSID_AVAIL:
          Serial.println("[ERR] SSID not available");
          break;
      case WL_CONNECT_FAILED:
          Serial.println("[ERR] Connection failed");
          break;
      case WL_CONNECTION_LOST:
          Serial.println("[ERR] Connection lost");
          break;
      case WL_DISCONNECTED:
          Serial.println("[ERR] WiFi disconnected");
          break;
      case WL_IDLE_STATUS:
          Serial.println("[ERR] WiFi idle status");
          break;
      case WL_SCAN_COMPLETED:
          Serial.println("[OK] WiFi scan completed");
          break;
      case WL_CONNECTED:
          Serial.println("[OK] WiFi connected");
          break;
      default:
          Serial.println("[ERR] unknown Status");
          break;
    }
    delay(500);
  }
  if(wifi_Status == WL_CONNECTED){
    // connected
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    // not connected
    Serial.println("");
    Serial.println("[ERR] unable to connect Wifi");
    return false;
  }
}
*/

String floToStr(float value)
{
  char buf[10];
  sprintf(buf, "%.1f", (roundf(value * 10.0))/10.0);
  return String(buf);
}

ViessmannApiSelection::Feature ReadViessmannFeatureFromSelection(const char * pSensorName, uint16_t pFeaturesCountMax100)
{
  // preset a 'returnFeature' with value = MAGIC_NUMBER_INVALID (999.9)
  ViessmannApiSelection::Feature returnFeature;
  strncpy(returnFeature.value, (floToStr(MAGIC_NUMBER_INVALID)).c_str(), sizeof(returnFeature.value) - 1);
  
  uint16_t featuresCount = pFeaturesCountMax100 < 100 ? pFeaturesCountMax100 : 100;
  for (uint16_t i = 0; i < featuresCount; i++)
  {
    if (strcmp((const char *)features[i].name, pSensorName) == 0)
    {
      returnFeature = features[i];
      break;
    }
  }
  return returnFeature;
}

AiOnTheEdgeApiSelection::Feature ReadAiOnTheEdgeApi_Analog_01(int pSensorIndex, const char* pSensorName, AiOnTheEdgeApiSelection * pAiOnTheEdgeApiSelectionPtr)
{
  // Use values read from the AiOnTheEge Rest API
  // pSensorIndex determins the position (from 4). pSensorName is the name of the feature (see AiOnTheEdgeSelection.h)
  
  // preset a 'returnFeature' with value = MAGIC_NUMBER_INVALID (999.9)
  AiOnTheEdgeApiSelection::Feature returnFeature;
  strncpy(returnFeature.value, (floToStr(MAGIC_NUMBER_INVALID)).c_str(), sizeof(returnFeature.value) - 1);
  
  // Only read features from AiOnTheEdgeDevice when readInterval has expired
  
  int32_t remainigSeconds = pAiOnTheEdgeApiSelectionPtr ->lastReadTime.secondstime() + pAiOnTheEdgeApiSelectionPtr -> readInterval.totalseconds() - dateTimeUTCNow.secondstime();
  
  if ((pAiOnTheEdgeApiSelectionPtr ->lastReadTime.operator+(pAiOnTheEdgeApiSelectionPtr -> readInterval)).operator<(dateTimeUTCNow))
  {
    char myUriEndpoint[50] = {0}; 
    strncpy(myUriEndpoint, "http://gasmeter/json", sizeof(myUriEndpoint) - 1);
    
    httpCode = readJsonFromRestApi(myX509Certificate, (const char *)myUriEndpoint, sizeof(myUriEndpoint), pAiOnTheEdgeApiSelectionPtr);   
    
    if (httpCode == t_http_codes::HTTP_CODE_OK)
    {
      pAiOnTheEdgeApiSelectionPtr -> lastReadTime = dateTimeUTCNow;  
    }
    else
    {
      pAiOnTheEdgeApiSelectionPtr ->lastReadTime = dateTimeUTCNow;
      Serial.println(F("Failed to read Features from Ai-On-The-Edge-Device")); 
      Serial.println((char*)bufferStorePtr);
    }
  }
   
  if (analogSensorMgr.HasToBeRead(pSensorIndex, dateTimeUTCNow, true))
  {
    for (int i = 0; i < AI_FEATURES_COUNT; i++)
    {       
      if (strcmp((const char *)ai_features[i].name, pSensorName) == 0)
      {       
        returnFeature = ai_features[i];

        Serial.printf("\nAnalogSensorMgr: Value is used. Index: %d Value: %s\n", pSensorIndex, returnFeature.value);
        
        analogSensorMgr.SetReadTimeAndValues(pSensorIndex, dateTimeUTCNow, atof(returnFeature.value), 0.0f, MAGIC_NUMBER_INVALID);                         
        break;
      }     
    } 
  } 
  return returnFeature;
}

ViessmannApiSelection::Feature ReadViessmannApi_Analog_01(int pSensorIndex, const char* pSensorName)
{
  
  // Use values read from the Viessmann API
  // pSensorIndex determins the position (from 4). pSensorName is the name of the feature (see ViessmannApiSelection.h)
  
  // preset a 'returnFeature' with value = MAGIC_NUMBER_INVALID (999.9)
  ViessmannApiSelection::Feature returnFeature;
  strncpy(returnFeature.value, (floToStr(MAGIC_NUMBER_INVALID)).c_str(), sizeof(returnFeature.value) - 1);
  
  // Only read features from the cloud when readInterval has expired
  
  if ((viessmannApiSelection.lastReadTime.operator+(viessmannApiSelection.readInterval)).operator<(dateTimeUTCNow))
  {
    uint32_t tryCounter = 1;
    httpCode = -1;
    while (tryCounter > 0 && httpCode != t_http_codes::HTTP_CODE_OK)
    {
      httpCode = readViessmannFeaturesFromApi(myX509Certificate, myViessmannApiAccountPtr, Data_0_Id, Gateways_0_Serial, Gateways_0_Devices_0_Id, viessmannApiSelectionPtr);
      if (httpCode == t_http_codes::HTTP_CODE_OK)
      {
        viessmannApiSelection.lastReadTime = dateTimeUTCNow;
        Serial.println(F("Succeeded to read Features from Viessmann Cloud\n"));
      }
      else
      {
        viessmannApiSelection.lastReadTime = dateTimeUTCNow;
        Serial.println(F("Failed to read Features from Viessmann Cloud")); 
        //Serial.println((char*)bufferStorePtr);
      }
      tryCounter--;
    }
  }
  
  if (analogSensorMgr_Api_01.HasToBeRead(pSensorIndex, dateTimeUTCNow))
  {
    //Serial.printf("\nAnalogSensorMgr_Api_01: Value is used %d\n", pSensorIndex); 
      
    for (int i = 0; i < VI_FEATURES_COUNT; i++)
    {       
      if (strcmp((const char *)features[i].name, pSensorName) == 0)
      {
        returnFeature = features[i];
        analogSensorMgr_Api_01.SetReadTimeAndValues(pSensorIndex, dateTimeUTCNow, atof(returnFeature.value), 0.0f, MAGIC_NUMBER_INVALID);           
        break;
      }
    } 
  } 
  return returnFeature;
}

ValueStruct ReadAnalogSensorStruct_01(int pSensorIndex)
{
  ValueStruct returnValueStruct = { 
      .displayValue = (float)MAGIC_NUMBER_INVALID,
      .unClippedValue = (float)MAGIC_NUMBER_INVALID};

  char consumption[12] = {0};

  switch(pSensorIndex)
  {
      case 0:
      { 
        // Total Consumption, DisplayValue and UnClippedValue 
        AiOnTheEdgeApiSelection * aiOnTheEdgeApiSelectionPtr = gasmeterApiSelectionPtr;                      
        // select Feature by its name (pSensorName value, raw, pre, error, rate, timestamp)
        
        AiOnTheEdgeApiSelection::Feature selectedFeature;
        if (isFirstGasmeterRead)
        {
          // Read raw instead of value
          selectedFeature = ReadAiOnTheEdgeApi_Analog_01(pSensorIndex, (const char *)"raw", aiOnTheEdgeApiSelectionPtr);
          isFirstGasmeterRead = false;
          char gasMeterUrl[] = "http://gasmeter";
           
          char preValue[12] = {0};
          
          strncpy(preValue, (const char *)selectedFeature.value, sizeof(preValue));
          preValue[sizeof(preValue) -1] = '\0';
          // Write the last raw Value to preValue
          setAiPreValueViaRestApi(myX509Certificate, gasMeterUrl, strlen(gasMeterUrl), (const char *)preValue);
        }
        else
        {
          selectedFeature = ReadAiOnTheEdgeApi_Analog_01(pSensorIndex, (const char *)"value", aiOnTheEdgeApiSelectionPtr);
          
        }

        strncpy(consumption, selectedFeature.value, sizeof(consumption));                                          
     
        float tempNumber = (float)MAGIC_NUMBER_INVALID;
        
        //Serial.printf("writing consumption: %s Length: %d\n", (const char *)consumption, strlen(consumption));
         
        if (isValidFloat(consumption) && strlen(consumption) > strlen("0.00"))
        {
          tempNumber = atof(consumption);
          #if SERIAL_PRINT == 1
            Serial.printf("\nString could be parsed to float: %s StringLength: %d\n", consumption, strlen(consumption));
          #endif
        }
        else
        {
          Serial.printf("Value invalid (0.00) or could not be parsed to float: %s\n", consumption);
          tempNumber = (float)MAGIC_NUMBER_INVALID;
          // RoSchmi
          /*
          while (true)
          {
            delay(500);
          }
          */
        }
        
        // convert values near MAGIC_NUMBER_INVALID to MAGIC_NUMBER_INVALID  
        tempNumber = (tempNumber > (float)MAGIC_NUMBER_INVALID - 0.01 && tempNumber < (float)MAGIC_NUMBER_INVALID + 0.01) ? (float)MAGIC_NUMBER_INVALID: tempNumber;
        
        if (tempNumber > (MAGIC_NUMBER_INVALID - 0.01) && tempNumber < (MAGIC_NUMBER_INVALID + 0.01))
        {
          // if tempNumber = MAGIC_NUMBER_INVALID return both with MAGIC_NUMBER_INVALID
          // was preset at the beginning
          // this means, that the values are ignored in further process
          return returnValueStruct;
        }
        // multply by 10, so that there remains 1 significant digit after decimal point
        sprintf(consumption, "%.1f", tempNumber * 10);
        returnValueStruct.unClippedValue = atof((const char *)consumption);
        
        // remove leading digits, so that there are maximal 2 digits 
        // before decimal point for .displayValue
        while (strlen(consumption) > 4)
        {
            memmove(consumption, consumption + 1, strlen(consumption));
        }
        returnValueStruct.displayValue = atof((const char *)consumption);
        
        //#if SERIAL_PRINT == 1
            Serial.printf("\nDisplayValue: %.1f  UnClippedValue: %.1f\n", returnValueStruct.displayValue, returnValueStruct.unClippedValue);
        //#endif                                                                                                                        
      }
      break;                   
      case 1:    // Consumption this day
      {
        // Calculate Day-Consumption from BaseValue and UnClippedValue 
        float copyBaseValue = dataContainer.SampleValues[0].BaseValue;
        float copyUnClippedValue = dataContainer.SampleValues[0].UnClippedValue;
        uint32_t LastSendTimeSeconds = dataContainer._lastSentTime.secondstime();
        uint32_t timeSinceLastSendSeconds = dateTimeUTCNow.secondstime() - LastSendTimeSeconds;
                 
        if (copyBaseValue <= copyUnClippedValue)
        {
          returnValueStruct.displayValue = (copyUnClippedValue - copyBaseValue); 
        }
        else
        {
          // Overflow of copyUnClippedValue has occured
          int preDecimalPoint = (int)copyBaseValue;
          int oneMoreDigit =  pow(10,(int)log10(preDecimalPoint) + 1);
          returnValueStruct.displayValue = copyUnClippedValue + (oneMoreDigit - copyBaseValue);
        }
              
        returnValueStruct.unClippedValue = copyUnClippedValue;
        if (timeSinceLastSendSeconds > dataContainer.SendInterval.totalseconds() -3)
        {                      
          Serial.printf("\nCase 1: Day-Consumption: %.1f\n", returnValueStruct.displayValue);
        }                      
      }
      break;
      case 2:   // rate
      {
        uint32_t sendIntervalSeconds = dataContainer.SendInterval.totalseconds();
        uint32_t LastSendTimeSeconds = dataContainer._lastSentTime.secondstime();       
        
        float copyLastSendUnClippedValue = dataContainer.SampleValues[0].LastSendUnClippedValue;
        float copyUnClippedValue = dataContainer.SampleValues[0].UnClippedValue;                    
        uint32_t timeSinceLastSendSeconds = dateTimeUTCNow.secondstime() - LastSendTimeSeconds;
             
          // Only print the last messages
          if (timeSinceLastSendSeconds > dataContainer.SendInterval.totalseconds() -3)
          {
            Serial.printf("LastSendTime: %d Actual: %.d Diff: %d Interval: %d Remain %d\n", LastSendTimeSeconds,  
            dateTimeUTCNow.secondstime(), dateTimeUTCNow.secondstime() - LastSendTimeSeconds, sendIntervalSeconds, 
            LastSendTimeSeconds + sendIntervalSeconds - dateTimeUTCNow.secondstime());
          }
   
        float valueDiffOverflowCorrected = 0;

        if (copyLastSendUnClippedValue <= copyUnClippedValue)
        {
          valueDiffOverflowCorrected = (copyUnClippedValue - copyLastSendUnClippedValue);
          //returnValueStruct.displayValue = (copyUnClippedValue - copyLastSendUnClippedValue); 
        }
        else
        {
          // Overflow of copyUnClippedValue has occured
          int preDecimalPoint = (int)copyLastSendUnClippedValue;
          int oneMoreDigit =  pow(10,(int)log10(preDecimalPoint) + 1);  
          valueDiffOverflowCorrected = copyUnClippedValue + (oneMoreDigit - copyLastSendUnClippedValue);
        }  
        
        float rate = 0.0;

        if (valueDiffOverflowCorrected != 0.0f)
        {
            if (timeSinceLastSendSeconds > 1.0)
            {             
              rate = valueDiffOverflowCorrected * 50.0f / ((float)timeSinceLastSendSeconds / 60.0f); // *50 gives reasonable size in         
            }
        }
          // RoSchmi
          // Only print the last messages
          if (timeSinceLastSendSeconds > dataContainer.SendInterval.totalseconds() -3)
          {
            Serial.printf("LastSendValue: %.1f Actual: %.1f Diff: %.1f Seconds: %d\n", copyLastSendUnClippedValue, 
            copyUnClippedValue, copyUnClippedValue - copyLastSendUnClippedValue, timeSinceLastSendSeconds);  
          
            Serial.printf("\nCase 2: Flow is: %.1f per minute. Read after: %d seconds\n\n", rate, timeSinceLastSendSeconds);
          }
          // Neglect (set to MAGIC_NUMBER_INVALID when timeSinceLastSendSeconds < 5 sec)
          returnValueStruct.displayValue = timeSinceLastSendSeconds > 5 ? rate : (float)MAGIC_NUMBER_INVALID;
          returnValueStruct.unClippedValue = copyUnClippedValue;                              
      }
      break;
      case 3:
      {

          // in the forth graph show Viessmann Vorlauftemperatur
          
          //SampleValueSet featureValueSet = dataContainerAnalogViessmann01.getCheckedSampleValues(dateTimeUTCNow, false);         
          //returnValueStruct.displayValue = featureValueSet.SampleValues[1].Value;
          //returnValueStruct.unClippedValue = returnValueStruct.displayValue;
                     
          // This is an alternative way to get a Viessmann Api Sensor valu                      
          
          //SampleValueSet featureValueSet = dataContainerAnalogViessmann01.getCheckedSampleValues(dateTimeUTCNow, false);         
          //theRead = featureValueSet.SampleValues[1].Value;
          
                      
          //theRead = atoi((char *)sSwiThresholdStr) / 10; // dummy
          //Show ascending lines from 0 to 5, so re-boots of the board are indicated                                                
          //theRead = ((double)(insertCounterAnalogTable % 50)) / 10;
          returnValueStruct.displayValue = ((float)(insertCounterAnalogTable % 50)) / 10;
          returnValueStruct.unClippedValue = 0.0;                      
      }                   
      break;
    }
  //}

    return returnValueStruct;
}
  
/*
float ReadAnalogSensor_01(int pSensorIndex)
{
#ifndef USE_SIMULATED_SENSORVALUES
            // Use values read from an analog source
            // Change the function for each sensor to your needs
            // This is an an adaption for a gasmeter
              
              //RoSchmi: Next line was deleted
              //static float unclippedBaseValue_01 = 0.0;
              double theRead = MAGIC_NUMBER_INVALID;           
              if (analogSensorMgr.HasToBeRead(pSensorIndex, dateTimeUTCNow, true))
              {                              
                switch (pSensorIndex)
                {
                  case 0:
                    { 
                      char consumption[12] = {0};
                      AiOnTheEdgeApiSelection * aiOnTheEdgeApiSelectionPtr = gasmeterApiSelectionPtr;
                      
                      // select Feature by its name (pSensorName value, raw, pre, error, rate, timestamp)
                      AiOnTheEdgeApiSelection::Feature selectedFeature = ReadAiOnTheEdgeApi_Analog_01(pSensorIndex, (const char *)"value", aiOnTheEdgeApiSelectionPtr);
                      strncpy(consumption, selectedFeature.value, sizeof(consumption));                        
                        
                      // Formatiere zu: Eine Nachkommastelle und nicht mehr als 4 Vorkommastellen
                      float tempNumber = (atof((const char *)consumption));
                      tempNumber = (tempNumber > 999.89 && tempNumber < 999.91) ? 999.9 : tempNumber * 10;
                      sprintf(consumption, "%.1f", tempNumber);
                      // Remove leading digits, so that the string is not longer than 6 characters
                      while (strlen(consumption) > 6)
                      {
                        memmove(consumption, consumption + 1, strlen(consumption));
                      }
                      

                      int32_t timeDiffUtcToLocal = localTime.secondstime() - dateTimeUTCNow.secondstime();                   
                      bool isNewDay = dateTimeUTCNow.operator+(timeDiffUtcToLocal).day() != dataContainer._lastSentTime.operator+(timeDiffUtcToLocal).day();
                      
                      //uint8_t dayUTC = dateTimeUTCNow.day();
                      //uint8_t daylocal = localTime.day();

                      Serial.printf("\n\nTimeDiffInSeconds: %d \n\n", timeDiffUtcToLocal);

                      if (dataContainer._isFirstTransmission || isNewDay)
                      {
                        dataContainer._baseValue = atof((const char *)consumption);                
                      }
                                                                
                      theRead = atof((const char *)consumption) / 100;
                     
                      // As an example we use it here to display an analog value read from the Viessmann Api   
                      
                      // Possible way to get a Viessmann Api Sensor value
                      //ViessmannApiSelection::Feature featureValue = ReadViessmannFeatureFromSelection((const char *)"_92_heating_dhw_outlet_temperature", VI_FEATURES_COUNT);
                      //theRead = atof((char *)featureValue.value);

                      // This is an alternative way to get a Viessmann Api Sensor valu                      
                      
                      //SampleValueSet featureValueSet = dataContainerAnalogViessmann01.getCheckedSampleValues(dateTimeUTCNow, false);         
                      //theRead = featureValueSet.SampleValues[1].Value;
                      
                     
                      #if SERIAL_PRINT == 1
                        //Serial.printf("\r\nApi-Sensor: %.1f\r\n", theRead);
                      #endif

                      // Take theRead (nearly) 0.0 as invalid
                      // (if no sensor is connected the function returns 0)
                                             
                      //if (theRead > - 0.00001 && theRead < 0.00001)
                      //{
                      //  theRead = MAGIC_NUMBER_INVALID;
                      //}
                                                                                                                          
                    }
                    break;                   
                  case 1:
                    { 
                      char consumption[12] = {0};
                      AiOnTheEdgeApiSelection * aiOnTheEdgeApiSelectionPtr = gasmeterApiSelectionPtr;
                      
                      // select Feature by its name (pSensorName: value, raw, pre, error, rate or timestamp)
                      AiOnTheEdgeApiSelection::Feature selectedFeature = ReadAiOnTheEdgeApi_Analog_01(pSensorIndex, (const char *)"value", aiOnTheEdgeApiSelectionPtr);
                      
                      strncpy(consumption, selectedFeature.value, sizeof(consumption));                        
                      
                      // Formatiere zu: Eine Nachkommastelle und nicht mehr als 3 Vorkommastellen
                      float tempNumber = (atof((const char *)consumption));
                      tempNumber = (tempNumber > 999.89 && tempNumber < 999.91) ? 999.9 : (tempNumber *10 );
                      sprintf(consumption, "%.1f", tempNumber);
                      // Remove leading digits, so that the string is not longer than 6 characters
                      while (strlen(consumption) > 6)
                      {
                        memmove(consumption, consumption + 1, strlen(consumption));
                      }

                      float actValue = atof((const char *)consumption);
                      float baseValue = dataContainer._baseValue;   // _baseValue was stored in case 0:
                          
                      // Get consumption from start of day or since programstart
                      theRead = actValue - baseValue;

                      printf("\nDay  -Consumption: %.1f\n", (float)theRead); 
                         
                    }
                    break;
                  case 2:
                    {
                      char consumption[12] = {0};
                      AiOnTheEdgeApiSelection * aiOnTheEdgeApiSelectionPtr = gasmeterApiSelectionPtr;
                      
                      // select Feature by its name (pSensorName value, raw, pre, error, rate, timestamp)
                      AiOnTheEdgeApiSelection::Feature selectedFeature = ReadAiOnTheEdgeApi_Analog_01(pSensorIndex, (const char *)"value", aiOnTheEdgeApiSelectionPtr);                      
                      strncpy(consumption, selectedFeature.value, sizeof(consumption));                        
                                         
                      // Formatiere zu: Eine Nachkommastelle und nicht mehr als 3 Vorkommastellen
                      float tempNumber = (atof((const char *)consumption));
                      tempNumber = (tempNumber > 999.89 && tempNumber < 999.91) ? 999.9 : (tempNumber *10 );
                      sprintf(consumption, "%.1f", tempNumber);
                      // Remove leading digits, so that the string is not longer than 6 characters
                      while (strlen(consumption) > 6)
                      {
                        memmove(consumption, consumption + 1, strlen(consumption));
                      }

                      float actValue = atof((const char *)consumption);
                      //float baseValue = dataContainer._baseValue;
                      float lastSentValue = dataContainer._SampleValuesSet.SampleValues[0].LastSendUnClippedValue;
                      uint32_t elapsedTime = dateTimeUTCNow.secondstime() - dataContainer._lastSentTime.secondstime();
                      if ((elapsedTime % 5) == 0)  // show only every fifth case
                      {                
                        Serial.printf("ReadAnalogSensor_01 Case 2: ActValue: %.1f LastSentValue: %.1f Elapsed: %d seconds\n", actValue, lastSentValue, elapsedTime);
                      }
                      
                      theRead = (actValue - lastSentValue) * 100 / ((dateTimeUTCNow.secondstime() - dataContainer._lastSentTime.secondstime())/ 60);


                      
                      
                    }
                    break;
                  case 3:
                    {
                      SampleValueSet featureValueSet = dataContainerAnalogViessmann01.getCheckedSampleValues(dateTimeUTCNow, false);         
                      theRead = featureValueSet.SampleValues[1].Value;
                      //theRead = atoi((char *)sSwiThresholdStr) / 10; // dummy
                      //Show ascending lines from 0 to 5, so re-boots of the board are indicated                                                
                        //theRead = ((double)(insertCounterAnalogTable % 50)) / 10;                      
                    }                   
                    break;
                }
              }                               
           return (float)theRead ;
#endif

#ifdef USE_SIMULATED_SENSORVALUES
      #ifdef USE_TEST_VALUES
            // Here you can select that diagnostic values (for debugging)
            // are sent to your storage table
            double theRead = MAGIC_NUMBER_INVALID;
            switch (pSensorIndex)
            {
                case 0:
                    {
                        theRead = timeNtpUpdateCounter;
                        theRead = theRead / 10; 
                    }
                    break;

                case 1:
                    {                       
                        theRead = sysTimeNtpDelta > 140 ? 140 : sysTimeNtpDelta < - 40 ? -40 : (double)sysTimeNtpDelta;                      
                    }
                    break;
                case 2:
                    {
                        theRead = insertCounterAnalogTable;
                        theRead = theRead / 10;                      
                    }
                    break;
                case 3:
                    {
                        theRead = lastResetCause;                       
                    }
                    break;
            }

            return theRead ;

  

        #endif
            
            onOffSwitcherWio.SetActive();
            // Only as an example we here return values which draw a sinus curve            
            int frequDeterminer = 4;
            int y_offset = 1;
            // different frequency and y_offset for aIn_0 to aIn_3
            if (pSensorIndex == 0)
            { frequDeterminer = 4; y_offset = 1; }
            if (pSensorIndex == 1)
            { frequDeterminer = 8; y_offset = 10; }
            if (pSensorIndex == 2)
            { frequDeterminer = 12; y_offset = 20; }
            if (pSensorIndex == 3)
            { frequDeterminer = 16; y_offset = 30; }
             
            int secondsOnDayElapsed = dateTimeUTCNow.second() + dateTimeUTCNow.minute() * 60 + dateTimeUTCNow.hour() *60 *60;

            // RoSchmi
            switch (pSensorIndex)
            {
              case 3:
              {

                //return (double)9.9;   // just something for now
                return lastResetCause;
              }
              break;
            
              case 2:
              { 
                uint32_t showInsertCounter = insertCounterAnalogTable % 50;               
                double theRead = ((double)showInsertCounter) / 10;
                return theRead;
              }
              break;
              case 0:
              case 1:
              {
                return roundf((float)25.0 * (float)sin(PI / 2.0 + (secondsOnDayElapsed * ((frequDeterminer * PI) / (float)86400)))) / 10  + y_offset;          
              }
              break;
              default:
              {
                return 0;
              }
            }
  #endif
}
*/

void createSampleTime(DateTime dateTimeUTCNow, int timeZoneOffsetUTC, char * sampleTime)
{
  int hoursOffset = timeZoneOffsetUTC / 60;
  int minutesOffset = timeZoneOffsetUTC % 60;
  char sign = timeZoneOffsetUTC < 0 ? '-' : '+';
  char TimeOffsetUTCString[10];
  sprintf(TimeOffsetUTCString, " %c%03i", sign, timeZoneOffsetUTC);
  TimeSpan timespanOffsetToUTC = TimeSpan(0, hoursOffset, minutesOffset, 0);
  DateTime newDateTime = dateTimeUTCNow + timespanOffsetToUTC;
  sprintf(sampleTime, "%02i/%02i/%04i %02i:%02i:%02i%s",newDateTime.month(), newDateTime.day(), newDateTime.year(), newDateTime.hour(), newDateTime.minute(), newDateTime.second(), TimeOffsetUTCString);
}
 
void makeRowKey(DateTime actDate,  az_span outSpan, size_t *outSpanLength)
{
  // formatting the RowKey (= reverseDate) this way to have the tables sorted with last added row upmost
  char rowKeyBuf[20] {0};

  sprintf(rowKeyBuf, "%4i%02i%02i%02i%02i%02i", (10000 - actDate.year()), (12 - actDate.month()), (31 - actDate.day()), (23 - actDate.hour()), (59 - actDate.minute()), (59 - actDate.second()));
  az_span retValue = az_span_create_from_str((char *)rowKeyBuf);
  az_span_copy(outSpan, retValue);
  *outSpanLength = retValue._internal.size;         
}

void makePartitionKey(const char * partitionKeyprefix, bool augmentWithYear, DateTime dateTime, az_span outSpan, size_t *outSpanLength)
{
  // if wanted, augment with year and month (12 - month for right order)                    
  char dateBuf[20] {0};
  sprintf(dateBuf, "%s%d-%02d", partitionKeyprefix, (dateTime.year()), (12 - dateTime.month()));                  
  az_span ret_1 = az_span_create_from_str((char *)dateBuf);
  az_span ret_2 = az_span_create_from_str((char *)partitionKeyprefix);                       
  if (augmentWithYear == true)
  {
    az_span_copy(outSpan, ret_1);            
    *outSpanLength = ret_1._internal.size; 
  }
    else
  {
    az_span_copy(outSpan, ret_2);
    *outSpanLength = ret_2._internal.size;
  }    
}

bool isValidFloat(const char* str) 
{
  char* endptr;
  // Versuch, den String in float umzuwandeln
  float value = strtof(str, &endptr);

  // Wenn endptr zum Ende des Strings zeigt, war die Konvertierung erfolgreich
  if (*endptr == '\0' && endptr != str) {
      return true; // Ist eine gültige float-Zahl
  }
  return false; // Ungültig
}

bool extractSubString (const char * source, const String startTag, const String endTag, char * result, const int maxResultLength)
{
  char * start = strstr(source, startTag.c_str());
  if (start)
  {
    start += startTag.length();
    const char * end = strstr(start, endTag.c_str());
    if (end)
    {
      size_t length = end - start < maxResultLength - 1 ? end - start : maxResultLength - 1;
      strncpy(result, start, length);
      result[length] = '\0';      
      return true;
    }
    else
    {
      result[0] = '\0';
      return false;
    }
  }
  else
  {
     result[0] = '\0';
     return false;
  }
}

t_httpCode setAiPreValueViaRestApi(X509Certificate pCaCert, const char * pUrl, int pUrlMaxlength, const char * pPreValue)
{
  
  
  //aiOnTheEdgeClient(pCaCert, httpPtr, wifi_client);
  
  AiOnTheEdgeClient aiOnTheEdgeClient(pCaCert, httpPtr, &plain_wifi_client);

t_httpCode responseCode = aiOnTheEdgeClient.SetPreValue((const char *)pUrl, pPreValue,  bufferStorePtr, bufferStoreLength);

if (responseCode == t_http_codes::HTTP_CODE_OK)
  {
    Serial.printf("\nPrevalue successfully set: %s\n", pPreValue);
  }

return responseCode;
}  

t_httpCode readJsonFromRestApi(X509Certificate pCaCert, const char * pUrl, int pUrlMaxlength, AiOnTheEdgeApiSelection * apiSelectionPtr)
{
  /*
  #if AIONTHEEDGE_TRANSPORT_PROTOCOL == 1
    static WiFiClientSecure wifi_client;
  #else  
    static WiFiClient wifi_client;
  #endif
  
  #if AIONTHEEDGE_TRANSPORT_PROTOCOL == 1 
    wifi_client.setCACert(myX509Certificate);
  #endif
  */

  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif
  
  memset(bufferStorePtr, '\0', bufferStoreLength);
  
  AiOnTheEdgeClient aiOnTheEdgeClient(pCaCert, httpPtr, &plain_wifi_client);
  
  Serial.printf("\r\n(%u) ", loadGasMeterJsonCount);
  Serial.printf("%i/%02d/%02d %02d:%02d ", localTime.year(), 
                                        localTime.month() , localTime.day(),
                                        localTime.hour() , localTime.minute());
  
  char url[pUrlMaxlength + 1] = {0};
  strncpy(url, pUrl, pUrlMaxlength);
     
   Serial.printf("readJsonFromRestApi: %s\n", (const char *)url);

  t_httpCode responseCode = aiOnTheEdgeClient.GetFeatures((const char *)url, bufferStorePtr, bufferStoreLength, apiSelectionPtr);
  loadGasMeterJsonCount++;
  if (responseCode == t_http_codes::HTTP_CODE_OK)
  {
    // Populate features array and replace the name read from Api
    // with the name used in this Application
    
    ai_features[0] = apiSelectionPtr ->_0_value;
    ai_features[1] = apiSelectionPtr ->_1_raw;
    ai_features[2] = apiSelectionPtr ->_2_pre;
    ai_features[3] = apiSelectionPtr ->_3_error;
    ai_features[4] = apiSelectionPtr ->_4_rate;
    ai_features[5] = apiSelectionPtr ->_5_timestamp;      
  }
  else
  {
    
    ai_features[0] = apiSelectionPtr ->_0_value;
    ai_features[1] = apiSelectionPtr ->_1_raw;
    ai_features[2] = apiSelectionPtr ->_2_pre;
    ai_features[3] = apiSelectionPtr ->_3_error;
    ai_features[4] = apiSelectionPtr ->_4_rate;
    ai_features[5] = apiSelectionPtr ->_5_timestamp;
    
    Serial.printf("Request from REST-Api failed: Code: %d Substitution: %s \n", responseCode, ai_features[0].value);
  }
  return responseCode;
} 

t_httpCode readViessmannFeaturesFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr, const uint32_t data_0_id, const char * p_gateways_0_serial, const char * p_gateways_0_devices_0_id, ViessmannApiSelection * apiSelectionPtr)
{
  secure_wifi_client.setInsecure();

  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif

  memset(bufferStorePtr, '\0', bufferStoreLength);
  ViessmannClient viessmannClient(myViessmannApiAccountPtr, pCaCert,  httpPtr, &secure_wifi_client, bufferStorePtr);
  Serial.printf("\r\n(%u) ",loadViFeaturesCount);
  Serial.printf("%i/%02d/%02d %02d:%02d ", localTime.year(), 
                                        localTime.month() , localTime.day(),
                                        localTime.hour() , localTime.minute());
  t_httpCode responseCode = viessmannClient.GetFeatures(bufferStorePtr, bufferStoreLength, data_0_id, Gateways_0_Serial, Gateways_0_Devices_0_Id, apiSelectionPtr);
  
  Serial.printf("(%u) Viessmann Features: httpResponseCode is: %d\r\n", loadViFeaturesCount, responseCode);
  if (responseCode == t_http_codes::HTTP_CODE_OK)
  {
    loadViFeaturesCount++;
    // After each successful request error counters are reset
    loadViFeaturesResp400Count = 0;
    loadViFeaturesRespOtherCount = 0;

    // Populate features array and replace the name read from Api
    // with the name used in this Application
    features[0] = apiSelectionPtr ->_3_temperature_main;
    strcpy(features[0].name, (const char *)"_3_temperature_main");
    features[1] = apiSelectionPtr ->_5_boiler_temperature;
    strcpy(features[1].name, (const char *)"_5_boiler_temperature");
    features[2] = apiSelectionPtr ->_7_burner_modulation;
    strcpy(features[2].name, (const char *)"_7_burner_modulation");
    features[3] = apiSelectionPtr ->_8_burner_hours;
    strcpy(features[3].name, (const char *)"_8_burner_hours");
    features[4] = apiSelectionPtr ->_8_burner_starts;
    strcpy(features[4].name, (const char *)"_8_burner_starts");
    features[5] = apiSelectionPtr ->_9_burner_is_active;
    strcpy(features[5].name, (const char *)"_9_burner_is_active");
    features[6] = apiSelectionPtr ->_23_heating_curve_shift;
    strcpy(features[6].name, (const char *)"_23_heating_curve_shift");
    features[7] = apiSelectionPtr ->_23_heating_curve_slope;
    strcpy(features[7].name, (const char *)"_23_heating_curve_slope");
    features[8] = apiSelectionPtr ->_77_temperature_supply;
    strcpy(features[8].name, (const char *)"_77_temperature_supply");
    features[9] = apiSelectionPtr ->_85_heating_dhw_charging;
    strcpy(features[9].name, (const char *)"_85_heating_dhw_charging");
    features[10] = apiSelectionPtr ->_86_heating_dhw_pump_status;
    strcpy(features[10].name, (const char *)"_86_heating_dhw_pump_status");
    features[11] = apiSelectionPtr ->_88_heating_dhw_pump_primary_status;
    strcpy(features[11].name, (const char *)"_88_heating_dhw_pump_primary_status");
    features[12] = apiSelectionPtr ->_90_heating_dhw_cylinder_temperature;
    strcpy(features[12].name, (const char *)"_90_heating_dhw_cylinder_temperature");
    features[13] = apiSelectionPtr ->_92_heating_dhw_outlet_temperature;
    strcpy(features[13].name, (const char *)"_92_heating_dhw_outlet_temperature");
    features[14] = apiSelectionPtr ->_93_heating_dhw_main_temperature;
    strcpy(features[14].name, (const char *)"_93_heating_dhw_main_temperature");
    features[15] = apiSelectionPtr ->_95_heating_temperature_outside;
    strcpy(features[15].name, (const char *)"_95_heating_temperature_outside");
    
    // Get 4 On/Off sensor values which were read from the Viessmann Api
    // and store them in a 'twin' of the sensor, reflecting its state 
    OnOffBurnerStatus.Feed(strcmp((const char *)(apiSelectionPtr ->_9_burner_is_active.value), (const char *)"true") == 0, dateTimeUTCNow);
    OnOffCirculationPumpStatus.Feed(strcmp((const char *)(apiSelectionPtr ->_11_circulation_pump_status.value), (const char *)"on") == 0, dateTimeUTCNow);
    OnOffHotWaterCircualtionPumpStatus.Feed(strcmp((const char *)(apiSelectionPtr ->_86_heating_dhw_pump_status.value), (const char *)"on") == 0, dateTimeUTCNow);
    OnOffHotWaterPrimaryPumpStatus.Feed(strcmp((apiSelectionPtr -> _88_heating_dhw_pump_primary_status.value), (const char *)"on") == 0, dateTimeUTCNow);   
  }
  else
  {
    
    if (responseCode == 400)
    {
      loadViFeaturesResp400Count++;
    }
    else
    {
      loadViFeaturesRespOtherCount++;
    }
    Serial.printf("Bad httpResponses, Code 400: %d, Others: %d\n", loadViFeaturesResp400Count, loadViFeaturesRespOtherCount);
  
    if (loadViFeaturesResp400Count > 50 || loadViFeaturesRespOtherCount > 50)
    {
      Serial.printf("Rebooting, failed Vi-Requests. 400: %d, others: %d\n", loadViFeaturesResp400Count, loadViFeaturesRespOtherCount);
      ESP.restart();
      while(true)
      {
        delay(500); //Wait for ever
      }
    }

    bufferStorePtr[bufferStoreLength - 1] = '\0';
    Serial.println((char *)bufferStorePtr);
  }
  return responseCode;
}

t_httpCode readUserFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr)
{
  secure_wifi_client.setInsecure();

  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif

  ViessmannClient viessmannClient(myViessmannApiAccountPtr, pCaCert,  httpPtr, &secure_wifi_client, bufferStorePtr);
   #if SERIAL_PRINT == 1
        // Serial.println(myViessmannApiAccount.ClientId);
      #endif
      memset(viessmannApiUser,'\0', viessmannUserBufLen);
      memset(bufferStorePtr,'\0', bufferStoreLength);
      t_httpCode responseCode = viessmannClient.GetUser(bufferStorePtr, bufferStoreLength);
      Serial.printf("\r\nUser: httpResponseCode is: %d\r\n", responseCode);
      
      if (responseCode == t_http_codes::HTTP_CODE_OK)
      {
        uint16_t cntToCopy = strlen((char*)bufferStorePtr) < viessmannUserBufLen ? strlen((char*)bufferStorePtr) : viessmannUserBufLen -1;
        memcpy(viessmannApiUser, bufferStorePtr, cntToCopy);       
      }    
return responseCode;
}

t_httpCode readEquipmentFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr, uint32_t * p_data_0_id, const int equipBufLen, char * p_data_0_description, char * p_data_0_address_street, char * p_data_0_address_houseNumber, char * p_gateways_0_serial, char * p_gateways_0_devices_0_id)
{
  secure_wifi_client.setInsecure();

  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif
  ViessmannClient viessmannClient(myViessmannApiAccountPtr, pCaCert,  httpPtr, &secure_wifi_client, bufferStorePtr);
   #if SERIAL_PRINT == 1
        //Serial.println(myViessmannApiAccount.ClientId);
      #endif
      
      memset(bufferStorePtr, '\0', bufferStoreLength); 
      t_httpCode responseCode = viessmannClient.GetEquipment(bufferStorePtr, bufferStoreLength);
          
      Serial.printf("\r\nEquipment httpResponseCode is: %d\r\n", responseCode);

      if (responseCode == t_http_codes::HTTP_CODE_OK)
      {
        const char* json = (char *)bufferStorePtr;
        JsonDocument doc;
        deserializeJson(doc, json);
        
        uint32_t data_0_id = doc["data"][0]["id"];
        const char * data_0_description = doc["data"][0]["description"];
        const char * data_0_address_street = doc["data"][0]["address"]["street"];
        const char * data_0_address_houseNumber = doc["data"][0]["address"]["houseNumber"];
        const char * gateways_0_serial = doc["data"][0]["gateways"][0]["serial"];
        const char * gateways_0_devices_0_id = doc["data"][0]["gateways"][0]["devices"][0]["id"];
        
        *p_data_0_id = data_0_id;
        memset(bufferStorePtr, '\0', bufferStoreLength);
         
        memset(p_data_0_description, '\0', equipBufLen);
        memset(p_data_0_address_street, '\0', equipBufLen);
        memset(p_data_0_address_houseNumber,'\0', equipBufLen);

        memset(p_gateways_0_serial,'\0', equipBufLen);
        memset(p_gateways_0_devices_0_id,'\0', equipBufLen);

        strncpy(p_data_0_description, data_0_description, equipBufLen - 1);
        strncpy(p_data_0_address_street, data_0_address_street, equipBufLen - 1);
        strncpy(p_data_0_address_houseNumber, data_0_address_houseNumber, equipBufLen - 1);

        strncpy(p_gateways_0_serial, gateways_0_serial, equipBufLen - 1);
        strncpy(p_gateways_0_devices_0_id, gateways_0_devices_0_id, equipBufLen - 1);       
      }
  return responseCode; 
}


t_httpCode refreshAccessTokenFromApi(X509Certificate pCaCert, ViessmannApiAccount * myViessmannApiAccountPtr, const char * refreshToken)
{
  secure_wifi_client.setInsecure();

  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif

  const char * accessTokenLabel = "access_token";
  const char * refreshTokenLabel = "refresh_token";
  const char * tokenTypeLabel = "token_type";
  
  
  ViessmannClient viessmannClient(myViessmannApiAccountPtr, pCaCert,  httpPtr, &secure_wifi_client, bufferStorePtr); 
      memset(bufferStorePtr,'\0', bufferStoreLength);
      t_httpCode responseCode = viessmannClient.RefreshAccessToken(bufferStorePtr, bufferStoreLength, refreshToken);
      
      Serial.printf("\n(%u) %i/%02d/%02d %02d:%02d ", loadRefreshTokenCount, localTime.year(), 
                                        localTime.month() , localTime.day(),
                                        localTime.hour() , localTime.minute());
      Serial.println(F("Refreshing Access Token"));
      Serial.printf("(%u) Refresh Token: httpResponseCode: %d\r\n\r\n", loadRefreshTokenCount++, responseCode);
      
      if (responseCode == t_http_codes::HTTP_CODE_OK)
      {    
         #if SERIAL_PRINT == 1
          // Serial.printf("%s\n", (char *)bufferStorePtr);
         #endif
         bool tokenIsValid = true;
         char * posAcTok = strnstr((char *)bufferStorePtr, (const char *)"access_token", 50);
         char * posRefrTok = strnstr((char *)bufferStorePtr, (const char *)"refresh_token", 2000);
         char * posTokType = strnstr((char *)bufferStorePtr, (const char *)"token_type", 2000);

         tokenIsValid = posAcTok == nullptr ? false : tokenIsValid;
         tokenIsValid = posRefrTok == nullptr ? false : tokenIsValid;
         tokenIsValid = posTokType == nullptr ? false : tokenIsValid;
         
         tokenIsValid = (tokenIsValid && (posAcTok < posRefrTok) && (posRefrTok < posTokType)) ? true : false;

         if (tokenIsValid)
         {
            char * startAcTok = posAcTok + strlen(accessTokenLabel) + 3;
            char * endAcTok = posRefrTok - 4;
            size_t acTokenLength = (size_t)endAcTok + 1 - (size_t)startAcTok;
            #if SERIAL_PRINT == 1
                Serial.printf("\nTokenlength is: %d\n", acTokenLength);
            #endif
            memcpy(viessmannAccessToken, startAcTok, acTokenLength);
            viessmannAccessToken[acTokenLength] = '\0';
            myViessmannApiAccountPtr ->RenewAccessToken(String(viessmannAccessToken));   
         }
         else
         {
            responseCode = HTTPC_ERROR_SEND_PAYLOAD_FAILED;
            Serial.println(F("Content error. Refreshing Accesstoken failed!\n"));
            bufferStorePtr[bufferStoreLength - 1] = '\0';
            Serial.println((char *)bufferStorePtr);
         }
      }
      else
      {  
        Serial.println(F("Response Error. Refreshing Accesstoken failed!\n"));   
        bufferStorePtr[bufferStoreLength - 1] = '\0';
        Serial.println((char *)bufferStorePtr);
      }
   return responseCode;
      
}

az_http_status_code createTable(CloudStorageAccount *pAccountPtr, X509Certificate pCaCert, const char * pTableName)
{ 

  #if TRANSPORT_PROTOCOL == 1
    static WiFiClientSecure wifi_client;
  #else
    static WiFiClient wifi_client;
  #endif

    #if TRANSPORT_PROTOCOL == 1
    wifi_client.setCACert(myX509Certificate);
    //wifi_client.setCACert(baltimore_corrupt_root_ca);
  #endif

  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif
  
  // RoSchmi
  TableClient table(pAccountPtr, pCaCert,  httpPtr, &wifi_client, bufferStorePtr);

  // Create Table
  az_http_status_code statusCode = table.CreateTable(pTableName, dateTimeUTCNow, ContType::contApplicationIatomIxml, AcceptType::acceptApplicationIjson, returnContent, false);
  
   // RoSchmi for tests: to simulate failed upload
   //az_http_status_code   statusCode = AZ_HTTP_STATUS_CODE_UNAUTHORIZED;

  char codeString[35] {0};
  if ((statusCode == AZ_HTTP_STATUS_CODE_CONFLICT) || (statusCode == AZ_HTTP_STATUS_CODE_CREATED))
  {
    #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
    #endif
   
      sprintf(codeString, "%s %i", "Table available: ", az_http_status_code(statusCode));
      #if SERIAL_PRINT == 1
        Serial.println((char *)codeString);
      #endif
  
  }
  else
  {
      sprintf(codeString, "%s %i", "Table Creation failed: ", az_http_status_code(statusCode));
      #if SERIAL_PRINT == 1   
        Serial.println((char *)codeString);
      #endif
 
    delay(1000);

    ESP.restart();
    
  }
  

return statusCode;
}

az_http_status_code insertTableEntity(CloudStorageAccount *pAccountPtr,  X509Certificate pCaCert, const char * pTableName, TableEntity pTableEntity, char * outInsertETag)
{ 
  #if TRANSPORT_PROTOCOL == 1
    static WiFiClientSecure wifi_client;
  #else
    static WiFiClient wifi_client;
  #endif
  
  #if TRANSPORT_PROTOCOL == 1
    wifi_client.setCACert(myX509Certificate); 
  #endif
  
  /*
  // For tests: Try second upload with corrupted certificate to provoke failure
  #if TRANSPORT_PROTOCOL == 1
    wifi_client.setCACert(myX509Certificate);
    if (insertCounterAnalogTable == 2)
    {
      wifi_client.setCACert(baltimore_corrupt_root_ca);
    }
  #endif
  */

  TableClient table(pAccountPtr, pCaCert,  httpPtr, &wifi_client, bufferStorePtr);

  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif
  
  DateTime responseHeaderDateTime = DateTime();   // Will be filled with DateTime value of the resonse from Azure Service

  // Insert Entity
  az_http_status_code statusCode = table.InsertTableEntity(pTableName, dateTimeUTCNow, pTableEntity, (char *)outInsertETag, &responseHeaderDateTime, ContType::contApplicationIatomIxml, AcceptType::acceptApplicationIjson, ResponseType::dont_returnContent, false);
  
  #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
  #endif

  lastResetCause = 0;
  tryUploadCounter++;

   // RoSchmi for tests: to simulate failed upload
  //az_http_status_code   statusCode = AZ_HTTP_STATUS_CODE_UNAUTHORIZED;
  
  if ((statusCode == AZ_HTTP_STATUS_CODE_NO_CONTENT) || (statusCode == AZ_HTTP_STATUS_CODE_CREATED))
  {
      char codeString[35] {0};
      sprintf(codeString, "Entity inserted: %i", statusCode);
      #if SERIAL_PRINT == 1
        Serial.println((char *)codeString);
      #endif

      Serial.printf("\r\n%s Entity inserted: %i\r\n", pTableName, az_http_status_code(statusCode));
    
    #if UPDATE_TIME_FROM_AZURE_RESPONSE == 1    // System time shall be updated from the DateTime value of the response ?
    
    dateTimeUTCNow = responseHeaderDateTime;
    
    char buffer[35] = {0};
    strcpy(buffer, "Azure-Utc: YYYY-MM-DD hh:mm:ss");
    dateTimeUTCNow.toString(buffer);
    #if SERIAL_PRINT == 1
      Serial.println((char *)buffer);
      Serial.println("");
    #endif
    
    #endif   
  }
  else            // request failed
  {               // note: internal error codes from -1 to -11 were converted for tests to error codes 401 to 411 since
                  // negative values cannot be returned as 'az_http_status_code' 

    failedUploadCounter++;
    //sendResultState = false;
    lastResetCause = 100;      // Set lastResetCause to arbitrary value of 100 to signal that post request failed
    
    
      char codeString[35] {0};
      sprintf(codeString, "%s %i", "Insertion failed: ", az_http_status_code(statusCode));
      #if SERIAL_PRINT == 1
        Serial.println((char *)codeString);
      #endif
  
    
    #if REBOOT_AFTER_FAILED_UPLOAD == 1   // When selected in config.h -> Reboot through SystemReset after failed uoload

        #if TRANSPORT_PROTOCOL == 1         
          ESP.restart();        
        #endif
        #if TRANSPORT_PROTOCOL == 0     // for http requests reboot after the second, not the first, failed request
          if(failedUploadCounter > 1)
          {
            ESP.restart();
          }
    #endif

    #endif

    #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();  
    #endif
    delay(1000);
  }  
  return statusCode;
}



void print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : Serial.println ("POWERON_RESET");break;          /**<1, Vbat power on reset*/
    case 3 : Serial.println ("SW_RESET");break;               /**<3, Software reset digital core*/
    case 4 : Serial.println ("OWDT_RESET");break;             /**<4, Legacy watch dog reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET");break;        /**<5, Deep Sleep reset digital core*/
    case 6 : Serial.println ("SDIO_RESET");break;             /**<6, Reset by SLC module, reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET");break;       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET");break;       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET");break;       /**<9, RTC Watch dog Reset digital core*/
    case 10 : Serial.println ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : Serial.println ("NO_MEAN");
  }
}

















