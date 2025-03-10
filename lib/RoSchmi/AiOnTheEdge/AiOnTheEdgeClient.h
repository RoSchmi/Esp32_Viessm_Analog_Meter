#include <Arduino.h>
#include "config.h"
#include "WiFiClientSecure.h"
#include "HTTPClient.h"
#include "DateTime.h"
#include "RestApiAccount.h"
#include "AiOnTheEdgeSelection.h"
//#include "ViessmannApiSelection.h"
#include "ArduinoJson.h"


#ifndef _AIONTHEEDGECLIENT_H_
#define _AIONTHEEDGECLIENT_H_

class AiOnTheEdgeClient
{
    public:    
    AiOnTheEdgeClient(RestApiAccount * account, const char * caCert, HTTPClient *httpClient, WiFiClient * wifiClient, uint8_t * bufferStore);
    ~AiOnTheEdgeClient();
    
    //int GetItems(uint8_t * reponsePtr, const uint16_t reponseBufferLength);
    int GetItems(uint8_t * reponsePtr, const uint16_t reponseBufferLength);
    /*
    int GetUser(uint8_t * reponsePtr, const uint16_t reponseBufferLength);
    int GetEquipment(uint8_t* responseBuffer, const uint16_t reponseBufferLength);
    int GetFeatures(uint8_t* responseBuffer, const uint16_t reponseBufferLength, const uint32_t data_0_id, const char * gateways_0_serial, const char * gateways_0_devices_0_id, ViessmannApiSelection * apiSelectionPtr);
    int RefreshAccessToken(uint8_t* responseBuffer, const uint16_t reponseBufferLength,  const char * refreshToken);  
    */
   private:
   char initName[FEATURENAMELENGTH] {0};
   char initTimestamp[FEATURESTAMPLENGTH] {0};
   char initValue[FEATUREVALUELENGTH] {0};
};
#endif