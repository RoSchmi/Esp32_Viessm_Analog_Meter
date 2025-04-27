#include <Arduino.h>
#include "DateTime.h"

#ifndef _AIONTHEEDGESELECTION_H_
#define _AIONTHEEDGESELECTION_H_

#define FEATUREVALUELENGTH 12
#define FEATURENAMELENGTH 60
#define FEATURESTAMPLENGTH 30

class AiOnTheEdgeApiSelection
{
    public:  
    int valueLength = FEATUREVALUELENGTH;
    int nameLenght = FEATURENAMELENGTH;
    int stampLength = FEATURESTAMPLENGTH;
    
    
    DateTime  lastReadTime;
    TimeSpan readInterval;
    
    typedef struct Feature
    { 
        int  idx = 0;    
        char name[FEATURENAMELENGTH] = {0};
        char timestamp[FEATURESTAMPLENGTH] = {0};     
        char value[FEATUREVALUELENGTH] = {0};       
    }Feature;

    AiOnTheEdgeApiSelection();
    AiOnTheEdgeApiSelection(DateTime pLastReadTime, TimeSpan pReadInterval);
    //~AiOnTheEdgeApiSelection();
    
    //static Feature featureEmpty;

    Feature _0_value;
    Feature _1_raw;
    Feature _2_pre;
    Feature _3_error;
    Feature _4_rate;
    Feature _5_timestamp;   
};

#endif