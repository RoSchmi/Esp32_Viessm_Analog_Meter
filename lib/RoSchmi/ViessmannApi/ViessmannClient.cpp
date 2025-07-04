#include "ViessmannClient.h"
#include "config.h"

WiFiClient * _viessmannWifiClient;

ViessmannApiAccount  * _viessmannAccountPtr;

HTTPClient * _viessmannHttpPtr;

const char * _viessmannCaCert;

typedef int t_httpCode;

// Constructor
ViessmannClient::ViessmannClient(ViessmannApiAccount * account, const char * caCert, HTTPClient * httpClient, WiFiClient * wifiClient, uint8_t * bufferStorePtr)
{  
    _viessmannAccountPtr = account;
    _viessmannCaCert = caCert;
    _viessmannWifiClient = wifiClient;  
    _viessmannHttpPtr = httpClient;
       
    _viessmannHttpPtr -> setReuse(false);

    //Serial.println("Viessmann Client Constructor\n");
 
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
    Serial.println("Viessmann Client start getting features\n");
    Serial.printf("In ViessmannClient. LastReadTimeSeconds: %u Interval: %u \n ", (uint32_t)apiSelectionPtr ->lastReadTimeSeconds, apiSelectionPtr ->readIntervalSeconds);
    char InstallationId[20] = {0};
    sprintf(InstallationId, "%d", data_0_id);

    char GatewaySerial[30] = {0};
    
    String addendum = "features/installations/" + (String)InstallationId + "/gateways/" + (String(gateways_0_serial) + "/devices/" + String(gateways_0_devices_0_id) + "/features"); 
    String Url = _viessmannAccountPtr -> UriEndPointIot + addendum;
    String authorizationHeader = "Bearer " + _viessmannAccountPtr ->AccessToken;
    //Serial.println(F("Loading Viessmann features from Cloud"));
    
    
    //apiSelectionPtr ->lastReadTimeSeconds
    /*
    int nameLen = apiSelectionPtr ->nameLenght;
    int stampLen = apiSelectionPtr -> stampLength;
    int valLen = apiSelectionPtr -> valueLength;
    */
    
    const int nameLen = VI_FEATURENAMELENGTH;
    const int stampLen = VI_FEATURESTAMPLENGTH;
    const int valLen = VI_FEATUREVALUELENGTH;

    //#if SERIAL_PRINT == 1
    //Serial.printf("VI-WiFiClient Address: %d\n\n", &_viessmannWifiClient);
    //#endif
    
    Serial.println(Url);

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
            //filter["data"][0]["feature"] = true,
            filter["data"][0]["timestamp"] = true,
            filter["data"][0]["properties"] = true 
            ;
            deserializeJson(doc, _viessmannHttpPtr ->getStream(),DeserializationOption::Filter(filter));
            
            #if SERIAL_PRINT == 1
            Serial.println(F("JsonDoc is deserialized"));
            #endif

            char tempVal[valLen] = {'\0'};
            
            if (!doc.overflowed())
            {
                #if SERIAL_PRINT == 1
                Serial.printf("Number of elements = %d\n", doc.size());
                #endif
                // From the long Features JSON string get some chosen entities
                
                apiSelectionPtr -> _2_temperature_main.idx = 2;
                strncpy(apiSelectionPtr-> _2_temperature_main.timestamp, doc["data"][2]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][2]["properties"]["value"]["value"]); 
                snprintf(apiSelectionPtr -> _2_temperature_main.value, valLen - 1, (const char*)tempVal);
                
                Serial.println("Step 2 (first)"); 

                apiSelectionPtr -> _4_boiler_temperature.idx = 4;
                strncpy(apiSelectionPtr-> _4_boiler_temperature.timestamp, doc["data"][4]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][4]["properties"]["value"]["value"]); 
                snprintf(apiSelectionPtr -> _4_boiler_temperature.value, valLen - 1, (const char*)tempVal);
                 
                Serial.println("Step 4");

                apiSelectionPtr -> _6_burner_modulation.idx = 6;
                strncpy(apiSelectionPtr-> _6_burner_modulation.timestamp, doc["data"][6]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][6]["properties"]["value"]["value"]); 
                snprintf(apiSelectionPtr -> _6_burner_modulation.value, valLen - 1, (const char*)tempVal);
                
                Serial.println("Step 6");

                apiSelectionPtr -> _7_burner_hours.idx = 7;
                strncpy(apiSelectionPtr-> _7_burner_hours.timestamp, doc["data"][7]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.2f", (float)doc["data"][7]["properties"]["hours"]["value"]);
                snprintf(apiSelectionPtr -> _7_burner_hours.value, valLen - 1, (const char*)tempVal);
                
                Serial.println("Step 7a");

                apiSelectionPtr -> _7_burner_starts.idx = 7;
                strncpy(apiSelectionPtr-> _7_burner_starts.timestamp, doc["data"][7]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][7]["properties"]["starts"]["value"]);
                snprintf(apiSelectionPtr -> _7_burner_starts.value, valLen - 1, (const char*)tempVal);
                
                Serial.println("Step 7b");

                apiSelectionPtr -> _8_burner_is_active.idx = 8;
                strncpy(apiSelectionPtr-> _8_burner_is_active.timestamp, doc["data"][8]["timestamp"], stampLen - 1);
                strcpy(apiSelectionPtr -> _8_burner_is_active.value, (boolean)doc["data"][8]["properties"]["active"]["value"] ? "true" : "false");
                
                Serial.println("Step 8");

                apiSelectionPtr -> _10_circulation_pump_status.idx = 10;
                strncpy(apiSelectionPtr -> _10_circulation_pump_status.timestamp, doc["data"][10]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr -> _10_circulation_pump_status.value, doc["data"][10]["properties"]["status"]["value"], valLen -1);
                
                Serial.println("Step 10");

                apiSelectionPtr -> _22_heating_curve_shift.idx = 22;
                strncpy(apiSelectionPtr-> _22_heating_curve_shift.timestamp, doc["data"][22]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][22]["properties"]["shift"]["value"]);
                snprintf(apiSelectionPtr -> _22_heating_curve_shift.value, valLen - 1, (const char*)tempVal);
                                      
                apiSelectionPtr -> _22_heating_curve_slope.idx = 22;
                strncpy(apiSelectionPtr -> _22_heating_curve_slope.timestamp, doc["data"][22]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][22]["properties"]["slope"]["value"]);
                snprintf(apiSelectionPtr -> _22_heating_curve_slope.value, valLen - 1, (const char*)tempVal);
                            
                apiSelectionPtr -> _76_temperature_supply.idx = 76;                              
                strncpy(apiSelectionPtr -> _76_temperature_supply.timestamp, doc["data"][76]["timestamp"], stampLen - 1);       
                //strncpy(apiSelectionPtr -> _76_temperature_supply.timestamp, (const char *)valueString, strlen(valueString));
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][76]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _76_temperature_supply.value, valLen - 1, (const char*)tempVal);
                
                Serial.println("Step 76");
                           
                apiSelectionPtr -> _84_heating_dhw_charging.idx = 84;
                strncpy(apiSelectionPtr-> _84_heating_dhw_charging.timestamp, doc["data"][84]["timestamp"], stampLen - 1);
                strcpy(apiSelectionPtr -> _84_heating_dhw_charging.value, (boolean)doc["data"][84]["properties"]["active"]["value"] ? "true" : "false");
             
                apiSelectionPtr -> _85_heating_dhw_pump_status.idx = 85;
                strncpy(apiSelectionPtr -> _85_heating_dhw_pump_status.timestamp, doc["data"][85]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr -> _85_heating_dhw_pump_status.value, doc["data"][85]["properties"]["status"]["value"], valLen -1);
            
                apiSelectionPtr -> _87_heating_dhw_pump_primary_status.idx = 87;
                strncpy(apiSelectionPtr -> _87_heating_dhw_pump_primary_status.timestamp, doc["data"][87]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr -> _87_heating_dhw_pump_primary_status.value, doc["data"][87]["properties"]["status"]["value"], valLen -1);

                apiSelectionPtr -> _89_heating_dhw_cylinder_temperature.idx = 89;
                strncpy(apiSelectionPtr-> _89_heating_dhw_cylinder_temperature.timestamp, doc["data"][89]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][89]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _89_heating_dhw_cylinder_temperature.value, valLen - 1, (const char*)tempVal);
                        
                apiSelectionPtr -> _91_heating_dhw_outlet_temperature.idx = 91;
                strncpy(apiSelectionPtr-> _91_heating_dhw_outlet_temperature.timestamp, doc["data"][91]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][91]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _91_heating_dhw_outlet_temperature.value, valLen - 1, (const char*)tempVal);
                          
                apiSelectionPtr -> _92_heating_dhw_main_temperature.idx = 92;
                strncpy(apiSelectionPtr-> _92_heating_dhw_main_temperature.timestamp, doc["data"][92]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][92]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _92_heating_dhw_main_temperature.value, valLen - 1, (const char*)tempVal);
                                   
                apiSelectionPtr -> _94_heating_temperature_outside.idx = 94;
                strncpy(apiSelectionPtr-> _94_heating_temperature_outside.timestamp, doc["data"][94]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][94]["properties"]["value"]["value"]);
                snprintf(apiSelectionPtr -> _94_heating_temperature_outside.value, valLen - 1, (const char*)tempVal);
                            
                Serial.println("Step 94 (last)");               
            }
            else
            {
                Serial.println(F("Deserialization doc was overflowed"));
            }
            doc.clear();   
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

/*
void ViessmannClient::doTheFirst(ViessmannApiSelection * apiSelectionPtr, JsonDocument doc)
{
    const int valLen = 30;
    const int nameLen = 60;
    const int stampLen = 30;
    char tempVal[valLen] = {'0'};
    //apiSelectionPtr -> _2_temperature_main.idx = 2;
    apiSelectionPtr -> _2_temperature_main.idx = 2;
    strncpy(apiSelectionPtr -> _2_temperature_main.name, doc["data"][2]["feature"], nameLen - 1);               
    strncpy(apiSelectionPtr-> _2_temperature_main.timestamp, doc["data"][2]["timestamp"], stampLen - 1);
    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][2]["properties"]["value"]["value"]); 
    snprintf(apiSelectionPtr -> _2_temperature_main.value, valLen - 1, (const char*)tempVal);
              
}
*/

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
    
        //#if SERIAL_PRINT == 1
            Serial.println(Url);
            Serial.println(body);
        //#endif

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
/*
ViessmannClient::~ViessmannClient()
{};
*/