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

void DataContainerWio::SetNewValue(uint32_t pIndex, DateTime pActDateTime, float pSampleValue)
{
    ValueStruct transferValueStruct = {
        .displayValue = pSampleValue,
        .unClippedValue = pSampleValue,
        .thisDayBaseValue = 55.0 };
    SetNewValueStruct(pIndex, pActDateTime, transferValueStruct, false);
}

void DataContainerWio::SetNewValueStruct(uint32_t pIndex, DateTime pActDateTime, ValueStruct pValueStruct, bool pIsConsumption)
{   
    // Ignore invalid readings with value 999.9 (MagicNumberInvalid)
    //Serial.printf("Arriving in SetNewValueStruct. Value: %.1f\n", pValueStruct.displayValue);
    
    if (pValueStruct.displayValue > (MagicNumberInvalid + 0.11) || pValueStruct.displayValue < (MagicNumberInvalid - 0.11))
    { 
        //Serial.printf("\nValid value added to DataContainer. Index = %d Value: %.1f\n", pIndex, pValueStruct.displayValue);     
        if (!pIsConsumption)
        {
            SampleValues[pIndex].feedCount++;
            SampleValues[pIndex].SummedValues += pValueStruct.displayValue;
            SampleValues[pIndex].AverageValue = SampleValues[pIndex].SummedValues / SampleValues[pIndex].feedCount;
        }

        //if (_isFirstTransmission || (pIsConsumption && (SampleValues[pIndex].LastSendTime.day() != pActDateTime.day())))
        extern TimeSpan timeDiffUtcToLocal;
        
        //bool isNewDay = pActDateTime.operator+(timeDiffUtcToLocal).day() != _lastSentTime.operator+(timeDiffUtcToLocal).day();
        
        bool isNewDay = pActDateTime.operator+(timeDiffUtcToLocal).day() != SampleValues[pIndex].LastUpdateValueTime.operator+(timeDiffUtcToLocal).day(); 
        
        if (pIsConsumption)
        {
            SampleValues[pIndex].BaseValue = pValueStruct.thisDayBaseValue;
            float copyBaseValue = SampleValues[0].BaseValue;
            volatile int dummy1 = 22;
            dummy1 =+ 1;
        }
        /*
        if (_isFirstTransmission || isNewDay)
        {
            //Serial.printf("Setting BaseValueStruct pSampleValue: %.2f\n", pValueStruct.unClippedValue);
            SampleValues[pIndex].BaseValue = pValueStruct.unClippedValue;
        }
        */

        SampleValues[pIndex].LastValue = SampleValues[pIndex].Value;
        SampleValues[pIndex].UnClippedLastValue = SampleValues[pIndex].UnClippedValue;
        
        SampleValues[pIndex].UnClippedValue = pValueStruct.unClippedValue;
        SampleValues[pIndex].Value = pValueStruct.displayValue;
        
        // RoSchmi evaluate if next line is correct
        SampleValues[pIndex].LastSendTime = pActDateTime;
        // RoSchmi new 04.04.25
        SampleValues[pIndex].LastUpdateValueTime = pActDateTime;
        _SampleValuesSet.LastUpdateTime = pActDateTime;
    
    
        if (_isFirstTransmission)
        {       
             _hasToBeSent = true;
            _lastSentTime = pActDateTime;
            _isFirstTransmission = false;

            SampleValues[pIndex].LastSendTime = pActDateTime;
            //SampleValues[pIndex].LastLastSendValue = SampleValues[pIndex].LastSendUnClippedValue; 
            //SampleValues[pIndex].LastSendUnClippedValue = 0.0;
            SampleValues[pIndex].LastSendUnClippedValue = SampleValues[pIndex].UnClippedValue;
            Serial.println(F("Set new value in first transmission"));  
        }
        else
        {                  // SendInterval elapsed
            if (_lastSentTime.operator<=(pActDateTime.operator-(SendInterval)))
            {
               #if SERIAL_PRINT == 1
               Serial.println(F("AiOnTheEdge Sent Flag set ************"));
               #endif
            
            // RoSchmi: next line deleted, is already done in getCheckedSampleValues
              _lastSentTime = pActDateTime;

            //SampleValues[pIndex].LastSendTime = pActDateTime;
            
            // RoSchmi: Think about deleting the next two lines
            //SampleValues[pIndex].LastLastSendUnClippedValue = SampleValues[pIndex].LastSendUnClippedValue;
            SampleValues[pIndex].LastSendUnClippedValue = SampleValues[pIndex].UnClippedValue;
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