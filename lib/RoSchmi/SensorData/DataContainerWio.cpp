#include "DataContainerWio.h"

SampleValue DataContainerWio::checkedSampleValue(SampleValue inSampleValue, float lowerLimit, float upperLimit, float invalidateSubstitute, DateTime actDateTime, TimeSpan invalidateTime)
{
    SampleValue workSampleValue = inSampleValue;
    
    workSampleValue.AverageValue = ((inSampleValue.AverageValue < lowerLimit) || (inSampleValue.AverageValue > upperLimit)) ? invalidateSubstitute : inSampleValue.AverageValue;
    workSampleValue.Value = ((inSampleValue.Value < lowerLimit) || (inSampleValue.Value > upperLimit)) ? invalidateSubstitute : inSampleValue.Value;
    if (actDateTime.operator-(invalidateTime).operator>=(workSampleValue.LastSendTime))
    {  
        workSampleValue.AverageValue = invalidateSubstitute;
        workSampleValue.Value = invalidateSubstitute;
    }
    return workSampleValue;
}

DataContainerWio::DataContainerWio(TimeSpan pSendInterval, TimeSpan pInvalidateInterval, float pLowerLimit, float pUpperLimit, float pMagicNumberInvalid)
{
    SendInterval = pSendInterval,
    InvalidateInterval = pInvalidateInterval;
    LowerLimit = pLowerLimit;
    UpperLimit = pUpperLimit;
    MagicNumberInvalid = pMagicNumberInvalid;
}

void DataContainerWio::SetNewValue(uint32_t pIndex, DateTime pActDateTime, float pSampleValue, bool pIsConsumption)
{
    // Ignore invalid readings with value 999.9 (MagicNumberInvalid)
    if (pSampleValue > (MagicNumberInvalid + 0.11) || pSampleValue < (MagicNumberInvalid - 0.11))
    {
        //Serial.println(F("Set new value 1"));
        if (!pIsConsumption)
        {
            SampleValues[pIndex].feedCount++;
            SampleValues[pIndex].SummedValues += pSampleValue;
            SampleValues[pIndex].AverageValue = SampleValues[pIndex].SummedValues / SampleValues[pIndex].feedCount;
        }

        SampleValues[pIndex].LastValue = SampleValues[pIndex].Value;
        SampleValues[pIndex].Value = pSampleValue;
        //Serial.println(F("Set new value 2")); 
        SampleValues[pIndex].LastSendTime = pActDateTime;
        _SampleValuesSet.LastUpdateTime = pActDateTime;
    }

    if (pIsConsumption && (SampleValues[pIndex].LastSendTime.day() != pActDateTime.day()))
    {
        SampleValues[pIndex].BaseValue = pSampleValue;
    }
    //Serial.println(F("Set new value 3"));
    if (_isFirstTransmission)
    {       
        _hasToBeSent = true;
        _lastSentTime = pActDateTime;
        _isFirstTransmission = false;
        SampleValues[pIndex].BaseValue = pSampleValue;
        Serial.println(F("Set new value in first transmission"));  
    }
    else
    {
        if (_lastSentTime.operator<=(pActDateTime.operator-(SendInterval)))
        {
            Serial.println(F("AiOnTheEdge Sent Flag set ************"));

            _hasToBeSent = true;     
        }       
    }   
}

void DataContainerWio::Set_Year(uint16_t year)
{
    Year = year;
}
void DataContainerWio::setUpperLimit(float pUpperLimit)
{
    UpperLimit = pUpperLimit;
}

void DataContainerWio::setLowerLimit(float pLowerLimit)
{
    LowerLimit = pLowerLimit;
}

void DataContainerWio::setMagigNumberInvalid(float pMagicNumberInvalid)
{
   MagicNumberInvalid = pMagicNumberInvalid;
}

bool DataContainerWio::hasToBeSent()
{
    return _hasToBeSent;
}

void DataContainerWio::setHasToBeSentFlag()
{
    _hasToBeSent = true;
}

SampleValueSet DataContainerWio::getSampleValues(DateTime pActDateTime, bool pUpdateSentFlags = true)
{ 
    if (pUpdateSentFlags)
    {
        _hasToBeSent = false;
        _isFirstTransmission = false;
        _SampleValuesSet.LastSendTime = _lastSentTime;
        _lastSentTime = pActDateTime;
           
        for (int i = 0; i < 4; i++)
        {
            SampleValues[i].feedCount = 0;
            SampleValues[i].SummedValues = 0.0;
        } 
    }

    _SampleValuesSet.SampleValues[0] = SampleValues[0];
    _SampleValuesSet.SampleValues[1] = SampleValues[1];
    _SampleValuesSet.SampleValues[2] = SampleValues[2];
    _SampleValuesSet.SampleValues[3] = SampleValues[3];
    return _SampleValuesSet;
}

SampleValueSet DataContainerWio::getCheckedSampleValues(DateTime pActDateTime, bool pUpdateSentFlags = true)
{
    if (pUpdateSentFlags)
    {
        _hasToBeSent = false;
        _isFirstTransmission = false;
        _SampleValuesSet.LastSendTime = _lastSentTime;
        _lastSentTime = pActDateTime;
        
        for (int i = 0; i < 4; i++)
        {      
            SampleValues[i].feedCount = 0;
            SampleValues[i].SummedValues = 0.0;
        }
    }

   _SampleValuesSet.SampleValues[0] = checkedSampleValue(SampleValues[0], LowerLimit, UpperLimit, MagicNumberInvalid, pActDateTime, InvalidateInterval);
   _SampleValuesSet.SampleValues[1] = checkedSampleValue(SampleValues[1], LowerLimit, UpperLimit, MagicNumberInvalid, pActDateTime, InvalidateInterval);
   _SampleValuesSet.SampleValues[2] = checkedSampleValue(SampleValues[2], LowerLimit, UpperLimit, MagicNumberInvalid, pActDateTime, InvalidateInterval);
   _SampleValuesSet.SampleValues[3] = checkedSampleValue(SampleValues[3], LowerLimit, UpperLimit, MagicNumberInvalid, pActDateTime, InvalidateInterval);
    
    return _SampleValuesSet;        
}