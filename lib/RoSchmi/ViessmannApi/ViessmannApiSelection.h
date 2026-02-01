#include <Arduino.h>
#include <ArduinoJson.h>
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

// Array of interesting feature entries to be extracted
// See definition in ViessmannApiSelection.cpp   
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

   public:
    int64_t lastReadTimeSeconds;
    int32_t readIntervalSeconds;

    ViessmannApiSelection();
    ViessmannApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds);
    
    // Extracts the items listed in .cpp (interestingProperties) out of all items in the JSON doc returned by the Viessmann API. 
    // - doc: JsonDocument containing the full Viessmann API response 
    // - features: Array of feature entries to be filled with the selected data
    // - featureCount: Number of items in the features array

    void extractFeatures(const JsonDocument& doc, VI_Feature* features, int featureCount);
         
};

#endif
