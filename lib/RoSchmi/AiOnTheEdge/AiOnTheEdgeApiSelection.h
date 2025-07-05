#include <Arduino.h>
//#include "DateTime.h"

#ifndef AIONTHEEDGEAPISELECTION_H_
#define AIONTHEEDGEAPISELECTION_H_

#define AI_FEATUREVALUELENGTH 30
#define AI_FEATURENAMELENGTH 60
#define AI_FEATURESTAMPLENGTH 30

class AiOnTheEdgeApiSelection
{
    private:
    //classLabel is used to find instance in memory (debugging)  
    char classLabel[11] = "Ai-Api-Sel";
    char objLabel[11] = "none";
    char endLabel[9] = "Endlabel";

    public:
      
    int64_t lastReadTimeSeconds;
    int32_t readIntervalSeconds;

    uint32_t baseValueOffset = 0;
    
    typedef struct Feature
    { 
        int  idx = 0;    
        char name[AI_FEATURENAMELENGTH] = {'\0'};
        char timestamp[AI_FEATURESTAMPLENGTH] = {'\0'};     
        char value[AI_FEATUREVALUELENGTH] = {'\0'};       
    }Feature;

    AiOnTheEdgeApiSelection();
    AiOnTheEdgeApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds, uint32_t pBaseValueOffset );
    
    Feature _0_value;
    Feature _1_raw;
    Feature _2_pre;
    Feature _3_error;
    Feature _4_rate;
    Feature _5_timestamp;   
};

#endif