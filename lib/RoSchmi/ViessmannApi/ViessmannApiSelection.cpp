#include "ViessmannApiSelection.h"

     

// Constructor

ViessmannApiSelection::ViessmannApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds)
{
    strncpy(objLabel, pObjLabel, sizeof(objLabel) - 1);
    lastReadTimeSeconds = pLastReadTimeSeconds;
    readIntervalSeconds = pReadIntervalSeconds;
}

