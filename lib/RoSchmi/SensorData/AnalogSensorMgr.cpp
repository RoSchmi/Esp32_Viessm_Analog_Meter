#include "AnalogSensorMgr.h"

 
 //AnalogSensor readValues[SENSOR_COUNT];
 float MagicNumberInvalid = 999.9;

 AnalogSensorMgr::AnalogSensorMgr(float pMagicNumberInvalid)
 {
     MagicNumberInvalid = pMagicNumberInvalid;

 }

void AnalogSensorMgr::SetReadInterval(uint32_t pInterval)
{
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        readValues[i].ReadInterval = pInterval;
    }

}

void AnalogSensorMgr::SetReadInterval(int sensorIndex, uint32_t pInterval)
{
    readValues[sensorIndex].ReadInterval = pInterval;
}

bool const AnalogSensorMgr::HasToBeRead(int pSensorIndex, DateTime now)
{
    //RoSchmi
    //Serial.println("Question Analog SensorMgr has to be read?");
    
    if (readValues[pSensorIndex].IsActive && now.operator>=(readValues[pSensorIndex].LastReadTime.operator+(readValues[pSensorIndex].ReadInterval)))
    {
        //Serial.println("Ret true");
        return true;
    }
    else
    {
        //Serial.println("Ret false");
        return false;
    }    
}

void AnalogSensorMgr::SetReadTimeAndValues(int pSensorIndex, DateTime now, float pReadValue_1, float pReadValue_2, float pReadValue_3)
{
    readValues[pSensorIndex].LastReadTime = now;
    readValues[pSensorIndex].Value_1 = pReadValue_1;
    readValues[pSensorIndex].Value_2 = pReadValue_2;
    readValues[pSensorIndex].Value_3 = pReadValue_3;
}


AnalogSensor AnalogSensorMgr::GetSensorDates(int pSensorIndex)
{   
    return readValues[pSensorIndex];  
}
