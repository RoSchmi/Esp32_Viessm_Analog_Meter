#include "ViessmannClient.h"
#include "config.h"

//WiFiClientSecure * _viessmannWifiClient;

WiFiClient * _viessmannWifiClient;

ViessmannApiAccount  * _viessmannAccountPtr;

HTTPClient * _viessmannHttpPtr;

const char * _viessmannCaCert;

typedef int t_httpCode;

/*
char initName[FEATURENAMELENGTH] {0};
char initTimestamp[FEATURESTAMPLENGTH] {0};
char initValue[FEATUREVALUELENGTH] {0};
*/

// Constructor
ViessmannClient::ViessmannClient(ViessmannApiAccount * account, const char * caCert, HTTPClient * httpClient, WiFiClient * wifiClient, uint8_t * bufferStorePtr)
{  
    _viessmannAccountPtr = account;
    _viessmannCaCert = caCert;
    _viessmannWifiClient = wifiClient;  
    _viessmannHttpPtr = httpClient;
       
    _viessmannHttpPtr -> setReuse(false);
 
    //uint64_t connectTimout = _viessmannWifiClient->getTimeout();
    
    //Serial.printf("\nTimeout is: %u\n");

    // Some buffers located in memory segment .dram0.bss are injected to achieve lesser stack consumption
    
    /*
    _requestPtr = bufferStorePtr;
    _propertiesPtr = bufferStorePtr + REQUEST_BODY_BUFFER_LENGTH;
    _authorizationHeaderBufferPtr = bufferStorePtr + REQUEST_BODY_BUFFER_LENGTH + PROPERTIES_BUFFER_LENGTH;
    _responsePtr = bufferStorePtr + REQUEST_BODY_BUFFER_LENGTH + PROPERTIES_BUFFER_LENGTH + AUTH_HEADER_BUFFER_LENGTH;
    */

    /*
   _accountPtr = account;
    _caCert = caCert;
    _httpPtr = httpClient;
    _wifiClient = wifiClient;
    */
    /*
       char responseString[RESPONSE_BUFFER_LENGTH];
       memset(&(responseString[0]), 0, RESPONSE_BUFFER_LENGTH);
    */
}

