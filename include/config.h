#include <Arduino.h>

#ifndef _CONFIG_H
#define _CONFIG_H

// This file is for 'not secret' user specific configurations
//
// Please set your timezone offset (time difference from your zone 
// to UTC time in units of minutes) and set the time difference
// used for DaylightSaving Time in minutes
// Define begin and end of Daylightsaving 
//
// Please select the transport protocol, https is needed to load data from
// the Viessmann Cloud. 
// For https access to the Azure service the Root Certificate of your Azure Account
// like here the 'digicert_globalroot_g2_ca' must be present in config.h
// Requests on the Azure service can be made using http protocol as well.
// Select the Sendinterval in minutes
// Select the Invalidate Interval in minutes (Sensor values not actualized
// within this interval are considered to be invalid)
// Define other settings according to your needs

// Uri defining where to get data from the Viessmann API
#define VIESSMANN_USER_BASE_URI  "api.viessmann.com/users/v1/users/me";
#define VIESSMANN_IOT_BASE_URI "api.viessmann.com/iot/v1/";
#define VIESSMANN_TOKEN_BASE_URI "iam.viessmann.com/idp/v3/token";

// The credentials of your WiFi router and the name and key of your
// Azure Storage Account are set through WiFi-Manager

#define USED_MICROPHONE 0        // 0 = SPH0645LM4H, 1 = INMP441

#define SOUNDSWITCHER_THRESHOLD "200"       // The arbitrary sound Threshold to toggle High/Low
                                            // (Can be changed in WiFi-Manager)
#define SOUNDSWITCHER_UPDATEINTERVAL 400    // Interval in ms for reading sound level
#define SOUNDSWITCHER_READ_DELAYTIME 4000   // Delay in ms from switch to displayed value

#define SENDINTERVAL_MINUTES_AI   1    // Sendinterval in minutes (5 - 10 is recommended), in this interval                                        
                                       // data are sent to the Cloud (is limited to be not below 1 second)
#define SENDINTERVAL_MINUTES_VI   5    // Sendinterval for Viessmann data in minutes (5 - 10 is recommended), in this interval                                        
                                       // data are sent to the Cloud (is limited to be not below 1 second)



                                    #define SERIAL_PRINT 0                    // 1 = yes, 0 = no. Select if Serial.print messages are printed

#define _ESPASYNC_WIFIMGR_LOGLEVEL_  0     // ( 0 - 4) Define EspAsync_WiFiManager Loglevel (Debug Messages)

// Cave: It's dangerous to display WiFi password in a Captive Portal page
// Should always be set to false
#define DISPLAY_STORED_CREDENTIALS_IN_CP false // Define if stored Router SSIDs are displayed (should normally by set to false)
                                                // Cave: it's dangerous to show the passwords. So don't do it !!!                                    



// Names for Tables in Azure Account, please obey rules for Azure Tablenames (e.g. no underscore allowed)
// regular expression "^[A-Za-z][A-Za-z0-9]{2,62}$".
// max length in this App is 45

#define ANALOG_TABLENAME "AnalogValuesX"                   // Name of the Azure Table to store 4 analog Values max length = 45

#define VIESSMANN_ANALOG_TABLENAME_01 "ApiAnalog01ValuesX" // Name of the Azure Table to store 4 analog Values max length = 45

#define ANALOG_TABLE_PART_PREFIX "Y2_"            // Prefix for PartitionKey of Analog Tables (default, no need to change)


                                               // Names of tables to be created in your Azure Storage Account
                                               // Per default the names are augmented with the actual year in this App
#define ON_OFF_TABLENAME_01 "OnOffx01x"          // Name of the 1. On/Off Table  max length = 45
#define ON_OFF_TABLENAME_02 "OnOffx02x"          // Name of the 2. On/Off Table  max length = 45
#define ON_OFF_TABLENAME_03 "OnOffx03x"          // Name of the 3. On/Off Table  max length = 45
#define ON_OFF_TABLENAME_04 "OnOffx04x"          // Name of the 4. On/Off Table  max length = 45

#define ON_OFF_TABLE_PART_PREFIX "Y3_"           // Prefix for PartitionKey of On/Off Tables (default, only change if needed)

#define INVALIDATEINTERVAL_MINUTES 10      // Invalidateinterval in minutes 
                                           // (limited to values between 1 - 60)
                                           // (Sensor readings are considered to be invalid if not successsfully
                                           // read within this timespan)

#define NTP_UPDATE_INTERVAL_MINUTES 14400  //  With this interval sytem time is updated via NTP 14400 = 10 days
                                           //  with internet time (is limited to be not below 1 min)

#define UPDATE_TIME_FROM_AZURE_RESPONSE 1  // 1 = yes, 0 = no. SystemTime is updated from the Post response from Azure.
                                           // With this option set, you can set  NTP_UPDATE_INTERVAL_MINUTES to a very
                                           // long time, so that it 'never' happens                                      

#define ANALOG_SENSOR_READ_INTERVAL_SECONDS 0   // Analog sensors are read with this interval (seconds, limeted to be not below 2)

#define GASMETER_AI_API_READ_INTERVAL_SECONDS 60  //Values from the AiOnTheEdge gasmeter are read with this timeinterval

