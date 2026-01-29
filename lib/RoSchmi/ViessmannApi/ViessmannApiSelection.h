#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ViessmannApiSelection.h"
#include "config.h"

#ifndef _VIESSMANNAPISELECTION_H_
#define _VIESSMANNAPISELECTION_H_

#define VI_FEATUREVALUELENGTH 30
#define VI_FEATUREKEYLENGTH 30
#define VI_FEATURENAMELENGTH 60
#define VI_FEATURESTAMPLENGTH 30

// Must be high enough to create an array 
// to hold all interesting features
#define VI_FEATURES_COUNT 18


// Maximal wie viele Werte ein Feature enthalten kann 
#define VI_MAX_VALUES_PER_FEATURE 4

typedef struct VI_FeatureValue {
        char key[VI_FEATUREKEYLENGTH];
        char value[VI_FEATUREVALUELENGTH];
    }VI_FeatureValue;

typedef struct VI_Feature {
        int  idx = 0;
        char name[VI_FEATURENAMELENGTH]; 
        char timestamp[VI_FEATURESTAMPLENGTH];
        VI_FeatureValue values[VI_MAX_VALUES_PER_FEATURE];
        int valueCount = 0;
    }VI_Feature;

    
extern VI_Feature vi_features[VI_FEATURES_COUNT];


VI_Feature* getFeatureByName(VI_Feature* features, int featureCount, const char* name);
    

class ViessmannApiSelection
{
    private:
    //classLabel is used to find instance in memory (debugging)  
    char classLabel[11] = "Vi-Api-Sel";
    char objLabel[11] = "none";
    char endLabel[9] = "Endlabel";
     
    public:



    // Ein Eintrag beschreibt: 
    // - welches Feature 
    // - welches Property innerhalb dieses Features 
    struct InterestingProperty { 
    const char* featureName; 
    const char* propertyName;
};




// Deklaration der statischen Liste 
static const InterestingProperty interestingProperties[]; 
// Anzahl der Einträge 
static const int NUM_INTERESTING_PROPERTIES;

//static const int VI_FEATURES_COUNT = 16;
/*
   static const InterestingProperty interestingProperties[] =
   {
        {"heating.boiler.sensors.temperature.main", "value"}, 
        {"heating.boiler.temperature", "value"}, 
        {"heating.burners.0.modulation", "value"},
        {"heating.burners.0.statistics", "hours"},
        {"heating.burners.0.statistics", "starts"},
        {"heating.circuits.0", "active"},
        {"heating.circuits.0.circulation.pump", "status"},
        {"heating.circuits.0.heating.curve", "shift"},
        {"heating.circuits.0.heating.curve", "slope"},
        {"heating.circuits.0.sensors.temperature.supply", "value"},       
        {"heating.dhw.charging", "active"},
        {"heating.dhw.pumps.circulation", "status"},
        {"heating.dhw.pumps.primary", "status"},
        {"heating.dhw.sensors.temperature.dhwCylinder", "value"},
        {"heating.dhw.sensors.temperature.outlet", "value"},
        {"heating.sensors.temperature.outside", "value"}
   };
   */

   //const int NUM_FEATURES = sizeof(interestingProperties) / sizeof(interestingProperties[0]);

   public:
    int64_t lastReadTimeSeconds;
    int32_t readIntervalSeconds;

    ViessmannApiSelection();
    ViessmannApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds);
    
    // Füllt das übergebene Feature-Array anhand des JSON-Dokuments. 
    // - doc: JsonDocument mit der Viessmann-API-Antwort 
    // - features: Array von Feature-Strukturen 
    // - featureCount: Anzahl der Einträge im Feature-Array

    //void ViessmannApiSelection::extractFeatures(const JsonDocument& doc,, Feature* features, int featureCount);
    void extractFeatures(const JsonDocument& doc, VI_Feature* features, int featureCount);
   
    
    //static Feature features[];
    
    private:
    //int ensureFeatureEntry(Feature features[], int featureCount, const char *featureName);
     
    /*
    Feature _3_temperature_main;
    Feature _4_boiler_temperature;
    Feature _7_burner_modulation;
    Feature _8_burner_hours;
    Feature _8_burner_starts;
    
    */    
};

#endif