t_httpCode ViessmannClient::GetFeatures(uint8_t* responseBuffer, const uint16_t reponseBufferLength, const uint32_t data_0_id, const char * gateways_0_serial, const char * gateways_0_devices_0_id, ViessmannApiSelection * apiSelectionPtr)
{
    char InstallationId[20] = {0};
    sprintf(InstallationId, "%d", data_0_id);

    char GatewaySerial[30] = {0};
    
    String addendum = "features/installations/" + (String)InstallationId + "/gateways/" + (String(gateways_0_serial) + "/devices/" + String(gateways_0_devices_0_id) + "/features"); 
    String Url = _viessmannAccountPtr -> UriEndPointIot + addendum;
    String authorizationHeader = "Bearer " + _viessmannAccountPtr ->AccessToken;
    Serial.println(F("Loading Viessmann features from Cloud"));
    
    #if SERIAL_PRINT == 1
    Serial.printf("VI-WiFiClient Address: %d\n\n", &_viessmannWifiClient);
    #endif
    
    //Serial.println(Url);

    //https://arduinojson.org/v7/how-to/use-arduinojson-with-httpclient/

    _viessmannHttpPtr ->useHTTP10(true);   // Must be reset to false for Azure requests
                                           // Is needed to load the long features JSON string

    //Serial.println(F("Set httpClient to true"));
    #if SERIAL_PRINT == 1
    Serial.printf("Free heapsize: %d Minimum: %d\n", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    #endif
    _viessmannHttpPtr ->begin(*_viessmannWifiClient, Url);
    
    _viessmannHttpPtr ->addHeader("Authorization", authorizationHeader); 
               
    t_httpCode httpResponseCode = _viessmannHttpPtr ->GET();
                
    if (httpResponseCode > 0) 
    { 
        #if SERIAL_PRINT == 1
            Serial.println(F("Viessmann Received ResponseCode > 0"));
        #endif

        if (httpResponseCode == HTTP_CODE_OK)
        {     
            #if SERIAL_PRINT == 1
              Serial.println(F("Viessmann Received ResponseCode OK"));
            #endif
             
            JsonDocument doc;
            StaticJsonDocument<64> filter;
            filter["data"][0]["feature"] = true,
            filter["data"][0]["timestamp"] = true,
            filter["data"][0]["properties"] = true 
            ;
            deserializeJson(doc, _viessmannHttpPtr ->getStream(),DeserializationOption::Filter(filter));
            
            #if SERIAL_PRINT == 1
            Serial.println(F("JsonDoc is deserialized"));
            #endif

            //_viessmannHttpPtr ->useHTTP10(false);
            //_viessmannHttpPtr->end();
            
            int nameLen = apiSelectionPtr ->nameLenght;
            int stampLen = apiSelectionPtr -> stampLength;
            int valLen = apiSelectionPtr -> valueLength;

            char tempVal[valLen] = {"\0"};
            
            if (!doc.overflowed())
            {
                #if SERIAL_PRINT == 1
                Serial.printf("Number of elements = %d\n", doc.size());
                #endif
                // From the long Features JSON string get the selected entities
        
                apiSelectionPtr -> _3_temperature_main.idx = 3;
                strncpy(apiSelectionPtr -> _3_temperature_main.name, doc["data"][3]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _3_temperature_main.timestamp, doc["data"][3]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][3]["properties"]["value"]["value"]); 
                snprintf(apiSelectionPtr -> _3_temperature_main.value, valLen - 1, (const char*)tempVal);
                
                #if SERIAL_PRINT == 1
                Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _3_temperature_main.name, apiSelectionPtr -> _3_temperature_main.timestamp, apiSelectionPtr -> _3_temperature_main.value);
                #endif

                apiSelectionPtr -> _5_boiler_temperature.idx = 5;
                strncpy(apiSelectionPtr -> _5_boiler_temperature.name, doc["data"][5]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _5_boiler_temperature.timestamp, doc["data"][5]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][5]["properties"]["value"]["value"]); 
                snprintf(apiSelectionPtr -> _5_boiler_temperature.value, valLen - 1, (const char*)tempVal);
                           
                #if SERIAL_PRINT == 1
                Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _5_boiler_temperature.name, apiSelectionPtr -> _5_boiler_temperature.timestamp, apiSelectionPtr -> _5_boiler_temperature.value);
                #endif

                apiSelectionPtr -> _7_burner_modulation.idx = 7;
                strncpy(apiSelectionPtr -> _7_burner_modulation.name, doc["data"][7]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _7_burner_modulation.timestamp, doc["data"][7]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][7]["properties"]["value"]["value"]); 
                snprintf(apiSelectionPtr -> _7_burner_modulation.value, valLen - 1, (const char*)tempVal);
                           
                #if SERIAL_PRINT == 1
                Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _7_burner_modulation.name, apiSelectionPtr -> _7_burner_modulation.timestamp, apiSelectionPtr -> _7_burner_modulation.value);
                #endif

                apiSelectionPtr -> _8_burner_hours.idx = 8;
                strncpy(apiSelectionPtr -> _8_burner_hours.name, doc["data"][8]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _8_burner_hours.timestamp, doc["data"][8]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.2f", (float)doc["data"][8]["properties"]["hours"]["value"]);
                snprintf(apiSelectionPtr -> _8_burner_hours.value, valLen - 1, (const char*)tempVal);
            
                apiSelectionPtr -> _8_burner_starts.idx = 8;
                strncpy(apiSelectionPtr -> _8_burner_starts.name, doc["data"][8]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _8_burner_starts.timestamp, doc["data"][8]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][8]["properties"]["starts"]["value"]);
                snprintf(apiSelectionPtr -> _8_burner_starts.value, valLen - 1, (const char*)tempVal);
            
                apiSelectionPtr -> _9_burner_is_active.idx = 9;
                strncpy(apiSelectionPtr -> _9_burner_is_active.name, doc["data"][9]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _9_burner_is_active.timestamp, doc["data"][9]["timestamp"], stampLen - 1);
                strcpy(apiSelectionPtr -> _9_burner_is_active.value, (boolean)doc["data"][9]["properties"]["active"]["value"] ? "true" : "false");
            
                apiSelectionPtr -> _11_circulation_pump_status.idx = 11;
                strncpy(apiSelectionPtr -> _11_circulation_pump_status.name, doc["data"][11]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr -> _11_circulation_pump_status.timestamp, doc["data"][11]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr -> _11_circulation_pump_status.value, doc["data"][11]["properties"]["status"]["value"], valLen -1);

                apiSelectionPtr -> _23_heating_curve_shift.idx = 23;
                strncpy(apiSelectionPtr -> _23_heating_curve_shift.name, doc["data"][23]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _23_heating_curve_shift.timestamp, doc["data"][23]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][23]["properties"]["shift"]["value"]);
                snprintf(apiSelectionPtr -> _23_heating_curve_shift.value, valLen - 1, (const char*)tempVal);
                       
                //Serial.println(F("Doc 8"));
                //Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _23_heating_curve_shift.name, apiSelectionPtr -> _23_heating_curve_shift.timestamp, apiSelectionPtr -> _23_heating_curve_shift.value);
        
                apiSelectionPtr -> _23_heating_curve_slope.idx = 23;
                strncpy(apiSelectionPtr -> _23_heating_curve_slope.name, doc["data"][23]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr -> _23_heating_curve_slope.timestamp, doc["data"][23]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][23]["properties"]["slope"]["value"]);
                snprintf(apiSelectionPtr -> _23_heating_curve_slope.value, valLen - 1, (const char*)tempVal);
                       
                //Serial.println(F("Doc 9"));
                //Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _23_heating_curve_slope.name, apiSelectionPtr -> _23_heating_curve_slope.timestamp, apiSelectionPtr -> _23_heating_curve_slope.value);
        
                apiSelectionPtr -> _77_temperature_supply.idx = 77;
                strncpy(apiSelectionPtr -> _77_temperature_supply.name, doc["data"][77]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _77_temperature_supply.timestamp, doc["data"][77]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][77]["properties"]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _77_temperature_supply.value, valLen - 1, (const char*)tempVal);
                       
                apiSelectionPtr -> _85_heating_dhw_charging.idx = 85;
                strncpy(apiSelectionPtr -> _85_heating_dhw_charging.name, doc["data"][85]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _85_heating_dhw_charging.timestamp, doc["data"][85]["timestamp"], stampLen - 1);
                strcpy(apiSelectionPtr -> _85_heating_dhw_charging.value, (boolean)doc["data"][85]["properties"]["active"]["value"] ? "true" : "false");

                //Serial.println(F("Doc 11"));

                apiSelectionPtr -> _86_heating_dhw_pump_status.idx = 86;
                strncpy(apiSelectionPtr -> _86_heating_dhw_pump_status.name, doc["data"][86]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr -> _86_heating_dhw_pump_status.timestamp, doc["data"][86]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr -> _86_heating_dhw_pump_status.value, doc["data"][86]["properties"]["status"]["value"], valLen -1);
            
                //Serial.println(F("Doc 12"));

                apiSelectionPtr -> _88_heating_dhw_pump_primary_status.idx = 88;
                strncpy(apiSelectionPtr -> _88_heating_dhw_pump_primary_status.name, doc["data"][88]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr -> _88_heating_dhw_pump_primary_status.timestamp, doc["data"][88]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr -> _88_heating_dhw_pump_primary_status.value, doc["data"][88]["properties"]["status"]["value"], valLen -1);

                //Serial.println(F("Doc 13"));

                apiSelectionPtr -> _90_heating_dhw_cylinder_temperature.idx = 90;
                strncpy(apiSelectionPtr -> _90_heating_dhw_cylinder_temperature.name, doc["data"][90]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _90_heating_dhw_cylinder_temperature.timestamp, doc["data"][90]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][90]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _90_heating_dhw_cylinder_temperature.value, valLen - 1, (const char*)tempVal);
                        
                //Serial.println(F("Doc 14"));

                apiSelectionPtr -> _92_heating_dhw_outlet_temperature.idx = 92;
                strncpy(apiSelectionPtr -> _92_heating_dhw_outlet_temperature.name, doc["data"][92]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _92_heating_dhw_outlet_temperature.timestamp, doc["data"][92]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][92]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _92_heating_dhw_outlet_temperature.value, valLen - 1, (const char*)tempVal);
            
                //Serial.println(F("Doc 15"));
                //Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _92_heating_dhw_outlet_temperature.name, apiSelectionPtr -> _92_heating_dhw_outlet_temperature.timestamp, apiSelectionPtr -> _92_heating_dhw_outlet_temperature.value);
        
                apiSelectionPtr -> _93_heating_dhw_main_temperature.idx = 93;
                strncpy(apiSelectionPtr -> _93_heating_dhw_main_temperature.name, doc["data"][93]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _93_heating_dhw_main_temperature.timestamp, doc["data"][93]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][93]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _93_heating_dhw_main_temperature.value, valLen - 1, (const char*)tempVal);
                      
                //Serial.println(F("Doc 16"));
                //Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _93_heating_dhw_main_temperature.name, apiSelectionPtr -> _93_heating_dhw_main_temperature.timestamp, apiSelectionPtr -> _93_heating_dhw_main_temperature.value);
        
                apiSelectionPtr -> _95_heating_temperature_outside.idx = 95;
                strncpy(apiSelectionPtr -> _95_heating_temperature_outside.name, doc["data"][95]["feature"], nameLen - 1);
                strncpy(apiSelectionPtr-> _95_heating_temperature_outside.timestamp, doc["data"][95]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][95]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _95_heating_temperature_outside.value, valLen - 1, (const char*)tempVal);
                            
                //Serial.println(F("Doc 17"));
                //Serial.printf("%s   %s   %s\n", apiSelectionPtr -> _95_heating_temperature_outside.name, apiSelectionPtr -> _95_heating_temperature_outside.timestamp, apiSelectionPtr -> _95_heating_temperature_outside.value);   
            }
            else
            {
                Serial.println(F("Deserialization doc was overflowed"));
            }   
        }
        else
        {
            // If request doesn't return with ok, we give back 
            // the begin of the response string in responseBuffer        
            Serial.printf("Viessmann Received ResponseCode %d\n",httpResponseCode);
            WiFiClient *stream = _viessmannHttpPtr ->getStreamPtr();                  
            uint32_t bytesRead = 0;
            uint32_t totalBytesRead = 0;
            uint32_t chunkSize = 1000;                  
            char chunkBuffer[chunkSize +1] = {0};
            
            while (stream->connected() || stream->available()) 
            {
                if (stream->available()) 
                {
                  // Read data in chunks
                  size_t bytesRead = stream->readBytes(chunkBuffer, chunkSize);
                  
                  // Write data to buffer
                  for (size_t i = 0; i < bytesRead; i++) 
                  {
                    if (totalBytesRead < reponseBufferLength) 
                    {
                      responseBuffer[totalBytesRead++] = chunkBuffer[i];
                    } 
                    else 
                    {
                      Serial.println("responseBuffer is full!");
                      break;
                    }
                  }
                  #if SERIAL_PRINT == 1                                 
                  Serial.write(chunkBuffer, bytesRead);
                  Serial.println();
                  #endif
                }
              }
              responseBuffer[reponseBufferLength -1] = '\0';                        
        }                      
    } 
    else 
    {    
        Serial.printf("Vi-Features: Error performing the request, HTTP-Code: %d\n", httpResponseCode);
    }
    
    _viessmannHttpPtr ->useHTTP10(false);
    _viessmannHttpPtr->end();
    //Serial.println(F("Returning"));
    return httpResponseCode;
}

