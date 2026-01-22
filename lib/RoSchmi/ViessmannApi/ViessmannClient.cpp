#include "ViessmannClient.h"
#include "config.h"

typedef int t_httpCode;

// Constructor
ViessmannClient::ViessmannClient(ViessmannApiAccount * account, const char * caCert, HTTPClient * httpClient, WiFiClient * wifiClient, uint8_t * bufferStorePtr)
{  
    _viessmannAccountPtr = account;
    _viessmannCaCert = (char *)caCert;
    _viessmannWifiClient = wifiClient;  
    _viessmannHttpPtr = httpClient;   
    _viessmannHttpPtr -> setReuse(false);
    
}

#pragma region ViessmannClient::GetFeatures(...)
t_httpCode ViessmannClient::GetFeatures(uint8_t* responseBuffer, const uint16_t reponseBufferLength, const uint32_t data_0_id, const char * gateways_0_serial, const char * gateways_0_devices_0_id, ViessmannApiSelection * apiSelectionPtr)
{
    Serial.println("Viessmann Client start getting features\n");
    //Serial.printf("In ViessmannClient. LastReadTimeSeconds: %u Interval: %u \n ", (uint32_t)apiSelectionPtr ->lastReadTimeSeconds, apiSelectionPtr ->readIntervalSeconds);
    char InstallationId[20] = {0};
    sprintf(InstallationId, "%d", data_0_id);

    char GatewaySerial[30] = {0};
    
    String addendum = "features/installations/" + (String)InstallationId + "/gateways/" + (String(gateways_0_serial) + "/devices/" + String(gateways_0_devices_0_id) + "/features"); 
    String Url = _viessmannAccountPtr -> UriEndPointIot + addendum;
    String authorizationHeader = "Bearer " + _viessmannAccountPtr ->AccessToken;
    //Serial.println(F("Loading Viessmann features from Cloud"));
    
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
     // Setting of timeout possibly needed to avoid 'IncompleteInput' errors (not sure)
           
     _viessmannWifiClient ->setTimeout(10);  //default of WiFiClientSecure is 5000 ms
                                            // here it is set to 7000 ms
     
     uint32_t wiFiSecureTimeOut = _viessmannWifiClient ->getTimeout();
     Serial.printf("WiFiClientSecure TimeOut = %d\n", wiFiSecureTimeOut);

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
            //#if SERIAL_PRINT == 1
              Serial.println(F("Viessmann Received ResponseCode OK"));
            //#endif
             
            JsonDocument doc;
            StaticJsonDocument<64> filter;           
            //filter["data"][0]["feature"] = true,
            filter["data"][0]["timestamp"] = true,
            filter["data"][0]["properties"] = true 
            ;

            DeserializationError error; 
            
            WiFiClient* stream = _viessmannHttpPtr->getStreamPtr();
            // Setting of timeout seems to be needed to avoid 'IncompleteInput' errors
            // Not sure if the argument plays a role, seems to work with '1' as well
            stream->setTimeout(2);  //Changed from 2 to 4
                          
            //#if SERIAL_PRINT == 1
                //ReadLoggingStream loggingStream(*stream, Serial);  //use this to print the JSON string
            //#else
               NullPrint nullPrint;
               ReadLoggingStream loggingStream(*stream, nullPrint);
            //#endif
                
            //error = deserializeJson(doc, _viessmannHttpPtr->getStream(), DeserializationOption::Filter(filter));
            error = deserializeJson(doc, loggingStream, DeserializationOption::Filter(filter));
            // RoSchmi                                           
            if (error == DeserializationError::Ok) // Ok; EmptyInput; IncompleteInput; InvalidInput; NoMemory           
            //if (true)
            {
            #pragma region if(DeserializationError::Ok)    
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
                
                    apiSelectionPtr -> _3_temperature_main.idx = 3;
                    strncpy(apiSelectionPtr-> _3_temperature_main.timestamp, doc["data"][3]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][3]["properties"]["value"]["value"]); 
                    snprintf(apiSelectionPtr -> _3_temperature_main.value, valLen - 1, (const char*)tempVal);
                
                    Serial.println("Step 3 (first)");
                                     
                    apiSelectionPtr -> _4_boiler_temperature.idx = 4;
                    strncpy(apiSelectionPtr-> _4_boiler_temperature.timestamp, doc["data"][4]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][4]["properties"]["value"]["value"]); 
                    snprintf(apiSelectionPtr -> _4_boiler_temperature.value, valLen - 1, (const char*)tempVal);
                 
                    Serial.println("Step 4");

                    apiSelectionPtr -> _7_burner_modulation.idx = 7;
                    strncpy(apiSelectionPtr-> _7_burner_modulation.timestamp, doc["data"][7]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][7]["properties"]["value"]["value"]); 
                    snprintf(apiSelectionPtr -> _7_burner_modulation.value, valLen - 1, (const char*)tempVal);
                
                    Serial.println("Step 7");

                    apiSelectionPtr -> _8_burner_hours.idx = 8;
                    strncpy(apiSelectionPtr-> _8_burner_hours.timestamp, doc["data"][8]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.2f", (float)doc["data"][8]["properties"]["hours"]["value"]);
                    snprintf(apiSelectionPtr -> _8_burner_hours.value, valLen - 1, (const char*)tempVal);
                
                    Serial.println("Step 8a");

                    apiSelectionPtr -> _8_burner_starts.idx = 8;
                    strncpy(apiSelectionPtr-> _8_burner_starts.timestamp, doc["data"][8]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][8]["properties"]["starts"]["value"]);
                    snprintf(apiSelectionPtr -> _8_burner_starts.value, valLen - 1, (const char*)tempVal);
                
                    Serial.println("Step 8b");

                    apiSelectionPtr -> _9_burner_is_active.idx = 9;               
                    strncpy(apiSelectionPtr-> _9_burner_is_active.timestamp, doc["data"][9]["timestamp"] | "null", stampLen - 1);
                    strcpy(apiSelectionPtr -> _9_burner_is_active.value, (boolean)doc["data"][9]["properties"]["active"]["value"] ? "true" : "false");
                
                    Serial.println("Step 9");

                    apiSelectionPtr -> _13_circulation_pump_status.idx = 13;
                    strncpy(apiSelectionPtr -> _13_circulation_pump_status.timestamp, doc["data"][13]["timestamp"] | "null", stampLen - 1);
                    strncpy(apiSelectionPtr -> _13_circulation_pump_status.value, doc["data"][13]["properties"]["status"]["value"], valLen -1);
                
                    Serial.println("Step 13");

                    apiSelectionPtr -> _22_heating_curve_shift.idx = 22;                
                    strncpy(apiSelectionPtr-> _22_heating_curve_shift.timestamp, doc["data"][22]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.0f", (float)doc["data"][22]["properties"]["shift"]["value"]);
                    snprintf(apiSelectionPtr -> _22_heating_curve_shift.value, valLen - 1, (const char*)tempVal);
                                      
                    apiSelectionPtr -> _22_heating_curve_slope.idx = 22;
                    strncpy(apiSelectionPtr -> _22_heating_curve_slope.timestamp, doc["data"][22]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][22]["properties"]["slope"]["value"]);
                    snprintf(apiSelectionPtr -> _22_heating_curve_slope.value, valLen - 1, (const char*)tempVal);
                            
                    apiSelectionPtr -> _76_temperature_supply.idx = 76;
                    strncpy(apiSelectionPtr -> _76_temperature_supply.timestamp, doc["data"][76]["timestamp"] | "null", stampLen - 1);       
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][76]["properties"]["value"]["value"]);
                    snprintf(apiSelectionPtr -> _76_temperature_supply.value, valLen - 1, (const char*)tempVal);
                
                    Serial.println("Step 76");
                           
                    apiSelectionPtr -> _84_heating_dhw_charging.idx = 84;
                    strncpy(apiSelectionPtr-> _84_heating_dhw_charging.timestamp, doc["data"][84]["timestamp"]  | "null", stampLen - 1);
                    strcpy(apiSelectionPtr -> _84_heating_dhw_charging.value, (boolean)doc["data"][84]["properties"]["active"]["value"] ? "true" : "false");
                    
                    Serial.println("Step 84");
                    
                    apiSelectionPtr -> _86_heating_dhw_pump_status.idx = 86;
                    strncpy(apiSelectionPtr -> _86_heating_dhw_pump_status.timestamp, doc["data"][86]["timestamp"]  | "null", stampLen - 1);
                    strncpy(apiSelectionPtr -> _86_heating_dhw_pump_status.value, doc["data"][86]["properties"]["status"]["value"], valLen -1);
                    
                    Serial.println("Step 86");

                    apiSelectionPtr -> _88_heating_dhw_pump_primary_status.idx = 88;               
                    strncpy(apiSelectionPtr -> _88_heating_dhw_pump_primary_status.timestamp, doc["data"][88]["timestamp"] | "null", stampLen - 1);
                    strncpy(apiSelectionPtr -> _88_heating_dhw_pump_primary_status.value, doc["data"][88]["properties"]["status"]["value"], valLen -1);
                    
                    //Serial.println("Step 88");
                    /*
                    apiSelectionPtr -> _89_heating_dhw_cylinder_temperature.idx = 89;
                    strncpy(apiSelectionPtr-> _89_heating_dhw_cylinder_temperature.timestamp, doc["data"][89]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][89]["properties"]["value"]["value"]);
                    snprintf(apiSelectionPtr -> _89_heating_dhw_cylinder_temperature.value, valLen - 1, (const char*)tempVal);
                    */
                    //Serial.println("Step 89");
                    /*
                    apiSelectionPtr -> _92_heating_dhw_main_temperature.idx = 92;                
                    strncpy(apiSelectionPtr-> _92_heating_dhw_main_temperature.timestamp, doc["data"][92]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][92]["properties"]["value"]["value"]);
                    snprintf(apiSelectionPtr -> _92_heating_dhw_main_temperature.value, valLen - 1, (const char*)tempVal);
                    */

                    apiSelectionPtr -> _92_heating_dhw_cylinder_temperature.idx = 92;
                    strncpy(apiSelectionPtr-> _92_heating_dhw_cylinder_temperature.timestamp, doc["data"][92]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][92]["properties"]["value"]["value"]);
                    snprintf(apiSelectionPtr -> _92_heating_dhw_cylinder_temperature.value, valLen - 1, (const char*)tempVal);
                    


                    Serial.println("Step 92");
                    
                    // RoSchmi change line with tempVal to 93 after 15.9.25
                    apiSelectionPtr -> _93_heating_dhw_outlet_temperature.idx = 93;
                    strncpy(apiSelectionPtr-> _93_heating_dhw_outlet_temperature.timestamp, doc["data"][93]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][92]["properties"]["value"]["value"]);
                    snprintf(apiSelectionPtr -> _93_heating_dhw_outlet_temperature.value, valLen - 1, (const char*)tempVal);
                    
                    Serial.println("Step 93");

                    apiSelectionPtr -> _97_heating_temperature_outside.idx = 97;               
                    strncpy(apiSelectionPtr-> _97_heating_temperature_outside.timestamp, doc["data"][97]["timestamp"] | "null", stampLen - 1);
                    snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["data"][97]["properties"]["value"]["value"]);
                    snprintf(apiSelectionPtr -> _97_heating_temperature_outside.value, valLen - 1, (const char*)tempVal);
                           
                    Serial.println("Step 97 (last)");               
                }
                else
                {
                    Serial.println(F("Deserialization doc was overflowed"));
                }
            #pragma endregion
            }
            else
            {
            #pragma region else (DeserializationError::Not Ok)
                Serial.printf("\nDeserializeJson() failed: %s\n\n", (const char *)error.c_str());
                Serial.println("Handled the same way as a -1 response");   
                
                
                // Set httpResponseCode to -1
                // so it is handled as a bad request/response
                //RoSchmi
                //httpResponseCode = -1;
            #pragma endregion           
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
    return httpResponseCode;
}
#pragma endregion

#pragma region ViessmannClient::RefreshAccessToken(...)
t_httpCode ViessmannClient::RefreshAccessToken(uint8_t* responseBuffer, const uint16_t reponseBufferLength, const char * refreshToken)
{ 
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
                Serial.println("Got payload:\n");
                Serial.println(payload);
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
#pragma endregion

#pragma region ViessmannClient::GetEquipment(...)
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
#pragma endregion 

#pragma region ViessmannClient::GetUser(...)
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
#pragma endregion
