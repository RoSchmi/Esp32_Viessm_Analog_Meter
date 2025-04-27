#include <Arduino.h>
#include "DateTime.h"

#ifndef _VIESSMANNAPISELECTION_H_
#define _VIESSMANNAPISELECTION_H_

//#define FEATUREVALUELENGTH 12
//#define FEATURENAMELENGTH 60
//#define FEATURESTAMPLENGTH 30

class ViessmannApiSelection
{

    public:
    static const int valueLength = 12;
    static const int nameLenght = 60;
    static const int stampLength = 30;

    public:
    int64_t lastReadTimeSeconds;
    int32_t readIntervalSeconds;

 
    //int  valueLength = FEATUREVALUELENGTH;
    //int nameLenght = FEATURENAMELENGTH;
    //int stampLength = FEATURESTAMPLENGTH;
    
    //int64_t lastReadTimeSeconds;
    //int32_t readIntervalSeconds;
    //DateTime  lastReadTime = DateTime();
    //TimeSpan readInterval = 0;
    
    typedef struct Feature
    { 
        int  idx = 0;    
        char name[nameLenght] = {0};
        char timestamp[stampLength] = {0};     
        char value[valueLength] = {0};       
    }Feature;

    ViessmannApiSelection();
    ViessmannApiSelection(int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds);
    //~ViessmannApiSelection();
    
    //static Feature featureEmpty;

    Feature _2_temperature_main;
    Feature _5_boiler_temperature;
    Feature _7_burner_modulation;
    Feature _8_burner_hours;
    Feature _8_burner_starts;
    Feature _9_burner_is_active;
    Feature _10_circulation_pump_status;
    Feature _23_heating_curve_shift;
    Feature _23_heating_curve_slope;
    Feature _77_temperature_supply;
    Feature _84_heating_dhw_charging;
    Feature _85_heating_dhw_pump_status;
    Feature _87_heating_dhw_pump_primary_status;
    Feature _89_heating_dhw_cylinder_temperature;
    Feature _91_heating_dhw_outlet_temperature;
    Feature _92_heating_dhw_main_temperature;
    Feature _94_heating_temperature_outside;
     
};

#endif