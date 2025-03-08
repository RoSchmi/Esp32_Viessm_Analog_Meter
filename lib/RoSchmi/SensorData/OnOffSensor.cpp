#include "OnOffSensor.h"

OnOffSensor::OnOffSensor(bool pIsActive)
{
    isActive = pIsActive;
}
OnOffSensor::OnOffSensor(bool pIsActive, bool pActState, bool pLastState, bool pChangedFlag,   DateTime pLastSwitchTime)
{ 
    actState = pActState;
    lastState = pLastState;
    changedFlag = pChangedFlag;
    isActive = pIsActive;
    lastSwitchTime = pLastSwitchTime;
}

void OnOffSensor:: Feed(bool pNewState, DateTime pDate)
{
    //RoSchmi
    //Serial.printf("Feeding. Newstate = %s, ActState = %s \n ", pNewState ? "true" : "false", actState ? "true" : "false");
    if (pNewState != actState)
    {
        // RoSchmi
        // Serial.println("Setting changedFlag");
        lastState = actState;
        actState = pNewState;
        changedFlag = true;
        lastSwitchTime = pDate;
    }
}

bool OnOffSensor::HasChangedState()
{
    if (isActive)
    {   
        return changedFlag;   
    }
    else
    {  
        return false;
    }
}

bool OnOffSensor::GetStateAndResetChangedFlag()
{
    changedFlag = false;
    return actState;
}

bool OnOffSensor::GetState()
{
    return actState;
}

void OnOffSensor::SetInactive()
{
    isActive = false;
}

void OnOffSensor::SetActive()
{
    isActive = true;
}