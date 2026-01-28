#include "ViessmannApiSelection.h"

// ------------------------------------------------------------ 
// Definition der interessierenden Feature-Properties 
// ------------------------------------------------------------ 


const ViessmannApiSelection::InterestingProperty ViessmannApiSelection::interestingProperties[] =
   {
        {"heating.boiler.sensors.temperature.main", "value"}, 
        {"heating.boiler.temperature", "value"}, 
        {"heating.burners.0.modulation", "value"},
        {"heating.burners.0.statistics", "hours"},
        {"heating.burners.0.statistics", "starts"},
        {"heating.burners.0", "active"},
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

   // ------------------------------------------------------------ 
   // Anzahl automatisch berechnen 
   // ------------------------------------------------------------ 
   
   const int ViessmannApiSelection::NUM_INTERESTING_PROPERTIES = 
   sizeof(ViessmannApiSelection::interestingProperties) / 
   sizeof(ViessmannApiSelection::interestingProperties[0]);

   // Definiton of vi_features (declared in ViessmannApiSelection.h)
VI_Feature vi_features[VI_FEATURES_COUNT];

// Constructor
ViessmannApiSelection::ViessmannApiSelection()
{
    lastReadTimeSeconds = 0;
    readIntervalSeconds = 0;    
};

ViessmannApiSelection::ViessmannApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds)
{
    strncpy(objLabel, pObjLabel, sizeof(objLabel) - 1);
    lastReadTimeSeconds = pLastReadTimeSeconds;
    readIntervalSeconds = pReadIntervalSeconds;
}

VI_Feature* getFeatureByName(VI_Feature* features, int featureCount, const char* name)
{
    for (int i = 0; i < featureCount; i++) 
    { 
        if (strncmp(features[i].name, name, VI_FEATURENAMELENGTH) == 0) 
        { 
            return &features[i]; 
        } 
    } 
        return nullptr;  
}
    

// Hilfsfunktion: JsonVariant in Text umwandeln
static void jsonValueToString(JsonVariantConst v, char* buffer, size_t buflen) {
    if (v.is<const char*>()) {
        strncpy(buffer, v.as<const char*>(), buflen);
        buffer[buflen - 1] = '\0';
    } else if (v.is<int>()) {
        snprintf(buffer, buflen, "%d", v.as<int>());
    } else if (v.is<long>()) {
        snprintf(buffer, buflen, "%ld", v.as<long>());
    } else if (v.is<float>()) {
        snprintf(buffer, buflen, "%.2f", v.as<float>());
    } else if (v.is<double>()) {
        snprintf(buffer, buflen, "%.2f", v.as<double>());
    } else if (v.is<bool>()) {
        strncpy(buffer, v.as<bool>() ? "true" : "false", buflen);
        buffer[buflen - 1] = '\0';
    } else if (v.isNull()) {
        strncpy(buffer, "-", buflen);
        buffer[buflen - 1] = '\0';
    } else {
        strncpy(buffer, "?", buflen);
        buffer[buflen - 1] = '\0';
    }
}


// Hilfsfunktion: Index eines Features im Feature-Array finden
// (anhand des Feature-Namens)
static int findFeatureIndex(VI_Feature* features, int featureCount, const char* featureName) {
    for (int i = 0; i < featureCount; i++) {
        if (strncmp(features[i].name, featureName, VI_FEATURENAMELENGTH) == 0) {
            return i;
        }
    }
    return -1;
}


// Hilfsfunktion: neuen Feature-Eintrag initialisieren (falls noch nicht vorhanden)
static int ensureFeatureEntry(VI_Feature* features, int featureCount, const char* featureName) 
{
    int idx = findFeatureIndex(features, featureCount, featureName);
    if (idx >= 0) {
        return idx;
    }

    // Freien Platz suchen
    for (int i = 0; i < featureCount; i++) {
        if (features[i].name[0] == '\0') 
        {
            features[i].idx = i;
            strncpy(features[i].name, featureName, VI_FEATURENAMELENGTH);
            features[i].name[VI_FEATURENAMELENGTH - 1] = '\0';
            features[i].timestamp[0] = '\0';
            features[i].valueCount = 0;
            return i;
        }
    }

    // Kein Platz mehr
    return -1;
}


