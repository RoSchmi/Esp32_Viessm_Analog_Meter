#include "AiOnTheEdgeClient.h"
#include "config.h"

typedef int t_httpCode;

// Constructor
AiOnTheEdgeClient::AiOnTheEdgeClient(RestApiAccount * account, const char * caCert, HTTPClient * httpClient, WiFiClient * pWifiClient)
{   
    _restApiAccountPtr = account;  
    _aiOnTheEdgeCaCert = (char *)caCert;
    _aiOnTheEdgeWifiClient = pWifiClient;
    _aiOnTheEdgeHttpPtr = httpClient;
    
    _aiOnTheEdgeHttpPtr -> setReuse(false);
    _aiOnTheEdgeHttpPtr ->useHTTP10(false);
    

    // Some buffers located in memory segment .dram0.bss are injected to achieve lesser stack consumption
    
    /*
    _requestPtr = bufferStorePtr;
    _propertiesPtr = bufferStorePtr + REQUEST_BODY_BUFFER_LENGTH;
    _authorizationHeaderBufferPtr = bufferStorePtr + REQUEST_BODY_BUFFER_LENGTH + PROPERTIES_BUFFER_LENGTH;
    _responsePtr = bufferStorePtr + REQUEST_BODY_BUFFER_LENGTH + PROPERTIES_BUFFER_LENGTH + AUTH_HEADER_BUFFER_LENGTH;
    */

    
    /*
       char responseString[RESPONSE_BUFFER_LENGTH];
       memset(&(responseString[0]), 0, RESPONSE_BUFFER_LENGTH);
    */
}