#define API_ANALOG_SENSOR_READ_INTERVAL_SECONDS 77  // Analog Sensor values from the Viessmann Api (seconds, can be 0)

#define VIESSMANN_API_READ_INTERVAL_SECONDS 75  //Values from the Viessmann Api are read with this timeinterval

#define ANALOG_SENSORS_USE_AVERAGE 0             // 1 means: The average from multiple Sensor readings are used
                                                 // 0 means: The last Sensor reading is used

#define VIESSMANN_TOKEN_REFRESH_INTERVAL_SECONDS 60 * 30 // Viessman AccessToken is refreshed using this timeInterval

#define WORK_WITH_WATCHDOG 0              // 1 = yes, 0 = no, Watchdog is used (1) or not used (0)
                                           // should be 1 for normal operation and 0 for testing
                                           
#define REBOOT_AFTER_FAILED_UPLOAD 0       // 1 = yes, 0 = no
                                           // should be 1 for normal operation and 0 for testing
                                            
// Set timezoneoffset and daylightsavingtime settings according to your zone
// https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
// https://en.wikipedia.org/wiki/List_of_time_zone_abbreviations

#define TIMEZONEOFFSET 60           // TimeZone time difference to UTC in minutes
#define DSTOFFSET 60                // Additional DaylightSaving Time offset in minutes

#define  DST_ON_NAME                "CEST"
#define  DST_START_MONTH            "Mar"    // Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov or Dec
#define  DST_START_WEEKDAY          "Sun"    // Sun, Mon, Tue, Wed, Thu, Fri, Sat
#define  DST_START_WEEK_OF_MONTH    "Fourth"   // Last, First, Second, Third, Fourth, 
#define  DST_START_HOUR              2        // 0 - 23

#define  DST_OFF_NAME               "CET"
#define  DST_STOP_MONTH             "Oct"    // Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov or Dec
#define  DST_STOP_WEEKDAY           "Sun"    // Sun, Tue, Wed, Thu, Fri, Sat
#define  DST_STOP_WEEK_OF_MONTH     "Last"   // Last, First, Second, Third, Fourth
#define  DST_STOP_HOUR               3       // 0 - 23
       
                                          // For Viessmann Api both must be https !!
#define TRANSPORT_PROTOCOL 1              // 0 = http, 1 = https
#define VIESSMANN_TRANSPORT_PROTOCOL 1    // 0 = http, 1 = https
#define AIONTHEEDGE_TRANSPORT_PROTOCOL 0  // 0 = http, 1 = https
                                 

#define USE_STATIC_IP 0                // 1 = use static IpAddress, 0 = use DHCP
                                        // for static IP: Ip-addresses have to be set in the code

#define MIN_DATAVALUE_VI -40.0             // Values below are treated as invalid (Temperatures)
#define MAX_DATAVALUE_VI 140.0             // Values above are treated as invalid (Temperatures)

#define MIN_DATAVALUE_AI -0.1             // Values below are treated as invalid (Gas consumption)
#define MAX_DATAVALUE_AI 999.0           // Values above are treated as invalid (Gas consumption)

#define MAGIC_NUMBER_INVALID 999.9      // Invalid values are replaced with this value (should be 999.9)
                                        // Not sure if it works with other values than 999.9

//#define USE_SIMULATED_SENSORVALUES      // Activates simulated sensor values (sinus curve) or (test values)
//#define USE_TEST_VALUES                 // Activates sending of test values, e.g. counter or last reset cause (see Code in main.cpp)
                                          // if activated we select test values, not sinus curves

#define SENSOR_1_OFFSET     0.0        // Calibration Offset to sensor No 1
#define SENSOR_2_OFFSET     0.0        // Calibration Offset to sensor No 2
#define SENSOR_3_OFFSET     0.0        // Calibration Offset to sensor No 3
#define SENSOR_4_OFFSET     0.0        // Calibration Offset to sensor No 4

// Not needed in version for Esp32
#define SENSOR_1_FAHRENHEIT 0         // 1 = yes, 0 = no - Display in Fahrenheit scale
#define SENSOR_2_FAHRENHEIT 0         // 1 = yes, 0 = no - Display in Fahrenheit scale
#define SENSOR_3_FAHRENHEIT 0         // 1 = yes, 0 = no - Display in Fahrenheit scale
#define SENSOR_4_FAHRENHEIT 0         // 1 = yes, 0 = no - Display in Fahrenheit scale


//#define USE_SIMULATED_SENSORVALUES      // Activates simulated sensor values (sinus curve) or (test values)
//#define USE_TEST_VALUES                 // Activates sending of test values, e.g. counter or last reset cause (see Code in main.cpp)
                                        // if activated we select test values, not sinus curves
//-https://techcommunity.microsoft.com/t5/azure-storage/azure-storage-tls-critical-changes-are-almost-here-and-why-you/ba-p/2741581
//
//
// To continue without disruption due to this change, 
// Microsoft recommends that client applications or devices 
// trust the root CA – DigiCert Global Root G2: 
// DigiCert Global Root G2 
// (Thumbprint: df3c24f9bfd666761b268073fe06d1cc8d4f82a4)

const char *digicert_globalroot_g2_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
"MrY=\n"
"-----END CERTIFICATE-----";

#endif // _CONFIG_H