t_httpCode ViessmannClient::RefreshAccessToken(uint8_t* responseBuffer, const uint16_t reponseBufferLength, const char * refreshToken)
{
    //#define MAXCOUNT 2
    
    // t_httpCode httpResponseCode = 0;

    //for (int i = 0; i < MAXCOUNT; i++)
    //{
        // Try first with invalid grant to avoid connection refused error
        //String body = i == 0 ? "grant_type=refresh_token&client_id=" + (String)_viessmannAccountPtr ->ClientId + "&refresh_token=" + (String)refreshToken + "*" 
        //      : "grant_type=refresh_token&client_id=" + (String)_viessmannAccountPtr ->ClientId + "&refresh_token=" + (String)refreshToken; 

        String body = "grant_type=refresh_token&client_id=" + (String)_viessmannAccountPtr ->ClientId + "&refresh_token=" + (String)refreshToken; 
           
        String Url = _viessmannAccountPtr -> UriEndPointToken;

        //Url = "https://iam.viessmann.com/idp/v3/token";
    
        #if SERIAL_PRINT == 1
            Serial.println(Url);
            Serial.println(body);
        #endif

        _viessmannHttpPtr ->begin(*_viessmannWifiClient, Url);

        _viessmannHttpPtr ->addHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
    
        t_httpCode httpResponseCode =_viessmannHttpPtr ->POST((String)body);
        if (httpResponseCode > 0) 
        {
            #if SERIAL_PRINT == 1
            Serial.printf("Refresh Token: ResponseCode is: %d\n", httpResponseCode);
            #endif

            String payload = _viessmannHttpPtr ->getString();
            
            #if SERIAL_PRINT == 1
                //Serial.println("Got payload:\n");
                //Serial.println(payload);
            #endif

            int charsToCopy = payload.length() < reponseBufferLength ? payload.length() : reponseBufferLength;
            for (int i = 0; i < charsToCopy; i++)
            {
                responseBuffer[i] = payload[i];
            } 
        } 
        else 
        {
            Serial.printf("Refresh token: Error performing the request, HTTP-Code: %d\n", httpResponseCode);
            if (httpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED)
            {
                Serial.println(F("Viessmann Server: Connection refused"));
                delay(500);          
            }
        }
        _viessmannHttpPtr->end();  
    
    return httpResponseCode;
}
    

