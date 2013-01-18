#pragma once

// http://www.zachburlingame.com/2011/03/integrating-the-subversion-revision-into-the-version-automatically-with-native-cc/
#include "VersionNo_SVN.h"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               0
#define VERSION_MINOR               5
#define VERSION_REVISION            1
#define VERSION_BUILD               SVN_REVISION
 
#define VER_FILE_DESCRIPTION_STR    "Built " STRINGIZE(SVN_TIME_NOW) " UTC from r" STRINGIZE(SVN_REVISION)
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
                                    "." STRINGIZE(VERSION_BUILD)