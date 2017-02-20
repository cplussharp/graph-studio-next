#pragma once

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               0
#define VERSION_MINOR               7
#define VERSION_REVISION            1

// If built on appveyor, pass build version information through
#if defined(APPVEYOR_BUILD_VERSION) && defined(APPVEYOR_BUILD_NUMBER)

#define VERSION_BUILD APPVEYOR_BUILD_NUMBER
#define APPVEYOR_BUILD_STRING "Appveyor build version" STRINGIZE(APPVEYOR_BUILD_VERSION)

#else

#define VERSION_BUILD               0 // todo revision from git
#define APPVEYOR_BUILD_STRING ""

#endif

#define VER_FILE_DESCRIPTION_STR    "Built " __DATE__ " " __TIME__ " " APPVEYOR_BUILD_STRING

#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
                                    "." STRINGIZE(VERSION_BUILD)

