#include <Arduino.h>
#include "ArduinoJson.h"
#include <cstring>
#include "config.h"
#include "WiFiClientSecure.h"
#include "HTTPClient.h"
#include "DateTime.h"
#include "RestApiAccount.h"
#include "AiOnTheEdgeApiSelection.h"

#ifndef AIONTHEEDGECLIENT_H_
#define AIONTHEEDGECLIENT_H_

class AiOnTheEdgeClient
{
    public:
    AiOnTheEdgeClient(RestApiAccount * account, const char * caCert, HTTPClient * httpClient, WiFiClient * pWifiClient);
    
    int GetFeatures(const char * url, uint8_t * reponsePtr, const uint16_t reponseBufferLength, AiOnTheEdgeApiSelection * aiApiSelectionPtr);
    int SetPreValue(const char * url, const char * preValue, uint8_t * reponsePtr, const uint16_t reponseBufferLength);
     
   private:

   WiFiClient * _aiOnTheEdgeWifiClient;
   HTTPClient * _aiOnTheEdgeHttpPtr;
   RestApiAccount * _restApiAccountPtr;
   char * _aiOnTheEdgeCaCert;
};
#endif