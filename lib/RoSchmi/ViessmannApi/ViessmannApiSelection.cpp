#include "ViessmannApiSelection.h"

// Constructor
ViessmannApiSelection::ViessmannApiSelection(DateTime pLastReadTime, TimeSpan pReadInterval)
{
    readInterval = pReadInterval;
    lastReadTime = pLastReadTime;
}

/**
 * destructor
 */
ViessmannApiSelection::~ViessmannApiSelection()
{}