//void ViessmannApiSelection::parseFeatures(const JsonDocument& doc, Feature* features, int featureCount)
void ViessmannApiSelection::parseFeatures(const JsonDocument& doc, VI_Feature* features, int featureCount)
{
    // Alle Feature-Einträge im Ziel-Array zurücksetzen
    Serial.println("Going to clear features");
    for (int i = 0; i < featureCount; i++) {
        features[i].idx = i;
        features[i].name[0] = '\0';
        features[i].timestamp[0] = '\0';
        features[i].valueCount = 0;
        
    }
    Serial.println("all features cleared");
    Serial.printf("Address of vi_features in parser: %p\n", vi_features);


    //Serial.printf("\r\nTimestamp: %s\r\n", doc["data"][3]["timestamp"]);
    
    JsonVariantConst dataVar = doc["data"]; 
    if (!dataVar.is<JsonArrayConst>()) 
    { Serial.println("ERROR: JSON has no 'data' array"); 
        serializeJsonPretty(doc, Serial); 
    return; 
    }

    serializeJsonPretty(doc, Serial);
    
    JsonArrayConst data = doc["data"].as<JsonArrayConst>(); 
    if (data.isNull()) 
    {
        Serial.printf("Data was null\n");
        return;
    }
    
    Serial.printf("\nReady with serializing\r\n");

    // Über alle Features aus der API iterieren
    for (JsonObjectConst obj : data) 
    {
        JsonVariantConst nameVar = obj["feature"];     

        const char* featureName = nameVar.as<const char*>();

        if (!featureName || featureName[0] == '\0') 
        { 
            Serial.println("Feature name missing or empty");           
            continue; 
        }
      
        Serial.printf("FeatureName to look for: %s\r\n", featureName);
        if (!featureName) {
            Serial.printf("Not found FeatureName: %s\r\n", featureName);
            continue;
        }
        
        // Prüfen, ob dieses Feature überhaupt interessant ist
        bool anyMatch = false;
        for (int i = 0; i < featureCount; i++) 
        {
            if (strcmp(featureName, ViessmannApiSelection::interestingProperties[i].featureName) == 0) {
                anyMatch = true;
                break;
            }
        }
        if (!anyMatch) {
            continue;
        }
        Serial.printf("Found selected FeatureName: %s\r\n", featureName);
        // Feature-Eintrag im Ziel-Array sicherstellen
        int fIdx = ensureFeatureEntry(features, featureCount, featureName);
        if (fIdx < 0) {
            // Kein Platz mehr im Feature-Array
            continue;
        }
        
        VI_Feature* featuresPtr = &features[fIdx];

        VI_Feature& f = features[fIdx];

        

         // Timestamp übernehmen
        const char* ts = obj["timestamp"] | "";
        strncpy(f.timestamp, ts, VI_FEATURESTAMPLENGTH);
        f.timestamp[VI_FEATURESTAMPLENGTH - 1] = '\0';

        JsonObjectConst props = obj["properties"].as<JsonObjectConst>();
        if (props.isNull()) {
            Serial.printf("\n properties was null\n");
            continue;
        }

        // Jetzt alle interessierenden Properties für dieses Feature durchgehen
        for (int i = 0; i < NUM_INTERESTING_PROPERTIES; i++) 
        {
            const auto& intrProp = ViessmannApiSelection::interestingProperties[i];

            if (strcmp(intrProp.featureName, featureName) != 0) {
                Serial.printf("\nintrProp.featurename is not %s.\n", featureName);       
                continue;
            }
        
            // Now index i points to actual row of interesting properties
            const char* key = ViessmannApiSelection::interestingProperties[i].propertyName;
        
            JsonVariantConst propVar = props[key]; 
            if (propVar.isNull()) 
            continue;

            // Property existiert?
            Serial.printf("\nintrProp.property is %s.\n", intrProp.propertyName);           
        
        
            // propVar ist ein Objekt mit "value" 
            JsonVariantConst valVar = propVar["value"]; 
            if (valVar.isNull()) 
            continue;

            Serial.printf("\npropVar ist ein Objekt mit 'value'\n");
        
            const char* valStr = valVar.as<const char*>();


            char tmp[VI_FEATUREVALUELENGTH];
        
            if (!valStr) { // Zahl oder bool → in String umwandeln 
                if (valVar.is<int>()) 
                    { 
                        snprintf(tmp, sizeof(tmp), "%d", valVar.as<int>()); 
                    } 
                else if (valVar.is<float>()) 
                { 
                    snprintf(tmp, sizeof(tmp), "%.2f", valVar.as<float>()); 
                } 
                else if (valVar.is<bool>()) 
                { 
                    snprintf(tmp, sizeof(tmp), "%s", valVar.as<bool>() ? "true" : "false"); 
                } 
                else 
                { 
                 continue; 
                } 
                valStr = tmp; 
            }

            Serial.printf("All values are stored as string\n");
        

            // Wert speichern 
            if (featuresPtr->valueCount < VI_MAX_VALUES_PER_FEATURE) 
            {                
                VI_FeatureValue* fv = &featuresPtr->values[featuresPtr->valueCount++];                 
                strncpy(fv->key, key, VI_FEATUREKEYLENGTH);
                fv->key[VI_FEATUREKEYLENGTH - 1] = '\0';      
                strncpy(fv->value, valStr, VI_FEATUREVALUELENGTH);   
                fv->value[VI_FEATUREVALUELENGTH - 1] = '\0';
            }
        }
        Serial.printf("\n Loop for NUM_INTERESTING_PROPERTIES is finished \n");   
    }
    Serial.printf("\nParsing is finished\n");
}

