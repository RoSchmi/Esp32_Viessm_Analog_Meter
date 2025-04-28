#include <Arduino.h>
#include "DateTime.h"

#ifndef _AIONTHEEDGESELECTION_H_
#define _AIONTHEEDGESELECTION_H_

//#define FEATUREVALUELENGTH 12
//#define FEATURENAMELENGTH 60
//#define FEATURESTAMPLENGTH 30

class AiOnTheEdgeApiSelection
{
    public:

    static const int valueLength = 12;
    static const int nameLenght = 60;
    static const int stampLength = 30;

    //int valueLength = FEATUREVALUELENGTH;
    //int nameLenght = FEATURENAMELENGTH;
    //int stampLength = FEATURESTAMPLENGTH;
    
    
    DateTime  lastReadTime;
    TimeSpan readInterval;
    
    typedef struct Feature
    { 
        int  idx = 0;    
        char name[nameLenght] = {0};
        char timestamp[stampLength] = {0};     
        char value[valueLength] = {0};       
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