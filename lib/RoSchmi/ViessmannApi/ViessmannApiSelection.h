#include <Arduino.h>
#include "DateTime.h"

#ifndef _VIESSMANNAPISELECTION_H_
#define _VIESSMANNAPISELECTION_H_

#define VI_FEATUREVALUELENGTH 12
#define VI_FEATURENAMELENGTH 60
#define VI_FEATURESTAMPLENGTH 30

class ViessmannApiSelection
{
    private:   
    char classLabel[11] = "Vi-Api-Sel";
    char objLabel[11] = "none";
    char endLabel[9] = "Endlabel";

    public:
    /*
    static const int valueLength = 12;
    static const int nameLenght = 60;
    static const int stampLength = 30;
    */

    public:
    int64_t lastReadTimeSeconds;
    int32_t readIntervalSeconds;

 
    int  valueLength = VI_FEATUREVALUELENGTH;
    int nameLenght = VI_FEATURENAMELENGTH;
    int stampLength = VI_FEATURESTAMPLENGTH;
    
    typedef struct Feature
    { 
        int  idx = 0;    
        char name[VI_FEATURENAMELENGTH] = {0};
        char timestamp[VI_FEATURESTAMPLENGTH] = {0};     
        char value[VI_FEATUREVALUELENGTH] = {0};       
    }Feature;

    ViessmannApiSelection();
    ViessmannApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds);
    
    
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