#include <Arduino.h>
#include "config.h"
#include "WiFiClientSecure.h"
#include "HTTPClient.h"
#include "DateTime.h"
#include "RestApiAccount.h"
#include "AiOnTheEdgeApiSelection.h"
#include "ArduinoJson.h"


#ifndef _AIONTHEEDGECLIENT_H_
#define _AIONTHEEDGECLIENT_H_

class AiOnTheEdgeClient
{
    public:
    AiOnTheEdgeClient(RestApiAccount * account, const char * caCert, HTTPClient * httpClient, WiFiClient * pWifiClient);
    
    int GetFeatures(const char * url, uint8_t * reponsePtr, const uint16_t reponseBufferLength, AiOnTheEdgeApiSelection * apiSelectionPtr);
    int SetPreValue(const char * url, const char * preValue, uint8_t * reponsePtr, const uint16_t reponseBufferLength);
     
   private:

   WiFiClient * _aiOnTheEdgeWifiClient;
   HTTPClient * _aiOnTheEdgeHttpPtr;
   RestApiAccount * _restApiAccountPtr;
   char * _aiOnTheEdgeCaCert;

   char initName[FEATURENAMELENGTH] {0};
   char initTimestamp[FEATURESTAMPLENGTH] {0};
   char initValue[FEATUREVALUELENGTH] {0};
};
#endif