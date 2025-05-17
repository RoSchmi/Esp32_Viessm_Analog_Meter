#include "AiOnTheEdgeApiSelection.h"

// Constructor
AiOnTheEdgeApiSelection::AiOnTheEdgeApiSelection()
{
    lastReadTimeSeconds = 0;
    readIntervalSeconds = 0;    
};

AiOnTheEdgeApiSelection::AiOnTheEdgeApiSelection(const char * pObjLabel, int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds)
{
    strncpy(objLabel, pObjLabel, sizeof(objLabel) - 1);
    lastReadTimeSeconds = pLastReadTimeSeconds;
    readIntervalSeconds = pReadIntervalSeconds;   
}