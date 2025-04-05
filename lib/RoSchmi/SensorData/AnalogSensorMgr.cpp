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
        readValues[i].ReadInterval = TimeSpan(pInterval);
        
    }

}

void AnalogSensorMgr::SetReadInterval(int sensorIndex, uint32_t pInterval)
{
    readValues[sensorIndex].ReadInterval = pInterval;
}

bool AnalogSensorMgr::HasToBeRead(int pSensorIndex, DateTime now, bool pReset)
{
    //RoSchmi
    //Serial.println("Question Analog SensorMgr has to be read?");
    

    int32_t remainSeconds = (readValues[pSensorIndex].LastReadTime.secondstime()- now.secondstime()) + readValues[pSensorIndex].ReadInterval.totalseconds();
    if (pSensorIndex == 2 && remainSeconds < 1)
    {
        printf("\nHas to be read? Index: %d, Remaining: %d seconds\n", pSensorIndex, remainSeconds);
    }

    if (readValues[pSensorIndex].IsActive && now.operator>(readValues[pSensorIndex].LastReadTime.operator+(readValues[pSensorIndex].ReadInterval)))
    {
        //printf("Ret true %d\n", pSensorIndex);
        // Set LastReadTime to actual time
        readValues[pSensorIndex].LastReadTime = pReset ? now : readValues[pSensorIndex].LastReadTime; 
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
