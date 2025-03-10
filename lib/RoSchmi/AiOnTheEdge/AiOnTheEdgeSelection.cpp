#include "AiOnTheEdgeSelection.h"

// Constructor
AiOnTheEdgeSelection::AiOnTheEdgeSelection(DateTime pLastReadTime, TimeSpan pReadInterval)
{
    readInterval = pReadInterval;
    lastReadTime = pLastReadTime;
}

/**
 * destructor
 */
AiOnTheEdgeSelection::~AiOnTheEdgeSelection()
{}