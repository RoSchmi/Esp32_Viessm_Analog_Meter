#include "AiOnTheEdgeApiSelection.h"

// Constructor
AiOnTheEdgeApiSelection::AiOnTheEdgeApiSelection(DateTime pLastReadTime, TimeSpan pReadInterval)
{
    readInterval = pReadInterval;
    lastReadTime = pLastReadTime;
}

/**
 * destructor
 */
/*
AiOnTheEdgeApiSelection::~AiOnTheEdgeApiSelection()
{}
*/