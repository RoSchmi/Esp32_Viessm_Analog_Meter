#include <Arduino.h>
#include "DateTime.h"

#ifndef AIONTHEEDGEAPISELECTION_H_
#define AIONTHEEDGEAPISELECTION_H_

#define AI_FEATUREVALUELENGTH 30
#define AI_FEATURENAMELENGTH 60
#define AI_FEATURESTAMPLENGTH 30

class AiOnTheEdgeApiSelection
{
    private:   
    char classLabel[11] = "Ai-Api-Sel";
    char objLabel[11] = "none";
    char endLabel[9] = "Endlabel";

    public:
      
    int64_t lastReadTimeSeconds;
    int32_t readIntervalSeconds;
    
    /*
    const int  valueLength = AI_FEATUREVALUELENGTH;
    const int nameLenght = AI_FEATURENAMELENGTH;
    const int stampLength = AI_FEATURESTAMPLENGTH;
    */

    typedef struct Feature
    { 
        int  idx = 0;    
        char name[AI_FEATURENAMELENGTH] = {'\0'};
        char timestamp[AI_FEATURESTAMPLENGTH] = {'\0'};     
        char value[AI_FEATUREVALUELENGTH] = {'\0'};       
    }Feature;

    AiOnTheEdgeApiSelection();
    AiOnTheEdgeApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds);
    
    Feature _0_value;
    Feature _1_raw;
    Feature _2_pre;
    Feature _3_error;
    Feature _4_rate;
    Feature _5_timestamp;   
};

#endif