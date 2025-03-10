#include <Arduino.h>
#include "DateTime.h"

#ifndef _AIONTHEEDGESELECTION_H_
#define _AIONTHEEDGESELECTION_H_

#define FEATUREVALUELENGTH 12
#define FEATURENAMELENGTH 60
#define FEATURESTAMPLENGTH 30

class AiOnTheEdgeSelection
{
    public:  
    int  valueLength = FEATUREVALUELENGTH;
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

    AiOnTheEdgeSelection();
    AiOnTheEdgeSelection(DateTime pLastReadTime, TimeSpan pReadInterval);
    ~AiOnTheEdgeSelection();
    
    //static Feature featureEmpty;

    Feature _value;
    /*
    Feature _5_boiler_temperature;
    Feature _7_burner_modulation;
    Feature _8_burner_hours;
    Feature _8_burner_starts;
    Feature _9_burner_is_active;
    Feature _11_circulation_pump_status;
    Feature _23_heating_curve_shift;
    Feature _23_heating_curve_slope;
    Feature _77_temperature_supply;
    Feature _85_heating_dhw_charging;
    Feature _86_heating_dhw_pump_status;
    Feature _88_heating_dhw_pump_primary_status;
    Feature _90_heating_dhw_cylinder_temperature;
    Feature _92_heating_dhw_outlet_temperature;
    Feature _93_heating_dhw_main_temperature;
    Feature _95_heating_temperature_outside;
    */
     
};

#endif