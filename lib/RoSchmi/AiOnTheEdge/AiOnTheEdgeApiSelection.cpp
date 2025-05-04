#include "AiOnTheEdgeApiSelection.h"

// Constructor
AiOnTheEdgeApiSelection::AiOnTheEdgeApiSelection()
{
    lastReadTimeSeconds = 0;
    readIntervalSeconds = 0;    
};

AiOnTheEdgeApiSelection::AiOnTheEdgeApiSelection(int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds)
{
    lastReadTimeSeconds = pLastReadTimeSeconds;
    readIntervalSeconds = pReadIntervalSeconds;   
}