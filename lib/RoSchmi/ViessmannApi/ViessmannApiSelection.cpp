#include "ViessmannApiSelection.h"

     

// Constructor

ViessmannApiSelection::ViessmannApiSelection(int64_t pLastReadTimeSeconds, int32_t pReadIntervalSeconds)
{
    readIntervalSeconds = pReadIntervalSeconds;

    lastReadTimeSeconds = pLastReadTimeSeconds;
    
}



/**
 * destructor
 */
ViessmannApiSelection::~ViessmannApiSelection()
{}