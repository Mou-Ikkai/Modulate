#pragma once

#include <string>

class CSettings
{
public:
    static bool mbVerbose;
    static bool mbOverwriteOutputFiles;
    static bool mbIgnoreNewFiles;
};

#define VERBOSE_OUT( out ) if( CSettings::mbVerbose ) std::cout << out 