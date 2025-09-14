#include <Arduino.h>

#ifndef _VIESSMANNAPISELECTION_H_
#define _VIESSMANNAPISELECTION_H_

#define VI_FEATUREVALUELENGTH 30
#define VI_FEATURENAMELENGTH 60
#define VI_FEATURESTAMPLENGTH 30

class ViessmannApiSelection
{
    private:
    //classLabel is used to find instance in memory (debugging)  
    char classLabel[11] = "Vi-Api-Sel";
    char objLabel[11] = "none";
    char endLabel[9] = "Endlabel";

    public:
    int64_t lastReadTimeSeconds;
    int32_t readIntervalSeconds;
    
    typedef struct Feature
    { 
        int  idx = 0;    
        char name[VI_FEATURENAMELENGTH] = {'\0'};
        char timestamp[VI_FEATURESTAMPLENGTH] = {'\0'};     
        char value[VI_FEATUREVALUELENGTH] = {'\0'};       
    }Feature;

    ViessmannApiSelection();
    ViessmannApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds);
    
    Feature _3_temperature_main;
    Feature _4_boiler_temperature;
    Feature _6_burner_modulation;
    Feature _7_burner_hours;
    Feature _7_burner_starts;
    Feature _8_burner_is_active;
    Feature _10_circulation_pump_status;
    Feature _22_heating_curve_shift;
    Feature _22_heating_curve_slope;
    Feature _76_temperature_supply;
    Feature _84_heating_dhw_charging;
    Feature _86_heating_dhw_pump_status;
    Feature _88_heating_dhw_pump_primary_status;
    Feature _92_heating_dhw_cylinder_temperature;  
    //Feature _92_heating_dhw_main_temperature;
    Feature _93_heating_dhw_outlet_temperature;
    Feature _97_heating_temperature_outside;    
};

#endif