#pragma region Method GetFeatures()
t_httpCode AiOnTheEdgeClient::GetFeatures(const char * url, uint8_t* responseBuffer, const uint16_t reponseBufferLength, AiOnTheEdgeApiSelection * aiApiSelectionPtr)
{
         
    //https://arduinojson.org/v7/how-to/use-arduinojson-with-httpclient
    // useHTTP10(false)
    // is needed to load the long features JSON string
    
    #if SERIAL_PRINT == 1                    
        printf("AI-WiFiClient Address: %d\n", &_aiOnTheEdgeWifiClient);
        printf("Free heapsize: %d Minimum: %d\n\n", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    #endif
    
    const int nameLen = AI_FEATURENAMELENGTH;
    const int stampLen = AI_FEATURESTAMPLENGTH;
    const int valLen = AI_FEATUREVALUELENGTH;
    
    _aiOnTheEdgeHttpPtr ->begin(*_aiOnTheEdgeWifiClient, url);
    t_httpCode httpResponseCode = _aiOnTheEdgeHttpPtr ->GET();
    
    if (httpResponseCode > 0) 
    { 
        if (httpResponseCode == HTTP_CODE_OK)
        {
            
           #if SERIAL_PRINT == 1        
              Serial.println(F("Received ResponseCode > 0"));
           #endif
            
           String payload = _aiOnTheEdgeHttpPtr ->getString();
           
           //#if SERIAL_PRINT == 1         
               Serial.printf("%s\n",payload.c_str());
               //Serial.printf("%s\n", (const char *) payload);
           //#endif
          
           int charsToCopy = payload.length() < reponseBufferLength ? payload.length() : reponseBufferLength - 1;
           for (int i = 0; i < charsToCopy; i++)
           {
               responseBuffer[i] = payload[i];
           }
           responseBuffer[charsToCopy] = '\0';
                     
           const char* json = (char *)responseBuffer;
           
            JsonDocument doc;
            deserializeJson(doc, json);
   
            //Serial.printf("The Vi-Lastreadtime (nach deserializeJson) %u\n", apiSelectionPtr ->lastReadTime);
            
            char tempVal[valLen] = {"\0"};
            
            if (!doc.overflowed())
            {               
                // From the Features JSON string get the selected entities
                
                aiApiSelectionPtr -> _0_value.idx = 0;
                Serial.println("Could access via pointer");             
                strncpy(aiApiSelectionPtr -> _0_value.name, "value", nameLen - 1);
                Serial.println("Could access value.name");
                strncpy(aiApiSelectionPtr-> _0_value.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.2f", (float)doc["main"]["value"]); 
                snprintf(aiApiSelectionPtr -> _0_value.value, valLen - 1, (const char*)tempVal);
                
                aiApiSelectionPtr -> _1_raw.idx = 1;               
                strncpy(aiApiSelectionPtr -> _1_raw.name, "raw", nameLen - 1);
                strncpy(aiApiSelectionPtr-> _1_raw.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["main"]["raw"]); 
                snprintf(aiApiSelectionPtr -> _1_raw.value, valLen - 1, (const char*)tempVal);
                
                aiApiSelectionPtr -> _2_pre.idx = 2;               
                strncpy(aiApiSelectionPtr -> _2_pre.name, "pre", nameLen - 1);
                strncpy(aiApiSelectionPtr-> _2_pre.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["main"]["pre"]); 
                snprintf(aiApiSelectionPtr -> _2_pre.value, valLen - 1, (const char*)tempVal);
                
                aiApiSelectionPtr -> _3_error.idx = 3;               
                strncpy(aiApiSelectionPtr -> _3_error.name, "error", nameLen - 1);
                strncpy(aiApiSelectionPtr-> _3_error.timestamp, doc["main"]["timestamp"], stampLen - 1);
                strncpy(aiApiSelectionPtr-> _3_error.value, doc["main"]["error"], nameLen - 1);

                aiApiSelectionPtr -> _4_rate.idx = 4;               
                strncpy(aiApiSelectionPtr -> _4_rate.name, "rate", nameLen - 1);
                strncpy(aiApiSelectionPtr-> _4_rate.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["main"]["rate"]); 
                snprintf(aiApiSelectionPtr -> _4_rate.value, valLen - 1, (const char*)tempVal);
                
                aiApiSelectionPtr -> _5_timestamp.idx = 3;               
                strncpy(aiApiSelectionPtr -> _5_timestamp.name, "timestamp", nameLen - 1);
                strncpy(aiApiSelectionPtr-> _5_timestamp.timestamp, doc["main"]["timestamp"], stampLen - 1);
                strncpy(aiApiSelectionPtr-> _5_timestamp.value, doc["main"]["timestamp"], nameLen - 1);
                Serial.println("Could access all AiOntheEdge Items");
                //Serial.printf("Value: %s\n", apiSelectionPtr -> _0_value.value);
                //Serial.printf("Timestamp: %s\n", apiSelectionPtr -> _0_value.timestamp);                
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
            Serial.printf("AiOnTheEdge Client received ResponseCode %d\n",httpResponseCode);
            WiFiClient *stream = _aiOnTheEdgeHttpPtr ->getStreamPtr();                  
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
        Serial.printf("AiOnTheEdge Client: Error performing the request, HTTP-Code: %d\n", httpResponseCode);
    }
    
    _aiOnTheEdgeHttpPtr->end();
    Serial.printf("Free heapsize: %d Minimum: %d\n\n", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    
    return httpResponseCode;
}
#pragma endregion


#pragma region Method SetPreValue()
t_httpCode AiOnTheEdgeClient::SetPreValue(const char * url, const char * preValue, uint8_t* responseBuffer, const uint16_t reponseBufferLength)
{  
    char extUrl[70] = {0}; 
    snprintf(extUrl, sizeof(extUrl) - 1, "%s/setPreValue?value=%s&numbers=main", (const char *)url, (const char *)preValue);
    
    //Serial.printf("Will use Request: %s\n", extUrl);
    _aiOnTheEdgeHttpPtr ->begin(*_aiOnTheEdgeWifiClient, extUrl);
       
    t_httpCode httpResponseCode = _aiOnTheEdgeHttpPtr ->GET();
    if (httpResponseCode > 0) 
    { 
        if (httpResponseCode == HTTP_CODE_OK)
        {
            //Serial.println("Request returned ok\n");
        }
        else
        {
            //Serial.printf("Request returned:%d\n", httpResponseCode);
        }
    }
    else
    {
        //Serial.printf("Request returned:%d\n", httpResponseCode);
    } 
    _aiOnTheEdgeHttpPtr->end();
    
    return httpResponseCode;
}
#pragma endregion

