#include "AiOnTheEdgeClient.h"
#include "config.h"

WiFiClient _aiOnTheEdgeWifiClient;

//RestApiAccount  * _restApiAccountPtr;
HTTPClient * _aiOnTheEdgeHttpPtr;

//char * _aiOnTheEdgeCaCert;

typedef int t_httpCode;

/*
char initName[FEATURENAMELENGTH] {0};
char initTimestamp[FEATURESTAMPLENGTH] {0};
char initValue[FEATUREVALUELENGTH] {0};
*/

// Constructor

AiOnTheEdgeClient::AiOnTheEdgeClient(RestApiAccount * account, const char * caCert, HTTPClient * httpClient, WiFiClient pWifiClient)
{   
    _restApiAccountPtr = account;  
    _aiOnTheEdgeCaCert = (char *)caCert;
    _aiOnTheEdgeHttpPtr = httpClient;
    // RoSchmi
    _aiOnTheEdgeHttpPtr -> setReuse(false);
  
    _aiOnTheEdgeWifiClient = pWifiClient;
    
   // _aiOnTheEdgeHttpPtr = httpClient;


    //_aiOnTheEdgeHttpPtr->begin("http://gasmeter/json");

    
    // RoSchmi
    //_aiOnTheEdgeHttpPtr -> setReuse(false);

    

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

t_httpCode AiOnTheEdgeClient::GetFeatures(uint8_t* responseBuffer, const uint16_t reponseBufferLength, AiOnTheEdgeApiSelection * apiSelectionPtr)
{
    char InstallationId[20] = {0};
      
    Serial.println("Im in GetFeatures");
    
    String Url = _restApiAccountPtr -> UriEndPointJson;
    
    Serial.println(F("Loading gasmeter Features"));
    Serial.println(Url);  

    //https://arduinojson.org/v7/how-to/use-arduinojson-with-httpclient/

    _aiOnTheEdgeHttpPtr ->useHTTP10(false);   // Must be reset to false for Azure requests
                                           // Is needed to load the long features JSON string
    
    _aiOnTheEdgeHttpPtr ->begin(_aiOnTheEdgeWifiClient, Url);
  
    Serial.println(F("I am after begin"));
              
    t_httpCode httpResponseCode = _aiOnTheEdgeHttpPtr ->GET();

    
                
    if (httpResponseCode > 0) 
    { 
        if (httpResponseCode == HTTP_CODE_OK)
        {
            /*
            #if SERIAL_PRINT == 1
              Serial.println("Received ResponseCode > 0");
            #endif
            */
           String payload = _aiOnTheEdgeHttpPtr ->getString();
           
           Serial.println("\nPrinting payload");
           printf(payload.c_str());

           
           int charsToCopy = payload.length() < reponseBufferLength ? payload.length() : reponseBufferLength;
           for (int i = 0; i < charsToCopy; i++)
           {
               responseBuffer[i] = payload[i];
           }
           
           const char* json = (char *)responseBuffer;
           
            JsonDocument doc;
            deserializeJson(doc, json);
            
            Serial.println(F("JsonDoc is deserialized"));
            
            int nameLen = apiSelectionPtr ->nameLenght;
            int stampLen = apiSelectionPtr -> stampLength;
            int valLen = apiSelectionPtr -> valueLength;
            
            char tempVal[valLen] = {"\0"};
            
            if (!doc.overflowed())
            {
                Serial.printf("Number of elements = %d\n", doc.size());
                // From the Features JSON string get the selected entities
                
                apiSelectionPtr -> _0_value.idx = 0;               
                strncpy(apiSelectionPtr -> _0_value.name, "value", nameLen - 1);
                strncpy(apiSelectionPtr-> _0_value.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["main"]["value"]); 
                snprintf(apiSelectionPtr -> _0_value.value, valLen - 1, (const char*)tempVal);
                
                apiSelectionPtr -> _1_raw.idx = 1;               
                strncpy(apiSelectionPtr -> _1_raw.name, "raw", nameLen - 1);
                strncpy(apiSelectionPtr-> _1_raw.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["main"]["raw"]); 
                snprintf(apiSelectionPtr -> _1_raw.value, valLen - 1, (const char*)tempVal);
                
                apiSelectionPtr -> _2_pre.idx = 2;               
                strncpy(apiSelectionPtr -> _2_pre.name, "pre", nameLen - 1);
                strncpy(apiSelectionPtr-> _2_pre.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["main"]["pre"]); 
                snprintf(apiSelectionPtr -> _2_pre.value, valLen - 1, (const char*)tempVal);
                
                apiSelectionPtr -> _3_error.idx = 3;               
                strncpy(apiSelectionPtr -> _3_error.name, "error", nameLen - 1);
                strncpy(apiSelectionPtr-> _3_error.timestamp, doc["main"]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr-> _3_error.value, doc["main"]["error"], nameLen - 1);

                apiSelectionPtr -> _4_rate.idx = 4;               
                strncpy(apiSelectionPtr -> _4_rate.name, "rate", nameLen - 1);
                strncpy(apiSelectionPtr-> _4_rate.timestamp, doc["main"]["timestamp"], stampLen - 1);
                snprintf(tempVal, sizeof(tempVal), "%.1f", (float)doc["main"]["rate"]); 
                snprintf(apiSelectionPtr -> _4_rate.value, valLen - 1, (const char*)tempVal);
                
                apiSelectionPtr -> _5_timestamp.idx = 3;               
                strncpy(apiSelectionPtr -> _5_timestamp.name, "timestamp", nameLen - 1);
                strncpy(apiSelectionPtr-> _5_timestamp.timestamp, doc["main"]["timestamp"], stampLen - 1);
                strncpy(apiSelectionPtr-> _5_timestamp.value, doc["main"]["timestamp"], nameLen - 1);
               
                Serial.printf("Name of first item is \n%s\n", apiSelectionPtr -> _0_value.name);
                Serial.printf("Index: %d\n", apiSelectionPtr -> _0_value.idx);
                Serial.printf("Value: %s\n", apiSelectionPtr -> _0_value.value);
                Serial.printf("Timestamp: %s\n", apiSelectionPtr -> _0_value.timestamp);                
            }
            else
            {
                Serial.println(F("Deserialization doc was overflowed"));
            }   
        }
        else
        {
            // If request doesn't return with ok, we printout the begin of the response string
            WiFiClient *stream = _aiOnTheEdgeHttpPtr ->getStreamPtr();        
            uint32_t bytesRead = 0;
            uint32_t chunkSize = 1000;       
            uint16_t maxRounds = 10;
            uint16_t rounds = 0;
            char chunkBuffer[chunkSize +1] = {0};

            while(rounds < maxRounds)
            {
            bytesRead += stream ->readBytes(chunkBuffer, chunkSize);
            chunkBuffer[chunkSize] = '\0'; 
            //Serial.println(chunkBuffer);
            strcat((char *)responseBuffer, chunkBuffer);
            responseBuffer += chunkSize;
            //Serial.printf("\r\n%d\r\n", bytesRead);
            }
        }                      
    } 
    else 
    {
        Serial.printf("Features: Error performing the request, HTTP-Code: %d\n", httpResponseCode);
    }
    _aiOnTheEdgeHttpPtr ->useHTTP10(false);
    _aiOnTheEdgeHttpPtr->end();
    //Serial.println(F("Returning"));
    return httpResponseCode;
}
AiOnTheEdgeClient::~AiOnTheEdgeClient()

{};