t_httpCode ViessmannClient::GetEquipment(uint8_t* responseBuffer, const uint16_t reponseBufferLength)
{
    String Url = _viessmannAccountPtr -> UriEndPointIot + "equipment/installations" + "?includeGateways=true"; 
    String authorizationHeader = "Bearer " + _viessmannAccountPtr ->AccessToken;
    Serial.println(Url);
    _viessmannHttpPtr ->begin(*_viessmannWifiClient, Url);
    _viessmannHttpPtr ->addHeader("Authorization", authorizationHeader);
    t_httpCode httpResponseCode = _viessmannHttpPtr ->GET();   
    if (httpResponseCode > 0) 
    {
        String payload = _viessmannHttpPtr ->getString();      
        int charsToCopy = payload.length() < reponseBufferLength ? payload.length() : reponseBufferLength;
        for (int i = 0; i < charsToCopy; i++)
        {
            responseBuffer[i] = payload[i];
        }               
    } 
    else 
    {
        Serial.printf("Eqipment: Error performing the request, HTTP-Code: %d\n", httpResponseCode);
    }
    _viessmannHttpPtr->end();
    return httpResponseCode;
} 

t_httpCode ViessmannClient::GetUser(uint8_t* responseBuffer, const uint16_t reponseBufferLength)
{
    String Url = _viessmannAccountPtr -> UriEndPointUser + "?sections=identity"; 

    String authorizationHeader = "Bearer " + _viessmannAccountPtr ->AccessToken;
    Serial.println(Url);

    _viessmannHttpPtr ->begin(*_viessmannWifiClient,Url);
    _viessmannHttpPtr ->addHeader("Authorization", authorizationHeader);
    t_httpCode httpResponseCode = _viessmannHttpPtr ->GET();   
    if (httpResponseCode > 0) 
    {
        String payload = _viessmannHttpPtr ->getString();      
        int charsToCopy = payload.length() < reponseBufferLength ? payload.length() : reponseBufferLength;
        for (int i = 0; i < charsToCopy; i++)
        {
            responseBuffer[i] = payload[i];
        }               
    } 
    else 
    {
        Serial.printf("Fehler bei der Anfrage, HTTP-Code: %d\n", httpResponseCode);
    }
    _viessmannHttpPtr->end();  
    return httpResponseCode; 
}

ViessmannClient::~ViessmannClient()
{};