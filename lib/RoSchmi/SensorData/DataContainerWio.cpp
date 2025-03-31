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
    //Serial.printf("I am in SetNewValue (1). Index: %d Arriving value: %.2f\n", pIndex, pSampleValue); 
    // Ignore invalid readings with value 999.9 (MagicNumberInvalid)
    if (pSampleValue > (MagicNumberInvalid + 0.11) || pSampleValue < (MagicNumberInvalid - 0.11))
    {
        if (pIndex == 1)
        { 
            Serial.printf("I am in SetNewValue (2). Index: %d Arriving value: %.2f\n", pIndex, pSampleValue); 
        }
        if (!pIsConsumption)
        {
            SampleValues[pIndex].feedCount++;
            SampleValues[pIndex].SummedValues += pSampleValue;
            SampleValues[pIndex].AverageValue = SampleValues[pIndex].SummedValues / SampleValues[pIndex].feedCount;
        }

        //if (_isFirstTransmission || (pIsConsumption && (SampleValues[pIndex].LastSendTime.day() != pActDateTime.day())))
        if (_isFirstTransmission)
        {
            Serial.printf("Setting BaseValue pSampleValue: %.2f\n", pSampleValue);
           // SampleValues[pIndex].BaseValue = pSampleValue;
        }

        SampleValues[pIndex].LastValue = SampleValues[pIndex].Value;
        SampleValues[pIndex].UnClippedLastValue = SampleValues[pIndex].UnClippedValue;
        SampleValues[pIndex].UnClippedValue = pSampleValue;
        SampleValues[pIndex].Value = pSampleValue;
        
         SampleValues[pIndex].LastSendTime = pActDateTime;
        _SampleValuesSet.LastUpdateTime = pActDateTime;
    
    
    if (_isFirstTransmission)
    {       
         _hasToBeSent = true;
        _lastSentTime = pActDateTime;
        _isFirstTransmission = false;

        SampleValues[pIndex].LastSendTime = pActDateTime;
        SampleValues[pIndex].LastLastSendValue = SampleValues[pIndex].LastUnClippedSendValue; 
        SampleValues[pIndex].LastUnClippedSendValue = 0.0;
        
            Serial.println(F("Set new value in first transmission"));  
    }
    else
    {
        if (_lastSentTime.operator<=(pActDateTime.operator-(SendInterval)))
        {
            Serial.println(F("AiOnTheEdge Sent Flag set ************"));
            
            // RoSchmi: next line deleted, is already done in getCheckedSampleValues
            //_lastSentTime = pActDateTime;

            //SampleValues[pIndex].LastSendTime = pActDateTime;
            
            // RoSchmi: Think about deleting the next two lines
            SampleValues[pIndex].LastLastSendValue = SampleValues[pIndex].LastUnClippedSendValue;
            SampleValues[pIndex].LastUnClippedSendValue = SampleValues[pIndex].Value;
            _hasToBeSent = true;

        }       
    }
    }     
}

void DataContainerWio::SetNewValueStruct(uint32_t pIndex, DateTime pActDateTime, ValueStruct pValueStruct, bool pIsConsumption)
{
    //Serial.printf("I am in SetNewValue (1). Index: %d Arriving value: %.2f\n", pIndex, pSampleValue); 
    // Ignore invalid readings with value 999.9 (MagicNumberInvalid)
    if (pValueStruct.displayValue > (MagicNumberInvalid + 0.11) || pValueStruct.displayValue < (MagicNumberInvalid - 0.11))
    {
        if (pIndex == 1)
        { 
            Serial.printf("I am in SetNewValueStruct (2). Index: %d Arriving value: %.2f\n", pIndex, pValueStruct.displayValue); 
        }
        if (!pIsConsumption)
        {
            SampleValues[pIndex].feedCount++;
            SampleValues[pIndex].SummedValues += pValueStruct.displayValue;
            SampleValues[pIndex].AverageValue = SampleValues[pIndex].SummedValues / SampleValues[pIndex].feedCount;
        }

        //if (_isFirstTransmission || (pIsConsumption && (SampleValues[pIndex].LastSendTime.day() != pActDateTime.day())))
        extern TimeSpan timeDiffUtcToLocal;
        
        bool isNewDay = pActDateTime.operator+(timeDiffUtcToLocal).day() != _lastSentTime.operator+(timeDiffUtcToLocal).day();
        
        if (_isFirstTransmission || isNewDay)
        {
            Serial.printf("Setting BaseValueStruct pSampleValue: %.2f\n", pValueStruct.unClippedValue);
            SampleValues[pIndex].BaseValue = pValueStruct.unClippedValue;
        }

        SampleValues[pIndex].LastValue = SampleValues[pIndex].Value;
        SampleValues[pIndex].UnClippedLastValue = SampleValues[pIndex].UnClippedValue;
        SampleValues[pIndex].UnClippedValue = pValueStruct.unClippedValue;
        SampleValues[pIndex].Value = pValueStruct.displayValue;
        
         SampleValues[pIndex].LastSendTime = pActDateTime;
        _SampleValuesSet.LastUpdateTime = pActDateTime;
    
    
    if (_isFirstTransmission)
    {       
         _hasToBeSent = true;
        _lastSentTime = pActDateTime;
        _isFirstTransmission = false;

        SampleValues[pIndex].LastSendTime = pActDateTime;
        SampleValues[pIndex].LastLastSendValue = SampleValues[pIndex].LastUnClippedSendValue; 
        SampleValues[pIndex].LastUnClippedSendValue = 0.0;
        
            Serial.println(F("Set new value in first transmission"));  
    }
    else
    {
        if (_lastSentTime.operator<=(pActDateTime.operator-(SendInterval)))
        {
            Serial.println(F("AiOnTheEdge Sent Flag set ************"));
            
            // RoSchmi: next line deleted, is already done in getCheckedSampleValues
            //_lastSentTime = pActDateTime;

            //SampleValues[pIndex].LastSendTime = pActDateTime;
            
            // RoSchmi: Think about deleting the next two lines
            SampleValues[pIndex].LastLastSendValue = SampleValues[pIndex].LastUnClippedSendValue;
            SampleValues[pIndex].LastUnClippedSendValue = SampleValues[pIndex].UnClippedValue;
            _hasToBeSent = true;

        }       
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