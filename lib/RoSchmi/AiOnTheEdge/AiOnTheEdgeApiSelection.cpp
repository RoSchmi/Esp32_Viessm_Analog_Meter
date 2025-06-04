#include "AiOnTheEdgeApiSelection.h"

// Constructor
AiOnTheEdgeApiSelection::AiOnTheEdgeApiSelection()
{
    lastReadTimeSeconds = 0;
    readIntervalSeconds = 0;
    baseValueOffset = 0;    
};

AiOnTheEdgeApiSelection::AiOnTheEdgeApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds, uint32_t pBaseValueOffset)
{
    strncpy(objLabel, pObjLabel, sizeof(objLabel) - 1);
    lastReadTimeSeconds = pLastReadTimeSeconds;
    readIntervalSeconds = pReadIntervalSeconds;
    baseValueOffset = pBaseValueOffset;  
}